// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
function Trajectory() {
  const VERTEX_SHADER = `
    varying vec4 vPos;

    attribute float distance;

    void main()
    {
        vPos = vec4(position.xyz, distance);
        vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );
        gl_Position = projectionMatrix * mvPosition;
    }
  `;

  const FRAG_SHADER = `
    uniform float totalDistance;
    uniform float targetDistance;

    varying vec4 vPos;

    void main( void ) {
        float dist = (vPos.w - targetDistance)/totalDistance;
        gl_FragColor = vec4(1.0, dist, 0.0, 0.8);
    }
  `;

  var geometry = new THREE.BufferGeometry();
  // keep position
  var vertices = new Float32Array(1000 * 3);
  // keep distance from start, used to draw gradient effect
  var distances = new Float32Array(1000);
  this.geometry = geometry;
  // itemSize = 3 because there are 3 values (components) per vertex
  geometry.addAttribute('position', new THREE.BufferAttribute(vertices, 3));
  geometry.addAttribute('distance', new THREE.BufferAttribute(distances, 1));
  this.position = geometry.getAttribute('position');
  this.distance = geometry.getAttribute('distance');

  this.uniforms = {
    totalDistance: { value: 0.0 },
    targetDistance: {value: 0.0 }
  };
  this._distance = 0;

  let material = new THREE.ShaderMaterial({
    // wireframe: true, wireframeLinewidth : 5,
    uniforms: this.uniforms,
    vertexShader: VERTEX_SHADER,
    fragmentShader: FRAG_SHADER
  });
  material.linewidth = 2;
  material.transparent = true;

  this.mesh = new THREE.Line(geometry, material);
  this.mesh.scale.x = 10;
  this.mesh.scale.y = 10;
  this.mesh.scale.z = 10;
  this.numVertices = 0;
}

Trajectory.prototype.reset = function() {
  this.uniforms.totalDistance.value = 0.0;
  this.uniforms.targetDistance.value = 0.0;
  this._distance = 0.0;
  this._lastPoint = null;
  this.numVertices = 0;
  this.update();
};

Trajectory.prototype.addPoint = function (x, y, z) {
  let p = new THREE.Vector3(x, y, z);
  let p_last = this._lastPoint ? this._lastPoint : p;
  this._distance += p.distanceTo(p_last);
  this._lastPoint = p;
  let index = this.numVertices++;
  this.position.setXYZ(index, x, y, z);
  this.distance.setX(index, this._distance);
  this.uniforms.totalDistance.value = this._distance;
};

/** call after addPoint */
Trajectory.prototype.update = function () {
  this.position.needsUpdate = true;
  this.distance.needsUpdate = true;
  this.geometry.setDrawRange(0, this.numVertices);
  this.mesh.visible = this.numVertices > 0;
};
