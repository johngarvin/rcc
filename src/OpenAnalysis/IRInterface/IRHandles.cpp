/*! \file
  
  \authors Andy Stone
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "IRHandles.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

using namespace OA;
using namespace std;

namespace OA {

ostream &operator<<(
    ostream &os,
    const ProcHandle &proc)
{
    os << "proc(" << proc.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const StmtHandle &stmt)
{
    os << "stmt(" << stmt.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const StmtLabel &stmLbl)
{
    os << "stmtLbl(" << stmLbl.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const SymHandle &sym)
{
    os << "sym(" << sym.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const ExprHandle &expr)
{
    os << "expr(" << expr.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const MemRefHandle &memref)
{
    os << "memref(" << memref.hval() << ")";
    return os;
}
ostream &operator<<(
    ostream &os,
    const CallHandle &call)
{
    os << "call(" << call.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const OpHandle &op)
{
    os << "op(" << op.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const ConstSymHandle &constSym)
{
    os << "constSym(" << constSym.hval() << ")";
    return os;
}

ostream &operator<<(
    ostream &os,
    const ConstValHandle &constVal)
{
    os << "constVal(" << constVal.hval() << ")";
    return os;
}

IRHandle::IRHandle() : h(0) { }
IRHandle::IRHandle(irhandle_t h_) : h(h_) { }
ProcHandle::ProcHandle() { }
StmtHandle::StmtHandle() { }
StmtLabel::StmtLabel() { }
SymHandle::SymHandle() { }
ExprHandle::ExprHandle() { }
MemRefHandle::MemRefHandle() { }
CallHandle::CallHandle() { }
OpHandle::OpHandle() { }
ConstSymHandle::ConstSymHandle() { }
ConstValHandle::ConstValHandle() { }
ConstValHandle::ConstValHandle(const ConstValHandle& x) : ExprHandle(x) { }
ConstValHandle::ConstValHandle(irhandle_t h_) : ExprHandle(h_) { }
LeafHandle::LeafHandle() { }

IRHandle::~IRHandle() { }
ProcHandle::~ProcHandle() { }
StmtHandle::~StmtHandle() { }
StmtLabel::~StmtLabel() { }
SymHandle::~SymHandle() { }
ExprHandle::~ExprHandle() { }
MemRefHandle::~MemRefHandle() { }
CallHandle::~CallHandle() { }
OpHandle::~OpHandle() { }
ConstSymHandle::~ConstSymHandle() { }
ConstValHandle::~ConstValHandle() { }
LeafHandle::~LeafHandle() { }

ProcHandleIterator::ProcHandleIterator() { }
ProcHandleIterator::~ProcHandleIterator() { }
ExprHandleIterator::ExprHandleIterator() { }
ExprHandleIterator::~ExprHandleIterator() { }
StmtHandleIterator::StmtHandleIterator() { }
StmtHandleIterator::~StmtHandleIterator() { }
MemRefHandleIterator::MemRefHandleIterator() { }
MemRefHandleIterator::~MemRefHandleIterator() { }
CallHandleIterator::CallHandleIterator() { }
CallHandleIterator::~CallHandleIterator() { }
SymHandleIterator::SymHandleIterator() { }
SymHandleIterator::~SymHandleIterator() { }
IRHandlesIRInterface::~IRHandlesIRInterface() { }

void IRHandle::output(OA::IRHandlesIRInterface& ir) const {
}

void ProcHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void StmtHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void SymHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void ExprHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void MemRefHandle ::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void CallHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void OpHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void ConstSymHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void ConstValHandle::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->outputIRHandle(*this, ir);
}

void LeafHandle::output(OA::IRHandlesIRInterface& ir) const {
}

void StmtLabel::output(OA::IRHandlesIRInterface& ir) const {
}

} // namespace OA

