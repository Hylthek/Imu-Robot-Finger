import time
import pandas as pd
import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
import sounddevice as sd
import scipy
import glob


def dist(x, y, z):
    return np.sqrt(x**2 + y**2 + z**2)


def plot_csv_autocorrelation():
    plt.close("all")
    glob_foo = glob.glob(
        r"C:\Users\hylth\Documents\Pico\ImuRobotFinger\PyTorch\csv/*.csv"
    )

    dataframes = [pd.read_csv(file) for file in glob_foo]

    data_amag_numpy = [
        dist(
            dataframe.iloc[:, 1].to_numpy(),
            dataframe.iloc[:, 2].to_numpy(),
            dataframe.iloc[:, 3].to_numpy(),
        )
        for dataframe in dataframes
    ]

    for data, plt_idx in zip(data_amag_numpy, range(1, 100)):
        print("calculating autocorrelation...")
        autocorrelation = GetAutocorrelationNumPy(data)
        print("calculating autocorrelation done")

        plt.subplot(3, 1, plt_idx)
        plt.plot(autocorrelation)

    plt.show()


def plot_csv_detrended_position():
    plt.close("all")
    glob_foo = glob.glob(
        r"C:\Users\hylth\Documents\Pico\ImuRobotFinger\PyTorch\csv/*.csv"
    )

    dataframes = [pd.read_csv(file) for file in glob_foo]

    data_amag_numpy = [
        dist(
            dataframe.iloc[:, 1].to_numpy(),
            dataframe.iloc[:, 2].to_numpy(),
            dataframe.iloc[:, 3].to_numpy(),
        )
        for dataframe in dataframes
    ]

    for data, plt_idx in zip(data_amag_numpy, range(1, 100)):
        detrended_velocity = np.cumsum(data - np.mean(data))
        detrended_position = np.cumsum(detrended_velocity - np.mean(detrended_velocity))

        plt.subplot(3, 1, plt_idx)
        plt.plot(detrended_position)

    plt.show()


def play_csv():
    glob_foo = glob.glob(
        r"C:\Users\hylth\Documents\Pico\ImuRobotFinger\PyTorch\csv\*.csv"
    )

    dataframes = [pd.read_csv(file) for file in glob_foo]

    for idx, df in enumerate(dataframes):
        sample_rate = int(1 / df.iloc[:, 0].diff().median())
        print(sample_rate)

        data_float32 = df.iloc[:, 1].to_numpy().astype(np.float32)

        # Cut off first and last n seconds
        n = 5
        data_trucated = data_float32[int(sample_rate * n) : -int(sample_rate * n)]

        # Play a random 1 second segment
        duration = 1
        duration_in_samples = int(sample_rate * duration)
        start_pos_in_samples = np.random.randint(
            0, len(data_trucated) - duration_in_samples
        )
        data_to_play = data_trucated[start_pos_in_samples:]
        sd.play(data_to_play, 1000)
        print(f"Playing... {glob_foo[idx]}")
        time.sleep(duration)
        sd.stop()


def GetAutocorrelation(series: pd.Series):
    n = len(series)
    series = series - series.mean()
    autocorr = [series.autocorr(lag=tau) for tau in range(n)]
    return np.array(autocorr)


def GetAutocorrelationNumPy(np_array: np.ndarray):
    n = len(np_array)
    np_array = np_array - np.mean(np_array)
    autocorr = np.correlate(np_array, np_array, mode="full")[n - 1 :]
    autocorr /= autocorr[0]  # Normalize
    return autocorr


def PlotSpectrogram(
    series: pd.Series,
    sample_rate: int,
    title: str = "Spectrogram",
    gamma: float = 1.0,
    NFFT: int = 64,
):
    # Parameters for the spectrogram
    nperseg = NFFT  # Window size
    noverlap = NFFT - 1  # Overlap between windows
    step = nperseg - noverlap  # Step size for sliding window
    nfft = nperseg  # Number of FFT points

    # Prepare the time and frequency axes
    num_windows = (len(series) - noverlap) // step
    t = np.arange(num_windows) * step / sample_rate
    f = np.fft.rfftfreq(nfft, d=1 / sample_rate)

    # Compute the spectrogram manually
    spectrogram = []
    for i in range(0, len(series) - nperseg + 1, step):
        window = series[i : i + nperseg]  # Extract the window
        window = window * np.hanning(nperseg)  # Apply a Hanning window
        spectrum = np.fft.rfft(window)  # Compute the FFT
        spectrogram.append(np.abs(spectrum))  # Store the magnitude

    spectrogram = np.array(spectrogram).T  # Transpose for plotting

    spectrogram = spectrogram / np.max(spectrogram)

    # Apply gamma correction if needed
    if gamma != 1.0:
        spectrogram = spectrogram**gamma

    # Plot the spectrogram
    plt.figure(figsize=(10, 4))
    plt.pcolormesh(t, f, spectrogram, cmap="binary")
    plt.title(title)
    plt.ylabel("Frequency (Hz)")
    plt.xlabel("Time (s)")
    plt.colorbar(label="Amplitude")
    plt.tight_layout()
    plt.show()


