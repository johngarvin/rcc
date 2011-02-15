/*! \file
  
  \brief Testdriver for MemRefExpr hierarchy

  \authors Michelle Strout
  \version $Id: mretest.cpp,v 1.3 2005/01/18 21:13:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <iostream>
#include "MemRefExpr.hpp"
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

class TestIRInterface : public virtual OA::IRHandlesIRInterface {

public:
  TestIRInterface() {}
  virtual ~TestIRInterface() {}

  //--------------------------------------------------------
  // create a string for the given handle, should be succinct
  // and there should be no newlines
  //--------------------------------------------------------  
  std::string toString(const OA::ProcHandle h) { return ""; }
  std::string toString(const OA::StmtHandle h) 
    { std::ostringstream os; os << "StmtHandle(" << h.hval() << ")"; return os.str(); } 
  std::string toString(const OA::ExprHandle h) { return ""; }
  std::string toString(const OA::CallHandle h) { return ""; }
  std::string toString(const OA::OpHandle h) { return ""; }
  std::string toString(const OA::MemRefHandle h) { return ""; }
  std::string toString(const OA::SymHandle h)
    { std::ostringstream os; os << "SymHandle(" << h.hval() << ")"; return os.str(); } 
  std::string toString(const OA::ConstSymHandle h) { return ""; }
  std::string toString(const OA::ConstValHandle h) { return ""; }

  //--------------------------------------------------------
  // dumps that do pretty prints
  //--------------------------------------------------------  

  //! Given a memory reference, pretty-print it to the output stream os.
  void dump(OA::MemRefHandle h, std::ostream& os) {}

  //! Given a statement, pretty-print it to the output stream os.
  void dump(OA::StmtHandle stmt, std::ostream& os) {}

  
  //--------------------------------------------------------
  // Other
  //--------------------------------------------------------  
  // FIXME: should eventually get rid of this
  void currentProc(OA::ProcHandle p) {}
};


int main() {
    OA::OA_ptr<TestIRInterface> ir;
    ir = new TestIRInterface;


    std::list<OA::OA_ptr<OA::MemRefExpr> > mList; 

    OA::StmtHandle stmt1(1);
    OA::StmtHandle stmt2(2);
    OA::SymHandle sym1(1);
    OA::SymHandle sym2(2);
    OA::MemRefHandle mhandle1(1);
    OA::MemRefHandle mhandle2(2);
    std::string string1("string1");
    std::string string2("string2");

    // build a ref of each kind and dump it
    std::cout << "===== NamedRef =====" << std::endl;
    OA::OA_ptr<OA::NamedRef> namedRef1;
    namedRef1 = new OA::NamedRef(OA::MemRefExpr::USE, sym1);
    std::cout << "== dump ==" << std::endl;
    namedRef1->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== output ==" << std::endl;
    //namedRef1->output(*ir);
    //std::cout << std::endl;
    //std::cout << "isaNamed() = " << namedRef1->isaNamed() << std::endl;
    //std::cout << "isaUnknown() = " << namedRef1->isaUnknown() << std::endl;
    //std::cout << "====================" << std::endl << std::endl;
    mList.push_back(namedRef1);


    std::cout << "===== NamedRef =====" << std::endl;
    OA::OA_ptr<OA::NamedRef> namedRef2;
    namedRef2 = new OA::NamedRef(OA::MemRefExpr::USE, sym2);
    std::cout << "== dump ==" << std::endl;
    namedRef2->dump(std::cout,ir);
    std::cout << std::endl;    
    //std::cout << "== output ==" << std::endl;
    //namedRef2->output(*ir);
    //std::cout << std::endl;
    //std::cout << "isaNamed() = " << namedRef2->isaNamed() << std::endl;
    //std::cout << "isaUnknown() = " << namedRef2->isaUnknown() << std::endl;
    //std::cout << "====================" << std::endl << std::endl;
    mList.push_back(namedRef2);

    
    std::cout << "===== NamedRef =====" << std::endl;
    OA::OA_ptr<OA::NamedRef> namedRef3;
    namedRef3 = new OA::NamedRef(OA::MemRefExpr::USE, sym1);
    std::cout << "== dump ==" << std::endl;
    namedRef3->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== output ==" << std::endl;
    //namedRef3->output(*ir);
    //std::cout << std::endl;
    //std::cout << "isaNamed() = " << namedRef3->isaNamed() << std::endl;
    //std::cout << "isaRefOp() = " << namedRef3->isaRefOp() << std::endl;
    //std::cout << "====================" << std::endl << std::endl;
    mList.push_back(namedRef3);

    
    std::cout << "===== UnnamedRef =====" << std::endl;
    OA::ExprHandle hm=OA::ExprHandle((OA::irhandle_t)stmt1);
    OA::OA_ptr<OA::UnnamedRef> unnamedRef;
    unnamedRef = new OA::UnnamedRef(OA::MemRefExpr::DEF, hm);
    std::cout << "== dump ==" << std::endl;
    unnamedRef->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== output ==" << std::endl;
    //unnamedRef->output(*ir);
    //std::cout << std::endl;
    //std::cout << "isaNamed() = " << unnamedRef->isaNamed() << std::endl;
    //std::cout << "isaUnnamed() = " << unnamedRef->isaUnnamed() << std::endl;
    //std::cout << "==========================================" 
    //          << std::endl << std::endl;
    mList.push_back(unnamedRef);
    

    std::cout << "===== UnKnownRef =====" << std::endl;
    OA::OA_ptr<OA::MemRefExpr::MemRefExpr> unknownRef;
    unknownRef = new OA::UnknownRef(OA::MemRefExpr::USE);
    std::cout << "== dump ==" << std::endl << std::endl;
    unknownRef->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== output ==" << std::endl << std::endl;
    //unknownRef->output(*ir);
    //std::cout << std::endl;
    //std::cout << "isaNamed() = " << unknownRef->isaNamed() << std::endl;
    //std::cout << "isaUnknown() = " << unknownRef->isaUnknown() << std::endl;
    //std::cout << "isaRefOp() = " << unknownRef->isaRefOp() << std::endl;
    //std::cout << "==========================================" 
    //          << std::endl << std::endl;
    mList.push_back(unknownRef);

    //*****************************************************************
    // RefOp decorator
    //*****************************************************************
    OA::OA_ptr<OA::Deref> derefMRE3;
    derefMRE3 = new OA::Deref(OA::MemRefExpr::USE, namedRef3, 2);
    mList.push_back(derefMRE3);
    OA::OA_ptr<OA::Deref> derefMRE1;
    derefMRE1 = new OA::Deref(OA::MemRefExpr::USE, namedRef1, 2);
    mList.push_back(derefMRE1);


    std::cout << "===== Deref =====" << std::endl << std::endl;
    std::cout << "== derefMRE3 dump ==" << std::endl << std::endl;
    derefMRE3->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== output ==" << std::endl << std::endl;
    //derefMRE3->output(*ir);
    //std::cout << std::endl;
    std::cout << "== derefMRE1 dump ==" << std::endl << std::endl;
    derefMRE1->dump(std::cout,ir);
    std::cout << std::endl;
 

    //derefMRE3->setPartialAccuracy();
    OA::OA_ptr<OA::SubSetRef> subset_mre;
    OA::OA_ptr<OA::MemRefExpr> nullMRE;
    OA::OA_ptr<OA::MemRefExpr> subset_derefMRE3, subset_derefMRE1;

    subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
    subset_derefMRE3 = subset_mre->composeWith(derefMRE3->clone());
    mList.push_back(subset_derefMRE3);
    

    std::cout << "Now should be false:" << std::endl << std::endl;
    derefMRE3->dump(std::cout,ir);
    std::cout << "(derefMRE3 == derefMRE1) = " << (*derefMRE3==*derefMRE1)
              << std::endl << std::endl;

    //derefMRE1->setPartialAccuracy();
    subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
    subset_derefMRE1 = subset_mre->composeWith(derefMRE1->clone());
    mList.push_back(subset_derefMRE1);
    

    OA::OA_ptr<OA::IdxAccess> idxAccess3;
    idxAccess3 = new OA::IdxAccess(OA::MemRefExpr::USE,subset_derefMRE3,5);
    OA::OA_ptr<OA::IdxAccess> idxAccess1;
    idxAccess1 = new OA::IdxAccess(OA::MemRefExpr::USE,subset_derefMRE1,5);
    OA::OA_ptr<OA::IdxAccess> idxAccessX;
    idxAccessX = new OA::IdxAccess(OA::MemRefExpr::USE,subset_derefMRE1,6);
    std::cout << "== idxAccess3 dump ==" << std::endl << std::endl;
    idxAccess3->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== idxAccess1 dump ==" << std::endl << std::endl;
    idxAccess1->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== idxAccessX dump ==" << std::endl << std::endl;
    idxAccessX->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== idxAccessX output ==" << std::endl << std::endl;
    //idxAccessX->output(*ir);
    //std::cout << std::endl;

    mList.push_back(idxAccess3);
    mList.push_back(idxAccess1);
    mList.push_back(idxAccessX);

    
    // IdxExprAccess
    OA::OA_ptr<OA::IdxExprAccess> idxExprAccess3;

    idxExprAccess3 
       = new OA::IdxExprAccess(OA::MemRefExpr::USE,subset_derefMRE3,mhandle1);
    OA::OA_ptr<OA::IdxExprAccess> idxExprAccess1;
    idxExprAccess1 
       = new OA::IdxExprAccess(OA::MemRefExpr::USE,subset_derefMRE1,mhandle1);
    OA::OA_ptr<OA::IdxExprAccess> idxExprAccessX;
    idxExprAccessX 
       = new OA::IdxExprAccess(OA::MemRefExpr::USE,subset_derefMRE1,mhandle2);
    std::cout << "== idxExprAccess3 dump ==" << std::endl << std::endl;
    idxExprAccess3->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== idxExprAccess1 dump ==" << std::endl << std::endl;
    idxExprAccess1->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== idxExprAccessX dump ==" << std::endl << std::endl;
    idxExprAccessX->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== idxExprAccessX output ==" << std::endl << std::endl;
    //idxExprAccessX->output(*ir);
    //std::cout << std::endl;
    mList.push_back(idxExprAccess3);
    mList.push_back(idxExprAccess1);
    mList.push_back(idxExprAccessX);

    
    //FieldAccess
    OA::OA_ptr<OA::FieldAccess> fieldAccess3;
    fieldAccess3 = new OA::FieldAccess(OA::MemRefExpr::USE,subset_derefMRE3,string1);
    OA::OA_ptr<OA::FieldAccess> fieldAccess1;
    fieldAccess1 = new OA::FieldAccess(OA::MemRefExpr::USE,subset_derefMRE1,string1);
    OA::OA_ptr<OA::FieldAccess> fieldAccessX;
    fieldAccessX = new OA::FieldAccess(OA::MemRefExpr::USE,subset_derefMRE1,string2);
    std::cout << "== fieldAccess3 dump ==" << std::endl << std::endl;
    fieldAccess3->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== fieldAccess1 dump ==" << std::endl << std::endl;
    fieldAccess1->dump(std::cout,ir);
    std::cout << std::endl;
    std::cout << "== fieldAccessX dump ==" << std::endl << std::endl;
    fieldAccessX->dump(std::cout,ir);
    std::cout << std::endl;
    //std::cout << "== fieldAccessX output ==" << std::endl << std::endl;
    //fieldAccessX->output(*ir);
    //std::cout << std::endl;
    mList.push_back(fieldAccess3);
    mList.push_back(fieldAccess1);
    mList.push_back(fieldAccessX);



    // MemRefExpr1
    // MemRefExpr(this=0xaa11830, type = USE)
    // Deref(this=0xaa11830, mMemRefExpr=MemRefExpr(this=0xa8c59b0, type = USE) 
    //   FieldAccess(this=0xa8c59b0, mMemRefExpr=MemRefExpr(this=0xa8c58e0, type = USE) 
    //     SubSetRef(this=0xa8c58e0, mMemRefExpr=MemRefExpr(this=0xa8c58c0, type = USE)  
    //       NamedRef(this=0xa8c58c0, mSymHandle.hval()=171680520)))
    //         mNumDerefs = 1 )

    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE4;
    subSetRefMRE4 = subset_mre->composeWith(namedRef1->clone());
    OA::OA_ptr<OA::FieldAccess> fieldAccess4;
    fieldAccess4 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE4,string1);
    OA::OA_ptr<OA::Deref> derefMRE4;
    derefMRE4 = new OA::Deref(OA::MemRefExpr::USE, fieldAccess4, 1);
    std::cout << "== derefMRE4 dump ==" << std::endl << std::endl;
    derefMRE4->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(derefMRE4);


    
    // MemRefExpr2
    // MemRefExpr(this=0xaa21ff8, type = USE)
    //   SubSetRef(this=0xaa21ff8, mMemRefExpr=MemRefExpr(this=0xa8c02e0, type = USE)  
    //     Deref(this=0xa8c02e0, mMemRefExpr=MemRefExpr(this=0xa8c0258, type = USE)  
    //       FieldAccess(this=0xa8c0258, mMemRefExpr=MemRefExpr(this=0xa8c01c8, type = USE) 
    //         SubSetRef(this=0xa8c01c8, mMemRefExpr=MemRefExpr(this=0xa8c01a8, type = USE)  
    //           NamedRef(this=0xa8c01a8, mSymHandle.hval()=171680520)
    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE5;
    subSetRefMRE5 = subset_mre->composeWith(derefMRE4->clone());
    std::cout << "== subSetRefMRE5 dump ==" << std::endl << std::endl;
    subSetRefMRE5->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(subSetRefMRE5);


    // MemRefExpr3
    // MemRefExpr(this=0xaa11950, type = USE) 
    //   Deref(this=0xaa11950, mMemRefExpr=MemRefExpr(this=0xa8c0258, type = USE)
    //     FieldAccess(this=0xa8c0258, mMemRefExpr=MemRefExpr(this=0xa8c01c8, type = USE) 
    //       SubSetRef(this=0xa8c01c8, mMemRefExpr=MemRefExpr(this=0xa8c01a8, type = USE) 
    //         NamedRef(this=0xa8c01a8, mSymHandle.hval()=171680520)
    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE6;
    subSetRefMRE6 = subset_mre->composeWith(namedRef1->clone());
    OA::OA_ptr<OA::FieldAccess> fieldAccess6;
    fieldAccess6 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE6,string1);
    OA::OA_ptr<OA::Deref> derefMRE6;
    derefMRE6 = new OA::Deref(OA::MemRefExpr::USE, fieldAccess6, 1);
    std::cout << "== derefMRE6 dump ==" << std::endl << std::endl;
    derefMRE6->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(derefMRE6);

   
    // MemRefExpr4
    // MemRefExpr(this=0xaa22010, type = DEF)
    //   SubSetRef(this=0xaa22010, mMemRefExpr=MemRefExpr(this=0xa86e590, type = USE)
    //     Deref(this=0xa86e590, mMemRefExpr=MemRefExpr(this=0xa373fd0, type = USE) 
    //       NamedRef(this=0xa373fd0, mSymHandle.hval()=171669608)
    //         mNumDerefs = 1 ))
    // same as subset_derefMRE1.
    std::cout << "== subset_derefMRE1 dump ==" << std::endl << std::endl;
    subset_derefMRE1->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(subset_derefMRE1);

    
    // MemRefExpr5
    // SubSetRef( USE, 
    //   Deref( USE, 
    //     FieldAccess( USE, 
    //       Deref( USE, 
    //         NamedRef( USE, SymHandle("node")), 1), basic_arc), 1))
    OA::OA_ptr<OA::FieldAccess> fieldAccess7;
    fieldAccess7 = new OA::FieldAccess(OA::MemRefExpr::USE,derefMRE1,string1);
    OA::OA_ptr<OA::Deref> derefMRE7;
    derefMRE7 = new OA::Deref(OA::MemRefExpr::USE, fieldAccess7, 1);
    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE7;
    subSetRefMRE7 = subset_mre->composeWith(derefMRE7->clone());
    std::cout << "== subSetRefMRE7 dump ==" << std::endl << std::endl;
    subSetRefMRE7->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(subSetRefMRE7);

    // MemRefExpr6
    // FieldAccess( DEF, 
    //   SubSetRef( USE, 
    //     Deref( USE, 
    //       NamedRef( USE, SymHandle("new")), 1)), head)
    OA::OA_ptr<OA::FieldAccess> fieldAccess8;
    fieldAccess8 = new OA::FieldAccess(OA::MemRefExpr::USE,subset_derefMRE1,string1);
    std::cout << "== fieldAccess8 dump ==" << std::endl << std::endl;
    fieldAccess8->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(fieldAccess8);


    // MemRefExpr7
    // FieldAccess( USE, 
    //   SubSetRef( USE, 
    //     Deref( USE, 
    //       FieldAccess( USE, 
    //         Deref( USE, 
    //           NamedRef( USE, SymHandle("arcout")), 1), head), 1)), firstout)

    OA::OA_ptr<OA::FieldAccess> fieldAccess9;
    fieldAccess9 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE7,string1);
    std::cout << "== fieldAccess9 dump ==" << std::endl << std::endl;
    fieldAccess9->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(fieldAccess9);


    // MemRefExpr8
    // FieldAccess( DEF, 
    //   SubSetRef( USE, 
    //     Deref( USE, 
    //       FieldAccess( USE, 
    //         SubSetRef( USE, 
    //           Deref( USE, 
    //             FieldAccess( USE, 
    //               SubSetRef( USE, 
    //                 Deref( USE, 
    //                   FieldAccess( USE, 
    //                     Deref( USE, NamedRef( USE, 
    //                       SymHandle("arcout")), 1), head), 1)), firstout), 1)), head), 1)), arc_tmp)
    OA::OA_ptr<OA::Deref> derefMRE10;
    derefMRE10 = new OA::Deref(OA::MemRefExpr::USE, fieldAccess9->clone(), 1);
    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE10;
    subSetRefMRE10 = subset_mre->composeWith(derefMRE10->clone());
    OA::OA_ptr<OA::FieldAccess> fieldAccess10;
    fieldAccess10 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE10,string1);
    derefMRE10 = new OA::Deref(OA::MemRefExpr::USE, fieldAccess10->clone(), 1);
    subSetRefMRE10 = subset_mre->composeWith(derefMRE10->clone());
    fieldAccess10 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE10,string1);
    std::cout << "== fieldAccess10 dump ==" << std::endl << std::endl;
    fieldAccess10->dump(std::cout,ir);
    std::cout << std::endl; 
    mList.push_back(fieldAccess10);


    

    // MemRefExpr9
    // IdxExprAccess( USE,
    //   SubSetRef( USE,
    //     NamedRef( USE,
    //       SymHandle("perm"))), MemRefHandle("i"))
    OA::OA_ptr<OA::IdxExprAccess> idxExprAccess11;
    idxExprAccess11 = new OA::IdxExprAccess(OA::MemRefExpr::USE,subSetRefMRE6,mhandle1);
    std::cout << "== idxExprAccess11 dump ==" << std::endl << std::endl;
    idxExprAccess11->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(idxExprAccess11);


    // MemRefExpr10
    // FieldAccess( USE, 
    //   SubSetRef( USE, 
    //     Deref( USE, 
    //       IdxExprAccess( USE, 
    //         SubSetRef( USE, 
    //           NamedRef( USE, SymHandle("perm"))), MemRefHandle("1")), 1)), cost)
    OA::OA_ptr<OA::Deref> derefMRE11;
    derefMRE11 = new OA::Deref(OA::MemRefExpr::USE, idxExprAccess11->clone(), 1);
    OA::OA_ptr<OA::MemRefExpr> subSetRefMRE11;
    subSetRefMRE11 = subset_mre->composeWith(derefMRE11->clone());
    OA::OA_ptr<OA::FieldAccess> fieldAccess11;
    fieldAccess11 = new OA::FieldAccess(OA::MemRefExpr::USE,subSetRefMRE11->clone(),string1);
    std::cout << "== fieldAccess11 dump ==" << std::endl << std::endl;
    fieldAccess11->dump(std::cout,ir);
    std::cout << std::endl;
    mList.push_back(fieldAccess11);

    std::cout << std::endl << std::endl << std::endl << std::endl;

    std::cout << "================================================"
              << std::endl;
    
    std::cout << "Start comparision operators <,==" << std::endl;

    std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator mIter1;
    std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator mIter2;

    for(mIter1 = mList.begin(); mIter1 != mList.end(); ++mIter1) {
        OA::OA_ptr<OA::MemRefExpr> mreA = *mIter1; 
        for(mIter2 = mList.begin(); mIter2 != mList.end(); ++mIter2) {
            std::cout << "==============================================" 
                      << std::endl;
            OA::OA_ptr<OA::MemRefExpr> mreB = *mIter2;
            std::cout << std::endl << "\tmre A = ";
            mreA->output(*ir);
            std::cout << std::endl;
            std::cout << std::endl << "\tmre B = ";
            mreB->output(*ir);
            std::cout << std::endl;
            std::cout << "A < B            :" << (mreA<mreB) << std::endl;
            std::cout << "B < A            :" << (mreB<mreA) << std::endl;
            std::cout << "A == B           :" << (mreA==mreB) << std::endl;
            std::cout << "B == A           :" << (mreB==mreA) << std::endl;
            std::cout << "(A==B) == (B==A) :"
                      << ((mreA == mreB) == (mreB == mreA))
                      << std::endl;
            
            int AlessthanB = (mreA<mreB);
            int BlessthanA = (mreB<mreA);
            int AequaltoB  = (mreA==mreB);

            if((AlessthanB + BlessthanA + AequaltoB) != 1) {
                std::cout << "more than one of (A<B B<A and A==B) are true" 
                          << std::endl;
                assert(0);
            } 


            if((mreA == mreB) != (mreB == mreA)) {
                std::cout << "(mreA == mreB) != (mreB == mreA)"
                          << std::endl;
                assert(0);
            }

            std::cout << "==============================================="
                      << std::endl;
        }
    }
 
    std::cout << "End comparision operators <,==" << std::endl;
    std::cout << "================================================"
              << std::endl;

    return 0;
}



