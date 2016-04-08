// See LICENSE_CELLO file for license and copyright information

/// @file     problem_Classname.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     2015-11-06
/// @brief    Implementation of the InitialTrace class

#include "problem.hpp"

int InitialTrace::id0_[MAX_NODE_SIZE] = {-1};

//----------------------------------------------------------------------

void InitialTrace::pup (PUP::er &p)
{
  TRACEPUP;
  // NOTE: change this function whenever attributes change
  p | field_;
  p | mpp_;
  p | dx_;
  p | dy_;
  p | dz_;
}

//======================================================================

void InitialTrace::enforce_block
 ( Block            * block, 
   const FieldDescr * field_descr,
   const ParticleDescr * particle_descr,
   const Hierarchy  * hierarchy
   ) throw()
{
  const int in = CkMyPe() % MAX_NODE_SIZE;

  if (id0_[in] == -1) id0_[in] = CkMyPe();

  Field    field    (block->data()->field());
  Particle particle (block->data()->particle());

  int ip = 0;

  if (mpp_ == 0.0) {
    ip = uniform_placement_ (block,field,particle);
  } else {
    ip = density_placement_ (block,field,particle);
  }
}

//----------------------------------------------------------------------

int InitialTrace::uniform_placement_ 
(Block * block, Field field, Particle particle)
{
  // Insert new tracer particles, one per cell

  // ... get number of cells

  int mx,my,mz;
  field.dimensions (0,&mx,&my,&mz);

  int nx,ny,nz;
  field.size (&nx,&ny,&nz);
  
  // ... insert uninitialized tracer particles

  const int it = particle.type_index("trace");

  particle.insert_particles (it,nx/dx_*ny/dy_*nz/dz_);

  // Initialize particle positions

  // ... get block extents

  double xm,ym,zm;
  double xp,yp,zp;
  block->lower(&xm,&ym,&zm);
  block->upper(&xp,&yp,&zp);

  const double xl = xp - xm;
  const double yl = yp - ym;
  const double zl = zp - zm;

  const double hx = xl / nx;
  const double hy = yl / ny;
  const double hz = zl / nz;

  const int ia_id = particle.attribute_index(it,"id");
  const int ia_x = particle.attribute_index (it,"x");
  const int ia_y = particle.attribute_index (it,"y");
  const int ia_z = particle.attribute_index (it,"z");

  const int np = particle.batch_size();

  int ib=0;  // batch counter
  int ip=0;  // particle counter 

  int64_t * id = 0;
  float * xa = 0;
  float * ya = 0;
  float * za = 0;

  const int in = CkMyPe() % MAX_NODE_SIZE;

  const int dp  = particle.stride(it,ia_x);
  const int did = particle.stride(it,ia_id);

  for (int iz=0; iz<nz; iz+=dz_) {
    float z = (mz>1) ? zm + (iz + 0.5)*hz : 0.0;
    for (int iy=0; iy<ny; iy+=dy_) {
      float y = (my>1) ? ym + (iy + 0.5)*hy : 0.0;

      for (int ix=0; ix<nx; ix+=dx_) {
	float x = (mx>1) ? xm + (ix + 0.5)*hx : 0.0;

	// ... if new batch then update position arrays
	if (ip % np == 0) {
	  id = (int64_t *) particle.attribute_array (it,ia_id,ib);
	  xa = (float *)   particle.attribute_array (it,ia_x,ib);
	  ya = (float *)   particle.attribute_array (it,ia_y,ib);
	  za = (float *)   particle.attribute_array (it,ia_z,ib);
	}

	
	id[ip*did] = id0_[in];
	xa[ip*dp] = x;
	ya[ip*dp] = y;
	za[ip*dp] = z;

	ip++;

	if (ip == np) {
	  ip=0;
	  ib++;
	}

	id0_[in] += CkNumPes();
    
      }
    }
      
  }
  return ip;
}

//----------------------------------------------------------------------

