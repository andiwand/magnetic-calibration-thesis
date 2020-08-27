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
            intensity: [0, 0.14285714285714285, 0.2857142857142857, 0.42857142857142855, 0.5714285714285714, 0.7142857142857143, 0.8571428571428571, 1],
            colorscale: [
                [0, 'rgb(255, 0, 255)'],
                [0.5, 'rgb(0, 255, 0)'],
                [1, 'rgb(0, 0, 255)']
            ],
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

    onEvent(event) {
        if (event.hasHello()) {
            var hello = event.getHello();
            // TODO
        } else if (event.hasQuaternion()) {
            this.onQuaternion(event);
        }
    }

    onQuaternion(event) {
        let tmp = event.getQuaternion();
        let camera = document.getElementById(this._plot)._fullLayout.scene._scene.camera;
        let distance = new THREE.Vector3().fromArray(camera.eye).length();

        // TODO should be normalized already
        let quaternion = new THREE.Quaternion(tmp.getX(), tmp.getY(), tmp.getZ(), tmp.getW());
        camera.eye = new THREE.Vector3(0, 0, distance).applyQuaternion(quaternion).toArray();
        camera.up = new THREE.Vector3(0, 1, 0).applyQuaternion(quaternion).toArray();
    }
}

export { Cube };
