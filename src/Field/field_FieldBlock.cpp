// $Id: field_FieldBlock.cpp 1388 2010-04-20 23:57:46Z bordner $
// See LICENSE_CELLO file for license and copyright information

/// @file     field_FieldBlock.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     Wed May 19 18:17:50 PDT 2010
/// @todo     Avoid void * arithmetic in allocate_array() if possible
/// @todo     hand-check allocate()
/// @brief    Implementation of the FieldBlock class

#include "assert.h"

#include "field_FieldBlock.hpp"

FieldBlock::FieldBlock() throw()
  : field_descr_(0),
    array_(0),
    field_values_(),
    ghosts_allocated_(true)
{
  for (int i=0; i<3; i++) {
    dimensions_[i] = 0.0;
    box_lower_[i] = 0.0;
    box_upper_[i] = 0.0;
  }
}

//----------------------------------------------------------------------

FieldBlock::~FieldBlock() throw()
{
}

//----------------------------------------------------------------------

FieldBlock::FieldBlock ( const FieldBlock & field_block ) throw ()
{
}

//----------------------------------------------------------------------

FieldBlock & FieldBlock::operator= ( const FieldBlock & field_block ) throw ()
{
  return *this;
}

//----------------------------------------------------------------------

void FieldBlock::dimensions( int * nx, int * ny, int * nz ) const throw()
{
  *nx = dimensions_[0];
  *ny = dimensions_[1];
  *nz = dimensions_[2];
}

//----------------------------------------------------------------------

void * FieldBlock::field_values ( int id_field ) throw (std::out_of_range)
{
  return field_values_.at(id_field);
}

//----------------------------------------------------------------------
	
void FieldBlock::index_range
(
 int * lower_x, int * lower_y, int *lower_z,
 int * upper_x, int * upper_y, int *upper_z ) const throw ()
{
}

//----------------------------------------------------------------------

void FieldBlock::box_extent
(
 double * lower_x, double * lower_y, double *lower_z,
 double * upper_x, double * upper_y, double *upper_z ) const throw ()
{
  *lower_x = box_lower_[0];
  *lower_y = box_lower_[1];
  *lower_z = box_lower_[2];

  *upper_x = box_upper_[0];
  *upper_y = box_upper_[1];
  *upper_z = box_upper_[2];
}

//----------------------------------------------------------------------

void FieldBlock::cell_width( double * hx, double * hy, double * hz ) const throw ()
{
}

//----------------------------------------------------------------------

FieldDescr * FieldBlock::field_descr() throw ()
{
  return field_descr_;
}

//----------------------------------------------------------------------

void FieldBlock::clear
(
 double value,
 int id_field_first,
 int id_field_last_plus ) throw()
{
  if ( array_allocated() ) {
  } else {
    // WARNING/ERROR clearing non-allocated array
  }
}

//----------------------------------------------------------------------

