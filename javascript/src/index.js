import './indoors/pipeline/protocol/event_pb'
import './scatter'

function connect() {
    console.log("connect");

    if (!"WebSocket" in window) {
        console.log("no websockets");
        return;
    }

    var scatter = null;
    var cube = null;

    //var ws = new WebSocket("ws://localhost:8080");
    var ws = new WebSocket("ws://192.168.159.234:8080");
    ws.binaryType = "arraybuffer";

    ws.onopen = function () {
        console.log("websocket open");

        scatter = new Scatter("scatter");
        cube = new Cube("cube");
    };

    ws.onmessage = function (evt) {
        var data = new Uint8Array(evt.data);
        var event = proto.indoors.pipeline.protocol.Event.deserializeBinary(data);

        scatter.onEvent(event);
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
