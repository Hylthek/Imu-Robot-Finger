from get_dataset import GetDataset
from play_dataset_audio import PlayDatasetAudio
from preprocess_imu_waveform import PreprocessIMUWaveform
import torch
from transformers import ClapModel, ClapProcessor
import numpy as np

dataset = GetDataset()

# PlayDatasetAudio(dataset)

# Preprocess all of the dataset dataframes.
audio_samplerate = 48000
processed_dataset = list(
    map(
        lambda df: PreprocessIMUWaveform(
            df["a_mag"].to_numpy(), int(df.attrs["sample_rate"]), audio_samplerate
        ),
        dataset,
    )
)

# Play with torch CLAP
print("Loading pretrained clap model...")
MODEL_ID = "laion/clap-htsat-unfused"
processor = ClapProcessor.from_pretrained(MODEL_ID, local_files_only=True)
device = "cuda" if torch.cuda.is_available() else "cpu"
model = ClapModel.from_pretrained(MODEL_ID, local_files_only=True).to(device).eval()



@torch.inference_mode()
def clap_audio_embed(audio: np.ndarray, sampling_rate: int) -> torch.Tensor:
    """
    audio: 1D mono waveform (np.ndarray) of floats recommended in [-1, 1]
    sampling_rate: e.g. 16000
    returns: torch.Tensor shape (D,) on CPU
    """
    if audio.ndim != 1:
        raise ValueError("audio must be a 1D mono waveform array")

    audio = audio.astype(np.float32)

    inputs = processor(audio=audio, sampling_rate=sampling_rate, return_tensors="pt")
    inputs = {k: v.to(device) for k, v in inputs.items()}

    feats = model.get_audio_features(**inputs).pooler_output  # (1, D)

    feats = torch.nn.functional.normalize(feats, p=2, dim=-1)
    return feats[0].detach().cpu()


# Make embeddings.
embeddings = clap_audio_embed(processed_dataset[0], audio_samplerate)
print(embeddings.squeeze().tolist())
