import Plotly from 'plotly.js'
import './indoors/pipeline/protocol/event_pb'

var channels = [];
var channel_to_trace = new Map();

function createPlot() {
  var layout = {margin: {
      l: 0,
      r: 0,
      b: 0,
      t: 0
    }};
  Plotly.newPlot('gd', [], layout);
}

function connect() {
  console.log("connect");

  if (! "WebSocket" in window) {
    console.log("no websockets");
    return;
  }

  //var ws = new WebSocket("ws://localhost:8080");
  var ws = new WebSocket("ws://192.168.159.234:8080");
  ws.binaryType = "arraybuffer";

  ws.onopen = function() {
    console.log("websocket open");
    createPlot();
  };

  ws.onmessage = function (evt) {
    var data = new Uint8Array(evt.data);
    var event = proto.indoors.pipeline.protocol.Event.deserializeBinary(data);

    if (event.hasHello()) {
      var hello = event.getHello();

      hello.getChannelsList().forEach(function(channel) {
        Plotly.addTraces('gd', [{
          type: 'scatter3d',
          name: channel.getAnnotation(),
          x: [],
          y: [],
          z: [],
          mode: 'markers',
          marker: {
            size: 5,
            opacity: 0.8
          }
        }]);
        channel_to_trace.set(channels.length, channel.getChannel());
        channels.push(channel.getChannel());
      });
    } else if (event.hasVector3()) {
      var vector3 = event.getVector3();
      var trace = channel_to_trace.get(event.getChannel());
      Plotly.extendTraces('gd', {x: [[vector3.getX()]], y: [[vector3.getY()]], z: [[vector3.getZ()]]}, [trace]);
    }
  };

  ws.onclose = function() {
    console.log("websocket closed");
    setTimeout(function() { connect(); }, 1000);
  };

  ws.onerror = function(err) {
    console.error('websocket error: ', err.message);
    ws.close();
  };
}

connect();
