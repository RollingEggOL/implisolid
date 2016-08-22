/* Copyright 2016  @sohale */

/**
 * AlteredQualia's Marching Cubes, C++ version, based on AQ's code based on Henrik Rydgård and @greggman.
 * https://github.com/WebGLSamples/WebGLSamples.github.io/blob/master/blob/marching_cubes.js
 *
 * Based on alteredq's version  https://github.com/mrdoob/three.js/blob/master/examples/js/MarchingCubes.js
 *
 * Port of greggman's ThreeD version of marching cubes to Three.js
 * http://webglsamples.googlecode.com/hg/blob/blob.html
 */

/*
Todo:
- WebWorker (incremental)
- Improve JS data exchange to Geometry
- Adding walls
- WebWorker wrappen in a Geometry
- Geometry in 3js r73
-
*/

#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "boost/multi_array.hpp"
#include "boost/array.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"


#include "../js_iteration_2/object_collector.hpp"

#include "../js_iteration_2/vertex_resampling.cpp"

// #include <math.h>

extern "C" {
    void produce_object_old2(float* verts, int *nv, int* faces, int *nf, float param);
    int main(int argc, char **argv);
}

const bool VERBOSE = false;
const bool REPORT_STATS = false;

// typedef unsigned short int size_t;
typedef uint16_t dim_t;  // small integers for example the size of one side of the grid
typedef float REAL;
// typedef unsigned long int index_t;

// boost::array will not work becasue the size of a boost:array has to be known in compile-time (static).
typedef  boost::multi_array<REAL, 1>  array1d;
// typedef array1d::index  array_index_t;
typedef boost::array<array1d::index, 1>  array_shape_t;
// #define array1d  boost::multi_array<REAL, 1>
typedef array1d::index  index_t;

typedef index_t index3_t;  // Range of the element type has to be large enough, larger than (size^3)*3.
typedef boost::multi_array<index3_t, 1>   array1d_e3;
typedef std::map<index3_t, int>  e3map_t;


struct callback_t { void call(void*) const { } callback_t(){} };

/*template<typename Index_Type=int>
boost::array<Index_Type, 1> make_shape_1d(Index_Type size)
{
    // Make a shape to be used in array initialisation
    // fixme: the type of size

    // ASSERT(size>=0);
    boost::array<Index_Type, 1> shape = {{ size, }};
    return shape;
}
*/

#include "mcc2_marching_cubes.hpp"
#include "tests/marching_cubes_mock.hpp"

void meta_balls(MarchingCubes& mc, int num_blobs, REAL time, REAL scale) {
    int numblobs = num_blobs;  // default: 4
    for (int ball_i = 0; ball_i < numblobs; ball_i++) {
        REAL D = 1;
        REAL ballx = sin(ball_i + 1.26 * time * (1.03 + 0.5*cos(0.21 * ball_i))) * 0.27 * D + 0.5;
        REAL bally = std::abs(cos(ball_i + 1.12 * time * cos(1.22 + 0.1424 * ball_i))) * 0.77 * D;  // dip into the floor
        REAL ballz = cos(ball_i + 1.32 * time * 0.1*sin((0.92 + 0.53 * ball_i))) * 0.27 * D + 0.5;
        REAL subtract = 12;
        REAL strength = 1.2 / ((sqrt(numblobs)- 1) / 4 + 1);
        mc.addBall(ballx, bally, ballz, strength, subtract, scale);
    }
}


/*********************************************************
    Old version, legacy
 *********************************************************/

