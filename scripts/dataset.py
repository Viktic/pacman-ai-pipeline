import pandas as pd
import os
import json
import math

dirpath = os.path.normpath(os.path.realpath(__file__))
processedSessoinsPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed/sessions"))
processedSessionsManifestPath = os.path.normpath(os.path.join(dirpath, "../../../data/processed/manifest.jsonl"))

sID_hash = {}

#inverse golden ratio for even distribution
alpha = (math.sqrt(5) - 1) / 2

with open(processedSessionsManifestPath, "r") as f: 
    for line in f:
        jsonObj = json.loads(line)
        session_id = int(jsonObj["session_id"])
        
        #distributed deterministic hash of session-IDs for better splitting 
        hashedID = (session_id * alpha) % 1
        sID_hash[hashedID] = session_id



#create a training and testing split based on the session-ID hashes
threshold = 0.8

trainingSplit = []
testSplit = []

for key in sID_hash.keys():
    if key > threshold:
        testSplit.append(sID_hash[key])
    else:
        trainingSplit.append(sID_hash[key])

