/*! \file
  
  \brief Configuration options that need to be known throughout OA.

  \authors Michelle Strout
  \version $Id: config.cpp,v 1.2 2005/01/18 21:13:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OutputBuilderText.hpp>

namespace OA {

// set up default output builder for all annotations
OutputBuilderText defaultOutputBuilder(std::cout);

}

