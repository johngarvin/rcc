// Copyright (c) 2009 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: RccBasicInterface.h
//
// Methods to create BasicInterface objects wanted by the OA interface.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RCC_BASIC_INTERFACE_H
#define RCC_BASIC_INTERFACE_H

#include <string>

#include <include/R/R_Internal.h>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/OpBasicInterface.hpp>

namespace RccBasicInterface {

class ConstVal : public OA::ConstValBasicInterface {
public:
  ConstVal(SEXP e);

  bool operator< (OA::ConstValBasicInterface & other);
  bool operator==(OA::ConstValBasicInterface & other);
  bool operator!=(OA::ConstValBasicInterface & other);
  std::string toString();

private:
  SEXP m_sexp;
};

class Op : public OA::OpBasicInterface {
public:
  Op(SEXP e);

  bool operator< (OA::OpBasicInterface & other);
  bool operator==(OA::OpBasicInterface & other);
  bool operator!=(OA::OpBasicInterface & other);
  std::string toString();  

private:
  SEXP m_sexp;
};

  OA::OA_ptr<OA::ConstValBasicInterface> make_const_val(SEXP e);
  OA::OA_ptr<OA::OpBasicInterface> make_op(SEXP e);
}

#endif
