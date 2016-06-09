//AlteredQualia's Marching Cubes, C++ version, based on AQ's code.

#include "boost/multi_array.hpp"
#include "boost/array.hpp"
#include <cassert>
#include <algorithm>

#include <iostream>

/**
 * Based on alteredq's version  https://github.com/mrdoob/three.js/blob/master/examples/js/MarchingCubes.js
 *
 * Port of greggman's ThreeD version of marching cubes to Three.js
 * http://webglsamples.googlecode.com/hg/blob/blob.html
 */


//typedef unsigned short int size_t;
typedef unsigned short int dim_t; //small integers for example the size of one side of the grid
typedef float REAL;
//typedef unsigned long int index_t;

//boost::array will not work becasue the size of a boost:array has to be known in compile-time (static).
typedef  boost::multi_array<REAL, 1>  array1d;
//typedef array1d::index  array_index_t;
typedef boost::array<array1d::index, 1>  array_shape_t;
//#define array1d  boost::multi_array<REAL, 1>
typedef array1d::index  index_t;

typedef struct { void call (void*) const {};} callback_t;

/*template<typename Index_Type=int>
boost::array<Index_Type, 1> make_shape_1d(Index_Type size)
{
    //Make a shape to be used in array initialisation
    //fixme: the type of size

    //ASSERT(size>=0);
    boost::array<Index_Type, 1> shape = {{ size, }};
    return shape;
}
*/

array_shape_t make_shape_1d(int size)
{
    //Make a shape to be used in array initialisation
    //fixme: the type of size

    //ASSERT(size>=0);
    array_shape_t shape = {{ size, }};
    return shape;
}


REAL lerp( REAL a, REAL b, REAL t )
{
    return a + ( b - a ) * t;
}


class MarchingCubes{
    bool enableUvs, enableColors;
    dim_t resolution;
    REAL isolation;
    index_t size, size2, size3;
    index_t  yd, zd;
    REAL halfsize;
    REAL delta;
    //array1d field;
    array1d field;
    array1d normal_cache;

    //parameters
    const static dim_t maxCount = 4096; // TODO: find the fastest size for this buffer

protected:

    index_t  temp_buffer_size = 12;

    //caching
    array1d vlist_buffer;
    array1d nlist_buffer;  // size: 12 x 3


    //counter of what?
    int count = 0;

    bool hasPositions = false;
    bool hasNormals = false;
    bool hasColors = false;
    bool hasUvs = false;

    array1d positionArray; //size: MaxCount x 3
    array1d normalArray;

    array1d colorArray;
    array1d uvArray;

    void kill();

static const int edgeTable[256];
static const int triTable[256*16];

protected:
    void init( dim_t resolution );

void VIntX( index_t q, array1d &pout, array1d &nout,
    int offset, REAL isol, REAL x, REAL y, REAL z, REAL valp1, REAL valp2 );
void VIntY( index_t q, array1d& pout, array1d& nout,
    int offset, REAL isol, REAL x, REAL y, REAL z, REAL valp1, REAL valp2 );
void VIntZ( index_t q, array1d& pout, array1d& nout,
    int offset, REAL isol, REAL x, REAL y, REAL z, REAL valp1, REAL valp2 );

void compNorm( index_t q );
void posnormtriv( array1d& pos__vlist, array1d& norm__nlist, int o1, int o2, int o3, const callback_t& renderCallback );

void begin();
void end( const callback_t& renderCallback );


public:
    MarchingCubes( dim_t resolution, bool enableUvs, bool enableColors );
    //int polygonize( REAL fx, REAL fy, REAL fz, index_t q, REAL isol );
    int polygonize( REAL fx, REAL fy, REAL fz, index_t q, REAL isol, const callback_t& callback );

//shape:
    void addBall( REAL ballx, REAL bally, REAL ballz, REAL strength, REAL subtract );
    void addPlaneX( REAL strength, REAL subtract );
    void addPlaneZ( REAL strength, REAL subtract );
    void addPlaneY( REAL strength, REAL subtract );
//field
    void reset(); //???? Nobody calls this.

//geometry/threejs interface side.
    void render(const callback_t& renderCallback );

};

//static dim_t MarchingCubes::maxCount = ...;


MarchingCubes::MarchingCubes( dim_t resolution, bool enableUvs=false, bool enableColors=false )
    :   //constructor's initialisation list: pre-constructor code
        //All memory allocation code is here. Because the size of arrays is determined in run-time.
        field(array1d( array_shape_t ({{ resolution*resolution*resolution }}) )),
        vlist_buffer(array1d( array_shape_t( {temp_buffer_size * 3} ) )),
        nlist_buffer(array1d( array_shape_t( {temp_buffer_size * 3} ) )),
        normal_cache(array1d( array_shape_t({ resolution*resolution*resolution*3 }) )),

        positionArray(array1d(make_shape_1d(MarchingCubes::maxCount * 3))),
        normalArray(array1d(make_shape_1d(MarchingCubes::maxCount * 3)))

{

    //THREE.ImmediateRenderObject.call( this, material );

    this->enableUvs = enableUvs;
    this->enableColors = enableColors;

    std::cout << resolution << " init"<< std::endl;

    this->init( resolution );

}

