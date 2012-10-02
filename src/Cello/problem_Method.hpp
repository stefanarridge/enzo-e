// See LICENSE_CELLO file for license and copyright information

/// @file     method_Method.hpp 
/// @author   James Bordner (jobordner@ucsd.edu) 
/// @date     Mon Jul 13 11:11:47 PDT 2009 
/// @brief    [\ref Method] Declaration for the Method class

#ifndef METHOD_METHOD_HPP
#define METHOD_METHOD_HPP

#ifdef CONFIG_USE_CHARM
class Method : public PUP::able 
#else
class Method 
#endif
{


  /// @class    Method
  /// @ingroup  Method
  /// @brief    [\ref Method] Interface to an application method / analysis / visualization function.

public: // interface

  /// Create a new Method
  Method () throw() {}

  /// Create a new Method
  Method(Parameters * parameters) throw()
  {};

  /// Destructor
  virtual ~Method() throw()
  {}

#ifdef CONFIG_USE_CHARM

  /// Charm++ PUP::able declarations
  PUPable_abstract(Method);

  /// CHARM++ Pack / Unpack function
  void pup (PUP::er &p)
  { TRACEPUP; PUP::able::pup(p); }

#endif

public: // virtual functions

  /// Apply the method to advance a block one timestep 

  virtual void compute_block
  ( FieldDescr * field_descr, Block * block) throw() = 0; 

protected: // functions


protected: // attributes

};

#endif /* METHOD_METHOD_HPP */
