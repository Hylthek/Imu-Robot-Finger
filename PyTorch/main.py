def main(nocache=False):
    # fmt: off
    print("importing...")
    import torch
    import numpy as np
    import matplotlib.pyplot as plt
    import sounddevice as sd

    from importlib import reload
    import play_dataset_audio; reload(play_dataset_audio);
    import clap_embed_audio; reload(clap_embed_audio);
    import preprocess_imu_waveform; reload(preprocess_imu_waveform);
    import get_dataset; reload(get_dataset);
    print("starting program...")
    # fmt: on

    # Get dataset.
    global dataset
    if "dataset" not in globals() or nocache:
        dataset = get_dataset.GetDataset()
        print("cache miss: dataset")
    else:
        print("cache hit: dataset")

    # Play audio.
    play_dataset_audio.PlayDatasetAudio2(dataset)

    # Preprocess all of the dataset dataframes.
    print("preprocessing dataset...")
    CLAP_SAMPLE_RATE = 48000
    global a_mag_audios
    if "a_mag_audios" not in globals() or nocache:
        a_mag_audios = [
            preprocess_imu_waveform.PreprocessIMUWaveform(
                df["a_mag"].to_numpy(), int(df.attrs["sample_rate"]), CLAP_SAMPLE_RATE
            )
            for df in dataset
        ]
        print("cache miss: a_mag_audios")
    else:
        print("cache hit: a_mag_audios")

    # Make embeddings.
    print("making embeddings...")
    global all_embeddings
    if "all_embeddings" not in globals() or nocache:
        all_embeddings = [clap_embed_audio.ClapEmbedAudio(pd) for pd in a_mag_audios]
        print("cache miss: all_embeddings")
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
    plt.close("all")
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
    gamma_correction = 0.95
    NFFT = 256
    a_mag = [df["a_mag"].to_numpy() for df in dataset]
    for i, idx in enumerate(
        [
            np.random.random_integers(0, 9),
            np.random.random_integers(0, 9),
            np.random.random_integers(15, 24),
            np.random.random_integers(15, 24),
        ]
    ):
        axes[i].specgram(
            a_mag,
            Fs=CLAP_SAMPLE_RATE,
            cmap="binary",
            NFFT=NFFT,
            noverlap=NFFT // 4,
        )
        axes[i].set_title(f"Spectrogram - {str_labels[idx]}")
        axes[i].set_ylabel("Frequency (Hz)")
        axes[i].set_xlabel("Time (s)")
    plt.tight_layout()
    plt.show()
