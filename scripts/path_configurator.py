import os
import json

dirPath = os.path.dirname(os.path.abspath(__file__))
jsonConfigPaths = os.path.normpath(os.path.join(dirPath, "../config/paths.json"))
rawDataDir = os.path.normpath(os.path.join(dirPath, "../../data/raw"))
rawDataDirManifest = os.path.normpath(os.path.join(dirPath, "../../data/raw/manifest.jsonl"))

with open(jsonConfigPaths, "w") as f: 
    data = {"rawDataDir": rawDataDir, 
            "rawDataDirManifest": rawDataDirManifest}
    json.dump(data, f)
    
