
import pandas as pd
import os
import numpy as np


dirPath = os.path.dirname(os.path.realpath(__file__))
model_path = os.path.normpath(os.path.join(dirPath, "model.joblib"))
presetFeaturesPath = os.path.normpath(os.path.join(dirPath, "features.joblib"))


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


def cleanData(df):

    df = df.dropna()

    #split the player screen position coordinates into sepearate columns
    player_posX = df["player_position_screen"].str[0]
    player_posY = df["player_position_screen"].str[1]

    #convert the player momentum into its corresponding index
    player_momentum = df["player_momentum"][0]
    player_momentumIndex = MOMENTUM_INDEX[tuple(player_momentum)]
    df["player_momentumIndex"] = player_momentumIndex

    #unnest the enemy positions into a seperate dataframe
    enemy_positions = pd.DataFrame(df["enemy_positions_screen"].tolist())

    distances = []

    for c in enemy_positions.columns:
        
        px = enemy_positions[c][0][0]
        py = enemy_positions[c][0][1]

        #calculate the distance between the enemy and the player coordinate wise
        dx = px - player_posX
        dy = py - player_posY

        distance = float(np.hypot(dx, dy))

        df[f"enemy{c}_distance"] = distance

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

    #1.0 means player and enemy ar moving towards each other 
    opposite_direction = max(momenta)

    vals = {
        "tick" : df["tick"],
        "player_posX" : player_posX,
        "player_posY" : player_posY,
        "player_momentumIndex" : player_momentumIndex, 
        "min_enemy_distance" : min_enemy_distance,
        "opposite_direction" : opposite_direction,
        "enemy0_distance": df["enemy0_distance"],
        "enemy1_distance": df["enemy1_distance"],
        "enemy2_distance": df["enemy2_distance"],
    }

    #return dataframe with aggregated features
    final_features = pd.DataFrame(data=vals)
    return final_features





