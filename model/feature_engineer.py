
import pandas as pd
import numpy as np




#momentum-index translation table
MOMENTUM_INDEX = {
    (0 , 0): 0,
    (-1, 0): 1,
    (1 , 0): 2,
    (0 ,-1): 3,
    (0 , 1): 4
}

OPPOSITE_MOMENTUM_INDEX = {
    (0 , 0): 0,
    (-1, 0): 2, 
    (1 , 0): 1,
    (0 ,-1): 4,
    (0 , 1): 3,
}


#normalization constants
game_width = 1280
game_height = 880

def encode_direction(dir_index): 
    vec = np.zeros(5)
    vec[int(dir_index)] = 1.0
    return vec


def cleanData(df):

    df = df.dropna()

    #extract the player screen position coordinates
    player_pos = df["player_position_screen"].iloc[0]
    
    #normalize player positions
    player_posX = float(player_pos[0]) / game_width
    player_posY = float(player_pos[1]) / game_height
    

    #convert the player momentum into its corresponding index
    player_momentum = df["player_momentum"][0]
    player_momentumIndex = MOMENTUM_INDEX[tuple(player_momentum)]
    df["player_momentumIndex"] = player_momentumIndex

    #unnest the enemy positions into a seperate dataframe
    enemy_positions = pd.DataFrame(df["enemy_positions_screen"].tolist())

    distances = []

    for c in enemy_positions.columns:
        
        px = enemy_positions[c][0][0] / game_width
        py = enemy_positions[c][0][1] / game_height

        #calculate the distance between the enemy and the player coordinate wise
        dx = px - player_posX
        dy = py - player_posY

        #lies in [0, sqrt(2)] therefore normalization 
        distance = float(np.hypot(dx, dy)) / np.sqrt(2)

        distances.append(distance)

    #get the minimum enemy-player distance
    min_enemy_distance = min(distances)

    #unnest the enemy momenta into a seperate dataframe
    enemy_momenta = pd.DataFrame(df["enemy_momenta"].tolist())

    momenta = []

    for c in enemy_momenta.columns:

        m = enemy_momenta[c][0]

        #check if the enemys momentum is opposite to the players momentum
        mIndex = MOMENTUM_INDEX[tuple(m)]
        comp = OPPOSITE_MOMENTUM_INDEX[tuple(player_momentum)]

        momenta.append(float(mIndex == comp))

    #1.0 means player and enemy are moving towards each other 
    opposite_direction = max(momenta)



    vals = {
        "player_posX" : player_posX, # norm 
        "player_posY" : player_posY, # norm 
        "min_enemy_distance" : min_enemy_distance, # norm 
        "opposite_direction" : opposite_direction, # norm 
        "enemy0_distance": distances[0], # norm 
        "enemy1_distance": distances[1], # norm 
        "enemy2_distance": distances[2], # norm 
    }
    #one-hot encoding for player momentum
    player_momentum_vec = encode_direction(player_momentumIndex)
    
    for i in range(5): 
        vals[f"player_momentum{i}"] = player_momentum_vec[i]

    #builds np.array with final features
    final_features = np.array(list(vals.values()), dtype=np.float32)

    return final_features





