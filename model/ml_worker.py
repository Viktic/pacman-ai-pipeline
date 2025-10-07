import pacman_env 
import warnings

#supress future warnings
warnings.filterwarnings("ignore", category=FutureWarning)
warnings.filterwarnings("ignore", category=UserWarning)




env = pacman_env.PacmanEnv()

#DEBUGGING ONLY:

done = False

while not done:
    env.step('[0]')