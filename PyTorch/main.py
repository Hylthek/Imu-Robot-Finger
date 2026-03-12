print("importing...")
from get_dataset import GetDataset
from play_dataset_audio import PlayDatasetAudio
from preprocess_imu_waveform import PreprocessIMUWaveform
import torch
from transformers import ClapModel, ClapProcessor
import numpy as np
from clap_embed_audio import ClapEmbedAudio

print("starting program...")
dataset = GetDataset()
# PlayDatasetAudio(dataset)

# Preprocess all of the dataset dataframes.
print("preprocessing dataset...")
CLAP_SAMPLE_RATE = 48000
processed_dataset = [
    PreprocessIMUWaveform(
        df["a_mag"].to_numpy(), int(df.attrs["sample_rate"]), CLAP_SAMPLE_RATE
    )
    for df in dataset
]

# Make embeddings.
print("making embeddings...")
all_embeddings = [ClapEmbedAudio(pd) for pd in processed_dataset]
# Compute dot products between all embeddings
print(all_embeddings[0].shape)
dot_products = np.array(
    [
        [torch.dot(emb1.squeeze(), emb2.squeeze()) for emb2 in all_embeddings]
        for emb1 in all_embeddings
    ]
)

import matplotlib.pyplot as plt

str_labels = [df.attrs["surface"] for df in dataset]
plt.figure(figsize=(10, 8))
plt.imshow(dot_products, cmap="hot", aspect="auto")
plt.colorbar(label="Dot Product")
plt.xticks(ticks=range(len(str_labels)), labels=str_labels, rotation=45, ha="right")
plt.yticks(ticks=range(len(str_labels)), labels=str_labels)
plt.xlabel("Embedding Index")
plt.ylabel("Embedding Index")
plt.title("Dot Products Heatmap")
plt.tight_layout()
# plt.show()

# Plot spectrograms of two processed dataset indices
fig, nd_axes = plt.subplots(2, 2, figsize=(16, 9))
axes = np.array(nd_axes).flatten()
for i, idx in enumerate([0, 16, 5, 23]):
    axes[i].specgram(processed_dataset[idx], Fs=CLAP_SAMPLE_RATE, cmap='viridis')
    axes[i].set_title(f"Spectrogram - {str_labels[idx]}")
    axes[i].set_ylabel("Frequency (Hz)")
    axes[i].set_xlabel("Time (s)")
plt.tight_layout()
plt.show()