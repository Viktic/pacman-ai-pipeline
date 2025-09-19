import pandas as pd
import os
import json
import math

dirpath = os.path.normpath(os.path.realpath(__file__))
processedDataPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed"))
processedSessionsManifestPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed/manifest.jsonl"))

trainDatasetPath1 = os.path.normpath(os.path.join(dirpath, "../../../data/datasets/train_1.parquet"))
trainDatasetPath2 = os.path.normpath(os.path.join(dirpath, "../../../data/datasets/train_2.parquet"))

testDatasetPath1 = os.path.normpath(os.path.join(dirpath, "../../../data/datasets/test_1.parquet"))
testDatasetPath2 = os.path.normpath(os.path.join(dirpath, "../../data/datasets/test_2.parquet"))
#inverse golden ratio for even distribution
alpha = (math.sqrt(5) - 1) / 2

threshold = 0.8

trainingSplit_1 = []
trainingSplit_2 = []
testSplit_1 = []
testSplit_2 = []

binaryBuffer = {
    0: 0, 
    1: 1,
    2: 1,
    3: 1
}

with open(processedSessionsManifestPath, "r") as f: 
    for line in f:
        jsonObj = json.loads(line)
        session_id = int(jsonObj["session_id"])
        
        #distributed deterministic hash of session-IDs for better splitting 
        hashedID = (session_id * alpha) % 1
        sessionPathRelative = jsonObj["file_path"]
        sessionPathAbsolute = os.path.normpath(os.path.join(processedDataPath, sessionPathRelative))

        #split the processed data into testing and training split
        session_df = pd.read_parquet(sessionPathAbsolute) 

        #create dataframe for the binary classifier
        session_df1 = session_df.copy()
        session_df1["player_bufferIndex"] = session_df1["player_bufferIndex"].apply(lambda i: binaryBuffer[i])

        #create dataframe for the multi-class classifier (indexing starting from 0)
        session_df2 = session_df.copy()
        session_df2 = session_df2.drop(session_df2[session_df2["player_bufferIndex"] == 0].index)
        session_df2["player_bufferIndex"] = session_df2["player_bufferIndex"].apply(lambda i: i - 1)

        if hashedID < threshold:
            trainingSplit_1.append(session_df1)
            trainingSplit_2.append(session_df2)
        else:
            testSplit_1.append(session_df1)
            testSplit_2.append(session_df2)

#concat all the session dataframes into a singular dataframe
test_df1 = pd.concat(testSplit_1)
test_df2 = pd.concat(testSplit_2)

train_df1 = pd.concat(trainingSplit_1)
train_df2 = pd.concat(trainingSplit_2)

#write the new datasets into a singular parquet file
test_df1.to_parquet(testDatasetPath1)
test_df2.to_parquet(testDatasetPath2)
train_df1.to_parquet(trainDatasetPath1)
train_df2.to_parquet(trainDatasetPath2)



