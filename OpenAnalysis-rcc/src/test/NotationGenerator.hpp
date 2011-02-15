/*! \file

  \brief Generate a notation file from an interface.

  \authors Andy Stone
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef NOTATIONGENERATOR_HPP_
#define NOTATIONGENERATOR_HPP_

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterfaceDefault.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <iostream>

//! Output a new line and indent appropriatly
std::ostream &indt(std::ostream &os);

//! Increase the indentation on all following lines
std::ostream &pushIndt(std::ostream &os);

//! Decrease the indentation on all following lines
std::ostream &popIndt(std::ostream &os);

//! Generates notation files. Notation files consist of a series of clauses that
//! describe the contents of an intermediate representation.
class NotationGenerator
{
    public:
    //! The notations generated will be outputted to the passed output stream.
    NotationGenerator(OA::OA_ptr<OA::Alias::AliasIRInterface> ir,
                      std::ostream &codeStream);
 
    //! Output notation for a given procedure to the code stream.
    void generate(OA::ProcHandle proc);

    private:
    //! output a PROCEDURE clause
    void outputProcedure(OA::ProcHandle proc);

    //! output a FORMALS clause
    void outputFormals(OA::ProcHandle proc);

    //! output a statement. Statements may contain MEMREFEXPRS, PTRASSIGNPAIRS,
    //! CALLSITES, and PARAMBINDPTRASSIGNPAIRS clauses.
    void outputStmt(OA::StmtHandle stmt);
    
    //! output a MEMREFEXPRS clause
    void outputMemRefExprs(OA::StmtHandle stmt);

    //! output a PTRASSIGNPAIRS clause
    void outputPtrAssignPairs(OA::StmtHandle stmt);

    //! output a CALLSITES clause
    void outputCallSites(OA::StmtHandle stmt);

    //! iterate over all calls in a statement and output a PARAMBINDPTRASSIGNPAIRS
    //! for them if needed
    void outputParamBindPtrAssignPairs(OA::StmtHandle stmt);

    //! output a PARAMBINDPTRASSIGNPAIRS clause
    void outputParamBindPtrAssignPairs(OA::StmtHandle stmt,
         OA::CallHandle call);

    //! output a memory reference expression
    void outputMemRefExpr(OA::OA_ptr<OA::MemRefExpr> expr);
    void outputMemRefExpr(OA::NamedRef expr);
    void outputMemRefExpr(OA::UnnamedRef expr);
    void outputMemRefExpr(OA::UnknownRef expr);
    void outputMemRefExpr(OA::Deref expr);

    //! ouput a handle
    void outputHandle(OA::StmtHandle expr);
    void outputHandle(OA::MemRefHandle h);
    void outputHandle(OA::ProcHandle h);
    void outputHandle(OA::SymHandle h);
    void outputHandle(OA::CallHandle h);

    //! output a memory reference expression's type. Ex: DEF, USE, DEFUSE, etc.
    void outputMemRefType(OA::MemRefExpr &expr);

    //! output T if the value is true, otherwise F
    void outputTF(bool value);
    
    //! output 'full' or 'part' based on the passed value.
    //! used to output whether an expression has full or partial accuracy
    void outputAccuracy(bool hasFullAccuracy);

    //! after having outputted all statements, output all LOCATION clauses for
    //! a procedure.
    void outputLocations(OA::ProcHandle proc);

    //! output a LOCATION clause
    void outputLocation(OA::ProcHandle proc, OA::SymHandle sym);

    //vars
    OA::OA_ptr<OA::Alias::AliasIRInterface> mIR;
    std::ostream &mCodeStream;

    //! In order to determine what locations to output the notation generator
    //! must construct a set of all sym handles encountered while generating
    //! notations for a procedure.
    std::set<OA::SymHandle> mSymHandles;
};

#endif
