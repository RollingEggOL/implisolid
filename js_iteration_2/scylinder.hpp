#pragma once
#include "basic_data_structures.hpp"
namespace mp5_implicit {

class scylinder : public transformable_implicit_function {

protected:
    REAL r; REAL h;
    REAL x; REAL y; REAL z;
    REAL* u; REAL* w; REAL c_len;
    REAL radius_u; REAL radius_v;
    REAL* transf_matrix;
    REAL* inv_transf_matrix;

public:
    scylinder(REAL matrix12[12]){
        this->r = 0.5;
        this->h = 1;

        this->x = 0.;
        this->y = 0.;
        this->z = 0.;

        this->transf_matrix = new REAL [12];
        this->inv_transf_matrix = new REAL [12];

        for (int i=0; i<12; i++){
            transf_matrix[i] = matrix12[i];
        }

        InvertMatrix(this->transf_matrix, this->inv_transf_matrix);
        my_assert(this->integrity_invariant(), "");
    }
    scylinder(REAL U[3], REAL W[3], REAL r_u, REAL r_v, REAL clen){
      this->radius_u = r_u;
      this->radius_v = r_v;
      this->c_len = clen;
      this->r = 0.5;
      this->h = 1;
      this->x = 0;
      this->y = 0;
      this->z = 0;

      this->w = new REAL[3];
      this->u = new REAL[3];

      for (int i=0; i<3; i++){
        this->u[i] = U[i];
        this->w[i] = W[i];
      }

        this->transf_matrix = new REAL [12];
        this->inv_transf_matrix = new REAL [12];
        for (int i=0; i<12; i++){
          if(i==0 || i==5 || i==10){
            this->transf_matrix[i] = 1;
            this->inv_transf_matrix[i] = 1;
          }
          else{
            this->transf_matrix[i] = 0;
            this->inv_transf_matrix[i] = 0;
          }
        }
    }

    scylinder(REAL U[3], REAL W[3], REAL r_u, REAL r_v, REAL clen, REAL center_x, REAL center_y, REAL center_z){
        this->radius_u = r_u;
        this->radius_v = r_v;
        this->c_len = clen;
        this->r = 0.5;
        this->h = 1;
        this->x = center_x;
        this->y = center_y;
        this->z = center_z;

        this->w = new REAL[3];
        this->u = new REAL[3];

        for (int i=0; i<3; i++){
          this->u[i] = U[i];
          this->w[i] = W[i];
        }

        this->transf_matrix = new REAL [12];
        this->inv_transf_matrix = new REAL [12];
        for (int i=0; i<12; i++){
          if(i==0 || i==5 || i==10){
            this->transf_matrix[i] = 1;
            this->inv_transf_matrix[i] = 1;
          }
          else{
            this->transf_matrix[i] = 0;
            this->inv_transf_matrix[i] = 0;
          }
        }

    }


    virtual void rotate(const REAL angle, const vectorized_vect axis) const {
      REAL ca = cos(angle);
      REAL sa = sin(angle);
      REAL norm = sqrt(axis[0][0]*axis[0][0] + axis[0][1]*axis[0][1] + axis[0][2]*axis[0][2]);
      REAL a1 = axis[0][0]/norm;
      REAL a2 = axis[0][1]/norm;
      REAL a3 = axis[0][2]/norm;

      REAL rotation[12];
      rotation[0] = ca + a1*a1*(1.-ca);
      rotation[1] = a1*a2*(1.-ca) - a3*sa;
      rotation[2] = a1*a3*(1.-ca) + a2*sa;
      rotation[3] = 0.;
      rotation[4] = a1*a2*(1.-ca) + a3*sa;
      rotation[5] = ca + a2*a2*(1.-ca);
      rotation[6] = a2*a3*(1.-ca) - a1*sa;
      rotation[7] = 0.;
      rotation[8] = a1*a3*(1.-ca) - a2*sa;
      rotation[9] = a2*a3*(1.-ca) + a1*sa;
      rotation[10] = ca + a3*a3*(1.-ca);
      rotation[11] = 0.;

      Matrix_Matrix_Product(this->transf_matrix, rotation);

      InvertMatrix(this->transf_matrix, this->inv_transf_matrix);

    }

    virtual void move(const vectorized_vect direction) const{
      this->transf_matrix[3] += direction[0][0];
      this->transf_matrix[7] += direction[0][1];
      this->transf_matrix[11] += direction[0][2];
      InvertMatrix(this->transf_matrix, this->inv_transf_matrix);

    }
    virtual void resize(const REAL ratio) const{
      for (int i=0; i<12; i++){
        if(i==3 || i==7 || i==11){
        }
        else{
        this->transf_matrix[i] *= ratio;
        }
      }
      InvertMatrix(this->transf_matrix, this->inv_transf_matrix);
    }

    virtual void eval_implicit(const vectorized_vect& x, vectorized_scalar* f_output) const {
        my_assert(assert_implicit_function_io(x, *f_output), "");
        my_assert(this->integrity_invariant(), "");

        vectorized_vect x_copy = x;

        Matrix_Vector_Product(this->inv_transf_matrix, x_copy);

        const REAL r2 = squared(this->r);

        int output_ctr=0;

        auto i = x_copy.begin();
        auto e = x_copy.end();
        for(; i<e; i++, output_ctr++){
          REAL t0 = ((*i)[0]-this->x)*w[0] + ((*i)[1]-this->y)*w[1] + ((*i)[2]-this->z)*w[2];
          REAL t1 = c_len - t0;
          REAL r_ = radius_u - sqrt(((*i)[0] - w[0]*t0 - this->x)*((*i)[0] - w[0]*t0 - this->x)
            + ((*i)[1] - w[1]*t0- this->y)*((*i)[1] - w[1]*t0 - this->y) +((*i)[2] - w[2]*t0 - this->z)*((*i)[2] - w[2]*t0 - this->z));

          (*f_output)[output_ctr] = min(t0,min(t1,r_));

        }
    }
    virtual void eval_gradient(const vectorized_vect& x, vectorized_vect* output) const {

        vectorized_vect x_copy = x;

        Matrix_Vector_Product(this->inv_transf_matrix, x_copy);

        int output_ctr=0;
        auto i = x_copy.begin();
        auto e = x_copy.end();
        const REAL r2 = squared(this->r);
        for(; i!=e; i++, output_ctr++){

          if((*i)[2]-this->z >= this->h/2 || (*i)[2]-this->z <= -this->h/2){

            (*output)[output_ctr][0] = -2. * ((*i)[0]-this->x);
            (*output)[output_ctr][1] = -2. * ((*i)[1]-this->y);
            (*output)[output_ctr][2] = 0.;

          }
          else {
            (*output)[output_ctr][0] = 0.;
            (*output)[output_ctr][1] = 0.;
            (*output)[output_ctr][2] = -1.;
          }

        }
    }
    bool integrity_invariant() const {
      if(this->r < MEAN_PRINTABLE_LENGTH || this->h < MEAN_PRINTABLE_LENGTH)
        return false;
      else
        return true;
    }

    virtual mp5_implicit::bounding_box  get_boundingbox() const {
        REAL max_size = norm_squared(r, h, 0.0);
        return mp5_implicit::bounding_box{-max_size, max_size, -max_size, max_size, -max_size, max_size};
    }
};

}
