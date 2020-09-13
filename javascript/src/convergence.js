import {Plot} from './plot';
import * as THREE from "three";

class Convergence extends Plot {
    constructor(plot) {
        super(plot, ['x', 'y'], 1000, {});
        this._total_rotation_step = 0.1;
    }

    onOpen() {
        super.onOpen();

        this._last_orientation = null;
        this._total_rotation = 0;
        this._last_total_rotation = 0;
    }

    onChannel(channel) {
        if (channel.getEventexample().hasQuaternion()) {
            // TODO
            return;
        }

        if (!channel.getEventexample().hasVector3()) return;

        const trace = this._addTrace({
            name: channel.getAnnotation(),
            x: [],
            y: [],
            marker: {
                size: 10
            }
        });
        return (event) => this.onVector3(event.getVector3(), trace);
    }

    onQuaternion(tmp) {
        const quaternion = new THREE.Quaternion(tmp.getX(), tmp.getY(), tmp.getZ(), tmp.getW());

        if (this._last_orientation) {
            const angle = this._last_orientation.angleTo(quaternion);
            this._total_rotation += angle;
        }
        this._last_orientation = quaternion;
    }

    onVector3(tmp, trace) {
        const vector3 = new THREE.Vector3(tmp.getX(), tmp.getY(), tmp.getZ());

        if (this._total_rotation - this._last_total_rotation > this._total_rotation_step) {
            this._push({x: this._total_rotation, y: vector3.length()}, trace);
            this._last_total_rotation = this._total_rotation;
        }
    }
}

export { Convergence };
