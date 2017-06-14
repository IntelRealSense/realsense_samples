// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

class LineSegment extends THREE.BufferGeometry {
    constructor(max_points) {
        super();
        this.max_count = max_points || 500;

        /* store xyzw */
        this.verts = new Float32Array(3 * this.max_count);
        this.distances = new Float32Array(1 * this.max_count);
        this.lastPoint = new THREE.Vector3();
        this._tmpPoint = new THREE.Vector3();
            
        this.setDrawRange(0, 0);
        var buf = new THREE.BufferAttribute(this.verts, 3);
        buf.dynamic = true;
        this.addAttribute('position', buf);
        this.addAttribute('lineDistance', new THREE.BufferAttribute(this.distances, 1));
        // this.add
    }

    addPoint(x, y, z, w) {
        var next_count = this.drawRange.count;
        // no more points will fit
        if (next_count >= this.max_count) return false;
        
        this.verts[next_count*3] = x;
        this.verts[next_count*3 + 1] = y;
        this.verts[next_count*3 + 2] = z;
        this.distances[next_count] = w;
        this.drawRange.count++;
        
        this.attributes.position.needsUpdate = true
        this.attributes.lineDistance.needsUpdate = true
        
        return true;
    }
}

let TrajectoryLineShaders = {
    vertexShader: `
    uniform float amplitude;
    attribute float lineDistance;
    
    varying vec4 vPosition;

    void main() {
        vPosition = vec4(position.xyz, lineDistance);
        gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
    }
    `,

    fragmentShader: `
    uniform float opacity;
    uniform float totalDistance;
    uniform float targetDistance;
    uniform vec3 targetColor;
    uniform vec3 baseColor;

    varying vec4 vPosition;

    void main() {
        float dist = vPosition.w;
        float sel = smoothstep(totalDistance - 10.0, totalDistance-5.0, dist);
        vec3 color = mix(baseColor, targetColor, sel);
        gl_FragColor = vec4(color, 1.0);
    }
    `
};

class TrajectoryLine extends THREE.Object3D {
    constructor(options) {
        super();
        options = options || {};

        this.segment_size = options.segment_size || 2048 ;
        
        var uniforms = {
            opacity:       { value: 0.5 },
            targetColor:   { value: new THREE.Color( 0xFF3C00 ) },
            baseColor:     { value: new THREE.Color( 0xDEB0A2) },
            totalDistance: { value: 0.0 },
            targetDistance: { value: 0.0 }
        };

        this.shaderMaterial = new THREE.ShaderMaterial( {
            uniforms:       uniforms,
            linewidth: 2,
            vertexShader:   TrajectoryLineShaders.vertexShader,
            fragmentShader: TrajectoryLineShaders.fragmentShader,
            blending:       THREE.AdditiveBlending,
            depthTest:      true,
            transparent:    true
        });

        this.materialLine = this.shaderMaterial;
        this.materialLineBg = new THREE.LineBasicMaterial( { linewidth: 6, color: 0x222222, transparent: true, opacity: 0.75} );
               
        this.reset();
    }

    reset() {
        this.dist = 0;
        this.lastSegment = null;
        this._tempPoint = new THREE.Vector3();
        this.dist = 0;

        this.children.forEach((c) => {
            c.geometry.dispose();
            this.remove(c);
        });
        this.children = [];

        this.lastPoint = new THREE.Vector3();
        // this.addSegment();
    }


    addSegment() {
        let geometry = new LineSegment(this.segment_size);
        // material.color.setHSL( Math.random(), 1, 0.8 );
        let lineShadow = new THREE.Line(geometry, this.materialLineBg);
        lineShadow.translateY(-0.005);
        this.lastSegment = new THREE.Line(geometry, this.materialLine);

        this.add(this.lastSegment);
        this.add(lineShadow);
    }

    addPoint(x, y, z) {
        this._tempPoint.set(x, y, z);
        this.dist += this._tempPoint.distanceTo(this.lastPoint);
        this.materialLine.uniforms.totalDistance.value = this.dist;
        this.lastPoint.set(x, y, z);
        if (this.lastSegment === null 
            || !this.lastSegment.geometry.addPoint(x, y, z, this.dist)) {
            
            this.addSegment();
            // duplicate last point from previous segment for continuous line
            this.lastSegment.geometry.addPoint(x, y, z, this.dist);
            this.lastSegment.geometry.addPoint(x, y, z, this.dist);
            // this.lastSegment.geometry
        }
    }
}
