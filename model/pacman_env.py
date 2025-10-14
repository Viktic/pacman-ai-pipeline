import pandas as pd
import feature_engineer
import numpy as np
import json
import replay_buffer
import rl_agent


class PacmanEnv(): 

    #constructor
    def __init__(self):

        #creates a replay buffer instance with capacity 10.000
        self.replay_buffer = replay_buffer.ReplayBuffer(10000)

        #creates an agent instance
        self.agent = rl_agent.Agent()

        self.previous_state = None
        self.current_state = None
        self.previous_action = None
    
    #translate the raw observation into a valid observation-format
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

    def _step(self, line):
        
        #cleans the input line
        line = line.strip()
        if not line:
            return

        #extracts json from raw input
        raw_data = json.loads(line)

        #converts the json into observation format
        obs = self._translate_obs(raw_data)
        reward = raw_data.get("reward")
        done = raw_data.get("done")
        truncated = raw_data.get("truncated")

        #adds the transition tuple to the replay buffer 
        if self.previous_state is not None and self.previous_action is not None:
            self.replay_buffer.add(
                self.previous_state, 
                self.previous_action, 
                reward,
                obs,
                done
            )

        #agent selects an action by querying the ql-network
        action = self.agent.select_action(obs)

        #updates the states
        self.previous_state = obs
        self.previous_action = action

        #automatically resets the game if truncated or terminated
        if done or truncated:
            self.previous_action = None
            self.previous_state = None
            print("[-1]", flush=True)
        else:
            #action bounce back
            print(f"[{action}]", flush=True)

        
        return done, truncated