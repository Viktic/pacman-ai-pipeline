import pandas as pd
import numpy as np
import logging

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
TILE_SIZE = 80
GAME_WIDTH = 16 * TILE_SIZE
GAME_HEIGHT = 11 * TILE_SIZE
danger_radius_tiles = 2.0


def encode_direction(dir_index): 
    vec = np.zeros(5)
    vec[int(dir_index)] = 1.0
    return vec


def cleanData(df):

    df = df.dropna()

    #extract the player screen position coordinates
    player_pos = df["player_position_screen"].iloc[0]
    
    #normalize player positions
    player_posX = float(player_pos[0]) / GAME_WIDTH
    player_posY = float(player_pos[1]) / GAME_HEIGHT
    

    #convert the player momentum into its corresponding index
    player_momentum = df["player_momentum"][0]
    player_momentumIndex = MOMENTUM_INDEX[tuple(player_momentum)]
    df["player_momentumIndex"] = player_momentumIndex

   
    #extracts the pellet positions
    pellet_positions = df["pellet_positions"].iloc[0] if "pellet_positions" in df.columns else []

    if len(pellet_positions) > 0:
        #finds nearest pellet
        pellet_dists = []
        for p in pellet_positions:
            px, py = p[0] / GAME_WIDTH, p[1] / GAME_HEIGHT
            dx, dy = px - player_posX, py - player_posY
            pellet_dists.append(float(np.hypot(dx, dy)) / np.sqrt(2))
        
        nearest_idx = np.argmin(pellet_dists)
        nearest_pellet = pellet_positions[nearest_idx]
        
        nearest_pellet_dist = pellet_dists[nearest_idx]
        nearest_pellet_dx = (nearest_pellet[0] / GAME_WIDTH) - player_posX
        nearest_pellet_dy = (nearest_pellet[1] / GAME_HEIGHT) - player_posY
        pellets_remaining = len(pellet_positions)
    else:
        nearest_pellet_dist = 0.0
        nearest_pellet_dx = 0.0
        nearest_pellet_dy = 0.0
        pellets_remaining = 0

    #extracts the direction information 
    directions = df["directions"]


    vals = [
        player_posX,
        player_posY,
        nearest_pellet_dist,     
        nearest_pellet_dx,       
        nearest_pellet_dy,        
        pellets_remaining / 50.0,  
    ]
    

    #one-hot encoding for player momentum
    player_momentum_vec = encode_direction(player_momentumIndex)

    #unnest the enemy positions into a seperate dataframe
    if "enemy_positions_screen" in df.columns:
        enemy_positions = pd.DataFrame(df["enemy_positions_screen"].tolist())

        distances = []

        for c in enemy_positions.columns:
            
            px = enemy_positions[c][0][0] / GAME_WIDTH
            py = enemy_positions[c][0][1] / GAME_HEIGHT

            #calculate the distance between the enemy and the player coordinate wise
            dx = px - player_posX
            dy = py - player_posY

            #lies in [0, sqrt(2)] therefore normalization 
            distance = float(np.hypot(dx, dy)) / np.sqrt(2)
            distances.append(distance)

        #relative enemy position vector
        enemy_rel = []
        for c in enemy_positions.columns:
            px = enemy_positions[c][0][0] / GAME_WIDTH
            py = enemy_positions[c][0][1] / GAME_HEIGHT
            dx = px - player_posX
            dy = py - player_posY
            enemy_rel.extend([dx, dy])

        #append the minimum enemy-player distance
        vals.append(min(distances))

        #danger signal to reinforce min(distance) implicit danger indicator
        danger_threshold = danger_radius_tiles * np.hypot(TILE_SIZE/GAME_WIDTH, TILE_SIZE/GAME_HEIGHT)
        if min(distances) < danger_threshold: 
            danger = 1.0
        else: 
            danger = 0.0
        vals.append(danger)

        #adds relative enemy positions (2 features: dx, dy for each enemy)
        vals.extend(enemy_rel)
  

    if "enemy_momenta" in df.columns:
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
        vals.append(max(momenta))


    #adds player momentum features (5 features)
    vals.extend(player_momentum_vec.tolist())

    #converts direction booleans into binary features
    if "directions" in df.columns: 
        directions = df["directions"].iloc[0]
        direction_features = [float(d) for d in directions]
        vals.extend(direction_features)
    
    #converts the wall distance features into 
    if "wall_distances" in df.columns:
        wall_distances = df["wall_distances"].iloc[0]
        vals.extend(wall_distances)

    #builds np.array with final features
    final_features = np.array(vals, dtype=np.float32)

    return final_features