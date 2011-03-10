// $Id: mesh_ItPatch.hpp 1942 2011-01-20 00:53:45Z bordner $
// See LICENSE_CELLO file for license and copyright information

#ifndef MESH_IT_PATCH_HPP
#define MESH_IT_PATCH_HPP

/// @file     mesh_ItPatch.hpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @todo     Move creation of iterator to iterated object: Mesh::create_iter() (factor method)
/// @date     Tue Feb  1 16:46:01 PST 2011
/// @brief    [\ref Mesh] Declaration of the ItPatch iterator

class ItPatch {

  /// @class    ItPatch
  /// @ingroup  Mesh
  /// @brief    [\ref Mesh] Iterator over Patchs in a Patch

public: // interface

  /// Create an ItPatch object
  ItPatch (Mesh * mesh) throw ();

  /// Delete the ItPatch object
  ~ItPatch () throw ();
  
  /// Iterate through all local Patchs in the Mesh
  Patch * operator++ () throw();

  /// Return whether the iteration is complete
  bool done() const throw();


private: // attributes

  /// The Mesh being iterated over
  Mesh * mesh_;

  /// Index of the current local Patch plus 1, or 0 if between iterations
  /// Always in the range 0 <= index1_ <= number of local patchs
  size_t index1_;
};

#endif /* MESH_IT_PATCH_HPP */