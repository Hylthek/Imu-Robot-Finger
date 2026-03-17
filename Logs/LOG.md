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