void build_vf(
    // std::vector<REAL>& verts3,
    // std::vector<int>& faces3
    REAL scale
    ) {
    //  Includes allocations.

    dim_t resolution = 28;  // 28;
    bool enableUvs = true;
    bool enableColors = true;
    mp5_implicit::bounding_box box = {0, 3, 0, 3, 0, 3};

    MarchingCubes mc(resolution, box, enableUvs, enableColors);


    REAL time = 0.1;
    meta_balls(mc, 4, time, scale);
    mc.seal_exterior();

    /*
    int numblobs = 4;
    REAL subtract = (REAL)12.;
    REAL strength = (REAL)(1.2 / ( ( sqrt( numblobs ) - 1. ) / 4. + 1. ));
    mc.addBall(0.5, 0.5, 0.5, strength, subtract, 1);
    */
    // MarchingCubes& object = mc;
    // mc.addBall(0.5, 0.5, 0.5, strength, subtract, 1);

    // mc.flush_geometry_queue(loger, mc.resultqueue_faces_start, mc.result_normals, verts3, faces3);

    const callback_t renderCallback;
    mc.render_geometry(renderCallback);

    if (VERBOSE)
        loger << "MC:: v,f: " << mc.result_verts.size() << " " << mc.result_faces.size() << std::endl;

    // verts3.resize(0);
    // faces3.resize(0);
}



// Not used
void produce_object_old2(REAL* verts, int *nv, int* faces, int *nf, REAL time, REAL scale) {
    // not used

    dim_t resolution = 28;  // 28;
    bool enableUvs = true;
    bool enableColors = true;

    mp5_implicit::bounding_box box = {0, 3, 0, 3, 0, 3};

    if (VERBOSE)
        loger << "Leak-free (old version)" << std::endl;


    MarchingCubes mc(resolution, box, enableUvs, enableColors);
    // MarchingCubes* mc0 = new MarchingCubes(resolution, enableUvs, enableColors);
    // MarchingCubes &mc = *mc0;
    // MarchingCubesMock mc(resolution, enableUvs, enableColors);

    // REAL time = 0.1 ;
    meta_balls(mc, 4, time, scale);
    mc.seal_exterior();

    /*
    int numblobs = 4;
    REAL subtract = (REAL)12.;
    REAL strength = (REAL)(1.2 / ( ( sqrt( numblobs ) - 1. ) / 4. + 1. ));
    mc.addBall(0.5, 0.5, 0.5, strength, subtract, scale);
    mc.addBall(0, 0, 0.5, strength, subtract, scale);
    */

    // todo: init-receiver side

    const callback_t renderCallback;
    mc.render_geometry(renderCallback);

    loger << "map2" << std::endl;

    // mc.result_faces.resize(100);

    if (VERBOSE)
        loger << "MC:: v,f: " << mc.result_verts.size() << " " << mc.result_faces.size() << std::endl;

    *nv = mc.result_verts.size()/3;
    *nf = mc.result_faces.size()/3;

    // Vertices
    int ctr = 0;
    for ( std::vector<REAL>::iterator it=mc.result_verts.begin(); it < mc.result_verts.end(); it+=3 ) {
        for (int di=0; di < 3; di++) {
            verts[ctr] = *(it + di);
            ctr++;
        }
      }

    // Faces
    ctr = 0;
    for ( std::vector<int>::iterator it=mc.result_faces.begin(); it < mc.result_faces.end(); it+=3 ) {
        for (int di=0; di < 3; di++) {
            faces[ctr] = *(it + di);
            ctr++;
        }
      }
}


/*
#include <emscripten/bind.h>
using namespace emscripten;
EMSCRIPTEN_BINDINGS(my_module) {
    // produce_object(REAL* verts, int *nv, int* faces, int *nf, REAL time);
    function("produce_object_bind", &produce_object);
}
*/
// #include "mcc1-glue.cpp"


/*void produce_v(){
}*/



/*********************************************************
    public interface for JavaScript
 *********************************************************/


extern "C" {
    void build_geometry(const char* shape_parameters_json, const char* mc_parameters_json);
    int get_v_size();
    int get_f_size();
    void get_f(int*, int);
    void get_v(REAL*, int);
    void finish_geometry();
    void* get_f_ptr();
    void* get_v_ptr();

    // void implicit_value(REAL x, REAL y, REAL z);
    // void implicit_grad_value(REAL x, REAL y, REAL z, REAL * x_out, REAL * y_out, REAL * z_out);

    /********************************************************
        New API for direct evaluation of implicit functions
    *********************************************************/
    //global variables: mp5_implicit::implicit_function* last_object, last_x, v_results, g_results,
    void set_object(const char* shape_parameters_json, bool ignore_root_matrix);  // call object_factory. Sets a global variable   last_object
    void unset_object();

    bool set_x(void* verts, int n);   // sets last_x, a multi_array.
    void unset_x();

    void calculate_implicit_values();  // multi_array or vector that has a .data().  // calculate v_results from  last_object.eval_implicit()
    void* get_values_ptr();  // return v_results.data()
    int get_values_size();   // return v_results

    void calculate_implicit_gradients(bool normalize_and_invert);  // calculate g_results from  last_object.eval_gradient()
    void* get_gradients_ptr(); // return g_results.data()
    int get_gradients_size(); // return g_results

    void about();  // shows build information

    //void calculate_gradients()

    // also: queue, etc.
    // bad: one instance only.
    //     Solution 1:  MarchingCubes* build_geometry();
    //     Solution 2: ids (for workers! ; a statically determined number of them (slots/workers/buckets).).
};


