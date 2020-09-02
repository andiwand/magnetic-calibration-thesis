import Plotly from "plotly.js";

class Scatter3d {
    constructor(plot) {
        this._plot = plot;
        this._channel_to_trace = new Map();
        this._buffer = {x: [], y: [], z: []};
        this._buffer_size = 0;

        let layout = {
            margin: {
                l: 0,
                r: 0,
                b: 0,
                t: 0
            },
            legend: {
                yanchor: "top",
                y: 0.99,
                xanchor: "left",
                x: 0.01
            }
        };
        Plotly.newPlot(this._plot, [], layout);

        setInterval(() => this.onUpdate(), 100, this);
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
        if (!channel.getEventexample().hasVector3()) return;

        Plotly.addTraces(this._plot, [{
            name: channel.getAnnotation(),
            type: 'scatter3d',
            x: [],
            y: [],
            z: [],
            mode: 'markers',
            marker: {
                size: 3
            }
        }]);

        this._channel_to_trace.set(channel.getChannel(), this._channel_to_trace.size);

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
        ++this._buffer_size;
    }

    onUpdate() {
        if (this._buffer_size <= 0) return;

        const traces = Array.from(this._channel_to_trace.values());
        Plotly.extendTraces(this._plot, this._buffer, traces, 1000);

        // clear buffer
        for (let coord in this._buffer) {
            this._buffer[coord].forEach((o, i, a) => a[i] = []);
        }
        this._buffer_size = 0;
    }
}

export { Scatter3d };
