// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
class OccupancyMapTile {
    constructor (width, height, left, bottom) {
        this.data = new Uint8Array(width * height);
        this.width = width;
        this.height = height;
        this.left = left;
        this.bottom = bottom;
    }

    indexOf(x, y) {
        const i = Math.floor(x - this.left);
        const j = Math.floor(y - this.bottom);
        if (i < 0 || i >= this.width || j < 0 || j >= this.height) {
            return -1;
        } else {
            return j*this.width + i;
        }
    }

    set(x, y, val) {
        this.data[this.indexOf(x,y)] = val ^ 0x80;
    }

    get(x, y) {
        return this.data[this.indexOf(x,y)] ^ 0x80;
    }
}

class OccupancyMap {
    constructor(widthInTiles, heightInTiles, tileSize=512) {
        this.tileWidth = tileSize | 0;
        this.tileHeight = tileSize | 0;
        this.widthInTiles = widthInTiles;
        this.heightInTiles = heightInTiles;

        this.width = widthInTiles * this.tileWidth;
        this.height = heightInTiles * this.tileHeight;
        this.left = -(this.width)/2;
        this.bottom = -(this.height)/2;

        this.tiles = [];
        this._numOccupiedTiles = 0;
        this._modifiedTiles = new Set();
    }

    reset() {
        this.tiles = [];
        this._numOccupiedTiles = 0;
        this._modifiedTiles.clear();
    }

    getTile(x, y) {
        return this.tiles[this.indexOfTile(x, y)];
    }

    getOccupied() {
        return this.tiles.filter((k, i, arr) => arr.propertyIsEnumerable(i));
    }

    get hasModifiedTiles() {
        return !!this._modifiedTiles.size;
    }

    setModifiedTile(tile) {
        this._modifiedTiles.add(tile);
    }

    /** @returns {Array.<OccupancyMapTile>} */
    getModifiedTiles(clear = true) {
        let mod = Array.from(this._modifiedTiles);
        if (clear) this._modifiedTiles.clear();
        return mod;
    }

    indexOfTile(x, y) {
        let i = Math.floor((x - this.left)/this.tileWidth);
        let j = Math.floor((y - this.bottom)/this.tileHeight);

        if (i < 0 || i >= this.widthInTiles || j < 0 || j >= this.heightInTiles) {
            return -1;
        } else {
            return j*this.widthInTiles + i;
        }
    }

    get(x, y) {
        let tile = this.getTile(x, y);
        return tile ? tile.get(x, y) : 0x80;
    }

    set(x, y, val) {
        let index = this.indexOfTile(x,y);
        if (index < 0) throw `position (${[x,y]}) out of bounds`;

        let tile = this.tiles[index];
        if (tile === undefined) {
            let ti = index % this.widthInTiles;
            let tj = index / this.widthInTiles | 0;
            tile = new OccupancyMapTile(this.tileWidth, this.tileHeight,
                    ti * this.tileWidth + this.left,
                    tj * this.tileHeight + this.bottom); // top
            this._numOccupiedTiles++;
            console.log("creating tile for ", tile.left, tile.bottom);
            this.tiles[index] = tile;
        }
        var oldVal = tile.get(x, y);
        tile.set(x, y, val);
        if (val !== oldVal) this.setModifiedTile(tile);
        return oldVal;
    }
}

const Shaders = {};

Shaders.VERTEX_SHADER = `
varying vec2 vUv;
void main()
{
    vUv = uv;
    vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );
    gl_Position = projectionMatrix * mvPosition;
}
`;

Shaders.FRAG_SHADER = `
uniform float time;
uniform vec2 resolution;

uniform sampler2D image;

varying vec2 vUv;

void main( void ) {
    vec2 position = -1.0 + 2.0 * vUv;
    vec4 fill = texture2D(image, vUv);

    if (fill.r < 0.2) discard;

    vec3 color = mix(vec3(1.0,1.0,0.9), vec3(0.0,0.1,0.2), 1.0-(fill.r*255.0 - 128.0)/100.0);

    gl_FragColor = vec4(color.rgb, 1.0 );
}
`;

/** @param {OccupancyMap} map */
function OccupancyMapRenderer(map, scene) {
    this.map = map;
    this.group = new THREE.Object3D();
    this.group.rotateX(-90.0 *DEG2RAD);
    this.group.translateZ(-0.975);

    scene.add(this.group);
}