typedef struct {
    bool active = 0;
    MarchingCubes* mc = 0;
} state_t;

state_t _state;

// _state.active = false;
// _state.mc = 0;

bool check_state() {
    if (!_state.active) {
        loger << "Error: There are no allocated geometry resources to deallocate.";
        return false;
    }
    return true;
}
bool check_state_null() {
    if (_state.active) {
        loger << "Error: There are non-deallocated geometry resources. Call finit_geometry() first. Not doing anything.";
        return false;
    }
    return true;
}


namespace mp5_implicit{

/* Holds all MC information that is transferred through json. Separate from the shape json (i.e. the MP5 Json). */
struct mc_settings {
    mp5_implicit::bounding_box box;
    int resolution;
    bool ignore_root_matrix;
};

}

mp5_implicit::mc_settings parse_mc_properties_json(const char* mc_parameters_json) {
    std::stringstream mc_json_stream;
    mc_json_stream << mc_parameters_json;

    namespace pt = boost::property_tree;
    pt::ptree mcparams_dict;


    // TODO(charles): find an alternativ to catch exceptions pt::json_parser::json_parser_error pt::ptree_bad_path
    // try{
    pt::read_json(mc_json_stream, mcparams_dict);

    REAL xmin = mcparams_dict.get<REAL>("box.xmin", NaN);
    REAL xmax = mcparams_dict.get<REAL>("box.xmax", NaN);
    REAL ymin = mcparams_dict.get<REAL>("box.ymin", NaN);
    REAL ymax = mcparams_dict.get<REAL>("box.ymax", NaN);
    REAL zmin = mcparams_dict.get<REAL>("box.zmin", NaN);
    REAL zmax = mcparams_dict.get<REAL>("box.zmax", NaN);

    int resolution = mcparams_dict.get<int>("resolution", -1);

    if ( isNaN(xmin) || isNaN(xmax) || isNaN(ymin) || isNaN(ymax) || isNaN(zmin) || isNaN(zmax) || resolution <= 2 ) {
        loger << "Error: missing or incorrect values in mc_parameters_json"<< std::endl;
        xmin = -1;
        xmax = 1;
        ymin = -1;
        ymax = 1;
        zmin = -1;
        zmax = 1;
        resolution = 28;
    }
    // loger << xmin << " " << xmax << " " << ymin << " " << ymax << " " << zmin << " " << zmax << " " << resolution << " " << std::endl;


    /*}catch(pt::json_parser::json_parser_error parse_exception){
        loger << "parse_exception"<< std::endl ;

    }catch(pt::ptree_bad_data bad_data_exception){
        loger << "bad_data_exception"<< std::endl ;

    }catch(pt::ptree_bad_path bad_path_exception){1
        loger << "bad_path_exception" << std::endl ;

    }catch(...){
        loger << "other_exception" << std::endl ;
    }*/


    mp5_implicit::mc_settings  mc_settings_from_json;  // settings
    mp5_implicit::bounding_box box = {xmin, xmax, ymin, ymax, zmin, zmax};  // {15,20,15,20,15,20};
    mc_settings_from_json.box = box;
    mc_settings_from_json.resolution = resolution;


    mc_settings_from_json.ignore_root_matrix = mcparams_dict.get<bool>("ignore_root_matrix", false);

    return mc_settings_from_json;
}



#include "../js_iteration_2/object_factory.hpp"

