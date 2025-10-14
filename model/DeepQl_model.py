import torch
from torch import nn

class NeuralNetwork(nn.Module):
    
    def __init__(self, observation_dim, action_dim):
        super().__init__()

        #defines the network architecture 
        self.linear_relu_stack = nn.Sequential(
            nn.Linear(observation_dim,128),
            nn.ReLU(),
            nn.Linear(128, 128),
            nn.ReLU(),
            nn.Linear(128, action_dim)
        )

    def forward(self, x):
        logits = self.linear_relu_stack(x)
        return logits 
