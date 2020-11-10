#!/usr/bin/env python

import recording
import numpy as np
import matplotlib.pyplot as plt

DPI = 200.0

# magnetometer interval statistics

def interval_statistics(rec):
    mag_uncalib = rec['channels'][5]['data']
    mag_uncalib_t = mag_uncalib['t'].values
    mag_uncalib_interval = mag_uncalib_t[1:] - mag_uncalib_t[:-1]

    stat = [mag_uncalib_interval.mean(), mag_uncalib_interval.std(), mag_uncalib_interval.min(),
            np.quantile(mag_uncalib_interval, 0.25), np.median(mag_uncalib_interval),
            np.quantile(mag_uncalib_interval, 0.75), mag_uncalib_interval.max()]
    stat = np.array(stat)
    print(np.around(stat * 1e3, 1))

pixel3_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_pixel3_1.rec')
pixel2_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_pixel2_1.rec')
s7_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_s7_1.rec')
nexus5x_1 = recording.load('/home/andreas/workspace/magnetic-calibration/data/selfmade/2020-10-30_nexus5x_1.rec')

recordings = [pixel3_1, pixel2_1, s7_1, nexus5x_1]

for r in recordings:
    interval_statistics(r)

# nexus5x magnetometer interval histogram

mag_uncalib = nexus5x_1['channels'][5]['data']
mag_uncalib_t = mag_uncalib['t'].values
mag_uncalib_interval = mag_uncalib_t[1:] - mag_uncalib_t[:-1]
plt.figure(figsize=(2000 / DPI, 1200 / DPI), dpi=DPI)
plt.hist(mag_uncalib_interval * 1e3, bins=100, density=True)
plt.yscale('log', nonposy='clip')
plt.gca().set_xlabel('interval [ms]')
plt.gca().set_ylabel('log density []')
plt.savefig('interval_nexus5x.png')


# s7 magnetometer scatter

def scatter(rec, output):
    ticks = rec['channels'][0]['data']
    mag_uncalib = rec['channels'][5]['data']

    last_tick = ticks['t'].values[-1]
    mag_uncalib_rot = mag_uncalib[mag_uncalib['t'] >= last_tick]

    fig, axs = plt.subplots(2, 2, figsize=(2000 / DPI, 2000 / DPI), dpi=DPI, sharex='col', sharey='row')
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
    plt.savefig(output)


scatter(s7_1, 'scatter_s7.png')


# replays

# from https://jekel.me/2015/Least-Squares-Sphere-Fit/
def sphere_fit(spX, spY, spZ):
    #   Assemble the A matrix
    spX = np.array(spX)
    spY = np.array(spY)
    spZ = np.array(spZ)
    A = np.zeros((len(spX), 4))
    A[:, 0] = spX * 2
    A[:, 1] = spY * 2
    A[:, 2] = spZ * 2
    A[:, 3] = 1

    #   Assemble the f matrix
    f = np.zeros((len(spX), 1))
    f[:, 0] = (spX * spX) + (spY * spY) + (spZ * spZ)
    C, residules, rank, singval = np.linalg.lstsq(A, f)

    #   solve for the radius
    t = (C[0] * C[0]) + (C[1] * C[1]) + (C[2] * C[2]) + C[3]
    radius = np.sqrt(t)

    return radius, C[0], C[1], C[2]


def evaluate_convergence(rep):
    ticks = rep['channels'][0]['data']['t'].values
    rotation = rep['channels'][1]['data']['t'].values
    mag = rep['channels'][2]['data']
    sys = rep['channels'][3]['data']
    pf = rep['channels'][4]['data']
    pf_var = rep['channels'][5]['data']

    # dirty fix for pixel2
    # ticks = np.array(list(ticks) + [30])

    for tick in ticks:
        print(tick)
        print('sys', sys[sys['t'] >= tick].iloc[0].values[1:])
        print('pf', pf[pf['t'] >= tick].iloc[0].values[1:])
        print('pf_err', pf_var[pf_var['t'] >= tick].iloc[0].values[1:] ** 0.5)

    print(sys['t'].iloc[-1])
    print('sys', sys.iloc[-1].values[1:])
    print('pf', pf.iloc[-1].values[1:])
    print('pf_err', pf_var.iloc[-1].values[1:] ** 0.5)

    stationary_mag = mag[mag['t'] > ticks[-1] + 0.5]
    print('ls', sphere_fit(stationary_mag['x'], stationary_mag['y'], stationary_mag['z']))


