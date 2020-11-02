import './indoors/pipeline/protocol/event_pb';
import {load_recording_from_url} from './recording';
import * as Plotly from "plotly.js";

function interval(t) {
    let result = [];
    for (let i = 1; i < t.length; ++i) {
        result.push(t[i] - t[i - 1]);
    }
    return result;
}

function plot_histogram(plot, x) {
    let trace = {
        x: x,
        type: 'histogram',
    };

    Plotly.newPlot(plot, [trace]);
}

function plot_scatter3d(plot, x, y, z) {
    let trace = {
        x: x,
        y: y,
        z: z,
        type: 'scatter3d',
        mode: 'markers',
        marker: {
            size: 3
        },
    };

    Plotly.newPlot(plot, [trace]);
}

load_recording_from_url('data/2020-10-30_nexus5x_1.rec', (recording) => {
    let ticks = recording.channels[0];
    let data = recording.channels[5];

    let i = 0;
    for (; i < data.t.length; ++i) {
        if (data.t[i] > ticks.t[5]) break;
    }

    plot_scatter3d('sphere', data.x.slice(i), data.y.slice(i), data.z.slice(i));
    plot_histogram('hist', interval(data.t));
});

// TODO magnetometer interval histogram
