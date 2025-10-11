import sys
import warnings

warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)

import pacman_env 

env = pacman_env.PacmanEnv()

for line in sys.stdin:
    try:
        done, truncated = env._step(line)
        
    except Exception as e:
        print(f"ERROR: {e}", flush=True)