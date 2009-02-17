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

// File: RccBasicInterface.cc
//
// Methods to create BasicInterface objects wanted by the OA interface.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "RccBasicInterface.h"

#include <support/StringUtils.h>

using namespace OA;

namespace RccBasicInterface {

ConstVal::ConstVal(SEXP e) : m_sexp(e) { }

  bool ConstVal::operator< (ConstValBasicInterface & other_iface) {
    ConstVal & other = dynamic_cast<ConstVal &>(other_iface);
    return (m_sexp < other.m_sexp);
  }

  bool ConstVal::operator==(ConstValBasicInterface & other_iface) {
    ConstVal & other = dynamic_cast<ConstVal &>(other_iface);
    return (m_sexp == other.m_sexp);
  }

  bool ConstVal::operator!=(ConstValBasicInterface & other_iface) {
    ConstVal & other = dynamic_cast<ConstVal &>(other_iface);
    return (m_sexp != other.m_sexp);
  }

  std::string ConstVal::toString() {
    return to_string(m_sexp);
  }

  Op::Op(SEXP e) : m_sexp(e) { }

  bool Op::operator< (OpBasicInterface & other_iface) {
    Op & other = dynamic_cast<Op &>(other_iface);
    return (m_sexp < other.m_sexp);
  }

  bool Op::operator==(OpBasicInterface & other_iface) {
    Op & other = dynamic_cast<Op &>(other_iface);
    return (m_sexp == other.m_sexp);
  }

  bool Op::operator!=(OpBasicInterface & other_iface) {
    Op & other = dynamic_cast<Op &>(other_iface);
    return (m_sexp != other.m_sexp);
  }

  std::string Op::toString() {
    return to_string(m_sexp);
  }

OA_ptr<ConstValBasicInterface> make_const_val(SEXP e) {
  OA_ptr<ConstValBasicInterface> retval; retval = new ConstVal(e);
  return retval;
}
  
OA_ptr<OpBasicInterface> make_op(SEXP e) {
  OA_ptr<OpBasicInterface> retval; retval = new Op(e);
  return retval;
}

}