void MarchingCubes::kill()
{
    ;
}


//only called by the constructor
void MarchingCubes::init( dim_t resolution )
{

        this->resolution = resolution;

        // parameters

        this->isolation = 80.0;

        // size of field, 32 is pushing it in Javascript :)

        this->size = resolution;
        this->size2 = this->size * this->size;
        this->size3 = this->size2 * this->size;
        this->halfsize = ((REAL)this->size) / 2.0;

        // deltas
        this->delta = 2.0 / (REAL)this->size;
        this->yd = this->size;
        this->zd = this->size2;

        array_shape_t size = {{(int)this->size3}};
        this->field = array1d(size);
        //this->field = new Float32Array( this->size3 );
        //this->field = boost::array<REAL, 1>(this->size3); //does not work
        //this->field = std::array<REAL, this->size3>();
        //need a guarantee:

        assert(this->size3 < 10000000); // todo: get available heap. make it an exception.
        assert(this->size3 > 0);








/**
 *  COMMENTED OUT
 *
        // auto field_shape = make_shape_1d((int)this->size3);
        // //array_shape_t  field_shape = {{ (int)this->size3, }};
        //
        //
        // std::cout << "trouble begins" << std::endl;
        // std::cout << (int)this->size3 << std::endl;
        //
        // //this->field = array1d( field_shape );
        // this->field = array1d( field_shape );
        // //this->field = array1d( field_shape );
*/
        std::cout << "trouble end" << std::endl;

        //this->normal_cache = new Float32Array( this->size3 * 3 );
        array_shape_t normals_shape = make_shape_1d( (int)this->size3 * 3 );
        //array_shape_t  normals_shape = {{ (int)this->size3 * 3, }};
        this->normal_cache = array1d( normals_shape );
        //std::fill_n(this->normal_cache.begin(), this->normal_cache.size(), 0.0 );  // from #include <algorithm>


        std::fill(this->normal_cache.begin(), this->normal_cache.end(), 0.0 );  // from #include <algorithm>



        // temp buffers used in polygonize

        //this->vlist_buffer = new Float32Array( 12 * 3 );
        //this->nlist_buffer = new Float32Array( 12 * 3 );
        //auto twelve3 = make_shape_1d( temp_buffer_size * 3 );

        //array_shape_t twelve3 = {{ 12 * 3, }};

        if(false){
            this->vlist_buffer = array1d( make_shape_1d( temp_buffer_size * 3 ) );
            this->nlist_buffer = array1d( make_shape_1d( temp_buffer_size * 3 ) );
        }

        // immediate render mode simulator

        //this::maxCount = 4096; // TODO: find the fastest size for this buffer

        //counter of what?
        this->count = 0;

        this->hasPositions = false;
        this->hasNormals = false;
        this->hasColors = false;
        this->hasUvs = false;

        //this->positionArray = new Float32Array( this->maxCount * 3 );
        //this->normalArray   = new Float32Array( this->maxCount * 3 );


        auto shape_maxCount_x_3 = make_shape_1d(MarchingCubes::maxCount * 3);
        //array_shape_t shape_maxCount_x_3 = {{ this->maxCount * 3, }};
        this->positionArray = array1d(shape_maxCount_x_3);
        this->normalArray   = array1d(shape_maxCount_x_3);



        exit(1);

        auto shape_maxCount_x_2 = make_shape_1d(MarchingCubes::maxCount * 2);
        //array_shape_t  shape_maxCount_x_2 = {{ this->maxCount * 2, }};

        if ( this->enableUvs ) {
            //this->uvArray = new Float32Array( this->maxCount * 2 );
            this->uvArray = array1d(shape_maxCount_x_2);
        }

        if ( this->enableColors ) {
            this->colorArray   = array1d(shape_maxCount_x_3);
            //new Float32Array( this->maxCount * 3 );
        }
}

void MarchingCubes:: VIntX(
    index_t q, array1d &pout, array1d &nout,
    int offset,
    REAL isol,
    REAL x, REAL y, REAL z,
    REAL valp1,
    REAL valp2 )
{
    //pout is vlist_buffer
    //nout is nlist_buffer

    REAL mu = ( isol - valp1 ) / ( valp2 - valp1 );
    const array1d& nc = this->normal_cache;

    pout[ offset ]     = x + mu * this->delta;
    pout[ offset + 1 ] = y;
    pout[ offset + 2 ] = z;

    //todo: check the type of q
    nout[ offset ]     = lerp( nc[ q ],     nc[ q + 3 ], mu );
    nout[ offset + 1 ] = lerp( nc[ q + 1 ], nc[ q + 4 ], mu );
    nout[ offset + 2 ] = lerp( nc[ q + 2 ], nc[ q + 5 ], mu );
}


