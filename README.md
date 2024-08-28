# Particle filter for online calibration of three axis magnetometers in smartphones

## Abstract

Nowadays smartphones usually come with a built-in magnetometer that can be used to estimate the horizontal orientation of the phone similar to a compass. Since phones contain magnetic parts like speakers they draw their own magnetic field which will bias the measurements of the magnetometer. A calibration is necessary to estimate the bias and to subtract it from the measurement. Such a calibration could require sensor fusion with the accelerometer and gyroscope and is imple- mented by the Operating System (OS) or manufacturer. These implementations are optimized for low computational effort and require the user to perform rota- tions of the phone in multiple directions. Since there are a lot of different phones on the market the implementations will behave differently and it will not be clear when the calibration is finished.

In this thesis, we propose a particle filter to realize a different type of magnetome- ter calibration which is performed continuously and without forcing the user to perform special gestures. This comes at the cost of being computationally more expensive. However, in contrast to the OS calibration, our technique will be able to quantitatively estimate the progress of the calibration.

We will show that our calibration is superior compared to those of the OS in pedestrian navigation, localization and wayfinding scenarios most of the time. Additionally, a wide range of devices is supported.

letex inspired by https://github.com/joerg/abschlussarbeit-tuwien-physik
