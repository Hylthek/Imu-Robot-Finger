print("importing...")
from get_dataset import GetDataset
from play_dataset_audio import PlayDatasetAudio
from preprocess_imu_waveform import PreprocessIMUWaveform
import torch
from transformers import ClapModel, ClapProcessor
import numpy as np
from clap_embed_audio import clap_embed_audio

print("starting program...")
dataset = GetDataset()
# PlayDatasetAudio(dataset)

# Preprocess all of the dataset dataframes.
CLAP_SAMPLE_RATE = 48000
processed_dataset = [
    PreprocessIMUWaveform(
        df["a_mag"].to_numpy(), int(df.attrs["sample_rate"]), CLAP_SAMPLE_RATE
    )
    for df in dataset
]

# Make embeddings.
embeddings = clap_embed_audio(processed_dataset[0], CLAP_SAMPLE_RATE)
print(embeddings.squeeze().tolist())
