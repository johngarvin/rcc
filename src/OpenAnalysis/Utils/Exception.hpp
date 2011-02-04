// $Id: Exception.hpp,v 1.5 2004/11/19 19:21:52 mstrout Exp $
// -*-C++-*-
// * BeginRiceCopyright *****************************************************
// 
// Copyright (c) 2002-2004, Rice University <br>
// Copyright (c) 2004, University of Chicago <br>  
// All rights reserved. <br>
// See ../../../Copyright.txt for details. <br>
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *

// Best seen in 120-column wide window (or print in landscape mode).
//--------------------------------------------------------------------------------------------------------------------
// This file is part of Mint.
// Arun Chauhan (achauhan@cs.rice.edu), Dept of Computer Science, Rice University, 2001.
//--------------------------------------------------------------------------------------------------------------------

#ifndef Exception_H
#define Exception_H

#include <iostream>

namespace OA {

/*! Exception is the abstract base class for all exceptions that 
    can be thrown by OA programs. */
class Exception {
public:
  Exception () {}
  virtual ~Exception () {}
  virtual void report (std::ostream&) const = 0;
};

} // end of OA namespace
#endif

