// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

/// <reference path="../../typings/index.d.ts" />

const DEG2RAD = THREE.Math.DEG2RAD;

class MapViewer {
	constructor(canvas) {
		this.scene = new THREE.Scene();

		this.camera = new THREE.PerspectiveCamera(20, canvas.width / canvas.height, 0.1, 1000);
		this.camera.position.z = 0.5;
		this.camera.position.y = 16;

		this.cameraControls = new THREE.OrbitControls(this.camera, canvas);
		this.cameraControls.dampingFactor = 0.3;
		this.cameraControls.keys = [65, 83, 68];

		this.renderer = new THREE.WebGLRenderer({
			canvas: canvas,
			antialias: true
		});
		this.renderer.setSize(canvas.width, canvas.height);
		this.renderer.setPixelRatio(window.devicePixelRatio);
	}

	initScene(gridsize = 30) {
		// Base grid pieces
		var grid2 = new THREE.GridHelper(gridsize, gridsize * 20, 0xaa44aa, 0xB8ABA3);
		grid2.material.transparent = true;
		grid2.material.opacity = 0.2;
		grid2.translateY(-0.97);
		this.scene.add(grid2);
		var grid = new THREE.GridHelper(gridsize, gridsize * 2, 0xFDD835, 0xC46041);
		grid.material.transparent = true;
		grid.material.opacity = 0.5;
		grid.translateY(-0.96);
		this.scene.add(grid);

		// trajectory line
		this.trajectoryLine = new TrajectoryLine();
		this.scene.add(this.trajectoryLine);

		// virtual camera
		this.virtualCam = new THREE.PerspectiveCamera(55, 4.0 / 3.0, 0.1, 0.3);
		var helper = new THREE.CameraHelper(this.virtualCam);
		helper.geometry.scale(1, -1, -1);
		var axis = new THREE.AxisHelper( 2 );
		axis.translateY(-0.96);
		axis.geometry.scale(1,-1,-1);
		this.scene.add(axis);
		this.scene.add(helper);
	}

	reset() {
		this.trajectoryLine.reset();
		this.virtualCam.matrixWorld.copy(new THREE.Matrix4());
	}

	updatePoseDisplay(pose) {
		this.trajectoryLine.addPoint(pose.pose[3], -pose.pose[7], -pose.pose[11]);

		// make pose matrix square
		var p = [];
		p = p.concat(pose.pose);
		p = p.concat(0, 0, 0, 1);
		var worldMat = new THREE.Matrix4();
		var fixHandMat = new THREE.Matrix4();
		fixHandMat.makeScale(1,-1,-1);
		worldMat.set.apply(worldMat, p);
		worldMat.premultiply(fixHandMat);

		this.virtualCam.matrixWorld.copy(worldMat);
		this.virtualCam.matrixWorld.needsUpdate = true;
	}

	render() {
		this.cameraControls.update();
		requestAnimationFrame(() => this.render());
		this.renderer.render(this.scene, this.camera);
	}
}

var mapview = new MapViewer(document.querySelector('canvas'));
mapview.initScene();
mapview.render();


// Useful for testing poses
var fake_transport = {
	make_tracking: function(x, y, z, tracking) {
		var p = {};
		p.pose = [];
		p.pose[11] = 0;
		p.pose = p.pose.fill(0);
		
		p.pose[0] = 1;
		p.pose[5] = 1;
		p.pose[10] = 1;
		p.pose[3] = x || 0;
		p.pose[7] = y || 0;
		p.pose[11] = z || 0;
		p.tracking = tracking || 3;
		return p;
	},

	step: function(x, y, z, tracking) {
		var t = fake_transport.make_tracking(x, y, z, tracking);
		transporter.onPoseUpdate(t);
	},

	test: function(steps) {
		steps = steps || 300;
		for (var i = 0; i < steps; i++) {
			fake_transport.step(Math.sin(i/steps/4)*2, 0, Math.sin(i/steps));
		}
	}
};
