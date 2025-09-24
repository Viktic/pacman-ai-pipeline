import sys
import json
import pandas as pd
import math
import joblib
import os
from xgboost import XGBClassifier

def cleanData(_df):

    _df.dropna()

    #split player screen position coordinates into seperate columns

    playerScreenX = _df["player_position_screen"].str[0].iloc[0]
    _df["player_position_screenX"] = playerScreenX
    playerScreenY = _df["player_position_screen"].str[1].iloc[0]
    _df["player_position_screenY"] = playerScreenY


    #convert enemy_momenta to dataframe
    enemy_momenta = pd.DataFrame(_df["enemy_momenta"].tolist())

    #split enemy_momenta into seperate columns
    enemy_momenta.columns = [f"enemy{i}_momentum" for i in enemy_momenta.columns]
    for c in enemy_momenta.columns:

        #split enemy_momentum coordinates into seperate columns
        _df[c+'X'] = enemy_momenta[c].str[0]
        _df[c+'Y'] = enemy_momenta[c].str[1]

    #split enemy screen position into seperate columns
    enemy_screen_positions = pd.DataFrame(_df["enemy_positions_screen"].tolist())
    enemy_screen_positions.columns = [f"enemy{i}_position_screen" for i in enemy_screen_positions.columns]

    #calculate distance between players and enemies
    for c in enemy_screen_positions.columns:

        x = enemy_screen_positions[c].str[0].iloc[0]
        y = enemy_screen_positions[c].str[1].iloc[0]

        _df[c+"_distance"] = math.sqrt((x - playerScreenX)**2 + (y - playerScreenY)**2)

    #split player momentum into seperate columns
    _df["player_momentumX"] = _df["player_momentum"].str[0]
    _df["player_momentumY"] = _df["player_momentum"].str[0]

    #drop the old columns from the dataframe
    _df.drop(["enemy_momenta", 
            "enemy_positions_screen", 
            "enemy_positions_grid", 
            "player_position_grid", 
            "player_position_screen", 
            "player_buffer", 
            "player_momentum"],
            axis=1,  
            inplace=True)

    return _df


#loads the model
file_path = os.path.dirname(os.path.realpath(__file__))
model_path = os.path.normpath(os.path.join(file_path, "model.joblib"))

model = joblib.load(model_path)


for line in sys.stdin: 

    snapshot = json.loads(line)
    
    #writes snapshot into dataframe
    df = pd.json_normalize(snapshot)
    #cleans the dataframe
    df = cleanData(df)
    #gets the model prediction 
    pred = model.predict(df) 

    print(pred, flush=True)