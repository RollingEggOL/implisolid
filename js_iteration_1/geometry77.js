'use strict';

/*
Three ways:
1- Using WGeometry77  using  faster=true
2- " " " using faster=false which uses  make_geometry_core()
3- " " " using faster=false which uses  make_geometry_core_slower()
4- UsingUpdatable geometry; WGeometry77.prototype.update()
*/

function make_geometry_core( verts, faces) {

    var indices = faces;
    var vertices = verts;
    var normals = null, uvs = null;
    //vertices = verts;
    //indices = faces;

    return {
        indices: indices,
        vertices: vertices,
        normals: normals,
        uvs: uvs
    };
}

//Core contains the mesh (or changes to the mesh)
function make_geometry_core_slower( verts, faces) {

    const ENABLE_NORMALS = false;

    var vertexCount = verts.length/3;
    var facecount = faces.length/3;
    var indexCount = facecount*3;

    if(VERBOSE){
        console.log("vertexCount="+vertexCount+ "   ,  facecount=" + facecount+ "   facecount*3="+(facecount*3));
    }

    // buffers
    var indices = new ( indexCount > 65535 ? Uint32Array : Uint16Array )( indexCount );
    var vertices = new Float32Array( vertexCount * 3 );

    var normals, uvs;  // plan
    if(ENABLE_NORMALS){
        normals = new Float32Array( vertexCount * 3 );
        uvs = new Float32Array( vertexCount * 2 );
    }
    else{
        normals = null;
        uvs = null;
    }

    // offset variables
    //var vertexBufferOffset = 0;
    //var uvBufferOffset = 0;
    //var indexBufferOffset = 0;
    //var numberOfVertices = 0;

    // group variables
    //var groupStart = 0;

    // build each side of the box geometry
    //output: vertices, normals, uvs, indices

    var nans_warnings = 0;
    //console.log("vertexCount " + vertexCount)
    var SCALE = 1.
    var x,y,z,d;
    for(var i=0; i < vertexCount; i++)
    {
        for(var di = 0; di < 3; di++)
            vertices[i*3+di] = verts[i*3+di] * SCALE;
        x = verts[i*3+0];
        y = verts[i*3+1];
        z = verts[i*3+2];
        //console.log(x+" , " + y + " , " + z);

        d = Math.sqrt(x*x+y*y+z*z);
        d=d+0.;
        //if(d==0) d=1.;
        if(isNaN(d)) {
            d=1.;x=1.;y=1.;z=1.;
            console.log("Warning");
        }
        //console.log("x y z"+x+" "+y+" "+z+"   / "+ d)

        if(ENABLE_NORMALS){
            var sgn = +1;
            normals[i*3 + 0] = x/d*sgn;
            normals[i*3 + 1] = y/d*sgn;
            normals[i*3 + 2] = z/d*sgn;

            if(isNaN(x/d))
                nans_warnings ++;

            var d2 = Math.sqrt(x*x+y*y);
            uvs[i*2+0] = x/d2;
            uvs[i*2+1] = y/d2;
        }
    }

    for(var i=0; i < facecount; i++)
        for(var si=0; si<3; si++)
            indices[i*3+si] = faces[i*3+si];

    //console.log(verts.subarray(0,3*3*3));
    //console.log(indices.subarray(0,3*3*3));


    if(nans_warnings > 0)
        console.error("WARNING: NaN in vertices. "+nans_warnings/978+" out of "+vertexCount/978+ "  subtract:"+(vertexCount-nans_warnings)/978);

    return {
        indices: indices,
        vertices: vertices,
        normals: normals,
        uvs: uvs
    };
}

