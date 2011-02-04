/*! \file
    
  \brief Functions to load notation files

  \authors Andy Stone
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LOADNOTATION_HPP_
#define LOADNOTATION_HPP_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include "SubsidiaryIR.hpp"
#include "StrToHandle.hpp"
#include "oa-tab.hpp"
#include "testSubIR.hpp"
using namespace std;

//! Load a notation file into a SubsidiaryIR
OA::OA_ptr<SubsidiaryIR> loadNotation(string file);

#endif