void MarchingCubes:: VIntY (index_t q, array1d& pout, array1d& nout, int offset, REAL isol, REAL x, REAL y, REAL z, REAL valp1, REAL valp2 )
{

    REAL mu = ( isol - valp1 ) / ( valp2 - valp1 );
    const array1d& nc = this->normal_cache;

    pout[ offset ]     = x;
    pout[ offset + 1 ] = y + mu * this->delta;
    pout[ offset + 2 ] = z;

    index_t q2 = q + this->yd * 3;

    nout[ offset ]     = lerp( nc[ q ],     nc[ q2 ],     mu );
    nout[ offset + 1 ] = lerp( nc[ q + 1 ], nc[ q2 + 1 ], mu );
    nout[ offset + 2 ] = lerp( nc[ q + 2 ], nc[ q2 + 2 ], mu );

}

void MarchingCubes:: VIntZ(index_t q, array1d& pout, array1d& nout, int offset, REAL isol, REAL x, REAL y, REAL z, REAL valp1, REAL valp2 )
{

    REAL mu = ( isol - valp1 ) / ( valp2 - valp1 );
    const array1d& nc = this->normal_cache;

    pout[ offset ]     = x;
    pout[ offset + 1 ] = y;
    pout[ offset + 2 ] = z + mu * this->delta;

    index_t q2 = q + this->zd * 3;

    nout[ offset ]     = lerp( nc[ q ],     nc[ q2 ],     mu );
    nout[ offset + 1 ] = lerp( nc[ q + 1 ], nc[ q2 + 1 ], mu );
    nout[ offset + 2 ] = lerp( nc[ q + 2 ], nc[ q2 + 2 ], mu );

}

void MarchingCubes:: compNorm( index_t q ) {

        index_t q3 = q * 3;

        //What if the x happens to be 0.0 ?
        if ( this->normal_cache[ q3 ] == 0.0 ) {

            this->normal_cache[ q3 ] = this->field[ q - 1 ]            - this->field[ q + 1 ];
            this->normal_cache[ q3 + 1 ] = this->field[ q - this->yd ] - this->field[ q + this->yd ];
            this->normal_cache[ q3 + 2 ] = this->field[ q - this->zd ] - this->field[ q + this->zd ];
        }

}




// Returns total number of triangles. Fills triangles.
// (this is where most of time is spent - it's inner work of O(n3) loop )


int MarchingCubes::polygonize( REAL fx, REAL fy, REAL fz, index_t q, REAL isol, const callback_t& renderCallback )
{
    // cache indices
    index_t q1 = q + 1,
        qy = q + this->yd,
        qz = q + this->zd,
        q1y = q1 + this->yd,
        q1z = q1 + this->zd,
        qyz = q + this->yd + this->zd,
        q1yz = q1 + this->yd + this->zd;

    int cubeindex = 0;

    REAL
        field0 = this->field[ q ],
        field1 = this->field[ q1 ],
        field2 = this->field[ qy ],
        field3 = this->field[ q1y ],
        field4 = this->field[ qz ],
        field5 = this->field[ q1z ],
        field6 = this->field[ qyz ],
        field7 = this->field[ q1yz ];

    if ( field0 < isol ) cubeindex |= 1;
    if ( field1 < isol ) cubeindex |= 2;
    if ( field2 < isol ) cubeindex |= 8;
    if ( field3 < isol ) cubeindex |= 4;
    if ( field4 < isol ) cubeindex |= 16;
    if ( field5 < isol ) cubeindex |= 32;
    if ( field6 < isol ) cubeindex |= 128;
    if ( field7 < isol ) cubeindex |= 64;

    // if cube is entirely in/out of the surface - bail, nothing to draw

    int bits = edgeTable[ cubeindex ];
    if ( bits == 0x00 ) return 0;

    REAL d = this->delta,
        fx2 = fx + d,
        fy2 = fy + d,
        fz2 = fz + d;

    // top of the cube

    if ( bits & 1 ) {

        this->compNorm( q );
        this->compNorm( q1 );
        this->VIntX( q * 3, this->vlist_buffer, this->nlist_buffer, 0, isol, fx, fy, fz, field0, field1 );

    }

    if ( bits & 2 ) {

        this->compNorm( q1 );
        this->compNorm( q1y );
        this->VIntY( q1 * 3, this->vlist_buffer, this->nlist_buffer, 3, isol, fx2, fy, fz, field1, field3 );

    }

    if ( bits & 4 ) {

        this->compNorm( qy );
        this->compNorm( q1y );
        this->VIntX( qy * 3, this->vlist_buffer, this->nlist_buffer, 6, isol, fx, fy2, fz, field2, field3 );

    }

    if ( bits & 8 ) {

        this->compNorm( q );
        this->compNorm( qy );
        this->VIntY( q * 3, this->vlist_buffer, this->nlist_buffer, 9, isol, fx, fy, fz, field0, field2 );

    }

    // bottom of the cube

    if ( bits & 16 ) {

        this->compNorm( qz );
        this->compNorm( q1z );
        this->VIntX( qz * 3, this->vlist_buffer, this->nlist_buffer, 12, isol, fx, fy, fz2, field4, field5 );

    }

    if ( bits & 32 ) {

        this->compNorm( q1z );
        this->compNorm( q1yz );
        this->VIntY( q1z * 3,  this->vlist_buffer, this->nlist_buffer, 15, isol, fx2, fy, fz2, field5, field7 );

    }

    if ( bits & 64 ) {

        this->compNorm( qyz );
        this->compNorm( q1yz );
        this->VIntX( qyz * 3, this->vlist_buffer, this->nlist_buffer, 18, isol, fx, fy2, fz2, field6, field7 );

    }

    if ( bits & 128 ) {

        this->compNorm( qz );
        this->compNorm( qyz );
        this->VIntY( qz * 3,  this->vlist_buffer, this->nlist_buffer, 21, isol, fx, fy, fz2, field4, field6 );

    }

    // vertical lines of the cube

    if ( bits & 256 ) {

        this->compNorm( q );
        this->compNorm( qz );
        this->VIntZ( q * 3, this->vlist_buffer, this->nlist_buffer, 24, isol, fx, fy, fz, field0, field4 );

    }

    if ( bits & 512 ) {

        this->compNorm( q1 );
        this->compNorm( q1z );
        this->VIntZ( q1 * 3,  this->vlist_buffer, this->nlist_buffer, 27, isol, fx2, fy,  fz, field1, field5 );

    }

    if ( bits & 1024 ) {

        this->compNorm( q1y );
        this->compNorm( q1yz );
        this->VIntZ( q1y * 3, this->vlist_buffer, this->nlist_buffer, 30, isol, fx2, fy2, fz, field3, field7 );

    }

    if ( bits & 2048 ) {

        this->compNorm( qy );
        this->compNorm( qyz );
        this->VIntZ( qy * 3, this->vlist_buffer, this->nlist_buffer, 33, isol, fx,  fy2, fz, field2, field6 );

    }

    cubeindex <<= 4;  // re-purpose cubeindex into an offset into triTable


    //not sure about the type:
    int o1, o2, o3, numtris = 0, i = 0;

    // here is where triangles are created

    while ( triTable[ cubeindex + i ] != - 1 ) {

        o1 = cubeindex + i;
        o2 = o1 + 1;
        o3 = o1 + 2;

        this->posnormtriv(
            this->vlist_buffer, this->nlist_buffer,
            3 * MarchingCubes::triTable[ o1 ],
            3 * MarchingCubes::triTable[ o2 ],
            3 * MarchingCubes::triTable[ o3 ],
            renderCallback );

        i += 3;
        numtris ++;

    }

    return numtris;

}


