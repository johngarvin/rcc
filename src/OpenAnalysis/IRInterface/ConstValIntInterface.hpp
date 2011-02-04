/*! \file
  
  \brief Abstract basic constant integer interface.
  
  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ConstValIntInterface.hpp,v 1.2 2004/11/19 19:21:50 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ConstValIntInterface_h
#define ConstValIntInterface_h

#include <string>

namespace OA {

class ConstValIntInterface : public virtual ConstValBasicInterface {
public:
  ConstValIntInterface() {}
  virtual ~ConstValIntInterface() {}
  
  virtual bool isaInteger() const = 0;
  virtual int getIntegerVal() const = 0; // FIXME: THROW EXCEPTION?
  
  /*
    virtual bool operator== (ConstValIntInterface& other) = 0;
    virtual bool operator!= (ConstValIntInterface& other) = 0;
    virtual std::string toString() = 0;
  */
};
  
} // end of namespace OA

#endif 
