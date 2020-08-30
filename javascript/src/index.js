import './indoors/pipeline/protocol/event_pb';
import {Scatter2d} from './scatter2d';
import {Scatter3d} from './scatter3d';
import {Cube} from './cube';

function connect() {
    console.log("connect");

    if (!"WebSocket" in window) {
        console.log("no websockets");
        return;
    }

    var north_confidence = null;
    var scatter3d = null;
    var cube = null;

    //var ws = new WebSocket("ws://localhost:8080");
    var ws = new WebSocket("ws://192.168.159.234:8080");
    ws.binaryType = "arraybuffer";

    ws.onopen = function () {
        console.log("websocket open");

        north_confidence = new Scatter2d("north_confidence");
        scatter3d = new Scatter3d("scatter3d");
        cube = new Cube("cube");
    };

    ws.onmessage = function (evt) {
        var data = new Uint8Array(evt.data);
        var event = proto.indoors.pipeline.protocol.Event.deserializeBinary(data);

        north_confidence.onEvent(event);
        scatter3d.onEvent(event);
        cube.onEvent(event);
    };

    ws.onclose = function () {
        console.log("websocket closed");
        setTimeout(function () {
            connect();
        }, 1000);
    };

    ws.onerror = function (err) {
        console.error('websocket error: ', err.message);
        ws.close();
    };
}

connect();
