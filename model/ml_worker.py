import sys
import json
import pandas as pd
import math

    
    
for line in sys.stdin: 

    snapshot = json.loads(line)
    
    #write snapshot into dataframe
    df = pd.json_normalize(snapshot)

    #split player screen position coordinates into seperate columns

    playerScreenX = df["player_position_screen"].str[0].iloc[0]
    df["player_position_screenX"] = playerScreenX
    playerScreenY = df["player_position_screen"].str[1].iloc[0]
    df["player_position_screenY"] = playerScreenY


    #convert enemy_momenta to dataframe
    enemy_momenta = pd.DataFrame(df["enemy_momenta"].tolist())

    #split enemy_momenta into seperate columns
    enemy_momenta.columns = [f"enemy{i}_momentum" for i in enemy_momenta.columns]
    for c in enemy_momenta.columns:

        #split enemy_momentum coordinates into seperate columns
        df[c+'X'] = enemy_momenta[c].str[0]
        df[c+'Y'] = enemy_momenta[c].str[1]

    #split enemy screen position into seperate columns
    enemy_screen_positions = pd.DataFrame(df["enemy_positions_screen"].tolist())
    enemy_screen_positions.columns = [f"enemy{i}_position_screen" for i in enemy_screen_positions.columns]

    #calculate distance between players and enemies
    for c in enemy_screen_positions.columns:

        x = enemy_screen_positions[c].str[0].iloc[0]
        y = enemy_screen_positions[c].str[1].iloc[0]

        df[c+" _distance"] = math.sqrt((x - playerScreenX)**2 + (y - playerScreenY)**2)

    #split player momentum into seperate columns
    df["player_momentumX"] = df["player_momentum"].str[0]
    df["player_momentumY"] = df["player_momentum"].str[0]

    #drop the old columns from the dataframe
    df.drop(["enemy_momenta", 
            "enemy_positions_screen", 
            "enemy_positions_grid", 
            "player_position_grid", 
            "player_position_screen", 
            "player_buffer", 
            "player_momentum"],
            axis=1,  
            inplace=True)

    print(df.shape[1], flush=True)