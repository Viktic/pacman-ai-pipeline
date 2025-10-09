import gymnasium as gym
from gymnasium import spaces
import numpy as np 
import sys
import json
import pandas as pd
import feature_engineer 
import threading
from queue import Queue
from typing import Optional
import time

#gym.Env wrapper for PacMan game

class PacmanEnv(gym.Env):
    
    def __init__(self):

        super().__init__()

        #observation space (input range of values)
        self.observation_space = spaces.Box (

            #input shape:
            # tick -- maximum undefined
            # player_posX
            # player_posY 
            # player_momentumIndex 
            # min_enemy_distance
            # opposite_direction
            # enemy0_distance
            # enemy1_distance
            # enemy2_distance

            low = np.array([0,0,0,0,0,0,0,0,0]),

            high = np.array([np.inf, 1600, 1000, 4, 1000, 1, 1000, 1000, 1000])
        )

        #action space (output range of values), ouputs: 0,1,2,3,4
        self.action_space = spaces.Discrete(5)

        #buffer variables for C++ snapshots
        self._latest_snapshot = None
        self._latest_observation = None
        self._observation_lock = threading.Lock()

        #action queue for actions to send to C++
        self._action_queue = Queue(maxsize=1)

        #background thread for C++ communication
        self._running = True
        self._comm_thread = threading.Thread(target=self._receive_Gamestates, daemon=True)
        self._comm_thread.start()
    

    #helper method to translate raw-input-states into np-arrays
    def _translate_obs(self, raw):

        raw_df = pd.json_normalize(raw)

        #cleans the raw-data-snapshot
        df = feature_engineer.cleanData(raw_df)        

        #turns the cleaned dataframe into a np-array 
        obs = np.array([
            df["tick"],
            df["player_posX"],
            df["player_posY"],
            df["player_momentumIndex"],
            df["min_enemy_distance"],
            df["opposite_direction"],
            df["enemy0_distance"],
            df["enemy1_distance"],
            df["enemy2_distance"]
        ], dtype=np.float32)
        return obs


    #access point to recieve game-states
    def _receive_Gamestates(self):

        for line in sys.stdin:
            if not  self._running: 
                break

            line = line.strip()
            if not line:
                continue

            raw_data = json.loads(line)
            #writes the raw data-snapshot into a threadsafe buffer
            with self._observation_lock:
                self._latest_snapshot = raw_data
                self._latest_observation = self._translate_obs(raw_data)

            #gets the first action in the queue
            action = self._action_queue.get()

            #sends the action to the C++ env via stdout stream
            print(action, flush=True)

    #initiates a learning step
    def step(self, action): 
        
        self._action_queue.put(action)

        timeout = 1.0
        start = time.time()
    
        #timeout for C++ sending actions
        while time.time() - start < timeout:

            #gets the last observation (threadsafe)
            with self._observation_lock:
                if self._latest_observation is not None:
                    obs = self._latest_observation
                    raw = self._latest_snapshot
                    break
            time.sleep(0.001)
        else:
            raise TimeoutError("No observation received from C++ within timeout")

        #extracts the reward from the C++ response (default: 0.0)
        reward = raw.get("reward", 0.0)
        #extracts the terminated-flag from the C++ response (default: False)
        terminated = raw.get("done", False)
        #extracts the truncated-flag from the C++ response
        truncated = raw.get("truncated", False)

        #information to evaluate agents performance (metric: score)
        info = {"score": raw.get("score", 0)}

        return obs, reward, truncated, terminated, info

    #resets the environment
    def reset(self, seed=None, options=None):

        #sends a restart signal to the C++ env
        print("[-1]", flush=True)

        #gets the latest observation (threadsafe)
        with self._observation_lock:
            if self._latest_observation is None:
                
                timeout = 5.0
                start = time.time()

                #timeout for C++ sending actions
                while time.time() - start < timeout: 
                    time.sleep(0.01)
                    if self._latest_observation is not None:
                        break
                else:
                    raise TimeoutError("No initial observation from C++")
                
            obs = self._latest_observation
        

        return obs, {}

    def close(self):
        #cleanup
        self._running = False
        if self._comm_thread.is_alive():
            self._comm_thread.join(timeout=1.0)
        super().close()