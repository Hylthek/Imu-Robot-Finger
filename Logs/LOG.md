# Log

## 3/13/26

Bug found, recording takes samples at 8 times the imu configured speed, repeated samples.

CLAP embeddings can't differentiate classes fft visual analysis can't really either. I suspect that the finger tip that we 3d printed is acting as a mechanical lowpass filter. Might want to change to a needle fingertip.

People speculate that fingerprints are useful because they allow for better tactile sensation. Perhaps many soft needles would be an ideal robotic finger pad, future work.

Gathered additional data, a control with no movement, a scraping on the sharp edge of the pad on a bumpy wood, and same thing on cardboard.

Playing the sound through a speaker, the wood sound like white noise and the cardboard like brown noise.

Taking the autocorrelation of the waveforms shows a visual difference between the classes.

Taking a double integral of the acceleration waveform (I.e position) shows a visual difference between classes as well.

Fft might not be a good metric for class differentiation, but treating it as a stochastic process seems like a better idea.

Nathan's Matlab model probably classifies based on orientation of the imu and not surface chars, this can be confirmed with a feature importance plot.

## TODO

- Fix bug on RasPi
- Collect new data!
- Create new signal visualization scripts in python (pos, autocorrelation)

## 3/16/26

Even with the three channels turned into one magnitude channel, the MATLAB classifier is able to use acceleration mean to classify surfaces (with little added help from other statistical features).
Looking at the plots in python, it seems that the wall recordings seem to have larger impulses more frequently, contributing to a larger mean.
It seems that the wall did contribute to more overall vibration after all.
Normalizing variance per-waveform decreases model accuracy as expected.
Looking at the plots with variance normalized, the only difference between the plots looks like frequency of spikes, if we somehow normalized this, performance would surely drop again.
I stand by the fact that more sophisticated features like timbre of surface are hard to extract with the large, rigid finger pad.

After analysis, I can't find any difference between "felt" and "cardboard" classes, even using methods described earlier. I will try these classes after a new fingertip is printed. Should switch to two easier classes now.

Comparing all classes, it looks like it will be hard to do any meaningful analysis when the data is just in one long recording and drags are very short.

## TODO

- Create needle tip for sensor.
- Create handle for sensor.
- Increase cable length of the sensor with soldering.
- Collect long continuous data with new setup.
- More analysis
  - play all six channels.
  - play FFT
  - play autocorrelation
  - play PSD
  - Integrate for position and angle.

# 3/17/26

- Bug found: imu_recorder doesn't always finish writing to local file before terminating program. "29.751297, -" line found at end of file.
- Thought: I'm thinking of adding a class for "nothing" which will have a larger cost for getting wrong. This will essentially automatically segment our data. This will require a special cost matrix because the normal classes will contain "nothing" as well.
  - I've figured what the cost matrix should be. For the "nothing" class, false negatives should cost more than normal and false positives should cost more than normal. This will have the effect of making sure most of dragging data is considered a surface class, with actual nothing sometimes accidentally seen as a surface class.
  - I've also confirmed that this approach can actually work, should implement in PyTorch.

## TODO

- Fix bug with last line of imu recording.
- Print new board chassis + tip.
- Solder longer wires to board.
- Collect lengthy continuous data for analysis.
