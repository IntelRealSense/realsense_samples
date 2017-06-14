// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

/// <reference path="../../typings/index.d.ts" />

const DEG2RAD = THREE.Math.DEG2RAD;

class MapViewer {

	init(canvas) {
		var scene = new THREE.Scene();
		var camera = new THREE.PerspectiveCamera(20, canvas.width / canvas.height, 0.1, 1000);
		camera.position.z = 0.5;
		camera.position.y = 16;

		var controls = new THREE.TrackballControls(camera, canvas);
		// controls.rotateSpeed = 1.0;
		// controls.zoomSpeed = 1.2;
		// controls.panSpeed = 0.8;

		// controls.noZoom = false;
		// controls.noPan = false;

		// controls.staticMoving = true;
		controls.dynamicDampingFactor = 0.3;

		controls.keys = [65, 83, 68];

		const GRID_SIZE = 30; // in meters
		var grid2 = new THREE.GridHelper(GRID_SIZE, GRID_SIZE * 20, 0xaa44aa, 0xB8ABA3);
		grid2.material.transparent = true;
		grid2.material.opacity = 0.2;

		grid2.translateY(-0.97);
		scene.add(grid2);

		var grid = new THREE.GridHelper(GRID_SIZE, GRID_SIZE * 2, 0xFDD835, 0xC46041);
		grid.material.transparent = true;
		grid.material.opacity = 0.5;
		grid.translateY(-0.96);
		scene.add(grid);

		// var boxg = new THREE.BoxGeometry(1,1,1);
		// var box = new THREE.Mesh(boxg, new THREE.MeshNormalMaterial());
		// box.translateX(0.5);
		// box.translateZ(0.5);

		// // box.position.x = 1;
		// // box.position.z = 1;
		// scene.add(box);

		var renderer = new THREE.WebGLRenderer({
			canvas: canvas,
			antialias: true
		});
		renderer.setSize(canvas.width, canvas.height);
		renderer.setPixelRatio(window.devicePixelRatio);

		this.scene = scene;
		this.camera = camera;
		this._renderer = renderer;
		this.controls = controls;

		this.render();
	}


	render() {
		this.controls.update();
		requestAnimationFrame(() => this.render());
		this._renderer.render(this.scene, this.camera);
	}
}

function getTranslation(pose) {
	return [pose[3], pose[7], pose[11]];
}


class PoseTrajectory {

}

var mapview = new MapViewer();
mapview.init(document.querySelector('canvas'));

var trajectory;
var line_verts;
function make_line() {
	var g = new THREE.BufferGeometry();
	line_verts = new Float32Array(3000 * 3);
	var pos = new THREE.BufferAttribute(line_verts, 3);
	pos.dynamic = true;
	g.addAttribute('position', pos);
	var material = new THREE.LineBasicMaterial( { linewidth: 2, color: 0xff3388 } );
	var line = new THREE.Line(g, material);
	//line.scale.set(100,100,100);
	mapview.scene.add(line);
	line.visible = false;
	trajectory = line;
}

var virtualCam;
function make_head() {
	const cam = new THREE.PerspectiveCamera(55, 4.0 / 3.0, 0.1, 0.3);

	const camHelper = new THREE.CameraHelper(cam);
	camHelper.geometry.scale(1, -1, -1);
	// camHelper.geometry.rotateY(180 * DEG2RAD);
	// var geometry = new THREE.CylinderGeometry( 0.0, 1.0, 1, 4, 1 , true, 45 * DEG2RAD);
	// var material = new THREE.MeshBasicMaterial( { color: 0xff00ff, wireframe: true } );
	// cube = new THREE.Mesh( geometry, material );
	// cube.visible = false;
	let axis = new THREE.AxisHelper( 2 );
	axis.translateY(-0.96);
	axis.geometry.scale(1,-1,-1);
	mapview.scene.add(axis);
	mapview.scene.add(camHelper);
	virtualCam = cam;
}

var object_recognition;
function init_object_recognition() {
    object_recognition = new ObjectRecognition();
    mapview.scene.add(object_recognition.get_obj_group());
}

make_head();
make_line();
init_object_recognition();

function drawORObjects(poses, label) {
    object_recognition.draw_or_label(poses[0], -poses[1], -poses[2], label);

}

function drawPersonData(pose) {
    object_recognition.draw_or_label(pose[0], -pose[1], -pose[2], "Person");
}

function reset_metadata() {
    console.log("reset_metadata");
    object_recognition.reset();
}

function drawPoses(poses) {
	var g = trajectory.geometry;

	var i = 0;

	poses.forEach(p => {
		var t = getTranslation(p.pose);
		line_verts[i++] = t[0];
		line_verts[i++] = -t[1];
		line_verts[i++] = -t[2];
	});

	// make pose matrix square
	var p = [];
	p = p.concat(poses[poses.length - 1].pose);
	p = p.concat(0, 0, 0, 1);
	var worldMat = new THREE.Matrix4();

	let fixHandMat = new THREE.Matrix4();
	fixHandMat.makeScale(1,-1,-1);
	worldMat.set.apply(worldMat, p);
	worldMat.premultiply(fixHandMat);

	// worldMat.scale(new THREE.Vector3(1,-1,-1));

	virtualCam.matrixWorld.copy(worldMat);
	virtualCam.matrixWorld.needsUpdate = true;

	// g.verticesNeedUpdate = true;
	g.attributes.position.needsUpdate = true;
	g.setDrawRange(0, poses.length-1);

	trajectory.visible = true;

	// var bounding = g.computeBoundingBox();
	// mapview.render();
}
