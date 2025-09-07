import pandas as pd
import os
import json
import ast

#relative pathing for easier setup 
dirPath = os.path.dirname(os.path.realpath(__file__))
jsonPath = os.path.join(dirPath, "../../data/raw/sessions/session_29.json")


#read the raw json data
with open(jsonPath) as f:
    raw_json = json.load(f)

#convert the json data into a dataframe
df = pd.json_normalize(raw_json["ticks"])


#constant position scalars to normalize coordinate values
grid_height = 11
grid_width = 16

screen_height = 1000
screen_width = 1600

#split enemy momenta into seperate columns
enemy_momenta = pd.DataFrame(df["enemy_momenta"].tolist())
enemy_momenta.columns = [f"enemy{i}_momentum" for i in enemy_momenta.columns]

#split the enemy momentum coordinates into seperate columns
for c in enemy_momenta.columns:
 
    enemy_momenta[c].str[0]
    df[c+'X'] = enemy_momenta[c].str[0]

    df[c+'Y'] = enemy_momenta[c].str[1]

#split enemy screen position into seperate columns
enemy_screen_position = pd.DataFrame(df["enemy_positions_screen"].tolist())
enemy_screen_position.columns = [f"enemy{i}_position_screen" for i in enemy_screen_position.columns]

#split enemy screen position coordinates into seperate columns
for c in enemy_screen_position.columns:
  
    #normalize enemy screen position coordinates into range [0,1]
    x = enemy_screen_position[c].str[0]
    x = x / screen_width
    df[c+'X'] = x

    y = enemy_screen_position[c].str[1]
    y = y / screen_height
    df[c+'Y'] = y

#split enemy grid position into seperate columns
enemy_grid_position = pd.DataFrame(df["enemy_positions_grid"].tolist())
enemy_grid_position.columns = [f"enemy{i}_position_grid" for i in enemy_grid_position.columns]

#split enemy grid position coordinates into seperate columns
for c in enemy_grid_position.columns:

    #normalize enemy grid position coordinates into range [0,1]
    x = enemy_grid_position[c].str[0]
    x = x / grid_width
    df[c+'X'] = x

    y = enemy_grid_position[c].str[1]
    y = y / grid_height
    df[c+'Y'] = y

#split player screen position coordinates into seperate columns
player_screenX = df["player_position_screen"].str[0] / screen_width
df["player_position_screenX"] = player_screenX

player_screenY = df["player_position_screen"].str[1] / screen_height
df["player_position_screenY"] = player_screenY

#split player grid coordinates into seperate columns
player_gridX = df["player_position_grid"].str[0] / grid_width
df["player_position_gridX"] = player_gridX

player_gridY = df["player_position_grid"].str[1] / grid_height
df["player_position_gridY"] = player_gridY

#split player buffer coordinates into seperate columns
df["player_bufferX"] = df["player_buffer"].str[0]
df["player_bufferY"] = df["player_buffer"].str[1]

#split the player momentum coordinates into seperate columns
df["player_momentumX"] = df["player_momentum"].str[0]
df["player_momentumY"] = df["player_momentum"].str[1]


df.drop(columns=["enemy_momenta", "enemy_positions_screen", "enemy_positions_grid", "player_position_screen", "player_position_grid", "player_buffer", "player_momentum"], inplace=True)


pd.set_option('display.max_columns', None) 
print(df.head())


