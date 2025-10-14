import random
from collections import deque
import numpy as np

class ReplayBuffer:

    #initializes FIFO-buffer
    def __init__(self, capacity):
        self.buffer = deque(maxlen=capacity)

    #adds a replay tuple to the buffer
    def add(self, state, action, reward, next_state, done):
        self.buffer.append((state, action, reward, next_state, done))

    #randomly samples replay tuples of batch_size
    def sample(self, batch_size):
        batch = random.sample(self.buffer, batch_size)
        states, actions, rewards, next_states, dones = map(np.array, zip(*batch))
        return states, actions, rewards, next_states, dones
    
    #returns the length of the buffer
    def __len__(self):
        return len(self.buffer)