def plot_convergence(rep, output):
    ticks = rep['channels'][0]['data']['t'].values
    rotation = rep['channels'][1]['data']
    mag = rep['channels'][2]['data']
    sys = rep['channels'][3]['data']
    pf = rep['channels'][4]['data']
    pf_var = rep['channels'][5]['data']

    mask = pf['t'].values == pf['t'].values# (pf['t'].values < ticks[-1])
    mask[1::2] = False
    mask[::2][1::2] = False
    mask[::2][::2][1::2] = False
    mask[::2][::2][::2][1::2] = False

    #t = rotation.iloc[1:]['t'].values[mask]
    t = pf['t'].values[mask]
    x = pf['x'].values[mask]
    x_err = np.sqrt(pf_var['x'].values)[mask]
    y = pf['y'].values[mask]
    y_err = np.sqrt(pf_var['y'].values)[mask]
    z = pf['z'].values[mask]
    z_err = np.sqrt(pf_var['z'].values)[mask]

    fig, axs = plt.subplots(3, figsize=(2000 / DPI, 1300 / DPI), dpi=DPI, sharex='col', sharey='row')
    axs[0].errorbar(t, x, yerr=x_err, fmt='.', capsize=2, elinewidth=1, markeredgewidth=1)
    axs[0].set_ylabel('x [µT]')
    axs[1].errorbar(t, y, yerr=y_err, fmt='.', capsize=2, elinewidth=1, markeredgewidth=1)
    axs[1].set_ylabel('y [µT]')
    axs[2].errorbar(t, z, yerr=z_err, fmt='.', capsize=2, elinewidth=1, markeredgewidth=1)
    axs[2].set_xlabel('time [s]')
    axs[2].set_ylabel('z [µT]')

    for ax in axs:
        for tick in ticks:
            ax.axvline(x=tick, c='grey', linewidth=1)

    stationary_mag = mag[mag['t'] > ticks[-1] + 0.5]
    fit = sphere_fit(stationary_mag['x'], stationary_mag['y'], stationary_mag['z'])
    axs[0].axhline(y=fit[1], c='r', linewidth=1)
    axs[1].axhline(y=fit[2], c='r', linewidth=1)
    axs[2].axhline(y=fit[3], c='r', linewidth=1)

    #axs[0].axhline(y=sys['x'].iloc[-1], c='g', linewidth=1)
    #axs[1].axhline(y=sys['y'].iloc[-1], c='g', linewidth=1)
    #axs[2].axhline(y=sys['z'].iloc[-1], c='g', linewidth=1)

    plt.savefig(output)


pixel3_1_rep = recording.load(
    '/home/andreas/workspace/magnetic-calibration/core/cmake-build-debug/cli/2020-10-30_pixel3_1.rep')
pixel2_1_rep = recording.load(
    '/home/andreas/workspace/magnetic-calibration/core/cmake-build-debug/cli/2020-10-30_pixel2_1.rep')
s7_1_rep = recording.load(
    '/home/andreas/workspace/magnetic-calibration/core/cmake-build-debug/cli/2020-10-30_s7_1.rep')
nexus5x_1_rep = recording.load(
    '/home/andreas/workspace/magnetic-calibration/core/cmake-build-debug/cli/2020-10-30_nexus5x_1.rep')

evaluate_convergence(pixel3_1_rep)
plot_convergence(pixel3_1_rep, 'convergence_pixel3.png')
plot_convergence(pixel2_1_rep, 'convergence_pixel2.png')
plot_convergence(s7_1_rep, 'convergence_s7.png')
plot_convergence(nexus5x_1_rep, 'convergence_nexus5x.png')
