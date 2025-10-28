import os
import json
import sys

dirPath = os.path.dirname(os.path.abspath(__file__))
jsonConfigPaths = os.path.normpath(os.path.join(dirPath, "../config/paths.json"))
rawDataDir = os.path.normpath(os.path.join(dirPath, "../../data/raw"))
rawDataDirManifest = os.path.normpath(os.path.join(dirPath, "../../data/raw/manifest.jsonl"))
ml_workerPath = os.path.normpath(os.path.join(dirPath, "../model/ml_worker.py"))
pythonPath = os.path.normpath(sys.executable)


with open(jsonConfigPaths, "w") as f: 
    data = {"rawDataDir": rawDataDir, 
            "rawDataDirManifest": rawDataDirManifest,
            "ml_workerPath": ml_workerPath,
            "pythonPath": pythonPath}
    json.dump(data, f)
    
