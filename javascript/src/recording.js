import {BinaryReader} from "google-protobuf";
import assert from "assert";

function load_recording(data) {
    let recording = {
        begin_time: null,
        end_time: null,
        duration: null,
        channels: [],
    };

    let reader = new BinaryReader(data);
    let first = true;
    while(reader.getCursor() < data.length) {
        let event = new proto.indoors.pipeline.protocol.Event();
        reader.nextWireType_ = 2; // TODO hack and BinaryConstants.WireType.DELIMITED;
        reader.readMessage(event, proto.indoors.pipeline.protocol.Event.deserializeBinaryFromReader);

        if (first) {
            assert(event.hasHello(), 'expecting hello as first message');

            recording.begin_time = event.getT();

            event.getHello().getChannelsList().forEach((channel) => {
                let mapping = {
                    annotation: channel.getAnnotation(),
                    t: [],
                };

                if (channel.getEventexample().hasClock())
                    mapping = Object.assign(mapping, {unixUtc: []});
                else if (channel.getEventexample().hasVector2())
                    mapping = Object.assign(mapping, {x: [], y: []});
                else if (channel.getEventexample().hasVector3())
                    mapping = Object.assign(mapping, {x: [], y: [], z: []});
                else if (channel.getEventexample().hasQuaternion())
                    mapping = Object.assign(mapping, {w: [], x: [], y: [], z: []});

                recording.channels[channel.getChannel()] = mapping;
            });

            first = false;
        } else if (event.hasBye()) {
            recording.end_time = event.getT();
            recording.duration = recording.end_time - recording.begin_time;
            break;
        } else {
            let mapping = recording.channels[event.getChannel()];
            mapping.t.push(event.getT());

            if (event.hasClock()) {
                mapping.unixUtc.push(event.getClock().getUnixUtc());
            } else if (event.hasVector2()) {
                mapping.x.push(event.getVector2().getX());
                mapping.y.push(event.getVector2().getY());
            } else if (event.hasVector3()) {
                mapping.x.push(event.getVector3().getX());
                mapping.y.push(event.getVector3().getY());
                mapping.z.push(event.getVector3().getZ());
            } else if (event.hasQuaternion()) {
                mapping.w.push(event.getQuaternion().getW());
                mapping.x.push(event.getQuaternion().getX());
                mapping.y.push(event.getQuaternion().getY());
                mapping.z.push(event.getQuaternion().getZ());
            }
        }
    }

    return recording;
}

function load_recording_from_url(url, callback) {
    let req = new XMLHttpRequest();
    req.open("GET", url, true);
    req.responseType = "arraybuffer";

    req.onload = () => {
        if (!req.response) {
            callback(null);
            return;
        }

        let data = new Uint8Array(req.response);
        let recording = load_recording(data);

        callback(recording);
    };

    req.send();
}

export { load_recording, load_recording_from_url }
