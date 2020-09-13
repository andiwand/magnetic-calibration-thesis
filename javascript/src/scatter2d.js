import {Plot} from './plot';

class Scatter2d extends Plot {
    constructor(plot) {
        super(plot, ['x', 'y'], 1000, {
            xaxis: {range: [-1, 1]},
            yaxis: {range: [-1, 1]},
        });
    }

    onChannel(channel) {
        if (!channel.getEventexample().hasVector2()) return;

        const trace = this._addTrace({
            name: channel.getAnnotation(),
            x: [],
            y: [],
            marker: {
                size: 10
            }
        });
        return (event) => this.onEvent(event, trace);
    }
}

export { Scatter2d };
