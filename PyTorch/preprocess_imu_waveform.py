import numpy as np
from scipy.signal import resample_poly


def PreprocessIMUWaveform(wav: np.narray, old_sample_rate: int, new_sample_rate: int):
    assert wav.ndim == 1

    # Subtract mean from waveform.
    wav_shifted = wav - np.mean(wav)

    # Add padding to reduce resample_poly edge effects.
    padding_size = len(wav_shifted) // 10
    wav_shifted_padded = np.pad(
        wav_shifted, (padding_size, padding_size), mode="mean"
    )

    # Use resample_poly for upsampling with anti-aliasing.
    wav_upsampled_padded: np.ndarray = resample_poly(
        wav_shifted_padded, new_sample_rate, old_sample_rate
    )

    # Remove padding.
    padding_size_new_sample_rate = int(padding_size * new_sample_rate / old_sample_rate)
    wav_upsampled = wav_upsampled_padded[
        padding_size_new_sample_rate:-padding_size_new_sample_rate
    ]

    # Normalize.
    wav_upsampled_normalized = wav_upsampled / max(abs(wav_upsampled))

    return wav_upsampled_normalized
