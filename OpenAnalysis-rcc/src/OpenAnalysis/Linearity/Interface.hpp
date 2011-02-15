/*! \file
  
  \brief Abstract interface that all Linearity analysis results must satisfy.

  \authors Luis Ramos
  \version $Id: Interface.hpp,v 1.4 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LINEARITYINTERFACE_H
#define LINEARITYINTERFACE_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
//#include <OpenAnalysis/IRInterface/LinearityInterface.hpp>

namespace OA {
  namespace Linearity {

class Interface {
public:
  Interface(){}
  virtual ~Interface(){}

  virtual OA_ptr<LinearityDepsSet> getDepsSet(OA_ptr<Location> v) = 0;

  virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;
};
  } // end of Linearity namespace
} // end of OA namespace

#endif
