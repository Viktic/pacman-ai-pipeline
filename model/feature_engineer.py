import sys
import json
import pandas as pd
import joblib
import os
import featuretools as ft
import numpy as np
import warnings
from featuretools import calculate_feature_matrix


dirPath = os.path.dirname(os.path.realpath(__file__))
model_path = os.path.normpath(os.path.join(dirPath, "model.joblib"))
presetFeaturesPath = os.path.normpath(os.path.join(dirPath, "features.joblib"))

#loads the model
model = joblib.load(model_path)

#loads the preset features
feature_defs = joblib.load(presetFeaturesPath)

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

    df = pd.json_normalize(df["ticks"])

    #relational schemes for feature extraction

    # -- tick scheme -- 
    ticks = df["tick"].values
    score = df["score"].values

    tick_data = {
        "tick_id" : ticks,
        "score" : score
    }

    tick_table = pd.DataFrame(data=tick_data)
    # -------------------

    # -- player scheme -- 
    df["player_posX"] = df["player_position_screen"].apply(lambda x: x[0])
    df["player_posY"] = df["player_position_screen"].apply(lambda x: x[1])

    #convert the player momentum into the corresponding momentumIndex
    momentumIndex = df["player_momentum"].apply(lambda x: MOMENTUM_INDEX[tuple(x)])

    player_data = {
        "tick" : ticks, 
        "posX" : df["player_posX"],
        "posY" : df["player_posY"], 
        "player_momentumIndex" : momentumIndex
    }

    player_table = pd.DataFrame(data=player_data) 
    # -------------------

    # -- enemy scheme --

    #unnest the momenta and screen position arrays
    df_long = df.explode(["enemy_momenta", "enemy_positions_screen"]).reset_index(drop=True)

    #fill the player enemy dataframe with player positions to avoid dimension mismatch
    df_long[["player_posX", "player_posY"]] = df_long.groupby("tick")[["player_posX", "player_posY"]].ffill()

    df_long["player_momentum"] = df_long.groupby("tick")["player_momentum"].ffill()
    df_long["player_momentumIndex"] = df_long["player_momentum"].apply(lambda x: MOMENTUM_INDEX[tuple(x)])

    df_long["enemy_id"] = df_long.groupby("tick").cumcount()

    df_long[["enemy_posX", "enemy_posY"]] = pd.DataFrame(df_long["enemy_positions_screen"].tolist(), index=df_long.index)

    df_long = df_long.drop(columns=["enemy_positions_screen"])

    enemy_id = df_long["enemy_id"].values

    #get the opposite Index of the current enem momentum
    opposite_directionIndex = df_long["player_momentum"].apply(lambda x: OPPOSITE_MOMENTUM_INDEX[tuple(x)])

    df_long["momentum_index"] = df_long["enemy_momenta"].apply(lambda x: MOMENTUM_INDEX[tuple(x)])
    df_long["opposite_direction"] = opposite_directionIndex == df_long["player_momentumIndex"]
    df_long["opposite_direction"] = df_long["opposite_direction"].apply(lambda x: int(x))

    #distance between enemy and player
    df_long["distance"] = np.hypot (
        df_long["enemy_posX"] - df_long["player_posX"],
        df_long["enemy_posY"] - df_long["player_posY"]
    )


    ticks = df_long["tick"].values

    enemy_data = {
        "tick" : ticks, 
        "enemy_id" : enemy_id,
        "enemy_momentumIndex" : df_long["momentum_index"],
        "posX" : df_long["enemy_posX"],
        "posY" : df_long["enemy_posY"],
        "player_dist": df_long["distance"],
        "opposite_direction": df_long["opposite_direction"]
    }

    enemy_table = pd.DataFrame(data=enemy_data)
    # -------------------

    #EntitySet for feature extraction
    es = ft.EntitySet(id="game_data")

    #adds tick table to entity set
    es = es.add_dataframe (
        dataframe=tick_table,
        dataframe_name="ticks",
        index="tick_id",
        make_index=False,
        already_sorted=True
    )

    #adds player table to entity set
    es = es.add_dataframe (
        dataframe=player_table,
        dataframe_name = "player",
        index="tick_id",
        time_index="tick",
        make_index=True,
        already_sorted=True
    )

    #adds enemy table to entity set
    es = es.add_dataframe (
        dataframe=enemy_table,
        dataframe_name="enemy",
        index = "tick_id",
        time_index="tick",
        make_index = True,
        already_sorted= True
    )

    #adds relationships between the tables
    es.add_relationship(
        parent_dataframe_name="ticks",
        parent_column_name="tick_id",
        child_dataframe_name="player",
        child_column_name="tick"
    )

    es.add_relationship(
        parent_dataframe_name="ticks",
        parent_column_name="tick_id",
        child_dataframe_name="enemy",
        child_column_name="tick"
    )

    #extracts complex features from the input-data
    feature_matrix = calculate_feature_matrix(
        features = feature_defs,
        entityset=es
    )

    #supress ft warnings
    with warnings.catch_warnings():
        warnings.simplefilter("ignore", category=FutureWarning)
        #extracts complex features from the input-data
        feature_matrix = calculate_feature_matrix(
            features = feature_defs,
            entityset=es
        )
        
    return feature_matrix





for line in sys.stdin: 

    snapshot = json.loads(line)
    
    #writes snapshot into dataframe
    df = pd.json_normalize(snapshot)
    
    #cleans the dataframe
    df = cleanData(df)

    #gets the model prediction 
    pred = model.predict(df) 

    print(pred, flush=True)