// void build_geometry(int resolution, char* mc_parameters_json, char* obj_name, REAL time){
void build_geometry(const char* shape_parameters_json, const char* mc_parameters_json) {
    if (!check_state_null())
        return;
    // loger << "In build_geometry obj_name : " << obj_name << std::endl;
    // loger << "Mc_params : " << mc_parameters_json << endl;
    // loger << "shape_json : " << shape_parameters_json << endl;

    mp5_implicit::mc_settings  mc_settings_from_json = parse_mc_properties_json(mc_parameters_json);

    bool use_metaball;
    std::string shape_parameters_json_str = std::string(shape_parameters_json);
    bool ignore_root_matrix = mc_settings_from_json.ignore_root_matrix;

    //unique_pointer<mp5_implicit::implicit_function> object = ...;

    mp5_implicit::implicit_function* object = object_factory(shape_parameters_json_str , use_metaball, ignore_root_matrix);

    // loger << "Leak-free : new" << std::endl;

    // dim_t resolution = 28;
    bool enableUvs = true;
    bool enableColors = true;

    // MarchingCubes mc(resolution, enableUvs, enableColors);
    _state.mc = new MarchingCubes(mc_settings_from_json.resolution, mc_settings_from_json.box, enableUvs, enableColors);
    // loger << "constructor called. " << _state.mc << std::endl;

    _state.mc -> isolation = 80.0/4*0;



    // ****************************
    // Does thismake things slower ?
    boost::multi_array<REAL, 2>  mcgrid_vectorized = _state.mc -> prepare_grid();  // 10.0
    _state.mc -> eval_shape(*object, mcgrid_vectorized);

     /*
     delete object;
     object = NULL;
     */
     if (use_metaball) {
         REAL metaball_time = 0;
         meta_balls(*_state.mc, 4, metaball_time, 1.0);
     }

    _state.mc->seal_exterior(-10000000.0);

    // loger << "balls added." << std::endl;

    const callback_t renderCallback;
    _state.mc->render_geometry(renderCallback);
    // loger << "MC executed" << std::endl;

    // loger << "map4" << std::endl;

    if (REPORT_STATS) {
    int mapctr = 0;
    for (auto& kv_pair : _state.mc->result_e3map) {
        if (0)
            loger << " [" << kv_pair.first << ':' << kv_pair.second << ']';
        mapctr++;
    }
    /*
    loger << "build_geometry(): ";
    loger << " e3Map: " << mapctr;
    loger << " Faces: " << _state.mc->result_faces.size()/3;
    loger << " Verts: " << _state.mc->result_verts.size()/3;
    loger << std::endl;
    */
    }

    // mc_settings_from_json.vertexresampleing.c
    // mc_settings_from_json.vertexresampleing.iterations
    int vresamp_iters = 0; //3;
    float c = 1.;
    for (int i=0; i < vresamp_iters; i++) {
        // result_verts is modified
        vertex_resampling(object, c, _state.mc -> result_verts, _state.mc->result_faces );
    }

    //delete object;
    object = NULL;
    gc_objects();

    _state.active = true;

    check_state();
    // loger << "MC:: v,f: " << _state.mc->result_verts.size() << " " << _state.mc->result_faces.size() << std::endl;
}
int get_f_size() {
    if (!check_state()) return -1;
    return _state.mc->result_faces.size()/3;
}
int get_v_size() {
    if (!check_state()) return -1;
    return _state.mc->result_verts.size()/3;
}
void get_v(REAL* v_out, int vcount) {
    if (!check_state())
        return;

    // int nf = get_f_size();
    // Vertices
    int ctr = 0;
    for ( std::vector<REAL>::iterator it=_state.mc->result_verts.begin(); it < _state.mc->result_verts.end(); it+=3 ) {
        for (int di=0; di < 3; di++) {
            v_out[ctr] = *(it + di);
            // if(ctr<3*3*3)
            //    loger << v_out[ctr] << " ";
            ctr++;
        }
    }
    // loger << std::endl;
    // assert nf*3 == ctr;
    if (vcount*3 != ctr)  loger << "sizes dont match: " << static_cast<float>(ctr)/3. << " " << vcount << std::endl;
}



