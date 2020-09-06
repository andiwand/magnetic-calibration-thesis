import Plotly from "plotly.js";

class Scatter2d {
    constructor(plot) {
        this._plot = plot;

        let layout = {
            xaxis: {range: [-1, 1]},
            yaxis: {range: [-1, 1]},
        };
        Plotly.newPlot(this._plot, [], layout);
    }

    _clearBuffer() {
        for (let coord in this._buffer) {
            this._buffer[coord].forEach((o, i, a) => a[i] = []);
        }
    }

    onOpen() {
        if (this._channel_to_trace) {
            const traces = Array.from(this._channel_to_trace.values());
            Plotly.deleteTraces(this._plot, traces);
        }

        this._channel_to_trace = new Map();
        this._buffer = {x: [], y: []};

        this._thread = setInterval(() => this.onUpdate(), 100, this);
    }

    onClose() {
        clearInterval(this._thread);
    }

    onEvent(event) {
        if (event.hasHello()) {
            var hello = event.getHello();
            hello.getChannelsList().forEach(function (channel) {
                this.onChannel(channel);
            }, this);
        } else if (event.hasVector2()) {
            this.onVector2(event);
        }
    }

    onChannel(channel) {
        Plotly.addTraces(this._plot, [{
            name: channel.getAnnotation(),
            x: [],
            y: [],
            marker: {
                size: 10
            }
        }]);
        this._channel_to_trace.set(this._channel_to_trace.size, channel.getChannel());

        for (let coord in this._buffer) {
            this._buffer[coord].push([]);
        }
    }

    onVector2(event) {
        let vector2 = event.getVector2();
        let trace = this._channel_to_trace.get(event.getChannel());
        this._buffer.x[trace].push(vector2.getX());
        this._buffer.y[trace].push(vector2.getY());
    }

    onUpdate() {
        const traces = Array.from(this._channel_to_trace.values());
        Plotly.extendTraces(this._plot, this._buffer, traces, 100);
        this._clearBuffer();
    }
}

export { Scatter2d };
