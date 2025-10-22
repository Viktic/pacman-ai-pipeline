import os
import DeepQl_model
import joblib
import random
import numpy as np
import torch
import replay_buffer
from torch import nn
import torch.optim as optim
import logging



class Agent():

    def __init__(self):

        #creates a replay buffer instance with capacity 10.000
        self.replay_buffer = replay_buffer.ReplayBuffer(50000)

        self.policy_network_path = "policy_model_params.pth"
        #start with high initial epsilon for maximum exploration 
        self.epsilon_start = 1.0
        self.epsilon_end = 0.05
        self.epsilon_decay = 0.999

        #DEBUGGING
        self.losses = []

        self.epsilon = self.epsilon_start

        #------ initialize the Q-Learning Models ------
        #policy network (used for queries)
        self.policy_model = DeepQl_model.NeuralNetwork(18, 5)
        
        #target network (gets trained in the background)
        self.target_model = DeepQl_model.NeuralNetwork(18,5)

        #loads the params of an already trained policy model if it exists
        if os.path.exists(self.policy_network_path):
            #DEBUGGING
            logging.info("COMM: loading existing model")
            
            checkpoint = torch.load(self.policy_network_path, weights_only=False)
            self.policy_model.load_state_dict(checkpoint["model_state_dict"])
            self.epsilon = checkpoint["epsilon"]
            self.losses = checkpoint["losses"]

        #syncs the target network with the policy network 
        self.target_model.load_state_dict(self.policy_model.state_dict())
    
        #optimizer for the model backpropagation
        self.optimizer = optim.Adam (self.policy_model.parameters(), lr=1e-3)


    def sync_target_net(self):
        #syncs the target network with the policy network 
        self.target_model.load_state_dict(self.policy_model.state_dict())

    def replay_buffer_add(self, prev_state, prev_action, reward, obs, done):
        #adds the transition tuple to the replay buffer 
        self.replay_buffer.add(
            prev_state,
            prev_action,
            reward,
            obs,
            done
        )
        #DEBUGGING OUTPUT
        logging.debug(f"replay_buffer_size: {self.replay_buffer.__len__()}")

    def select_action(self, obs):
        #generate a random p-value for the epsilon-greedy algorithm
        p = random.random()
        
        #DEBUGGING OUTPUT: 
        logging.debug(f"AGENT: current epsilon: {self.epsilon}")

        if p > self.epsilon: 
          
            obs_tensor = torch.tensor(obs, dtype=torch.float32)

            #DEBUGGING OUTPUT:
            logging.debug(f"QUERY: obs_tensor_shape: {obs_tensor.shape}")

            #queries the ql-network 
            with torch.no_grad():
                q_values = self.policy_model(obs_tensor)
            
            #DEBUGGING OUTPUT:
            propabilities = torch.softmax(q_values, dim=-1)
            logging.debug(f"QUERY: model_output_propabs: {propabilities}")

            action = torch.argmax(q_values).item()
            #DEBUGGING OUTPUT:
            logging.info(f"QUERY: selected action: {action}")
        else: 

            #explore
            action = random.randint(0, 4)

        return action

    def reduce_epsilon(self):
        if self.epsilon > self.epsilon_end:
            self.epsilon *= self.epsilon_decay

    def train_step(self, batch_size, gamma=0.95):
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
        self.losses.append(loss)

        #DEBUGGING OUTPUT:
        logging.debug(f"TRAINING: loss: {loss}")

        #backpropagation
        self.optimizer.zero_grad()  
        loss.backward()  
        #gradient clipping      
        torch.nn.utils.clip_grad_norm_(self.policy_model.parameters(), max_norm=1.0)
        self.optimizer.step()


    def save_model(self):

        checkpoint = {
            "model_state_dict" : self.policy_model.state_dict(),
            "epsilon" : self.epsilon,
            "losses": self.losses,
        }

        torch.save(checkpoint, self.policy_network_path)


