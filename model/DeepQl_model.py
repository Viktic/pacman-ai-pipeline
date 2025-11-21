import torch
from torch import nn

class NeuralNetwork(nn.Module):
    
    def __init__(self, observation_dim, action_dim):
        super().__init__()

        #defines the network architecture 
        self.linear_relu_stack = nn.Sequential(
            nn.Linear(observation_dim,256),
            nn.LeakyReLU(),
            nn.Linear(256, 256),
            nn.LeakyReLU(),
            nn.Linear(256, action_dim)
        )

        self.apply(self._initWeights)

    def forward(self, x):
        logits = self.linear_relu_stack(x)
        return logits 

    #orthogonal weight initialization 
    def _initWeights(self, module):
        if isinstance(module, nn.Linear):
            nn.init.orthogonal_(module.weight)
            if module.bias is not None:
                nn.init.zeros_(module.bias)