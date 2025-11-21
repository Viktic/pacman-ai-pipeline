import os
import DeepQl_model
import random
import numpy as np
import torch
import replay_buffer
from torch import nn
import torch.optim as optim
import logging



class Agent():

    def __init__(self):

        #creates a replay buffer instance with capacity 20.000
        self.replay_buffer = replay_buffer.ReplayBuffer(20000)

        self.policy_network_path = "policy_model_params.pth"
        self.epsilon_start = 1.0

        #epsilon range determines the number of episodes over which the epsilon decay takes place
        self.epsilon_range = 2000

        self.epsilon_decay = 1/self.epsilon_range        
        self.epsilon_end = 0.05

        self.epsilon = self.epsilon_start
        self.lr = 5e-4
        self.prev_lr = self.lr

        #------ initialize the Q-Learning Models ------
        #policy network (used for queries)
        self.policy_model = DeepQl_model.NeuralNetwork(24, 5)
        
        #target network (gets trained in the background)
        self.target_model = DeepQl_model.NeuralNetwork(24,5)

        #optimizer for the model backpropagation
        self.optimizer = optim.Adam (self.policy_model.parameters(), lr=self.lr)

        #dynamic learning rate scheduler
        self.scheduler = optim.lr_scheduler.ReduceLROnPlateau (
            optimizer=self.optimizer,
            mode = "max",
            factor=0.5,
            min_lr = 1e-6,
            patience=100)

        #loads the params of an already trained policy model if it exists
        if os.path.exists(self.policy_network_path):
            #DEBUGGING
            logging.info("COMM: loading existing model")
            
            checkpoint = torch.load(self.policy_network_path, weights_only=False)
            self.policy_model.load_state_dict(checkpoint["model_state_dict"])
            self.epsilon = checkpoint["epsilon"]
            if "optimizer_state_dict" in checkpoint: 
                self.optimizer.load_state_dict(checkpoint["optimizer_state_dict"])
            if "scheduler_state_dict" in checkpoint: 
                self.scheduler.load_state_dict(checkpoint["scheduler_state_dict"])

        #syncs the target network with the policy network 
        self.target_model.load_state_dict(self.policy_model.state_dict())


    def sync_target_net(self, tau=0.001):
        #syncs the target network with the policy network 

        for target_param, policy_param in zip(
            self.target_model.parameters(),
            self.policy_model.parameters()
        ):
            target_param.data.copy_(
                tau * policy_param.data + (1 - tau) * target_param.data
            )

    def replay_buffer_add(self, prev_state, prev_action, reward, obs, done):
        #adds the transition tuple to the replay buffer 
        self.replay_buffer.add(
            prev_state,
            prev_action,
            reward,
            obs,
            done
        )
      
    def select_action(self, obs):
        #generate a random p-value for the epsilon-greedy algorithm
        p = random.random()
        
        if p > self.epsilon: 
          
            obs_tensor = torch.tensor(obs, dtype=torch.float32)

            #queries the ql-network 
            with torch.no_grad():
                q_values = self.policy_model(obs_tensor)
            

            action = torch.argmax(q_values).item()

        else: 
            #explore
            action = random.randint(0, 4)

        return action

    def reduce_epsilon(self):
        if self.epsilon > self.epsilon_end:
            self.epsilon -= self.epsilon_decay
        
        #DEBUGGING OUTPUT
        logging.debug(f"current epsilon: {self.epsilon}")

    def train_step(self, batch_size, gamma=0.99):

        #check buffer length
        if self.replay_buffer.__len__() < batch_size: 
            return

        states, actions, rewards, next_states, done = self.replay_buffer.sample(batch_size)

        #convert the batch samples into tensors
        states = torch.tensor(states, dtype=torch.float32).reshape(states.shape[0], -1)
        actions = torch.tensor(actions, dtype=torch.int64).unsqueeze(1)
        rewards = torch.tensor(rewards, dtype=torch.float32).unsqueeze(1)
        next_states = torch.tensor(next_states, dtype=torch.float32).reshape(next_states.shape[0], -1)
        dones = torch.tensor(done, dtype=torch.float32).unsqueeze(1)
        

        #gets the Q(s, a) from the policy model
        q_values = self.policy_model(states).gather(dim=1, index=actions) 

        with torch.no_grad():
            next_q_values = self.target_model(next_states).max(1)[0].unsqueeze(1)
            
            #bellman equation for target q-values
            target_q_values = rewards + gamma * next_q_values * (1 - dones)

        #calculates loss
        loss = nn.HuberLoss()(q_values, target_q_values)
        
        #backpropagation
        self.optimizer.zero_grad()  
        loss.backward()  
        #gradient clipping      
        torch.nn.utils.clip_grad_norm_(self.policy_model.parameters(), max_norm=1.0)
        self.optimizer.step()

    def reduce_lr(self, avg_reward): 
        self.scheduler.step(avg_reward)
        updated_lr = self.scheduler.get_last_lr()
        if updated_lr != self.prev_lr: 
            #DEBUGGING OUTPUT
            logging.debug(f"reduced learning rate: {updated_lr}")
            self.prev_lr = updated_lr

    def save_model(self):

        checkpoint = {
            "model_state_dict" : self.policy_model.state_dict(),
            "epsilon" : self.epsilon,
            "optimizer_state_dict" : self.optimizer.state_dict(),
            "scheduler_state_dict" : self.scheduler.state_dict()
        }

        torch.save(checkpoint, self.policy_network_path)


