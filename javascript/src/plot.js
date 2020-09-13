import * as Plotly from "plotly.js";

class Plot {
    constructor(plot, coords, max_points, layout) {
        this._plot = plot;
        this._coords = coords;
        this._max_points = max_points;
        Plotly.newPlot(this._plot, [], layout);
    }

    onOpen() {
        if (this._traces)
            Plotly.deleteTraces(this._plot, this._traces);

        this._traces = [];
        this._buffer = {};
        this._coords.forEach((coord) => this._buffer[coord] = []);
        this._buffer_size = 0;

        this._thread = setInterval(() => this._update(), 100, this);
    }

    onClose() {
        clearInterval(this._thread);
    }

    onChannel(channel) {
        const trace = this._addTrace({
            name: channel.getAnnotation(),
        });
        return (event) => this.onEvent(event, trace);
    }

    onEvent(event, trace) {
        if (event.hasVector2()) {
            this._push({x: event.getVector2().getX(), y: event.getVector2().getY()}, trace);
        } else if (event.hasVector3()) {
            this._push({x: event.getVector3().getX(), y: event.getVector3().getY(), z: event.getVector3().getZ()}, trace);
        }
    }

    _addTrace(definition) {
        Plotly.addTraces(this._plot, definition);

        const trace = this._traces.length;
        this._traces.push(trace);

        for (let coord in this._buffer) {
            this._buffer[coord].push([]);
        }

        return trace;
    }

    _push(data, trace) {
        this._coords.forEach((coord) => this._buffer[coord][trace].push(data[coord]));
        ++this._buffer_size;
    }

    _clear() {
        for (let coord in this._buffer) {
            this._buffer[coord].forEach((o, i, a) => a[i] = []);
        }
        this._buffer_size = 0;
    }

    _update() {
        if (this._buffer_size <= 0) return;
        Plotly.extendTraces(this._plot, this._buffer, this._traces, this._max_points);
        this._clear();
    }
}

export { Plot };
