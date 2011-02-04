/*! \file
     Tests the SubsidiaryIR class.
     The calls in here are what should occur if the file Input/testSubIR.oa
     were being parsed.

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SubsidiaryIR.hpp"
#include "TestAliasIRInterface.hpp"
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>

#define DLINE cout << __LINE__ << endl;

using namespace OA;

void testSubsidiaryIR()
{
    OA_ptr<SubsidiaryIR> ir;
    ir = new SubsidiaryIR;
    
    // int main()
    ir->addProcString(ProcHandle(1), "main");
    ir->addSymString(SymHandle(2), "main");
    ir->startProc(ProcHandle(1), SymHandle(2));
    
    // add some locations
    OA_ptr<Location> loc;

    // int x; 
    loc  = new NamedLoc( SymHandle(3), true );
    ir->addSymString(SymHandle(3), "x");
    ir->addLocation( loc );

    // int *p; 
    loc  = new NamedLoc( SymHandle(4), true );
    ir->addSymString(SymHandle(4), "p");
    ir->addLocation( loc );
  
    // g, all other symbols visible to this procedure
    loc  = new NamedLoc( SymHandle(5), false );
    ir->addSymString(SymHandle(5), "g");
    ir->addLocation( loc );

    // add some stmts

    // x = g;
    ir->addStmtString(StmtHandle(6),"x = g;");
    ir->addStmt(StmtHandle(6));

    ir->addMemRefString(MemRefHandle(7),"x_1");
    OA_ptr<MemRefExpr> mre;
    OA_ptr<MemRefExpr> mre1;
    OA_ptr<MemRefExpr> mre2;
    mre = new NamedRef(MemRefExpr::DEF, SymHandle(3));
    ir->addMemRefExpr(StmtHandle(6), MemRefHandle(7), mre);

    ir->addMemRefString(MemRefHandle(8),"g_1");
    mre = new NamedRef(MemRefExpr::USE, SymHandle(5));
    ir->addMemRefExpr(StmtHandle(6), MemRefHandle(8), mre);
    
    // p = &x
    ir->addStmtString(StmtHandle(9),"p = &x;");
    ir->addStmt(StmtHandle(9));

    ir->addMemRefString(MemRefHandle(10),"p_1");
    mre1 = new NamedRef(MemRefExpr::DEF, SymHandle(4));
    ir->addMemRefExpr(StmtHandle(9), MemRefHandle(10), mre1);

    ir->addMemRefString(MemRefHandle(11),"&x_1");
    mre2 = new NamedRef(true, true, MemRefExpr::USE, SymHandle(3));
    ir->addMemRefExpr(StmtHandle(9), MemRefHandle(11), mre2);

    ir->addPtrAssignPair(StmtHandle(9), mre1, mre2);


    // char *cptr1, **cptr2, c, d;
    loc  = new NamedLoc( SymHandle(12), true );
    ir->addSymString(SymHandle(12), "cptr1");
    ir->addLocation( loc );
    loc  = new NamedLoc( SymHandle(13), true );
    ir->addSymString(SymHandle(13), "cptr2");
    ir->addLocation( loc );
    loc  = new NamedLoc( SymHandle(14), true );
    ir->addSymString(SymHandle(14), "c");
    ir->addLocation( loc );
    loc  = new NamedLoc( SymHandle(15), true );
    ir->addSymString(SymHandle(15), "d");
    ir->addLocation( loc );
    
    // cptr1 = &c;
    ir->addStmtString(StmtHandle(16),"cptr1 = &c;");
    ir->addStmt(StmtHandle(16));

    ir->addMemRefString(MemRefHandle(17),"cptr1_1");
    mre1 = new NamedRef(MemRefExpr::DEF, SymHandle(12));
    ir->addMemRefExpr(StmtHandle(16), MemRefHandle(17), mre1);

    ir->addMemRefString(MemRefHandle(18),"&c_1");
    mre2 = new NamedRef(true, true, MemRefExpr::USE, SymHandle(14));
    ir->addMemRefExpr(StmtHandle(16), MemRefHandle(18), mre2);

    ir->addPtrAssignPair(StmtHandle(16), mre1, mre2);

    // cptr2 = &cptr1;
    ir->addStmtString(StmtHandle(19),"cptr2 = &cptr1;");
    ir->addStmt(StmtHandle(19));

    ir->addMemRefString(MemRefHandle(20),"cptr2_1");
    mre1 = new NamedRef(MemRefExpr::DEF, SymHandle(13));
    ir->addMemRefExpr(StmtHandle(19), MemRefHandle(20), mre1);

    ir->addMemRefString(MemRefHandle(21),"&cptr1_1");
    mre2 = new NamedRef(true, true, MemRefExpr::USE, SymHandle(12));
    ir->addMemRefExpr(StmtHandle(19), MemRefHandle(21), mre2);

    ir->addPtrAssignPair(StmtHandle(19), mre1, mre2);

    // c = 'h';
    ir->addStmtString(StmtHandle(22),"c = 'h';");
    ir->addStmt(StmtHandle(22));

    ir->addMemRefString(MemRefHandle(23),"c_1");
    mre = new NamedRef(MemRefExpr::DEF, SymHandle(14));
    ir->addMemRefExpr(StmtHandle(22), MemRefHandle(23), mre);

    // d = **cptr2;
    ir->addStmtString(StmtHandle(24),"d = **cptr2;");
    ir->addStmt(StmtHandle(24));

    ir->addMemRefString(MemRefHandle(25),"d_1");
    mre = new NamedRef(MemRefExpr::DEF, SymHandle(15));
    ir->addMemRefExpr(StmtHandle(24), MemRefHandle(25), mre);

    ir->addMemRefString(MemRefHandle(26),"cptr2_2");
    mre = new NamedRef(MemRefExpr::USE, SymHandle(13));
    ir->addMemRefExpr(StmtHandle(24), MemRefHandle(26), mre);

    ir->addMemRefString(MemRefHandle(27),"*cptr2_1");
    mre = new NamedRef(MemRefExpr::USE, SymHandle(13));
    mre = new Deref(false, true, MemRefExpr::USE, mre, 1);
    ir->addMemRefExpr(StmtHandle(24), MemRefHandle(27), mre);

    ir->addMemRefString(MemRefHandle(28),"**cptr2_1");
    mre = new NamedRef(MemRefExpr::USE, SymHandle(13));
    mre = new Deref(false, true, MemRefExpr::USE, mre, 1);
    mre = new Deref(false, true, MemRefExpr::USE, mre, 1);
    ir->addMemRefExpr(StmtHandle(24), MemRefHandle(28), mre);

    // double A[10];
    loc  = new NamedLoc( SymHandle(29), true );
    ir->addSymString(SymHandle(29), "A");
    ir->addLocation( loc );

    // double *B;
    loc  = new NamedLoc( SymHandle(30), true );
    ir->addSymString(SymHandle(30), "B");
    ir->addLocation( loc );
    
    // struct { double lf; int d; } S;
    loc  = new NamedLoc( SymHandle(31), true );
    ir->addSymString(SymHandle(31), "S");
    ir->addLocation( loc );
    
    // B = &S.lf;
    ir->addStmtString(StmtHandle(32),"B = &S.lf;");
    ir->addStmt(StmtHandle(32));

    ir->addMemRefString(MemRefHandle(33),"B_1");
    mre1 = new NamedRef(MemRefExpr::DEF, SymHandle(30));
    ir->addMemRefExpr(StmtHandle(32), MemRefHandle(33), mre1);

    ir->addMemRefString(MemRefHandle(34),"&S.lf");
    mre2 = new NamedRef(true, false, MemRefExpr::USE, SymHandle(31));
    ir->addMemRefExpr(StmtHandle(32), MemRefHandle(34), mre2);

    ir->addPtrAssignPair(StmtHandle(32), mre1, mre2);

    // foo( S.lf, B, c)
    ir->addStmtString(StmtHandle(38),"foo( S.lf, B, c);");
    ir->addStmt(StmtHandle(38));

//MEMREFEXPRS
    ir->addMemRefString(MemRefHandle(40),"S.lf_1");
    mre = new NamedRef(false, false, MemRefExpr::USE, SymHandle(31));
    ir->addMemRefExpr(StmtHandle(38), MemRefHandle(40), mre);

    ir->addMemRefString(MemRefHandle(42),"B_2");
    mre = new NamedRef(true, true, MemRefExpr::USE, SymHandle(30));
    ir->addMemRefExpr(StmtHandle(38), MemRefHandle(42), mre);

    ir->addMemRefString(MemRefHandle(44),"c_2");
    mre = new NamedRef(true, true, MemRefExpr::USE, SymHandle(14));
    ir->addMemRefExpr(StmtHandle(38), MemRefHandle(44), mre);
//CALLSITES
    ir->addCallString(CallHandle(1),"foo( S.lf, B, c);");
    ir->addSymString(SymHandle(45), "foo");
    mre = new NamedRef(MemRefExpr::USE, SymHandle(45));
    ir->addCallSite(StmtHandle(38), CallHandle(1), mre);
//PARAMBINDPTRASSIGNPAIRS
    mre = new NamedRef(true, true, MemRefExpr::USE, SymHandle());
    ir->addParamBindPtrAssignPair(CallHandle(30), 2, mre);
    mre = new NamedRef(true, true, MemRefExpr::USE, SymHandle());
    ir->addParamBindPtrAssignPair(CallHandle(14), 3, mre);

    // another function declaration
    // void foo( double p1, double *p2, char &p3 ) { ... }
    ir->addProcString(ProcHandle(2),"foo");
    ir->startProc(ProcHandle(2),SymHandle(45));
//FORMALS
    ir->addSymString(SymHandle(46), "p1");
    ir->addFormal(1, SymHandle(46));
    ir->addSymString(SymHandle(47), "p2");
    ir->addFormal(2, SymHandle(46));
    ir->addSymString(SymHandle(48), "p3");
    ir->addFormal(3, SymHandle(46));

    loc  = new NamedLoc( SymHandle(46), true );
    ir->addLocation( loc );
    loc  = new NamedLoc( SymHandle(47), true );
    ir->addLocation( loc );
    loc  = new NamedLoc( SymHandle(48), true );
    ir->addLocation( loc );


    
    // dump the subsidiary IR
    //ir->dump(std::cout);

    // actually do the alias analysis using the input data
    OA_ptr<TestAliasIRInterface> aliasIR;
    aliasIR = new TestAliasIRInterface(ir);
    ir->output(aliasIR);

// call FIAlias
    /*
    OA_ptr<Alias::ManagerFIAlias> aliasman;
    aliasman = new Alias::ManagerFIAlias(aliasIR);
    OA_ptr<Alias::EquivSets> alias
        = aliasman->performAnalysis(aliasIR->getProcIterator());
    alias->dump(std::cout, aliasIR);
    */

    exit(0);
}

