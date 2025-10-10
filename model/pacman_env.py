import pandas as pd
import feature_engineer
import numpy as np
import json


class PacmanEnv(): 

    #constructor
    def __init__(self):
        
        pass

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

        #clean the input line
        line = line.strip()
        if not line:
            return

        #extract json from raw input
        raw_data = json.loads(line)

        #convert the json into observation format
        obs = self._translate_obs(raw_data)

        done = raw_data.get("done")
        truncated = raw_data.get("truncated")

        with open("C:/Users/vikto/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/comm_debug.txt", "a") as f:
            f.write(f"terminated: {done}, truncated: {truncated}" + "\n")

        #automatically reset the game if truncated or terminated
        if done or truncated: 
            with open("C:/Users/vikto/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/comm_debug.txt", "a") as f:
                f.write(f"sending restart signal to C++" + "\n")

            print([-1], flush=True)
    
        else:
            #DEBUGGING ONLY: score bounce back
            print([0], flush=True)

        return done, truncated

