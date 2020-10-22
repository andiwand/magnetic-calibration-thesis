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
            return (event) => this.onQuaternion(event.getQuaternion());
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
            const delta = quaternion.clone().multiply(this._last_orientation.clone().conjugate());
            const w = Math.min(1, Math.max(-1, delta.w));
            const angle = 2 * Math.acos(w);
            this._total_rotation += Math.abs(angle);
        }
        this._last_orientation = quaternion;
    }

    onVector3(tmp, trace) {
        const vector3 = new THREE.Vector3(tmp.getX(), tmp.getY(), tmp.getZ());

        this._push({x: this._total_rotation, y: vector3.length()}, trace);
    }

    _update() {
        if (this._total_rotation - this._last_total_rotation > this._total_rotation_step) {
            this._last_total_rotation = this._total_rotation;
            super._update();
        }
    }
}

export { Convergence };
