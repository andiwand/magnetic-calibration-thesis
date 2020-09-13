import './indoors/pipeline/protocol/event_pb';
import {Distributor} from './distributor';
import {Scatter3d} from './scatter3d';
import {Cube} from './cube';
import {Heading} from './heading';
import {Convergence} from './convergence';
import * as Plotly from 'plotly.js';

let distributor = new Distributor();

function createPlots(walls, ground_truth) {
    distributor.addClient(new Scatter3d("scatter3d"));
    distributor.addClient(new Cube("cube"));
    //if (walls && ground_truth)
    //    distributor.addClient(new Position("position", walls, ground_truth, 2));
    distributor.addClient(new Heading("heading"));
    distributor.addClient(new Convergence("convergence"));
}

function load(web_socket_url, walls_url, ground_truth_url) {
    console.log("load");

    let walls = null;
    let ground_truth = null;

    function loaded() {
        if (walls_url && !walls) return;
        if (ground_truth_url && !ground_truth) return;

        console.log("loaded");

        createPlots(walls, ground_truth);

        connect(web_socket_url);
    }

    if (walls_url) {
        Plotly.d3.csv(walls_url, function(data) {
            walls = data;
            loaded();
        });
    }

    if (ground_truth_url) {
        Plotly.d3.csv(ground_truth_url, function(data) {
            ground_truth = data;
            loaded();
        });
    }

    loaded();
}

function connect(url) {
    console.log("connect");

    if (!"WebSocket" in window) {
        console.log("no websockets");
        return;
    }

    let ws = new WebSocket(url);
    ws.binaryType = "arraybuffer";

    ws.onopen = function () {
        console.log("websocket open");
        distributor.onOpen();
    };

    ws.onmessage = function (evt) {
        let data = new Uint8Array(evt.data);
        let event = proto.indoors.pipeline.protocol.Event.deserializeBinary(data);
        distributor.onEvent(event);
    };

    ws.onclose = function () {
        console.log("websocket closed");
        setTimeout(function () {
            connect(url);
        }, 1000);
        distributor.onClose();
    };

    ws.onerror = function (err) {
        console.error('websocket error: ', err.message);
        ws.close();
    };
}

load("ws://localhost:8000", null, null);
//load("ws://192.168.159.234:8000", "data/1174182629_walls.csv", "data/29303_ground_truth.csv");
//load("ws://192.168.15.134:8000", "data/1174182629_walls.csv", "data/29303_ground_truth.csv");
//load("ws://10.195.37.100:8000", "data/1174182629_walls.csv", "data/29303_ground_truth.csv");
