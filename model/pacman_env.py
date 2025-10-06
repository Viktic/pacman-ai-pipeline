import gymnasium as gym
from gymnasium import spaces
import numpy as np 
import sys
import json
import pandas as pd
import ml_worker

#gym.Env wrapper for PacMan game

class PacmanEnv(gym.Env):
    
    def __init__(self):

        super().__init__()

        #observation space (input range of values)
        self.observation_space = spaces.Box (

            #input shape:
            # tick -- maximum undefined
            # posX  
            # posY  
            # player_momentumIndex 
            # ticks.MAX(enemy.opposite_direction)
            # ticks.MEAN(enemy.posX)
            # ticks.MEAN(enemy.posY)
            # ticks.MIN(enemy.player_dist)

            low = np.array([0,0,0,0,0,0,0,0]),

            high = np.array([np.inf, 1600, 1000, 4, 1, 1600, 1000, 1000])
        )

        #action space (output range of values), ouputs: 0,1,2,3,4
        self.action_space = spaces.Discrete(5)
    

    #helper method to translate raw-input-states into np-arrays
    def _translate_obs(self, raw):

        raw_df = pd.json_normalize(raw)

        #cleans the raw-data-snapshot
        df = ml_worker.cleanData(raw_df)        

        #turns the cleaned dataframe into a np-array 
        obs = np.array([
            df["tick"],
            df["posX"],
            df["posY"],
            df["player_momentumIndex"],
            df["ticks.MAX(enemy.opposite_direction)"],
            df["ticks.MEAN(enemy.posX)"],
            df["ticks.MEAN(enemy.posY)"],
            df["ticks.MIN(enemy.player_dist)"]
        ], dtype=np.float32)
        return obs
    
    #initiates a learning step
    def step(self, action): 
        
        #sends the action to the C++ env via stdout stream
        print(action, flush=True)

        #waits for C++ response (input)
        line = sys.stdin.readline() 
        
        #reads the response
        raw = json.loads(line)

        #turns the response into a proper observation-format
        obs = self._translate_obs(raw)

        #extracts the reward from the C++ response (default: 0.0)
        reward = raw.get("reward", 0.0)

        #extracts the terminated-flag from the C++ response (default: False)
        terminated = raw.get("done", False)

        #exctracts the truncated-flag from the C++ response (default: False)
        truncated = raw.get("truncated", False)

        #information to evaluate agents performance (metric: score)
        info = {"score": raw.get("score", 0)}

        return obs, reward, terminated, truncated, info

    #resets the environment
    def reset(self, seed=None, options=None):

        #sends a restart signal to the C++ env
        print("RESET", flush=True)

        #waits for C++ response (input)
        line = sys.stdin.readline()

        #reads the response
        raw = json.loads(line)

        #turns the response into a proper observation-format
        obs = self._translate_obs(raw)

        return obs, {}