int InitialTrace::density_placement_ 
(Block * block, Field field, Particle particle)
{
  // Get density field d

  int did;
  int mx,my,mz;
  int nx,ny,nz;
  int gx,gy,gz;


  did = field.field_id( (field_ == "") ? "density" : field_);

  field.dimensions  (did,&mx,&my,&mz);
  field.size           (&nx,&ny,&nz);
  field.ghost_depth (did,&gx,&gy,&gz);

  double * density = (double *) field.values(did);

  // Get cell widths hx,hy,hz

  Data * data = block->data();

  double xm,ym,zm;
  double xp,yp,zp;
  double hx,hy,hz;

  data->lower(&xm,&ym,&zm);
  data->upper(&xp,&yp,&zp);
  field.cell_width(xm,xp,&hx, ym,yp,&hy, zm,zp,&hz);

  const double xl = xp - xm;
  const double yl = yp - ym;
  const double zl = zp - zm;

  const int rank = block->rank();
  if (rank < 3) hz = 1.0;
  if (rank < 2) hy = 1.0;

  // ... create vector of integrated mass in the Block

  std::vector<double> ms,xs,ys,zs;
  ms.resize(nx*ny*nz + 1);
  xs.resize(nx*ny*nz + 1);
  ys.resize(nx*ny*nz + 1);
  zs.resize(nx*ny*nz + 1);

  ms[0] = 0.0;
  xs[0] = xm ;
  ys[0] = ym ;
  zs[0] = zm ;
  int ims=1;

  for (int iz=gz; iz<nz+gz; iz++) {
    for (int iy=gy; iy<ny+gy; iy++) {
      for (int ix=gx; ix<nx+gx; ix++) {
	int id = ix + mx*(iy + my*iz);
	double m = density[id] *(hx*hy*hz);
	ms[ims] = ms[ims-1] + m;
	xs[ims-1] = xm + (ix-gx)*hx;
	ys[ims-1] = ym + (iy-gy)*hy;
	zs[ims-1] = zm + (iz-gz)*hz;
	++ ims;
      }
    }
  }
  const double rmax = ms[nx*ny*nz];

  // ... compute number of particles to place in the block

  const int np = ms[nx*ny*nz] / mpp_;

  // ... insert uninitialized tracer particles

  const int it = particle.type_index("trace");

  CkPrintf ("Inserting %d particles\n",np);
  particle.insert_particles (it,np);

  const int ia_id = particle.attribute_index(it,"id");
  const int ia_x = particle.attribute_index (it,"x");
  const int ia_y = particle.attribute_index (it,"y");
  const int ia_z = particle.attribute_index (it,"z");

  const int npb = particle.batch_size();

  int ib=0;  // batch counter
  int ipb=0;  // particle / batch counter 

  int64_t * id = 0;
  float * xa = 0;
  float * ya = 0;
  float * za = 0;

  const int in = CkMyPe() % MAX_NODE_SIZE;

  const int ps  = particle.stride(it,ia_x);
  const int ids = particle.stride(it,ia_id);

  for (int ip=0; ip<np; ip++) {

    double r = rmax*rand()/RAND_MAX;

    int imin=0;
    int imax=nx*ny*nz;
    int ims;
    do {
      ims = (imin+imax)/2;
      if (ms[ims] <= r) {
    	imin = ims;
      } else if ( r < ms[ims+1]) {
    	imax = ims;
      }
    } while (imax-imin > 1);
    // for (ims=0; ims<nx*ny*nz-1; ims++) {
    //   if (ms[ims] <= r && r <= ms[ims+1]) break;
    // }
    ims = imin;
    ASSERT6( "InitialTrace",
	     "[%d %d %d] %f <= %f < %f",imin,ims,imax,ms[ims],r,ms[ims+1],
	     (ms[ims] <= r && r < ms[ims+1]));

    //    CkPrintf ("%d %f <= %f < %f\n",ims,ms[ims],r,ms[ims+1]);

    // assert (ims < 0 || ims >= nx*ny*nz) ||
    // ;

    // randomize within cell
    double x = xs[ims] + hx*rand()/(RAND_MAX+1.0);
    double y = ys[ims] + hy*rand()/(RAND_MAX+1.0);
    double z = zs[ims] + hz*rand()/(RAND_MAX+1.0);
    
    // ... if new batch then update position arrays
    if (ipb % npb == 0) {
      id = (int64_t *) particle.attribute_array (it,ia_id,ib);
      xa = (float *)   particle.attribute_array (it,ia_x,ib);
      ya = (float *)   particle.attribute_array (it,ia_y,ib);
      za = (float *)   particle.attribute_array (it,ia_z,ib);
    }

    id[ipb*ids] = id0_[in];
    xa[ipb*ps] = x;
    ya[ipb*ps] = y;
    za[ipb*ps] = z;

    ipb++;

    if (ipb == npb) {
      ipb=0;
      ib++;
    }

    id0_[in] += CkNumPes();
    
  }
  return np;
}

//----------------------------------------------------------------------
