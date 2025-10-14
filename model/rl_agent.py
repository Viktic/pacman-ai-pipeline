import os
import DeepQl_model
import joblib
import random
import numpy as np
import torch

class Agent():

    def __init__(self):
                
        model_path = "Ql_model.joblib"
        #start with high initial epsilon for maximum exploration 
        self.epsilon = 0.7

        #------ initialize the Q-Learning Model ------
        if os.path.exists(model_path):
            self.Ql_model = joblib.load(model_path)
        else:
            self.Ql_model = DeepQl_model.NeuralNetwork(9, 5)
        pass

    def select_action(self, obs):
        #generate a random p-value for the epsilon-greedy algorithm
        p = random.random()
        
        if p > self.epsilon: 
          
            #ensure correct tensor shape
            obs_tensor = torch.tensor(obs, dtype=torch.float32).T

            #queries the ql-network 
            with torch.no_grad():
                q_values = self.Ql_model(obs_tensor)
            
            action = torch.argmax(q_values).item()
        else: 
            #explore
            action = random.randint(0, 4)

        return action
