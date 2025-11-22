import sys
import warnings
import logging

warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)

import pacman_env 

#Debugging
logging.basicConfig(filename='/Users/viktorbrandmaier/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/python_worker_debug.log', level=logging.DEBUG)

env = pacman_env.PacmanEnv()

for line in sys.stdin:
    try:
        done, truncated = env._step(line)
        
    except Exception as e:
       logging.debug(f"ERROR: {e}")