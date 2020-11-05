#!/usr/bin/env python

import sys
import argparse
import recording
import numpy as np
import matplotlib.pyplot as plt

DPI = 300.0

# magnetometer interval statistics

pixel3_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_pixel3_1.rec')
pixel2_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_pixel2_1.rec')
s7_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_s7_1.rec')
nexus5x_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_nexus5x_1.rec')
recordings = [pixel3_1, pixel2_1, s7_1, nexus5x_1]
for r in recordings:
    mag_uncalib = r['channels'][5]['data']
    mag_uncalib_t = mag_uncalib['t'].values
    mag_uncalib_interval = mag_uncalib_t[1:] - mag_uncalib_t[:-1]

    stat = [mag_uncalib_interval.mean(), mag_uncalib_interval.std(), mag_uncalib_interval.min(),
            np.quantile(mag_uncalib_interval, 0.25), np.median(mag_uncalib_interval),
            np.quantile(mag_uncalib_interval, 0.75), mag_uncalib_interval.max()]
    stat = np.array(stat)
    print(np.around(stat * 1e3, 1))

# nexus5x magnetometer interval histogram

mag_uncalib = nexus5x_1['channels'][5]['data']
mag_uncalib_t = mag_uncalib['t'].values
mag_uncalib_interval = mag_uncalib_t[1:] - mag_uncalib_t[:-1]
plt.figure(figsize=(2000/DPI, 1200/DPI), dpi=DPI)
plt.hist(mag_uncalib_interval * 1e3, bins=100, density=True)
plt.yscale('log', nonposy='clip')
plt.gca().set_xlabel('interval [ms]')
plt.gca().set_ylabel('log density []')
plt.savefig('interval_nexus5x.png')

# s7 magnetometer scatter

ticks = s7_1['channels'][0]['data']
last_tick = ticks['t'].values[-1]
mag_uncalib_rot = mag_uncalib[mag_uncalib['t'] >= last_tick]
fig, axs = plt.subplots(2, 2, figsize=(2000/DPI, 2000/DPI), dpi=DPI, sharex='col', sharey='row')
axs[0, 0].scatter(mag_uncalib_rot['x'], mag_uncalib_rot['z'], marker='.')
axs[0, 0].set_ylabel('z [µT]')
# axs[0, 0].set_aspect('equal')
axs[0, 1].axis('off')
axs[1, 0].scatter(mag_uncalib_rot['x'], mag_uncalib_rot['y'], marker='.')
axs[1, 0].set_xlabel('x [µT]')
axs[1, 0].set_ylabel('y [µT]')
# axs[1, 0].set_aspect('equal')
axs[1, 1].scatter(mag_uncalib_rot['z'], mag_uncalib_rot['y'], marker='.')
axs[1, 1].set_xlabel('z [µT]')
# axs[1, 1].set_aspect('equal')
plt.savefig('scatter_s7.png')
