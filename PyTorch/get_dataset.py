import glob
from pathlib import Path
import pandas as pd
import numpy as np

def GetDataset():
    # Get iterables for csv datasets.
    cwd = Path(__file__).parent
    table_csv_wildcard = cwd / "./csv/GuanYinSurfaces/GuanYinTable/*.csv"
    wall_csv_wildcard = cwd / "./csv/GuanYinSurfaces/GuanYinWall/*.csv"
    table_csv_paths = glob.glob(str(table_csv_wildcard))
    wall_csv_paths = glob.glob(str(wall_csv_wildcard))

    # Load the CSV files
    table_dataframes = list(
        map(lambda file_path: pd.read_csv(file_path), table_csv_paths)
    )
    wall_dataframes = list(
        map(lambda file_path: pd.read_csv(file_path), wall_csv_paths)
    )
    all_dataframes = table_dataframes + wall_dataframes

    for df in all_dataframes:
        # Enforce numbers are numeric.
        assert all(df.dtypes.apply(pd.api.types.is_numeric_dtype))
        # Assert that the data has 7 columns.
        assert df.shape[1] == 7, "Data should have 7 columns."
        # Rename columns.
        df.columns = ["t", "ax", "ay", "az", "gx", "gy", "gz"]
        # Calculate acceleration magnitude.
        df["a_mag"] = (df["ax"] ** 2 + df["ay"] ** 2 + df["az"] ** 2) ** 0.5
        # Calculate sample rate.
        df.attrs["sample_rate"] = 1 / df["t"].diff().median()
        # Calculate fft of entire a_mag.
        df.attrs["a_mag_fft"] = np.fft.fft(df["a_mag"])

    for idx, df in enumerate(table_dataframes):
        df.attrs["surface"] = "table " + str(idx)
    for idx, df in enumerate(wall_dataframes):
        df.attrs["surface"] = "wall " + str(idx)

    return all_dataframes