/////////////////////////////////////
// Immediate render mode simulator
/////////////////////////////////////

void MarchingCubes::posnormtriv(
    array1d& pos__vlist, array1d& norm__nlist,
    int o1, int o2, int o3,
    const callback_t& renderCallback ) {

    int c = this->count * 3;

    // positions

    this->positionArray[ c ]     = pos__vlist[ o1 ];
    this->positionArray[ c + 1 ] = pos__vlist[ o1 + 1 ];
    this->positionArray[ c + 2 ] = pos__vlist[ o1 + 2 ];

    this->positionArray[ c + 3 ] = pos__vlist[ o2 ];
    this->positionArray[ c + 4 ] = pos__vlist[ o2 + 1 ];
    this->positionArray[ c + 5 ] = pos__vlist[ o2 + 2 ];

    this->positionArray[ c + 6 ] = pos__vlist[ o3 ];
    this->positionArray[ c + 7 ] = pos__vlist[ o3 + 1 ];
    this->positionArray[ c + 8 ] = pos__vlist[ o3 + 2 ];

    // normals

    this->normalArray[ c ]     = norm__nlist[ o1 ];
    this->normalArray[ c + 1 ] = norm__nlist[ o1 + 1 ];
    this->normalArray[ c + 2 ] = norm__nlist[ o1 + 2 ];

    this->normalArray[ c + 3 ] = norm__nlist[ o2 ];
    this->normalArray[ c + 4 ] = norm__nlist[ o2 + 1 ];
    this->normalArray[ c + 5 ] = norm__nlist[ o2 + 2 ];

    this->normalArray[ c + 6 ] = norm__nlist[ o3 ];
    this->normalArray[ c + 7 ] = norm__nlist[ o3 + 1 ];
    this->normalArray[ c + 8 ] = norm__nlist[ o3 + 2 ];

    // uvs

    if ( this->enableUvs ) {

        int d = this->count * 2;

        this->uvArray[ d ]     = pos__vlist[ o1 ];
        this->uvArray[ d + 1 ] = pos__vlist[ o1 + 2 ];

        this->uvArray[ d + 2 ] = pos__vlist[ o2 ];
        this->uvArray[ d + 3 ] = pos__vlist[ o2 + 2 ];

        this->uvArray[ d + 4 ] = pos__vlist[ o3 ];
        this->uvArray[ d + 5 ] = pos__vlist[ o3 + 2 ];

    }

    // colors

    if ( this->enableColors ) {

        this->colorArray[ c ]     = pos__vlist[ o1 ];
        this->colorArray[ c + 1 ] = pos__vlist[ o1 + 1 ];
        this->colorArray[ c + 2 ] = pos__vlist[ o1 + 2 ];

        this->colorArray[ c + 3 ] = pos__vlist[ o2 ];
        this->colorArray[ c + 4 ] = pos__vlist[ o2 + 1 ];
        this->colorArray[ c + 5 ] = pos__vlist[ o2 + 2 ];

        this->colorArray[ c + 6 ] = pos__vlist[ o3 ];
        this->colorArray[ c + 7 ] = pos__vlist[ o3 + 1 ];
        this->colorArray[ c + 8 ] = pos__vlist[ o3 + 2 ];

    }

    this->count += 3;

    if ( this->count >= this->maxCount - 3 )  //why equal?
    {
        this->hasPositions = true;
        this->hasNormals = true;
        if ( this->enableUvs ) {
            this->hasUvs = true;
        }
        if ( this->enableColors ) {
            this->hasColors = true;
        }
        renderCallback.call( (void*)this );

    }
}