OccupancyMapRenderer.prototype.setScale = function(scale_mm) {
    this.group.scale.x = scale_mm/1000.0;
    this.group.scale.y = scale_mm/1000.0;
};

/**
 * @param {Uint8Array} map
 * @return {THREE.Mesh}
 */
function makeTileMesh(map, width, height) {
    let geometry = new THREE.PlaneGeometry(1,1);
    let texture = new THREE.DataTexture(map, width, height, THREE.LuminanceFormat, THREE.UnsignedByteType);
    // texture.flipY = true;
    // to update texture with new data:
    //   texture.needsUpdate = true;

    let uniforms = {
        time:       { value: 1.0 },
        resolution: {
            type: "v2",
            value: new THREE.Vector2()
        },
        image: {
            type: "t",
            value: texture
        }
    };
//     let material = new THREE.MeshNormalMaterial( { overdraw: 0.5} );
    let material = new THREE.ShaderMaterial({
        //side: THREE.DoubleSide,
        uniforms: uniforms,
        vertexShader: Shaders.VERTEX_SHADER,
        fragmentShader: Shaders.FRAG_SHADER
    });
    // Create a mesh and insert the geometry and the material. Translate the whole mesh
    // by 1.5 on the x axis and by 4 on the z axis and add the mesh to the scene.
    return new THREE.Mesh(geometry, material);        //        this.mesh.rotateX(-90*DEG2RAD);
}

/**
 * reupload texture
 * @returns {THREE.DataTexture}
 */
function updateMeshTexture(mesh) {
    let tex = mesh.material.uniforms.image.value;
    tex.needsUpdate = true;
    return tex;
}

/** properly dispose of mesh to avoid heap leak
 * @param {THREE.Mesh} mesh - to be disposed of
 */
function disposeMeshTexture(mesh) {
    // dispose of texture
    let texture = mesh.material.uniforms.image.value;
    // dispose of material
    mesh.material.dispose();
    texture.dispose();
    mesh.geometry.dispose();
}

OccupancyMapRenderer.prototype.reset = function() {
    const g = this.group;
    let wasVisibile = this.group.visible;
    this.group.visible = false;
    while (g.children.length > 0 ) {
        let c = g.children[0];
        disposeMeshTexture(c);
        g.remove(c);
    }
    this.group.visible = wasVisibile;

    // todo: something for wireframe helper
    // for (let obj in this.group.parent.children) {
    //     if (obj instanceof THREE.WireframeHelper) this.group.parent.remove(obj);
    // }
};

OccupancyMapRenderer.prototype.update = function() {
    let tiles = this.map.getModifiedTiles();
    tiles.forEach((tile) => {
        // store reference to mesh in tile
        let mesh = tile._mesh;
        if (!mesh) {
            mesh = makeTileMesh(tile.data, tile.width, tile.height);
            mesh.geometry.translate(0.5, 0.5, 0);
            mesh.geometry.scale(tile.width, tile.height, 1.0);
            mesh.geometry.translate(tile.left, tile.bottom, 0);
            this.group.add(mesh);
            if (this.drawWireframe) {
                let wireframe = new THREE.WireframeHelper( mesh, 0x55dddd );
                // add to parent
                this.group.parent.add(wireframe);
            }
            tile._mesh = mesh;
        }
        updateMeshTexture(mesh);
    });
};

function test__ocupancyMap() {
    let omap = new OccupancyMap(32,32,16);
    let orender = new OccupancyMapRenderer(omap, mapview.scene, 16);

    function loadStatic() {
        let occ = TEST_OCCUPANCY[0];
        let numKeys = Object.keys(occ).length;
        for (let i = 0; i < numKeys;) {
            omap.set(occ[i++], occ[i++], occ[i++]);
        }
        orender.update();
    }

    setTimeout(() => loadStatic(), 1000);
}


function test__ocupancyMap2() {
    // let omap = new OccupancyMap(32,32,16);
    // let orender = new OccupancyMapRenderer(omap, mapview.scene, 16);

    function loadStatic() {
        let occ = TEST_OCCUPANCY[0];
        let numKeys = Object.keys(occ).length;
        for (let i = 0; i < numKeys;) {
            omap.set(occ[i++], occ[i++], occ[i++]);
        }
        orender.update();
    }

    setTimeout(() => loadStatic(), 1000);
}

function powerOfTwo(n) {
    return n > 0 && !(n & (n-1));
}
