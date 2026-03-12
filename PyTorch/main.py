def main(usecache = True):
    print("importing...")
    from get_dataset import GetDataset
    from play_dataset_audio import PlayDatasetAudio
    from preprocess_imu_waveform import PreprocessIMUWaveform
    import torch
    import numpy as np
    from clap_embed_audio import ClapEmbedAudio
    import matplotlib.pyplot as plt

    print("starting program...")
    global dataset
    if "dataset" not in globals() and usecache:
        dataset = GetDataset()
    else:
        print("cache hit: dataset")
    # PlayDatasetAudio(dataset)

    # Preprocess all of the dataset dataframes.
    print("preprocessing dataset...")
    CLAP_SAMPLE_RATE = 48000
    global processed_dataset
    if "processed_dataset" not in globals() and usecache:
        processed_dataset = [
            PreprocessIMUWaveform(
                df["a_mag"].to_numpy(), int(df.attrs["sample_rate"]), CLAP_SAMPLE_RATE
            )
            for df in dataset
        ]
    else:
        print("cache hit: processed_dataset")

    # Make embeddings.
    print("making embeddings...")
    global all_embeddings
    if "all_embeddings" not in globals() and usecache:
        all_embeddings = [ClapEmbedAudio(pd) for pd in processed_dataset]
    else:
        print("cache hit: all_embeddings")
    # Compute dot products between all embeddings
    dot_products = np.array(
        [
            [torch.dot(emb1.squeeze(), emb2.squeeze()) for emb2 in all_embeddings]
            for emb1 in all_embeddings
        ]
    )

    print("plotting...")
    str_labels = [df.attrs["surface"] for df in dataset]
    # plt.figure(figsize=(10, 8))
    # plt.imshow(dot_products, cmap="hot", aspect="auto")
    # plt.colorbar(label="Dot Product")
    # plt.xticks(ticks=range(len(str_labels)), labels=str_labels, rotation=45, ha="right")
    # plt.yticks(ticks=range(len(str_labels)), labels=str_labels)
    # plt.xlabel("Embedding Index")
    # plt.ylabel("Embedding Index")
    # plt.title("Dot Products Heatmap")
    # plt.tight_layout()
    # plt.show()

    # Plot spectrograms of two processed dataset indices
    fig, nd_axes = plt.subplots(2, 2, figsize=(10, 6))
    axes = np.array(nd_axes).flatten()
    gamma_correction = 1
    for i, idx in enumerate([0, 16, 5, 23]):
        axes[i].specgram(
            np.power(processed_dataset[idx], gamma_correction),
            Fs=CLAP_SAMPLE_RATE,
            cmap="binary",
            # NFFT=1024,
            # noverlap=512,
        )
        axes[i].set_title(f"Spectrogram - {str_labels[idx]}")
        axes[i].set_ylabel("Frequency (Hz)")
        axes[i].set_xlabel("Time (s)")
    plt.tight_layout()
    plt.show()
