import Plotly from "plotly.js";
import * as THREE from "three";

class Cube {
    constructor(plot) {
        this._plot = plot;

        let data = [{
            type: "mesh3d",
            x: [0, 0, 1, 1, 0, 0, 1, 1],
            y: [0, 1, 1, 0, 0, 1, 1, 0],
            z: [0, 0, 0, 0, 1, 1, 1, 1],
            i: [7, 0, 0, 0, 4, 4, 6, 6, 4, 0, 3, 2],
            j: [3, 4, 1, 2, 5, 6, 5, 2, 0, 1, 6, 3],
            k: [0, 7, 2, 3, 6, 7, 1, 1, 5, 5, 7, 6],
            intensity: [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5],
            intensitymode: 'cell',
            showscale: false,
            hoverinfo: 'skip',
        }];
        let axisLayout = {
            showgrid: false,
            zeroline: false,
            showline: false,
            autotick: false,
            ticks: '',
            showticklabels: false,
            visible: false,
        };
        let layout = {
            scene: {
                xaxis: axisLayout,
                yaxis: axisLayout,
                zaxis: axisLayout,
            }
        };
        Plotly.newPlot(this._plot, data, layout);
    }

    onOpen() {}

    onClose() {}

    onChannel(channel) {
        if (channel.getEventexample().hasQuaternion())
            return (event) => this.onQuaternion(event);
        return null;
    }

    onQuaternion(event) {
        const tmp = event.getQuaternion();
        const camera = document.getElementById(this._plot)._fullLayout.scene._scene.camera;
        const distance = new THREE.Vector3().fromArray(camera.eye).length();

        const quaternion = new THREE.Quaternion(tmp.getX(), tmp.getY(), tmp.getZ(), tmp.getW());
        camera.eye = new THREE.Vector3(0, 0, distance).applyQuaternion(quaternion).toArray();
        camera.up = new THREE.Vector3(0, 1, 0).applyQuaternion(quaternion).toArray();
    }
}

export { Cube };
