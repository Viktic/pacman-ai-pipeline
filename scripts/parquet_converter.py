import pyarrow as pa
import pandas as pd
import pyarrow.parquet as pq
import os

dirPath = os.path.dirname(os.path.realpath(__file__))
jsonPath = os.path.join(dirPath, "../../data/raw/sessions/session_29.json")
parquetPath = os.path.join(dirPath, "../../data/processed/session_29.parquet")


#convert the json file into parquet if the parquetted file does not already exist
if (os.path.exists(parquetPath) == False): 
    df = pd.read_json(jsonPath)

    table = pa.Table.from_pandas(df)
    pq.write_table(table, parquetPath)
    
    print(f"succesfully converted {jsonPath} into parquet")

#DEBUGGING ONLY 
pd.set_option('display.max_colwidth', None)  
df = pd.read_parquet(parquetPath)
print(df.head(5))



