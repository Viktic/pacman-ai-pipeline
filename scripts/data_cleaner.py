import pandas as pd
import os
import json
import pyarrow as pa
import numpy as np


#relative pathing for easier setup 
dirPath = os.path.dirname(os.path.realpath(__file__))
rawSessionsPath = os.path.normpath(os.path.join(dirPath, "../../data/raw/sessions"))


#translation dict that assigns every buffer direction vector an index to make the feature one-dimensional
bufferIndex = {
    (0 , 0): 0,
    (-1, 0): 1,
    (1 , 0): 2,
    (0 ,-1): 3,
    (0 , 1): 4
}

def cleanData(_jsonPath, _parquetPath):
    
    #return if the file has already been converted
    if os.path.exists(_parquetPath):
        print("parquet file already exists")
        return
     
    #read the raw json data
    with open(_jsonPath) as f:
        raw_json = json.load(f)

    #convert the json data into a dataframe
    df = pd.json_normalize(raw_json["ticks"])

    #drop all rows that contain NaN-values
    df = df.dropna()

    #split player screen position coordinates into seperate columns
    player_screenX = df["player_position_screen"].str[0] 
    df["player_position_screenX"] = player_screenX

    player_screenY = df["player_position_screen"].str[1] 
    df["player_position_screenY"] = player_screenY


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



    #calculate the distance between player and enemies
    for c in enemy_screen_position.columns:

        arr = np.array(enemy_screen_position[c].tolist(), dtype=float)  # shape (n,2)
        dx = arr[:, 0] - player_screenX
        dy = arr[:, 1] - player_screenY
        df[c + "_distance"] = np.hypot(dx, dy)


    #shift player buffer-collumn by -1 so that the datapoints are labeled with the players reaction to the current state
    df["player_buffer"].shift(-1)

    #replace the player buffer by its corresponding one-dimensional index
    df["player_bufferIndex"] = df["player_buffer"].apply(lambda b: bufferIndex[tuple(b)])

    #split the player momentum coordinates into seperate columns
    df["player_momentumX"] = df["player_momentum"].str[0]
    df["player_momentumY"] = df["player_momentum"].str[1]

    #drop the old columns from the dataframe
    df.drop(columns=["enemy_momenta", "enemy_positions_screen", "enemy_positions_grid", "player_position_screen", "player_position_grid", "player_buffer", "player_momentum"], inplace=True)



    #drop the last row because there is no player reaction to that game-state
    last_row = len(df)-1
    df.drop(df.index[last_row], inplace=True)

    #turn the clean-data-dataframe into parquet
    df.to_parquet(_parquetPath)


    print("parquet conversion succesful")

#iterate over all sessions files
sessions_dir = rawSessionsPath
#clean-data-manifest path
manifestPath = os.path.normpath(os.path.join(dirPath, "../../data/processed/manifest.jsonl"))

#clean every raw-data-session-file and add save it as parquet in the processed-sessions-directory
for file in os.listdir(sessions_dir):
    #skip hidden files in directory
    if file.startswith('.'):
        continue 
 
    filename = file
    session_id = filename[8:-5]
    jsonPath = os.path.normpath(os.path.join(sessions_dir, filename))
    parquetPath = os.path.normpath(os.path.join(dirPath, f"../../data/processed/sessions/session_{session_id}.parquet"))
    
    #create processed-data-manifest if it does not already exist
    if os.path.exists(manifestPath) == False: 
        with open(manifestPath, "a") as f: 
                data = {"session_id": session_id, "file_path": f"sessions/session_{session_id}.parquet"}
                jsonString = json.dumps(data)
                #only write into the file if the current session is not already in there 
                f.write(jsonString)
        cleanData(jsonPath, parquetPath)
        continue

    #turn data in into json-object
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
            
    #clean the file and write the parquet file into data/processed/sessions 
    cleanData(jsonPath, parquetPath)
    