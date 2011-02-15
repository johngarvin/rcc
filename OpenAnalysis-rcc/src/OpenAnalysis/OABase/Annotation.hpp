/*! \file
  
  \brief All analysis results have this as their base class.

  \authors Michelle Strout
  \version $Id: Annotation.hpp,v 1.1.10.1 2005/09/14 16:53:40 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef Annotation_H
#define Annotation_H

#include <iostream>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>

namespace OA {

class OutputBuilder;
class IRHandlesIRInterface;

class Annotation {
  protected:
    // output builder for all annotations
    static OA_ptr<OutputBuilder> sOutBuild;

  public:
    Annotation();

    virtual ~Annotation() {}

    static void configOutput(OA_ptr<OutputBuilder> ob) { sOutBuild = ob; }

    // methods all annotations must provide
    virtual void output(IRHandlesIRInterface& ir) const = 0;
};

} // end of OA namespace

#endif