def foo():
    plt.close("all")

    glob_foo = glob.glob(
        r"C:\Users\hylth\Documents\Pico\ImuRobotFinger\PyTorch\csv\AirVersusTable/*"
    )

    dataframes = [pd.read_csv(file) for file in glob_foo]

    for df, plt_idx in zip(dataframes, range(1, 100)):
        plt.subplot(3, 1, plt_idx)
        plt.plot(df.iloc[:, 1])

    plt.show()


def test():
    cwd = Path(__file__).parent
    data1 = pd.read_csv("")
    data2 = pd.read_csv("")
    data1.columns = ["t", "ax", "ay", "az", "gx", "gy", "gz"]
    data2.columns = ["t", "ax", "ay", "az", "gx", "gy", "gz"]

    # Remove mean from each column except 't'
    for col in ["ax", "ay", "az", "gx", "gy", "gz"]:
        data1[col] = data1[col] - data1[col].mean()
        data2[col] = data2[col] - data2[col].mean()

    # Sample rate.
    data1.attrs["sample_rate"] = 1 / data1["t"].diff().median()
    data2.attrs["sample_rate"] = 1 / data2["t"].diff().median()

    # Segment.
    data1 = data1.iloc[5600:5800].reset_index(drop=True)
    data2 = data2.iloc[3550:3800].reset_index(drop=True)

    # Add mag
    data1["a_mag"] = np.sqrt(data1["ax"] ** 2 + data1["ay"] ** 2 + data1["az"] ** 2)
    data1["g_mag"] = np.sqrt(data1["gx"] ** 2 + data1["gy"] ** 2 + data1["gz"] ** 2)
    data2["a_mag"] = np.sqrt(data2["ax"] ** 2 + data2["ay"] ** 2 + data2["az"] ** 2)
    data2["g_mag"] = np.sqrt(data2["gx"] ** 2 + data2["gy"] ** 2 + data2["gz"] ** 2)

    # Plots.
    # Play sound.
    # plt.close("all")
    # fig, axes = plt.subplots(2, 3, figsize=(15, 8))
    # plt.tight_layout()
    # for axis, col, data in zip(
    #     axes.flat,
    #     ["ax", "ay", "az", "ax", "ay", "az"],
    #     [data1, data1, data1, data2, data2, data2],
    # ):
    #     data = GetAutocorrelation(data[col])
    #     axis.clear()
    #     axis.plot(data)
    #     axis.set_title(col)
    #     plt.show(block=False)
    #     plt.pause(0.1)
    #     sd.play(data, 2000, loop=True)
    #     time.sleep(0.5)
    #     sd.stop()

    plt.close("all")

    # plt.plot(data1["a_mag"])
    # plt.plot(data2["a_mag"])

    # PlotSpectrogram(data1["a_mag"], data1.attrs["sample_rate"], "data1", gamma=0.1, NFFT=64)
    # PlotSpectrogram(data2["a_mag"], data2.attrs["sample_rate"], "data2", gamma=0.1, NFFT=64)

    data1 = data1["a_mag"] - np.mean(data1["a_mag"])
    data1 = np.cumsum(data1)
    data1 -= np.mean(data1)
    data1 = np.cumsum(data1)
    plt.plot(data1)

    data2 = data2["a_mag"] - np.mean(data2["a_mag"])
    data2 = np.cumsum(data2)
    data2 -= np.mean(data2)
    data2 = np.cumsum(data2)
    plt.plot(data2)

    plt.show()


def dumbtest():
    cwd = Path(__file__).parent
    data_table = pd.read_csv(
        "C:/Users/hylth/Documents/Pico/ImuRobotFinger/PyTorch/csv/GuanYinSurfaces/GuanYinTable/recording_2026-02-06_16-41-35.csv"
    )
    data_wall = pd.read_csv(
        "C:/Users/hylth/Documents/Pico/ImuRobotFinger/PyTorch/csv/GuanYinSurfaces/GuanYinWall/recording_2026-02-06_16-42-47.csv"
    )
    data_table.columns = ["t", "ax", "ay", "az", "gx", "gy", "gz"]
    data_wall.columns = ["t", "ax", "ay", "az", "gx", "gy", "gz"]

    # Sample rate.
    data_table.attrs["sample_rate"] = 1 / data_table["t"].diff().median()
    data_wall.attrs["sample_rate"] = 1 / data_wall["t"].diff().median()

    # Calculate magnitude of acceleration and gyroscope
    data_table["a_mag"] = np.sqrt(
        data_table["ax"] ** 2 + data_table["ay"] ** 2 + data_table["az"] ** 2
    )
    data_table["g_mag"] = np.sqrt(
        data_table["gx"] ** 2 + data_table["gy"] ** 2 + data_table["gz"] ** 2
    )
    data_wall["a_mag"] = np.sqrt(
        data_wall["ax"] ** 2 + data_wall["ay"] ** 2 + data_wall["az"] ** 2
    )
    data_wall["g_mag"] = np.sqrt(
        data_wall["gx"] ** 2 + data_wall["gy"] ** 2 + data_wall["gz"] ** 2
    )

    # Normalize variance of a_mag and g_mag
    for col in ["a_mag", "g_mag"]:
        data_table[col] = data_table[col] / data_table[col].std()
        data_wall[col] = data_wall[col] / data_wall[col].std()

    plt.plot(data_table["a_mag"])
    plt.figure()
    plt.plot(data_wall["a_mag"])
    plt.show()


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
