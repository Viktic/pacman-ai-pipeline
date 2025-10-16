import pandas as pd
import feature_engineer
import numpy as np
import json
import rl_agent


class PacmanEnv(): 

    #constructor
    def __init__(self):

        #creates an agent instance
        self.agent = rl_agent.Agent()

        self.previous_state = None
        self.current_state = None
        self.previous_action = None
        
        self.step_count = 0
    
    #translate the raw observation into a valid observation-format
    def _translate_obs(self, raw): 

        raw_df = pd.json_normalize(raw)

        #cleans the raw-data-snapshot
        obs = feature_engineer.cleanData(raw_df)        
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
            self.agent.replay_buffer_add(
                self.previous_state, 
                self.previous_action, 
                reward,
                obs,
                done
            )
        
        #DEBUGGING ONLY 
        if self.step_count % 50 == 0 and not (self.step_count % 1000 == 0):
            with open("/Users/viktorbrandmaier/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/python_worker_debug.log", "a") as f: 
                f.write(str(self.step_count) + "\n")

        #sync the target policy and target net every 1000 steps
        if self.step_count > 0 and self.step_count % 1000 == 0: 
            #DEBUGGING ONLY
            with open("/Users/viktorbrandmaier/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/python_worker_debug.log", "a") as f: 
                f.write("trying to save model checkpoint")
            
            self.agent.sync_target_net()
            #TESTING ONLY: 
            self.agent.save_model()


        self.agent.train_step(128)


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

        #increases the step count
        self.step_count += 1
        #reduces the agents epsilon rate
        self.agent.reduce_epsilon()

        return done, truncated