/** Simply creates a geometry . This is static and cannot be modified when displayed. Instantiate a new one and make a new THREE.Mesh() */
var
MyBufferGeometry77 = function ( verts, faces ) {

    THREE.BufferGeometry.call( this );
    this.type = 'MyBufferGeometry77';

    this.parameters = { };

    const faster = true;
    if(faster){

        if(1){
            var materialIndex = 0;
            this.addGroup( 0, faces.length*1-10, materialIndex ); //not sure about *3 . Why??
            //console.log("ok2");
        }
        // build geometry
        this.setIndex( new THREE.BufferAttribute( faces, 3 ) ); //new Uint32Array(faces) ??
        this.addAttribute( 'position', new THREE.BufferAttribute( verts, 3 ) );

    }else{
        var mesh_core = make_geometry_core(verts, faces);
        //var mesh_core = make_geometry_core_slower(verts, faces);

        if(1){
            var materialIndex = 0;
            this.addGroup( 0, mesh_core.indices.length, materialIndex ); //not sure about *3 . Why??
        }

        // build geometry
        this.setIndex( new THREE.BufferAttribute( mesh_core.indices, 3 ) );
        this.addAttribute( 'position', new THREE.BufferAttribute( mesh_core.vertices, 3 ) );
        if(mesh_core.normals) {
            this.addAttribute( 'normal', new THREE.BufferAttribute( mesh_core.normals, 3 ) );
            this.addAttribute( 'uv', new THREE.BufferAttribute( mesh_core.uvs, 2 ) );
        }
    }

};

MyBufferGeometry77.prototype = Object.create( THREE.BufferGeometry.prototype );
MyBufferGeometry77.prototype.constructor = MyBufferGeometry77;


// This class is not actually necessary.
function WGeometry77(verts, faces) {
    //vects, faces

    THREE.Geometry.call( this );

    this.type = 'ImplicitGeometry'; //?

    this.parameters = {
        /*width: width,
        height: height,
        depth: depth,
        widthSegments: widthSegments,
        heightSegments: heightSegments,
        depthSegments: depthSegments*/
    };


    //faces = faces.subarray(0, 300);
    this.fromBufferGeometry( new MyBufferGeometry77( verts, faces ) );
    //this.mergeVertices();

};

WGeometry77.prototype = Object.create( THREE.Geometry.prototype );
WGeometry77.prototype.constructor = WGeometry77;

/* The following code will not work, but is in progress.
WGeometry77.prototype.update = function(verts, faces) {
    var geom = this;
    //for ( var vi = 0, l = geom.vertices.length; vi < l; vi ++ ) {
    //    var time = Math.random();
    //    geom.vertices[ vi ].y += 0.01*(Math.random()-0.5) ; // 0.1* 35 * Math.sin( vi / 5 + ( time + vi ) / 7 );
    //}
    var l = Math.min(geom.vertices.length, verts.length/3);
    for ( var vi = 0; vi < l; vi ++ ) {
        //geom.vertices[ vi ].x += verts[vi*3+0];
        //geom.vertices[ vi ].y += verts[vi*3+1];
        //geom.vertices[ vi ].z += verts[vi*3+2];
        geom.vertices[ vi ].x = verts[vi*3+0];
        geom.vertices[ vi ].y = verts[vi*3+1];
        geom.vertices[ vi ].z = verts[vi*3+2];
    }
    var lv = Math.min(geom.faces.length, faces.length/3);
    for ( var fi = 0; fi < lv; fi ++ ) {
        geom.faces[ fi ].a = faces[fi*3+0];
        geom.faces[ fi ].b = faces[fi*3+1];
        geom.faces[ fi ].c = faces[fi*3+2];
    }

    geom.verticesNeedUpdate = true;
}
*/

/*
function UpdatableGeometry77(verts, faces)
{
    WGeometry77.call( this, verts, faces );
    this.type = 'UpdatableGeometry77';

}

//this.geo = new WGeometry77(verts, faces);
UpdatableGeometry77.prototype.update = function(verts, faces) {
    geom = this;
    var time = math.random();
    for ( var vi = 0, l = geom.vertices.length; vi < l; vi ++ ) {
            geom.vertices[ vi ].y += 35 * Math.sin( vi / 5 + ( time + vi ) / 7 );
        }
    geom.verticesNeedUpdate = true;
}

UpdatableGeometry77.prototype = Object.create( WGeometry77.prototype );
UpdatableGeometry77.prototype.constructor = UpdatableGeometry77;
*/
