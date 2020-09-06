import * as Plotly from "plotly.js";

class Heading {
    constructor(plot) {
        this._plot = plot;

        let layout = {
            type: 'barpolar',
            polar: {
                radialaxis: {
                    zeroline: false,
                    showline: false,
                    autotick: false,
                    ticks: '',
                    showticklabels: false,
                },
                angularaxis: {
                    direction: 'clockwise',
                },
            },
        };

        Plotly.newPlot(this._plot, [], layout);
    }

    onOpen() {
        if (this._channel_to_trace) {
            const traces = Array.from(this._channel_to_trace.values());
            Plotly.deleteTraces(this._plot, traces);
        }

        this._channel_to_trace = new Map();
        this._buffer = {r: [], theta: [], width: []};
        this._buffer_size = 0;

        this._thread = setInterval(() => this.onUpdate(), 100, this);
    }

    onClose() {
        clearInterval(this._thread);
    }

    onEvent(event) {
        if (event.hasHello()) {
            let hello = event.getHello();
            hello.getChannelsList().forEach(function (channel) {
                this.onChannel(channel);
            }, this);
        } else if (event.hasHeading()) {
            this.onHeading(event);
        }
    }

    onChannel(channel) {
        if (!channel.getEventexample().hasHeading()) return;

        Plotly.addTraces(this._plot, [{
            name: channel.getAnnotation(),
            type: 'barpolar',
            r: [],
            theta: [],
            width: [],
            opacity: 0.7,
        }]);

        this._channel_to_trace.set(channel.getChannel(), this._channel_to_trace.size);

        for (let coord in this._buffer) {
            this._buffer[coord].push([]);
        }
    }

    onHeading(event) {
        let heading = event.getHeading();
        let trace = this._channel_to_trace.get(event.getChannel());

        let r = 1 + 0.1 * trace;
        let north = heading.getNorth() * 180 / Math.PI;
        let width = 2 * Math.sqrt(heading.getVarNorth()) * 180 / Math.PI;

        if (width < 20) width = 20;
        else if (width > 180) {
            r = 0.5 + 0.1 * trace;
            width = 360;
        }

        this._buffer.r[trace].push(r);
        this._buffer.theta[trace].push(north);
        this._buffer.width[trace].push(width);
        ++this._buffer_size;
    }

    onUpdate() {
        if (this._buffer_size <= 0) return;

        const traces = Array.from(this._channel_to_trace.values());
        Plotly.extendTraces(this._plot, this._buffer, traces, 1);

        // clear buffer
        for (let coord in this._buffer) {
            this._buffer[coord].forEach((o, i, a) => a[i] = []);
        }
        this._buffer_size = 0;
    }
}

export { Heading };