void get_f(int* f_out, int fcount) {
    if (!check_state())
        return;
    // int nf = get_f_size();
    int ctr = 0;
    for ( std::vector<int>::iterator it=_state.mc->result_faces.begin(); it < _state.mc->result_faces.end(); it+=3 ) {
        for (int di=0; di < 3; di++) {
            f_out[ctr] = *(it + di);
            // if(ctr<3*3*3)
            //    loger << f_out[ctr] << " ";
            ctr++;
        }
    }
    if (fcount*3 != ctr)  loger << "sizes dont match: " << static_cast<float>(ctr)/3. << " " << fcount << std::endl;
    // loger << std::endl;
}


/* Data is already there, so why copy it? Also, keep it until next round. */
void* get_v_ptr() {
    if (!check_state())
        return NULL;
    return reinterpret_cast<void*>(_state.mc->result_verts.data());
}

void* get_f_ptr() {
    if (!check_state())
        return NULL;
    return reinterpret_cast<void*>(_state.mc->result_faces.data());
}


// int get_v_size(){};
// int get_f_size(){};
// void get_f(int*){};
// void get_v(REAL*){};
// void* get_f_ptr();
// void* get_v_ptr();
// void finish_geometry();

//  Can cause an exception (but not when null).
void finish_geometry() {
    if (!check_state())
        return;
    if (_state.mc == 0) {
        std::cerr << "Error: finish_geometry() before producing the shape()" << std::endl;
    }
    if (!_state.active) {
        // loger << "Cannot finish_geometry() while still active." << std::endl;
    } else {
        // loger << "_state.active " << _state.active << "  _state.mc " << _state.mc << std::endl;
    }
    // Dos not cause an exception if null. But it causes exception.
    delete _state.mc;
    _state.active = false;
    _state.mc = 0;
}

// information
void about() {
    loger << "Build 1" << std::endl;
    loger << __DATE__ << " " << __TIME__ << std::endl;
}











#include "../js_iteration_2/basic_data_structures.hpp"
#include "centroids_projection.cpp"

typedef boost::array<vectorized_vect::index, 2>  shape_t;

mp5_implicit::implicit_function* current_object = NULL;

vectorized_vect* current_x = NULL;
vectorized_vect* current_grad = NULL;
vectorized_scalar* current_f = NULL;


void set_object(const char* shape_parameters_json, bool ignore_root_matrix) {
    if(current_object != NULL){
        loger << "Error: You cannot unset() the object before a set_object(json)." << std::endl;
        return;
    }

    //loger << "before: current_object " << current_object << std::endl;

    std::string str = std::string(shape_parameters_json);
    bool dummy;
    current_object = object_factory(str , dummy, ignore_root_matrix);

    //loger << "after: current_object " << current_object << std::endl;
}
void unset_object() {
    if(current_object == NULL){
        loger << "Error: You cannot unset() the object before a set_object(json)." << std::endl;
        return;
    }

    //delete current_object;
    //current_object = NULL;
    gc_objects();
    current_object = NULL;
}

bool set_x(void* verts, int n) {
    if(current_x != NULL){
        loger << "Error: You set() before unset()ing the previous set()." << std::endl;
        return false;
    }
    if( n < 0 || n >= 10000) {
        loger << "Error: n is outside [0, 10000]." << std::endl;
        return false;
    }

    current_x = new vectorized_vect( shape_t{n, 3} );
    REAL* real_verts = reinterpret_cast<REAL*>(verts);
    for(int i = 0; i < n; i++) {
        (*current_x)[i][0] = real_verts[i*3 + 0];
        (*current_x)[i][1] = real_verts[i*3 + 1];
        (*current_x)[i][2] = real_verts[i*3 + 2];
        /*
        if(i < 10) {
            loger
                << (*current_x)[i][0] << " "
                << (*current_x)[i][1] << " "
                << (*current_x)[i][2] << " "
                << std::endl;
        }
        */
    }
    //loger << std::endl;
    current_f = new vectorized_scalar( shape_t{n}  );  // n x 0 (?)
    //loger << "warning: size may be n x 0:  " << current_f->shape()[0] << "x" << current_f->shape()[1] << std::endl;
    current_grad = new vectorized_vect( shape_t{n, 3}  );
    return true;
}
void unset_x() {
    if(current_x == NULL){
        loger << "Error: You cannot unset() before a set()." << std::endl;
        return;
    }

    delete current_x;
    delete current_f;
    delete current_grad;
    current_x = NULL;
    current_f = NULL;
    current_grad = NULL;
}


