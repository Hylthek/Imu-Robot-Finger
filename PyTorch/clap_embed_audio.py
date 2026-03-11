import torch
from transformers import ClapProcessor, ClapModel
import numpy as np

MODEL_ID = "laion/clap-htsat-unfused"
device = "cuda" if torch.cuda.is_available() else "cpu"
processor = None
model = None


def LoadModel():
    global processor
    global model
    processor = ClapProcessor.from_pretrained(MODEL_ID, local_files_only=True)
    model = ClapModel.from_pretrained(MODEL_ID, local_files_only=True).to(device).eval()


@torch.inference_mode()
def clap_embed_audio(audio: np.ndarray, sampling_rate: int) -> torch.Tensor:
    """
    audio: 1D mono waveform (np.ndarray) of floats recommended in [-1, 1]
    sampling_rate: e.g. 16000
    returns: torch.Tensor shape (D,) on CPU
    """

    if audio.ndim != 1:
        raise ValueError("audio must be a 1D mono waveform array")

    LoadModel()

    audio = audio.astype(np.float32)

    inputs = processor(audio=audio, sampling_rate=sampling_rate, return_tensors="pt")
    inputs = {k: v.to(device) for k, v in inputs.items()}

    feats = model.get_audio_features(**inputs).pooler_output  # (1, D)

    feats = torch.nn.functional.normalize(feats, p=2, dim=-1)
    return feats[0].detach().cpu()
