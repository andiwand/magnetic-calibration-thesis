import {Plot} from './plot';

class Heading extends Plot {
    constructor(plot) {
        super(plot, ['r', 'theta', 'width'], 1, {
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
        });
    }

    onChannel(channel) {
        if (!channel.getEventexample().hasHeading()) return null;

        const trace = this._addTrace({
            name: channel.getAnnotation(),
            type: 'barpolar',
            r: [],
            theta: [],
            width: [],
            opacity: 0.7,
        });
        return (event) => this.onHeading(event.getHeading(), trace);
    }

    onHeading(heading, trace) {
        let r = 1 + 0.1 * trace;
        let north = heading.getNorth() * 180 / Math.PI;
        let width = 2 * Math.sqrt(heading.getVarNorth()) * 180 / Math.PI;

        if (width < 20) width = 20;
        else if (width > 180) {
            r = 0.5 + 0.1 * trace;
            width = 360;
        }

        this._push({r: r, theta: north, width: width}, trace);
    }
}

export { Heading };
