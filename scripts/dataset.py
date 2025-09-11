import pandas as pd
import os
import json
import math

dirpath = os.path.normpath(os.path.realpath(__file__))
processedDataPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed"))
processedSessionsManifestPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed/manifest.jsonl"))

trainDatasetPath = os.path.normpath(os.path.join(dirpath, "../../../data/datasets/train.parquet"))
testDatasetPath = os.path.normpath(os.path.join(dirpath, "../../../data/datasets/test.parquet"))

#inverse golden ratio for even distribution
alpha = (math.sqrt(5) - 1) / 2

threshold = 0.8

trainingSplit = []
testSplit = []

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
        if hashedID < threshold:
            trainingSplit.append(session_df)
        else:
            testSplit.append(session_df)

#concat all the session dataframes into a singular dataframe
test_df = pd.concat(testSplit)
train_df = pd.concat(trainingSplit)

#write the new datasets into a singular parquet file
test_df.to_parquet(testDatasetPath)
train_df.to_parquet(trainDatasetPath)



