/*! \file
  
  \brief Need to initialize static variable

  \authors Michelle Strout
  \version $Id: Annotation.cpp,v 1.1.10.1 2005/09/14 16:53:40 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "Annotation.hpp"
#include <OpenAnalysis/Utils/OutputBuilderText.hpp>

namespace OA {

// initializing class static variable
OA_ptr<OutputBuilder> Annotation::sOutBuild;

Annotation::Annotation() {
    // FIXME: this could lead to performance problems but it
    // seems the only other option is to have some kind of
    // initialization method called before calling any output
    // methods or hardcode to a specific OutputBuilder subclass
    if (sOutBuild.ptrEqual(0)) sOutBuild = new OutputBuilderText;
}


} // end of OA namespace


