import './indoors/pipeline/protocol/event_pb';
import {Distributor} from './distributor';
import {Scatter3d} from './scatter3d';
import {Cube} from './cube';
import {Heading} from './heading';
import {Convergence} from './convergence';
import * as Plotly from 'plotly.js';

class LiveDemo {
    constructor(web_socket_url, walls_url, ground_truth_url) {
        this._web_socket_url = web_socket_url;
        this._walls_url = walls_url;
        this._ground_truth_url = ground_truth_url;

        this._distributor = new Distributor();
        this._walls = null;
        this._ground_truth = null;

        this._load();
    }

    _load() {
        console.log("load");

        if (this._walls_url) {
            Plotly.d3.csv(this._walls_url, (data) => {
                this._walls = data;
                this._loaded();
            });
        }

        if (this._ground_truth_url) {
            Plotly.d3.csv(this._ground_truth_url, (data) => {
                this._ground_truth = data;
                this._loaded();
            });
        }

        this._loaded();
    }

    _loaded() {
        if (this._walls_url && !this._walls) return;
        if (this._ground_truth_url && !this._ground_truth) return;

        console.log("loaded");

        this._create_plots(this._walls, this._ground_truth);

        this._connect();
    }

    _connect() {
        console.log("connect");

        if (!"WebSocket" in window) {
            console.log("no websockets");
            return;
        }

        let ws = new WebSocket(this._web_socket_url);
        ws.binaryType = "arraybuffer";

        ws.onopen = () => {
            console.log("websocket open");
            this._distributor.onOpen();
        };

        ws.onmessage = (evt) => {
            let data = new Uint8Array(evt.data);
            let event = proto.indoors.pipeline.protocol.Event.deserializeBinary(data);
            this._distributor.onEvent(event);
        };

        ws.onclose = () => {
            console.log("websocket closed");
            setTimeout(() => this._connect(), 1000);
            this._distributor.onClose();
        };

        ws.onerror = (err) => {
            console.error('websocket error: ', err.message);
            ws.close();
        };
    }

    _create_plots(walls, ground_truth) {
        this._distributor.addClient(new Scatter3d("scatter3d"));
        this._distributor.addClient(new Cube("cube"));
        //if (walls && ground_truth)
        //    distributor.addClient(new Position("position", walls, ground_truth, 2));
        //distributor.addClient(new Heading("heading"));
        this._distributor.addClient(new Convergence("convergence"));
    }
}

export { LiveDemo }
