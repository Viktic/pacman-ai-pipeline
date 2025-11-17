import pandas as pd
import feature_engineer
import numpy as np
import json
import rl_agent
import logging


class PacmanEnv(): 

    #constructor
    def __init__(self):

        #creates an agent instance
        self.agent = rl_agent.Agent()

        self.previous_state = None
        self.current_state = None
        self.previous_action = None
        self.batch_size = 128
        self.train_freq = 4
        self.update_freq = 1
        
        self.episode_reward = 0
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
            return False, False

        #extracts json from raw input
        raw_data = json.loads(line)

        #converts the json into observation format
        obs = self._translate_obs(raw_data)
        reward = raw_data.get("reward", 0)

        done = raw_data.get("done", False)
        truncated = raw_data.get("truncated", False)

        self.episode_reward += reward

        
        if self.step_count > 0 and self.step_count % 1000 == 0: 
            #DEBUGGING output:
            #logging.info(f"COMM: saving model...")
            self.agent.save_model()


        action = self.agent.select_action(obs)
            
        #adds the transition tuple to the replay buffer 
        if self.previous_state is not None and self.previous_action is not None:
            self.agent.replay_buffer_add(
                self.previous_state, 
                self.previous_action, 
                reward,
                obs,
                done
            )

        if self.agent.replay_buffer.__len__() >= self.batch_size and self.step_count % self.train_freq == 0:
            self.agent.train_step(self.batch_size)

        #sync the policy and target net
        if self.step_count % self.update_freq == 0: 
            self.agent.sync_target_net()
            

        #updates the states
        self.previous_state = obs
        self.previous_action = action

        #automatically resets the game if truncated or terminated
        if done or truncated:
            self.previous_action = None
            self.previous_state = None
            #reduces the agents epsilon rate every episode
            self.agent.reduce_epsilon()

            #DEBUGGING OUTPUT
            logging.debug(f"reward: {self.episode_reward}")
            self.episode_reward = 0

            print("[-1]", flush=True)

        else:
            
            #DEBUGGING output: 
            #logging.debug(f"COMM: sending action: {action} to env")

            #action bounce back
            print(f"[{action}]", flush=True)

        #increases the step count
        self.step_count += 1

        return done, truncated