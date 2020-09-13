import {Plot} from './plot';

class Scatter3d extends Plot {
    constructor(plot) {
        super(plot, ['x', 'y', 'z'], 1000, {
            type: 'scatter3d',
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
        });
    }

    onChannel(channel) {
        if (!channel.getEventexample().hasVector3()) return;

        const trace = this._addTrace({
            name: channel.getAnnotation(),
            type: 'scatter3d',
            x: [],
            y: [],
            z: [],
            mode: 'markers',
            marker: {
                size: 3
            },
        });
        return (event) => this.onEvent(event, trace);
    }
}

export { Scatter3d };
