import Plotly from "plotly.js";

class Position {
    constructor(plot, walls, ground_truth, floor) {
        this._plot = plot;
        this._ground_truth = ground_truth;

        let trace1 = {
            name: "ground truth path",
            x: [],
            y: [],
        };
        let trace2 = {
            name: "ground truth position",
            x: [],
            y: [],
            type: 'scatter',
            mode: 'markers',
            marker: {
                size: 10,
                opacity: 0.8,
            },
        };
        let data = [trace1, trace2];

        let layout = {
            shapes: [],
            xaxis: {
                zeroline: false,
            },
            yaxis: {
                zeroline: false,
                autorange: 'reversed',
                scaleanchor: 'x',
                scaleratio: 1,
            },
            height: 800,
        };

        ground_truth.forEach(function (point) {
            trace1.x.push(point.x);
            trace1.y.push(point.y);
        });

        walls.forEach(function (wall) {
            if (wall.floor != floor) return;

            layout.shapes.push({
                type: 'line',
                x0: wall.x0,
                y0: wall.y0,
                x1: wall.x1,
                y1: wall.y1,
                line: {
                    color: 'rgb(100, 100, 100)',
                    width: 2
                }
            });
        });

        Plotly.newPlot(this._plot, data, layout);
    }

    onOpen() {
        let time = 0;
        this._thread = setInterval(() => {
            this.onUpdate(time);
            time += 1;
        }, 1000);
    }

    onClose() {
        clearInterval(this._thread);
    }

    onChannel(channel) {
        // TODO
        return null;
    }

    onUpdate(time) {
        for (let i = 0; i < this._ground_truth.length - 1; ++i) {
            if ((time >= this._ground_truth[i].time) && (time < this._ground_truth[i + 1].time)) {
                const a = (time - this._ground_truth[i].time) / (this._ground_truth[i + 1].time - this._ground_truth[i].time);
                const x = a * this._ground_truth[i + 1].x + (1 - a) * this._ground_truth[i].x;
                const y = a * this._ground_truth[i + 1].y + (1 - a) * this._ground_truth[i].y;
                Plotly.extendTraces(this._plot, {x: [[x]], y: [[y]]}, [1], 1);
                return;
            }
        }
    }
}

export { Position };