void MarchingCubes::begin()
{
    this->count = 0;

    this->hasPositions = false;
    this->hasNormals = false;
    this->hasUvs = false;
    this->hasColors = false;
}

void MarchingCubes::end( const callback_t& renderCallback )
{

    // count := number of prepared (?)
    if ( this->count == 0 ) return;

    //for ( int i = this->count * 3; i < this->positionArray.length; i ++ ) {
    //    this->positionArray[ i ] = 0.0;
    //}

    std::fill(this->positionArray.begin(), this->positionArray.end(), 0.0 );

    this->hasPositions = true;
    this->hasNormals = true;

    if ( this->enableUvs ) {
        this->hasUvs = true;
    }

    if ( this->enableColors ) {
        this->hasColors = true;
    }

    renderCallback.call( this );

}


//todo: separate the following into the `field` [part of the] class.

/////////////////////////////////////
// Metaballs
/////////////////////////////////////

// Adds a reciprocal ball (nice and blobby) that, to be fast, fades to zero after
// a fixed distance, determined by strength and subtract.

void MarchingCubes::addBall( REAL ballx, REAL bally, REAL ballz, REAL strength, REAL subtract ) {

    // Let's solve the equation to find the radius:
    // 1.0 / (0.000001 + radius^2) * strength - subtract = 0
    // strength / (radius^2) = subtract
    // strength = subtract * radius^2
    // radius^2 = strength / subtract
    // radius = sqrt(strength / subtract)

    REAL radius = this->size * sqrt( strength / subtract ),
        zs = ballz * this->size,
        ys = bally * this->size,
        xs = ballx * this->size;

    int min_z = floor( zs - radius ); if ( min_z < 1 ) min_z = 1;
    int max_z = floor( zs + radius ); if ( max_z > this->size - 1 ) max_z = this->size - 1;
    int min_y = floor( ys - radius ); if ( min_y < 1 ) min_y = 1;
    int max_y = floor( ys + radius ); if ( max_y > this->size - 1 ) max_y = this->size - 1;
    int min_x = floor( xs - radius ); if ( min_x < 1  ) min_x = 1;
    int max_x = floor( xs + radius ); if ( max_x > this->size - 1 ) max_x = this->size - 1;


    // Don't polygonize in the outer layer because normals aren't
    // well-defined there.

    //var x, y, z, y_offset, z_offset, fx, fy, fz, fz2, fy2, val;
    int x, y, z;
    REAL fx, fy, fz, fz2, fy2, val;  //Does doing like this make it faster?
    int y_offset, z_offset;

    for ( z = min_z; z < max_z; z ++ ) {

        z_offset = this->size2 * z,
        fz = z / (REAL)this->size - ballz,
        fz2 = fz * fz;

        for ( y = min_y; y < max_y; y ++ ) {

            y_offset = z_offset + this->size * y;
            fy = y / (REAL)this->size - bally;
            fy2 = fy * fy;

            for ( x = min_x; x < max_x; x ++ ) {

                fx = x / (REAL)this->size - ballx;
                val = strength / ( 0.000001 + fx * fx + fy2 + fz2 ) - subtract;
                if ( val > 0.0 ) this->field[ y_offset + x ] += val;

            }

        }

    }

}

void MarchingCubes::addPlaneX( REAL strength, REAL subtract ) {
    int x, y, z;
    REAL val;
    REAL xx, xdiv;
    int cxy;

    // cache attribute lookups
    int yd = this->yd;
    int size = this->size;
    int zd = this->zd;
    array1d& field = this->field;
    REAL dist = size * sqrt( strength / (REAL)subtract );

    if ( dist > size ) dist = size;
    for ( x = 0; x < dist; x ++ ) {
        xdiv = x / (REAL)size;
        xx = xdiv * xdiv;
        val = strength / (REAL)( 0.0001 + xx ) - subtract;
        if ( val > 0.0 ) {
            for ( y = 0; y < size; y ++ ) {
                cxy = x + y * yd;
                for ( z = 0; z < size; z ++ ) {
                    field[ zd * z + cxy ] += val;
                }
            }
        }
    }
}


void MarchingCubes::addPlaneY( REAL strength, REAL subtract ) {

    int x, y, z;
    REAL yy;
    REAL val;
    REAL ydiv;
    int cy;
    int cxy;

    // cache attribute lookups
    int size = this->size;
    int yd = this->yd;
    int zd = this->zd;
    array1d& field = this->field;
    REAL dist = size * sqrt( strength / subtract );

    if ( dist > size ) dist = size;

    for ( y = 0; y < dist; y ++ ) {
        ydiv = y / (REAL)size;
        yy = ydiv * ydiv;
        val = strength / (REAL)( 0.0001 + yy ) - subtract;
        if ( val > 0.0 ) {
            cy = y * yd;
            for ( x = 0; x < size; x ++ ) {
                cxy = cy + x;
                for ( z = 0; z < size; z ++ )
                    field[ zd * z + cxy ] += val;
            }
        }
    }
}

