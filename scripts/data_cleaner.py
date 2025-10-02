import pandas as pd
import featuretools as ft
import numpy as np
import os
import json
import pyarrow 

Index = {
    (0 , 0): 0,
    (-1, 0): 1,
    (1 , 0): 2,
    (0 ,-1): 3,
    (0 , 1): 4
}

oppositeIndex = {
    (0 , 0): 0,
    (-1, 0): 2, 
    (1 , 0): 1,
    (0 ,-1): 4,
    (0 , 1): 3,
}



dirPath = os.path.dirname(os.path.realpath(__file__))
rawSessionsPath = os.path.normpath(os.path.join(dirPath, "../../data/raw/sessions"))



def cleanData(raw_path):


    
    #read the raw json data
    with open(raw_path) as f:
        raw = json.load(f)

    df = pd.json_normalize(raw["ticks"])

    df.dropna()

    ticks = df["tick"].values
    score = df["score"].values

    tick_data = {
        "tick_id" : ticks,
        "score" : score
    }

    tick_table = pd.DataFrame(data=tick_data)


    #read the player specific data into a seperate player table

    df["player_posX"] = df["player_position_screen"].str[0]
    df["player_posY"] = df["player_position_screen"].str[1]

    player_posX = df["player_posX"]
    player_posY = df["player_posY"]


    #shift the player momentum buffer by -1 to get the players reaction to the game-state
    df["player_buffer"] = df["player_buffer"].shift(-1)

    df.drop(df.tail(1).index, inplace=True)

    #convert the player momentum buffer into the corresponding bufferIndex
    bufferIndex = df["player_buffer"].apply(lambda x: Index[tuple(x)])

    #convert the player momentum into the corresponding momentumIndex
    momentumIndex = df["player_momentum"].apply(lambda x: Index[tuple(x)])

    player_data = {
        "tick" : ticks, 
        "posX" : player_posX,
        "posY" : player_posY, 
        "player_bufferIndex" : bufferIndex, 
        "player_momentumIndex" : momentumIndex
    }

    player_table = pd.DataFrame(data=player_data) 

    #drop the last row because there is no momentum buffer for it
    player_table.drop(player_table.tail(1).index, inplace=True)


    #read the enemy specific data into a seperate enemy table

    #unnest the momenta and screen position arrays
    df_long = df.explode(["enemy_momenta", "enemy_positions_screen"]).reset_index(drop=True)

    #fill the player enemy dataframe with player positions to avoid dimension mismatch
    df_long[["player_posX", "player_posY"]] = df_long.groupby("tick")[["player_posX", "player_posY"]].ffill()

    df_long["player_momentum"] = df_long.groupby("tick")["player_momentum"].ffill()
    df_long["player_momentumIndex"] = df_long["player_momentum"].apply(lambda x: Index[tuple(x)])

    df_long["enemy_id"] = df_long.groupby("tick").cumcount()

    df_long[["enemy_posX", "enemy_posY"]] = pd.DataFrame(df_long["enemy_positions_screen"].tolist(), index=df_long.index)

    df_long = df_long.drop(columns=["enemy_positions_screen"])

    enemy_id = df_long["enemy_id"].values

    #get the opposite Index of the current enem momentum
    opposite_directionIndex = df_long["player_momentum"].apply(lambda x: oppositeIndex[tuple(x)])

    df_long["momentum_index"] = df_long["enemy_momenta"].apply(lambda x: Index[tuple(x)])
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

    es = ft.EntitySet(id="game_data")

    #add the tick table to the entity set
    es = es.add_dataframe (
        dataframe=tick_table,
        dataframe_name="ticks",
        index="tick_id",
        make_index=False,
        already_sorted=True
    )

    #add the player table to the entity set
    es = es.add_dataframe (
        dataframe=player_table,
        dataframe_name = "player",
        index="tick_id",
        time_index="tick",
        make_index=True,
        already_sorted=True
    )

    #add the enemy table to the entity set
    es = es.add_dataframe (
        dataframe=enemy_table,
        dataframe_name="enemy",
        index = "tick_id",
        time_index="tick",
        make_index = True,
        already_sorted= True
    )

    #add the relationships between the tables
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


    #default aggregation primitives
    default_agg_primitives = ["sum", "max", "min", "mean", "count"]

    #deep feature synthesis (DFS)
    features = ft.dfs(
        entityset=es, 
        target_dataframe_name="player",
        agg_primitives=default_agg_primitives,
        max_depth=2,
        features_only=True
    )

    #feature matrix
    feature_matrix, feature_names = ft.dfs(
        entityset=es, 
        target_dataframe_name="player",
        agg_primitives=default_agg_primitives,
        max_depth=2,
        features_only=False, 
        verbose=True
    )

    feature_matrix = pd.DataFrame(feature_matrix)

    #remove highly correlated features
    feature_matrix, feature_names = ft.selection.remove_highly_correlated_features(
        feature_matrix=feature_matrix, 
        features=features
        )

    #drop useless features
    feature_matrix.drop(columns=["ticks.COUNT(player)", 
                        "ticks.COUNT(enemy)", 
                        "ticks.MAX(enemy.enemy_id)", 
                        "ticks.MAX(enemy.enemy_momentumIndex)", 
                        "ticks.MEAN(enemy.enemy_id)",
                        "ticks.MEAN(enemy.enemy_momentumIndex)", 
                        "ticks.MIN(enemy.enemy_id)",
                        "ticks.MIN(enemy.enemy_momentumIndex)",
                        "ticks.MIN(enemy.posX)",
                        "ticks.MIN(enemy.posY)",
                        "ticks.MAX(enemy.posX)",
                        "ticks.MAX(enemy.posY)",
                        "ticks.SUM(enemy.enemy_id)",
                        "ticks.MAX(enemy.player_dist)",
                        "ticks.MEAN(enemy.player_dist)"
                        ])

    return feature_matrix

sessionsDir = rawSessionsPath

manifestPath = os.path.normpath(os.path.join(dirPath, "../../data/processed/manifest.jsonl"))

#create the empty processed-data manifest if does not yet exist
if not os.path.exists(manifestPath):
    open (manifestPath, "w").close()


for file in os.listdir(sessionsDir):
    #skip hidden files in directory
    if file.startswith('.'):
        continue

    filename = file
    session_id = filename[8:-5]
    jsonPath = os.path.normpath(os.path.join(sessionsDir, filename))
    parquetPath = os.path.normpath(os.path.join(dirPath, f"../../data/processed/sessions/session_{session_id}.parquet"))

    #check if the file has already been converted
    if os.path.exists(parquetPath):
        print("parquet file already exists")
        continue

    try:
        df = cleanData(jsonPath)
        print(f"feature engineering succesful")
    except Exception as e:
        print(f"{file} does not contain enough features -- file skipped")
        continue


    #turn data into json-object
    data = {"session_id": session_id, "file_path": f"sessions/session_{session_id}.parquet"}
    jsonString = json.dumps(data)

    #check if the file thats currently processed has already been logged in the manifest
    exists = False
    with open(manifestPath, "r") as f: 
        for line in f: 
            if line.strip() == jsonString:
                exists = True 
                break
    
    #log the current file in the manifest
    if not exists: 
        with open(manifestPath, "a") as f: 
            f.write("\n" + jsonString) 
            
    #convert the file to parquet
    try: 
        df.to_parquet(parquetPath)
        print(f"{file} parquet conversion succesful")
    except Exception as e:
        print(f"{file} parquet conversion failed ")

    


