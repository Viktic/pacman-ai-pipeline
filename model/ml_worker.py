import sys
import warnings
import logging
import os 

warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)

import pacman_env 

script_dir = os.path.dirname(os.path.abspath(__file__))
eval_dir = os.path.normpath(os.path.join(script_dir, "../eval/"))

#create eval dir if not exists
os.makedirs(eval_dir, exist_ok=True)

log_path = os.path.join(eval_dir, "log.txt")

#configure logging
logging.basicConfig(filename=log_path, level=logging.DEBUG)

env = pacman_env.PacmanEnv()

for line in sys.stdin:
    try:
        done, truncated = env._step(line)
        
    except Exception as e:
       logging.debug(f"ERROR: {e}")