void calculate_implicit_values() {
    if(current_x == NULL || current_f == NULL || current_object == NULL) {
        loger << "Error: You need to set_x() and set_object() first." << std::endl;
        return;
    }

    current_object -> eval_implicit(*current_x, current_f);
}
void* get_values_ptr() {
    return current_f->data();
}
int get_values_size() {
    return current_f->shape()[0];
}

void calculate_implicit_gradients(bool normalize_and_invert) {
    if(current_x == NULL || current_grad == NULL || current_object == NULL) {
        loger << "Error: You need to set_x() and set_object() first." << std::endl;
        return;
    }

    current_object -> eval_gradient(*current_x, current_grad);
    if(normalize_and_invert) {
        for(auto it = current_grad->begin(); it < current_grad->end(); it++) {
            REAL x = (*it)[0];
            REAL y = (*it)[1];
            REAL z = (*it)[2];
            REAL norm = std::sqrt(x*x + y*y + z*z);

            REAL norm_factor;
            if (norm > 0.000000001) {
                norm_factor = -1.0 / norm;
            } else {
                norm_factor = -42.0; // how to avoid look black
            }

            (*it)[0] = x * norm_factor;
            (*it)[1] = y * norm_factor;
            (*it)[2] = z * norm_factor;
        }
    }


    /*
    loger << "calculated grad: "
        << (*current_grad)[0][0] << " "
        << (*current_grad)[0][1] << " "
        << (*current_grad)[0][2] << " "
        << "  calculated from x ="
        << (*current_x)[0][0] << " "
        << (*current_x)[0][1] << " "
        << (*current_x)[0][2] << " "
        << std::endl;
    */

}
void* get_gradients_ptr() {
    if(current_x == NULL || current_f == NULL || current_object == NULL) {
        loger << "Error: You need to set_x() and set_object() first." << std::endl;
        return NULL;
    }
    /*
    loger << "current_grad: "
        << (*current_grad)[0][0] << " "
        << (*current_grad)[0][1] << " "
        << (*current_grad)[0][2] << " "
        << std::endl;
    */
    return current_grad->data();
}
int get_gradients_size() {
    if(current_x == NULL || current_f == NULL || current_object == NULL) {
        loger << "Error: You need to set_x() and set_object() first." << std::endl;
        return 0;
    }

    return current_grad->shape()[0] * 3;
}


#include "timer.hpp"

/*int main() {

    timer t;
    t.stop();
    // MarchingCubes mc( dim_t resolution, bool enableUvs, bool enableColors );
    dim_t resolution = 28;  // 28;
    bool enableUvs = true;
    bool enableColors = true;
    MarchingCubes mc(resolution, enableUvs, enableColors);
    t.stop();

    int numblobs = 4;
    REAL subtract = (REAL)12.;
    REAL strength = (REAL)(1.2 / ( ( sqrt( numblobs ) - 1. ) / 4. + 1. ));

    REAL scale = 1;
    mc.addBall(0.5, 0.5, 0.5, strength, subtract, scale);

    const callback_t renderCallback;
    mc.render_geometry(renderCallback);
    t.stop();


    std::vector<REAL> verts3;
    std::vector<int> faces3;
    MarchingCubes& object = mc;

    // int normals_start = 0;
    mc.flush_geometry_queue(loger, mc.resultqueue_faces_start, mc.result_normals, verts3, faces3);

    t.stop();

    cout << resolution << endl;

    cout << endl;


    cout << "verts, faces: ";
    cout << mc.result_verts.size();
    cout << " ";
    cout << mc.result_faces.size();
    cout << endl;

    t.stop();

    // build_vf( verts3, faces3 );  // 21.3 msec using O3
    build_vf(  );  // 26 msec.


    t.stop();

    loger << "main();" << std::endl;
    return 0;
}
*/
