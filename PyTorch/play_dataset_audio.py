import sounddevice as sd
import random
import numpy as np
import matplotlib.pyplot as plt
from preprocess_imu_waveform import PreprocessIMUWaveform


# Play the sound
def PlayDatasetAudio(dataset):
    for idx, df in enumerate(random.sample(dataset, len(dataset))):
        # Get preprocessed waveform.
        a_mag = df["a_mag"].to_numpy()
        imu_sample_rate = int(df.attrs["sample_rate"])
        a_mag_upscaled = PreprocessIMUWaveform(a_mag, imu_sample_rate)
        audio_sample_rate = 16000

        # Plot waveform.
        print(f"Waveform for Surface: {df.attrs['surface']}")
        time = np.linspace(
            0,
            len(a_mag_upscaled) / audio_sample_rate,
            len(a_mag_upscaled),
        )
        plt.figure(figsize=(10, 4))
        plt.plot(time, a_mag_upscaled, label="Waveform")
        plt.title(f"Surface: {df.attrs['surface']}")
        plt.show(block=False)
        plt.pause(0.2)

        # Play imu waveform.
        sd.play(a_mag_upscaled, samplerate=audio_sample_rate)
        sd.wait()

        # Close plot.
        plt.close()
