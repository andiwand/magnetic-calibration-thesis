import Plotly from "plotly.js";

export class Scatter {
    constructor(plot) {
        this._plot = plot;
        this._channel_to_trace = new Map();
        this._buffer = {x: [], y: [], z: []};

        let layout = {
            margin: {
                l: 0,
                r: 0,
                b: 0,
                t: 0
            }
        };
        Plotly.newPlot(this._plot, [], layout);

        setInterval(() => this.onUpdate(), 100, this);
    }

    _clearBuffer() {
        for (let coord in this._buffer) {
            this._buffer[coord].forEach((o, i, a) => a[i] = []);
        }
    }

    onEvent(event) {
        if (event.hasHello()) {
            var hello = event.getHello();
            hello.getChannelsList().forEach(function (channel) {
                this.onChannel(channel);
            }, this);
        } else if (event.hasVector3()) {
            this.onVector3(event);
        }
    }

    onChannel(channel) {
        Plotly.addTraces(this._plot, [{
            type: 'scatter3d',
            name: channel.getAnnotation(),
            x: [],
            y: [],
            z: [],
            mode: 'markers',
            marker: {
                size: 3
            }
        }]);
        this._channel_to_trace.set(this._channel_to_trace.size, channel.getChannel());

        for (let coord in this._buffer) {
            this._buffer[coord].push([]);
        }
    }

    onVector3(event) {
        let vector3 = event.getVector3();
        let trace = this._channel_to_trace.get(event.getChannel());
        this._buffer.x[trace].push(vector3.getX());
        this._buffer.y[trace].push(vector3.getY());
        this._buffer.z[trace].push(vector3.getZ());
    }

    onUpdate() {
        const traces = Array.from(this._channel_to_trace.values());
        Plotly.extendTraces(this._plot, this._buffer, traces, 1000);
        this._clearBuffer();
    }
}
