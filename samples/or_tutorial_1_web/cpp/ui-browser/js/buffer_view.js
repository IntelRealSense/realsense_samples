// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

/**
 * @param {THREE.Scene} scene
 */
class BufferViewer {
    constructor() {
        this.uniforms = {};
    }

    /** @param {HTMLCanvasElement} canvas */
    init(canvas) {
        this.scene = new THREE.Scene();
        this.camera = new THREE.Camera();
        this.camera.position.z = 1;

        this.texture = new THREE.DataTexture(new Uint8Array(4), 1, 1,
            THREE.RGBAFormat, THREE.UnsignedByteType);
        this.texture.flipY = true;

        this.uniforms = {
            resolution: {type: "v2", value: new THREE.Vector2()},
            image: {type: "t", value: this.texture}
        };

        let material = new THREE.ShaderMaterial({
            uniforms: this.uniforms,
            vertexShader: BufferViewerShader.VERTEX,
            fragmentShader: BufferViewerShader.FRAGMENT_YUV
        });

        let mesh = new THREE.Mesh(
            new THREE.PlaneBufferGeometry(2,2), material);

        this.scene.add(mesh);
        this.renderer = new THREE.WebGLRenderer({
            canvas: canvas
        });
        this.renderer.setSize(canvas.width, canvas.height);
        this.uniforms.resolution.value.x = canvas.width;
        this.uniforms.resolution.value.y = canvas.height;
    }

    updateBuffer(buffer, width, height, format) {
        this.texture.image.data = buffer;
        this.texture.image.width = width;
        this.texture.image.height = height;
        if (format) this.texture.format = format;
        this.texture.needsUpdate = true;
    }

    render() {
        this.renderer.render(this.scene, this.camera);
    }
}

const BufferViewerShader = {};
BufferViewerShader.VERTEX = `
    void main() {
        gl_Position = vec4( position, 1.0 );
    }
`;

// use for color
BufferViewerShader.FRAGMENT_YUV = `
    uniform vec2 resolution;
    uniform float time;
    uniform sampler2D image;

    vec3 ycc2rgb(vec3 ycc) {
        vec3 rgb;
        float Y = ycc.r*255.0;
        float Cb = ycc.g*255.0;
        float Cr = ycc.b*255.0;

        rgb.r = Y - 179.456 + 1.402 * Cr;
        rgb.g = Y + 135.459 - .344 * Cb - .714 * Cr;
        rgb.b = Y - 226.816 + 1.772 * Cb;
        return rgb/255.0;
    }

    void main() {
        vec2 p = gl_FragCoord.xy / resolution.xy;
        vec4 color = texture2D(image, p);
        gl_FragColor=vec4(ycc2rgb(color.rgb).rgb, 1.0);
    }
`;

BufferViewerShader.FRAGMENT = `
    uniform vec2 resolution;
    uniform float time;
    uniform sampler2D image;

    void main() {
        vec2 p = gl_FragCoord.xy / resolution.xy;
        vec4 color = texture2D(image, p);
        gl_FragColor=vec4(color.rgb, 1.0);
    }
`;