void FieldBlock::allocate_array() throw()
{
  if ( ! array_allocated() ) {
    
    int padding   = field_descr_->padding();
    int alignment = field_descr_->alignment();

    int array_size = 0;

    int  gx,gy,gz;
    bool cx,cy,cz;

    for (int id_field=0; id_field<field_descr_->field_count(); id_field++) {

      // Adjust memory usage due to ghosts if needed

      if ( ghosts_allocated() ) {
	field_descr_->ghosts(id_field,&gx,&gy,&gz);
      } else {
	gx = gy = gz =0;
      }

      // Adjust memory usage due to field centering if needed

      field_descr_->centering(id_field,&cx,&cy,&cz);

      // Determine array size

      int nx = dimensions_[0] + (cx ? 0 : 1) + 2*gx;
      int ny = dimensions_[1] + (cy ? 0 : 1) + 2*gy;
      int nz = dimensions_[2] + (cz ? 0 : 1) + 2*gz;

      // Determine field precision size

      int precision_size = field_descr_->precision_size(id_field);

      // Increment array_size, including padding and alignment adjustment

      array_size += precision_size*(nx*ny*nz) + padding;
      int alignment_adjust = alignment_adjust_((long long) array_size,alignment);
      array_size += alignment_adjust;

    }

    // Adjust for possible initial misalignment

    array_size += alignment - 1;

    // Allocate array

    array_ = new void * [array_size];

    // Initialize 

    void * field_begin = array_ + alignment_adjust_((long long)array_,alignment);

    int field_offset = 0;

    for (int id_field=0; id_field<field_descr_->field_count(); id_field++) {

      field_values_.push_back(field_begin + field_offset);

      // Adjust memory usage due to ghosts if needed


      if ( ghosts_allocated() ) {
	field_descr_->ghosts(id_field,&gx,&gy,&gz);
      } else {
	gx = gy = gz = 0;
      }

      // Adjust memory usage due to field centering if needed

      field_descr_->centering(id_field,&cx,&cy,&cz);

      // Determine array size

      int nx = dimensions_[0] + (cx ? 0 : 1) + 2*gx;
      int ny = dimensions_[1] + (cy ? 0 : 1) + 2*gy;
      int nz = dimensions_[2] + (cz ? 0 : 1) + 2*gz;

      // Determine field precision size

      int precision_size = field_descr_->precision_size(id_field);

      // Increment field_begin

      field_offset += precision_size*(nx*ny*nz) + padding;
      int alignment_adjust = alignment_adjust_(field_offset,alignment);
      field_offset += alignment_adjust;
    }

    assert (array_size >= (field_offset + alignment - 1));

  } else {
    // WARNING/ERROR allocating array when already allocated
  }
}

//----------------------------------------------------------------------

void FieldBlock::deallocate_array () throw()
{
  if ( array_allocated() ) {
  } else {
    // WARNING/ERROR: deallocating preallocated array
  }
}

//----------------------------------------------------------------------

bool FieldBlock::array_allocated() const throw()
{
  return array_ != 0;
}

//----------------------------------------------------------------------
	
bool FieldBlock::ghosts_allocated() const throw ()
{
  return ghosts_allocated_;
}

//----------------------------------------------------------------------

void FieldBlock::allocate_ghosts() throw ()
{
  if (! ghosts_allocated() ) {
  }
}

//----------------------------------------------------------------------

void FieldBlock::deallocate_ghosts() throw ()
{
  if (ghosts_allocated() ) {
  }
}

//----------------------------------------------------------------------

void FieldBlock::split
(
 bool split_x, bool split_y, bool split_z,
 FieldBlock ** new_field_blocks ) throw ()
{
}

//----------------------------------------------------------------------

FieldBlock * FieldBlock::merge
(
 bool merge_x, bool merge_y, bool merge_z,
 FieldBlock ** field_blocks ) throw ()
{
  FieldBlock * new_field_block = 0;
  return new_field_block;
}

//----------------------------------------------------------------------
	
FieldDescr * FieldBlock::read
(
 DiskFile   * disk_file, 
 FieldDescr * field_descr ) throw ()
{
  FieldDescr * new_field_descr = 0;

  return new_field_descr;
}

//----------------------------------------------------------------------

void FieldBlock::write
(
 DiskFile   * disk_file,
 FieldDescr * field_descr ) const throw ()
{
  
}

//----------------------------------------------------------------------

void FieldBlock::set_dimensions(int nx, int ny, int nz) throw()
{
  if ( ! array_allocated() ) {
    dimensions_[0] = nx;
    dimensions_[1] = ny;
    dimensions_[2] = nz;
  } else {
    // WARNING/ERROR: changing dimensions of allocated array
  }
}

//----------------------------------------------------------------------

void FieldBlock::set_field_values 
(
 int    id_field, 
 void * field_values) throw()
{
}

//----------------------------------------------------------------------

void FieldBlock::set_field_descr(FieldDescr * field_descr) throw()
{
  field_descr_ = field_descr;
}

//----------------------------------------------------------------------

void FieldBlock::set_box_extent
(
 double lower_x, double lower_y, double lower_z,
 double upper_x, double upper_y, double upper_z ) throw ()

{
  box_lower_[0] = lower_x;
  box_lower_[1] = lower_y;
  box_lower_[2] = lower_z;

  box_upper_[0] = upper_x;
  box_upper_[1] = upper_y;
  box_upper_[2] = upper_z;
}

//----------------------------------------------------------------------
