import * as Plotly from "plotly.js";

class Heading {
    constructor(plot) {
        this._plot = plot;
        this._channel_to_trace = new Map();

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
            opacity: 0.8,
        }]);

        this._channel_to_trace.set(channel.getChannel(), this._channel_to_trace.size);
    }

    onHeading(event) {
        let heading = event.getHeading();
        let trace = this._channel_to_trace.get(event.getChannel());

        let north = heading.getNorth() * 180 / Math.PI;
        let width = 2 * Math.sqrt(heading.getVarNorth()) * 180 / Math.PI;
        Plotly.extendTraces(this._plot, {r: [[1]], theta: [[north]], width: [[width]]}, [trace], 1);
    }
}

export { Heading };