void MarchingCubes::addPlaneZ( REAL strength, REAL subtract )
{
    int x, y, z;
    REAL zz, val, zdiv;
    int cz, cyz;

    // cache attribute lookups
    int size = this->size;
    int yd = this->yd;
    int zd = this->zd;
    array1d& field = this->field;
    REAL dist = size * sqrt( strength / subtract );

    if ( dist > size ) dist = size;
    for ( z = 0; z < dist; z ++ ) {
        zdiv = z / (REAL)size;
        zz = zdiv * zdiv;
        val = strength / (REAL)( 0.0001 + zz ) - subtract;
        if ( val > 0.0 ) {
            cz = zd * z;
            for ( y = 0; y < size; y ++ ) {
                cyz = cz + y * yd;
                for ( x = 0; x < size; x ++ )
                    field[ cyz + x ] += val;
            }
        }
    }
}




/////////////////////////////////////
// Updates
/////////////////////////////////////

void MarchingCubes::reset()
{
    // wipe the normal cache
    for (int i = 0; i < this->size3; i ++ ) {
        this->normal_cache[ i * 3 ] = 0.0; // Why the other elements are not done?
        this->field[ i ] = 0.0;
    }
}



//renderes a geometry.
void MarchingCubes::render(const callback_t& renderCallback )
{

    this->begin();

    // Triangulate. Yeah, this is slow.

    int smin2 = this->size - 2;

    for ( int z = 1; z < smin2; z ++ ) {

        index_t z_offset = this->size2 * z;
        REAL fz = ( z - this->halfsize ) / (REAL)this->halfsize; //+ 1

        for ( int y = 1; y < smin2; y ++ ) {

            index_t y_offset = z_offset + this->size * y;
            REAL fy = ( y - this->halfsize ) / (REAL)this->halfsize; //+ 1

            for ( int x = 1; x < smin2; x ++ ) {

                REAL fx = ( x - this->halfsize ) / (REAL)this->halfsize; //+ 1
                index_t q = y_offset + x;

                this->polygonize( fx, fy, fz, q, this->isolation, renderCallback );

            }

        }

    }

    this->end( renderCallback );

}

/*

var geo_callback = function( object ) {

    var i, x, y, z, vertex, normal,
        face, a, b, c, na, nb, nc, nfaces;


    for ( i = 0; i < object.count; i ++ ) {

        a = i * 3;
        b = a + 1;
        c = a + 2;

        x = object.positionArray[ a ];
        y = object.positionArray[ b ];
        z = object.positionArray[ c ];
        vertex = new THREE.Vector3( x, y, z );

        x = object.normalArray[ a ];
        y = object.normalArray[ b ];
        z = object.normalArray[ c ];
        normal = new THREE.Vector3( x, y, z );
        normal.normalize();

        geo.vertices.push( vertex );
        normals.push( normal );

    }

    nfaces = object.count / 3;

    for ( i = 0; i < nfaces; i ++ ) {

        a = ( start + i ) * 3;
        b = a + 1;
        c = a + 2;

        na = normals[ a ];
        nb = normals[ b ];
        nc = normals[ c ];

        face = new THREE.Face3( a, b, c, [ na, nb, nc ] );

        geo.faces.push( face );

    }

    start += nfaces;
    object.count = 0;

};

//this->generateGeometry = function() {...}
Geometry generateGeometry = function()
{
    var start = 0, geo = new THREE.Geometry();
    var normals = [];
    this->render( geo_callback );
    // console.log( "generated " + geo.faces.length + " triangles" );
    return geo;
};
*/


/*
THREE.MarchingCubes.prototype = Object.create( THREE.ImmediateRenderObject.prototype );
THREE.MarchingCubes.prototype.constructor = THREE.MarchingCubes;
*/

/////////////////////////////////////
// Marching cubes lookup tables
/////////////////////////////////////

// These tables are straight from Paul Bourke's page:
// http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/
// who in turn got them from Cory Gene Bloyd.

const int MarchingCubes::edgeTable[256] = {
0x0,   0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99,  0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};


