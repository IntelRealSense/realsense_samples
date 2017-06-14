// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
function ObjectRecognition() {
    this.loader = new THREE.FontLoader();

    var thiz = this;
    this.loader.load( 'fonts/helvetiker_regular.typeface.json', function ( font ) {
        thiz.m_font = font;
    } );

    this.obj_group = new THREE.Group();
    this.obj_group.visible = true;
}

ObjectRecognition.prototype.get_obj_group = function() {
    return this.obj_group;
}

ObjectRecognition.prototype.draw_or_point = function(x, y, z, label) {
    var particles = 1;
    var geometry = new THREE.BufferGeometry();
    var positions = new Float32Array( particles * 3 );
    var colors = new Float32Array( particles * 3 );
    var color = new THREE.Color( 0x00ffff );;
    for ( var i = 0; i < positions.length; i += 3 ) {
        positions[ i ]     = x;
        positions[ i + 1 ] = y;
        positions[ i + 2 ] = z;
        // colors
        colors[ i ]     = color.r;
        colors[ i + 1 ] = color.g;
        colors[ i + 2 ] = color.b;
    }
    geometry.addAttribute( 'position', new THREE.BufferAttribute( positions, 3 ) );
    geometry.addAttribute( 'color', new THREE.BufferAttribute( colors, 3 ) );

    var material = new THREE.PointsMaterial( { size: 0.5, vertexColors: THREE.VertexColors } );
    points = new THREE.Points( geometry, material );
    points.visible = true;

    this.obj_group.add(points);
}

//generate label texture
ObjectRecognition.prototype.draw_or_label = function(x, y, z, label) {
    var geometry = new THREE.TextGeometry( label, {
        font: this.m_font,
        size: 0.15,
        height: 0,
        curveSegments: 1
    });

    var material = new THREE.MultiMaterial( [
        new THREE.MeshBasicMaterial( { color: Math.random() * 0xffffff, overdraw: 0.5 } ),
        new THREE.MeshBasicMaterial( { color: 0x000000, overdraw: 0.5 } )
    ] );
    var mesh = new THREE.Mesh( geometry, material );
    mesh.position.x = x + 0.1;
    mesh.position.y = y;
    mesh.position.z = z;
    mesh.visible = true;

    mesh.rotation.x = Math.PI * 1.5;
    mesh.rotation.y = Math.PI * 0;
    mesh.rotation.z = Math.PI * 0;

    this.obj_group.add(mesh);

    this.draw_or_point(x, y, z, label);
}

//clear all the objects on mapveiw
ObjectRecognition.prototype.reset = function() {
    // console.log("obj group size is:", this.obj_group.children.length);
    var obj, i;
    for ( i = this.obj_group.children.length - 1; i >= 0 ; i -- ) {
        obj = this.obj_group.children[ i ];
        this.obj_group.remove(obj);
    }
}
