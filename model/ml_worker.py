import pacman_env 
import warnings
import sys

#supress future warnings
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)


env = pacman_env.PacmanEnv()

#DEBUGGING ONLY:

for line in sys.stdin:

    done, truncated = env._step(line)
    #DEBUG LOGGING
    with open("C:/Users/vikto/Desktop/Pacman-Pipeline/pacman-ai-pipeline/tests/comm_debug.txt", "a") as f:
        f.write(f"terminated: {done}, truncated: {truncated}" + "\n")
    f.close()