//const int triTable[256][16] =
const int MarchingCubes::triTable[256*16] =
{
- 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 1, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 8, 3, 9, 8, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, 1, 2, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 2, 10, 0, 2, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 8, 3, 2, 10, 8, 10, 9, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 11, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 11, 2, 8, 11, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 9, 0, 2, 3, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 11, 2, 1, 9, 11, 9, 8, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 10, 1, 11, 10, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 10, 1, 0, 8, 10, 8, 11, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 9, 0, 3, 11, 9, 11, 10, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 8, 10, 10, 8, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 7, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 3, 0, 7, 3, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 1, 9, 8, 4, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 1, 9, 4, 7, 1, 7, 3, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, 8, 4, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 4, 7, 3, 0, 4, 1, 2, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 2, 10, 9, 0, 2, 8, 4, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, - 1, - 1, - 1, - 1,
8, 4, 7, 3, 11, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 4, 7, 11, 2, 4, 2, 0, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 0, 1, 8, 4, 7, 2, 3, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, - 1, - 1, - 1, - 1,
3, 10, 1, 3, 11, 10, 7, 8, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, - 1, - 1, - 1, - 1,
4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, - 1, - 1, - 1, - 1,
4, 7, 11, 4, 11, 9, 9, 11, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 5, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 5, 4, 0, 8, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 5, 4, 1, 5, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 5, 4, 8, 3, 5, 3, 1, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, 9, 5, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 0, 8, 1, 2, 10, 4, 9, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 2, 10, 5, 4, 2, 4, 0, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, - 1, - 1, - 1, - 1,
9, 5, 4, 2, 3, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 11, 2, 0, 8, 11, 4, 9, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 5, 4, 0, 1, 5, 2, 3, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, - 1, - 1, - 1, - 1,
10, 3, 11, 10, 1, 3, 9, 5, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, - 1, - 1, - 1, - 1,
5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, - 1, - 1, - 1, - 1,
5, 4, 8, 5, 8, 10, 10, 8, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 7, 8, 5, 7, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 3, 0, 9, 5, 3, 5, 7, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 7, 8, 0, 1, 7, 1, 5, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 5, 3, 3, 5, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 7, 8, 9, 5, 7, 10, 1, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, - 1, - 1, - 1, - 1,
8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, - 1, - 1, - 1, - 1,
2, 10, 5, 2, 5, 3, 3, 5, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 9, 5, 7, 8, 9, 3, 11, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, - 1, - 1, - 1, - 1,
2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, - 1, - 1, - 1, - 1,
11, 2, 1, 11, 1, 7, 7, 1, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, - 1, - 1, - 1, - 1,
5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, - 1,
11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, - 1,
11, 10, 5, 7, 11, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 6, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, 5, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 0, 1, 5, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 8, 3, 1, 9, 8, 5, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 6, 5, 2, 6, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 6, 5, 1, 2, 6, 3, 0, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 6, 5, 9, 0, 6, 0, 2, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, - 1, - 1, - 1, - 1,
2, 3, 11, 10, 6, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 0, 8, 11, 2, 0, 10, 6, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 1, 9, 2, 3, 11, 5, 10, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, - 1, - 1, - 1, - 1,
6, 3, 11, 6, 5, 3, 5, 1, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, - 1, - 1, - 1, - 1,
3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, - 1, - 1, - 1, - 1,
6, 5, 9, 6, 9, 11, 11, 9, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 10, 6, 4, 7, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 3, 0, 4, 7, 3, 6, 5, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 9, 0, 5, 10, 6, 8, 4, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, - 1, - 1, - 1, - 1,
6, 1, 2, 6, 5, 1, 4, 7, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, - 1, - 1, - 1, - 1,
8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, - 1, - 1, - 1, - 1,
7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, - 1,
3, 11, 2, 7, 8, 4, 10, 6, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, - 1, - 1, - 1, - 1,
0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, - 1, - 1, - 1, - 1,
9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, - 1,
8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, - 1, - 1, - 1, - 1,
5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, - 1,
0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, - 1,
6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, - 1, - 1, - 1, - 1,
10, 4, 9, 6, 4, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 10, 6, 4, 9, 10, 0, 8, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 0, 1, 10, 6, 0, 6, 4, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, - 1, - 1, - 1, - 1,
1, 4, 9, 1, 2, 4, 2, 6, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, - 1, - 1, - 1, - 1,
0, 2, 4, 4, 2, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 3, 2, 8, 2, 4, 4, 2, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 4, 9, 10, 6, 4, 11, 2, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, - 1, - 1, - 1, - 1,
3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, - 1, - 1, - 1, - 1,
6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, - 1,
9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, - 1, - 1, - 1, - 1,
8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, - 1,
3, 11, 6, 3, 6, 0, 0, 6, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
6, 4, 8, 11, 6, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 10, 6, 7, 8, 10, 8, 9, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, - 1, - 1, - 1, - 1,
10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, - 1, - 1, - 1, - 1,
10, 6, 7, 10, 7, 1, 1, 7, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, - 1, - 1, - 1, - 1,
2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, - 1,
7, 8, 0, 7, 0, 6, 6, 0, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 3, 2, 6, 7, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, - 1, - 1, - 1, - 1,
2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, - 1,
1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, - 1,
11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, - 1, - 1, - 1, - 1,
8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, - 1,
0, 9, 1, 11, 6, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, - 1, - 1, - 1, - 1,
7, 11, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 6, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 0, 8, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 1, 9, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 1, 9, 8, 3, 1, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 1, 2, 6, 11, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, 3, 0, 8, 6, 11, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 9, 0, 2, 10, 9, 6, 11, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, - 1, - 1, - 1, - 1,
7, 2, 3, 6, 2, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
7, 0, 8, 7, 6, 0, 6, 2, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 7, 6, 2, 3, 7, 0, 1, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, - 1, - 1, - 1, - 1,
10, 7, 6, 10, 1, 7, 1, 3, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, - 1, - 1, - 1, - 1,
0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, - 1, - 1, - 1, - 1,
7, 6, 10, 7, 10, 8, 8, 10, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
6, 8, 4, 11, 8, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 6, 11, 3, 0, 6, 0, 4, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 6, 11, 8, 4, 6, 9, 0, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, - 1, - 1, - 1, - 1,
6, 8, 4, 6, 11, 8, 2, 10, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, - 1, - 1, - 1, - 1,
4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, - 1, - 1, - 1, - 1,
10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, - 1,
8, 2, 3, 8, 4, 2, 4, 6, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 4, 2, 4, 6, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, - 1, - 1, - 1, - 1,
1, 9, 4, 1, 4, 2, 2, 4, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, - 1, - 1, - 1, - 1,
10, 1, 0, 10, 0, 6, 6, 0, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, - 1,
10, 9, 4, 6, 10, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 9, 5, 7, 6, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, 4, 9, 5, 11, 7, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 0, 1, 5, 4, 0, 7, 6, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, - 1, - 1, - 1, - 1,
9, 5, 4, 10, 1, 2, 7, 6, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, - 1, - 1, - 1, - 1,
7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, - 1, - 1, - 1, - 1,
3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, - 1,
7, 2, 3, 7, 6, 2, 5, 4, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, - 1, - 1, - 1, - 1,
3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, - 1, - 1, - 1, - 1,
6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, - 1,
9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, - 1, - 1, - 1, - 1,
1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, - 1,
4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, - 1,
7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, - 1, - 1, - 1, - 1,
6, 9, 5, 6, 11, 9, 11, 8, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, - 1, - 1, - 1, - 1,
0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, - 1, - 1, - 1, - 1,
6, 11, 3, 6, 3, 5, 5, 3, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, - 1, - 1, - 1, - 1,
0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, - 1,
11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, - 1,
6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, - 1, - 1, - 1, - 1,
5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, - 1, - 1, - 1, - 1,
9, 5, 6, 9, 6, 0, 0, 6, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, - 1,
1, 5, 6, 2, 1, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, - 1,
10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, - 1, - 1, - 1, - 1,
0, 3, 8, 5, 6, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 5, 6, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 5, 10, 7, 5, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 5, 10, 11, 7, 5, 8, 3, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 11, 7, 5, 10, 11, 1, 9, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, - 1, - 1, - 1, - 1,
11, 1, 2, 11, 7, 1, 7, 5, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, - 1, - 1, - 1, - 1,
9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, - 1, - 1, - 1, - 1,
7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, - 1,
2, 5, 10, 2, 3, 5, 3, 7, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, - 1, - 1, - 1, - 1,
9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, - 1, - 1, - 1, - 1,
9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, - 1,
1, 3, 5, 3, 7, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 7, 0, 7, 1, 1, 7, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 0, 3, 9, 3, 5, 5, 3, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 8, 7, 5, 9, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 8, 4, 5, 10, 8, 10, 11, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, - 1, - 1, - 1, - 1,
0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, - 1, - 1, - 1, - 1,
10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, - 1,
2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, - 1, - 1, - 1, - 1,
0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, - 1,
0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, - 1,
9, 4, 5, 2, 11, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, - 1, - 1, - 1, - 1,
5, 10, 2, 5, 2, 4, 4, 2, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, - 1,
5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, - 1, - 1, - 1, - 1,
8, 4, 5, 8, 5, 3, 3, 5, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 4, 5, 1, 0, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, - 1, - 1, - 1, - 1,
9, 4, 5, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 11, 7, 4, 9, 11, 9, 10, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, - 1, - 1, - 1, - 1,
1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, - 1, - 1, - 1, - 1,
3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, - 1,
4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, - 1, - 1, - 1, - 1,
9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, - 1,
11, 7, 4, 11, 4, 2, 2, 4, 0, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, - 1, - 1, - 1, - 1,
2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, - 1, - 1, - 1, - 1,
9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, - 1,
3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, - 1,
1, 10, 2, 8, 7, 4, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 9, 1, 4, 1, 7, 7, 1, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, - 1, - 1, - 1, - 1,
4, 0, 3, 7, 4, 3, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
4, 8, 7, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 10, 8, 10, 11, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 0, 9, 3, 9, 11, 11, 9, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 1, 10, 0, 10, 8, 8, 10, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 1, 10, 11, 3, 10, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 2, 11, 1, 11, 9, 9, 11, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, - 1, - 1, - 1, - 1,
0, 2, 11, 8, 0, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
3, 2, 11, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 3, 8, 2, 8, 10, 10, 8, 9, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
9, 10, 2, 0, 9, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, - 1, - 1, - 1, - 1,
1, 10, 2, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
1, 3, 8, 9, 1, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 9, 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
0, 3, 8, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1,
- 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1, - 1
};


#include "timer.hpp"

int main(){
    timer t;
    //MarchingCubes mc( dim_t resolution, bool enableUvs, bool enableColors );
    dim_t resolution = 4; //28;
    bool enableUvs = true;
    bool enableColors = true;
    MarchingCubes mc( resolution, enableUvs, enableColors );

    cout << resolution << endl;
    t.stop();
    return 0;
}