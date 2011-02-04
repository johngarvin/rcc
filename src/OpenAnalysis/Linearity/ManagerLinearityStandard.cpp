// standard headers

#ifdef NO_STD_CHEADERS
# include <stdlib.h>
# include <string.h>
# include <assert.h>
#else
# include <cstdlib>
# include <cstring>
# include <cassert>
using namespace std; // For compatibility with non-std C headers
#endif

#include <iostream>
using std::ostream;
using std::endl;
using std::cout;
using std::cerr;
#include <list>
#include <set>
#include <map>

// Local headers
#include "ManagerLinearityStandard.hpp"

//using namespace OA;
namespace OA {
  namespace Linearity {

static bool debug = false;

ManagerLinearity::ManagerLinearity(OA_ptr<LinearityIRInterface> _ir) : mIR(_ir)
{
   mSolver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward,*this);
}

OA_ptr<DataFlow::DataFlowSet> ManagerLinearity::initializeTop()
{
    OA_ptr<LinearityMatrix>  retval;
    retval = new LinearityMatrix;
    return retval;
}
     
OA_ptr<DataFlow::DataFlowSet> ManagerLinearity::initializeBottom()
{
    OA_ptr<LinearityMatrix>  retval;
    retval = new LinearityMatrix;
    return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerLinearity::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
    OA_ptr<LinearityMatrix>  retval;
    retval = new LinearityMatrix;
    return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerLinearity::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
    OA_ptr<LinearityMatrix>  retval;
    retval = new LinearityMatrix;
    return retval;
}

OA_ptr<Linearity::LinearityMatrix> ManagerLinearity::performAnalysis(
                                        ProcHandle proc,
                                        OA_ptr<CFG::CFGInterface> cfg,
                                        OA_ptr<Alias::Interface> alias,
                                        OA_ptr<DataFlow::ParamBindings> paramBind, DataFlow::DFPImplement algorithm)
{
    mProc = proc;

    mLM = new LinearityMatrix;

    mCFG = cfg;

    mAlias = alias;

    mParamBind = paramBind;

    mExitNode = cfg->getExit();

    mSolver->solve(cfg,algorithm);

    OA_ptr<DataFlow::DataFlowSet> exitclone = mSolver->getOutSet(mExitNode);
    exitclone = exitclone->clone();
    mLM = exitclone.convert<LinearityMatrix>();
   

/*
  TestCase1:
                                  *********
				  * Entry *
				  *********
                                      |
                                      v
                                  *********
                                  * a=0.0 *
                                  * b=0.0 *
                                  * c=0.0 *
                                  *********
                                      |
                                      V
                                  *********
                                  * a = b *
                                  * c = a *
                                  *********
                                      |
                                      v
                                  *********
				  * Exit  *
				  *********
 *
 * START of TestCase1: 
 */
/*
std::cout <<"  TestCase1:\n";
std::cout <<"                                  *********\n";
std::cout <<"				  * Entry *\n";
std::cout <<"				  *********\n";
std::cout <<"                                      |\n";
std::cout <<"                                      v\n";
std::cout <<"                                  *********\n";
std::cout <<"                                  * a=0.0 *\n";
std::cout <<"                                  * b=0.0 *\n";
std::cout <<"                                  * c=0.0 *\n";
std::cout <<"                                  *********\n";
std::cout <<"                                      |\n";
std::cout <<"                                      V\n";
std::cout <<"                                  *********\n";
std::cout <<"                                  * a = b *\n";
std::cout <<"                                  * c = a *\n";
std::cout <<"                                  *********\n";
std::cout <<"                                      |\n";
std::cout <<"                                      v\n";
std::cout <<"                                  *********\n";
std::cout <<"				  * Exit  *\n";
std::cout <<"				  *********\n";

std::cout <<"  START of TestCase1: WORKING    \n";


//OA_ptr<LinearityMatrix> LM; LM = new LinearityMatrix;
mLM = new LinearityMatrix;
OA_ptr<LinearityDepsSet> deps;

//Variables ->
OA_ptr<Location> a; a = new NamedLoc(SymHandle((irhandle_t)'a'),true);
OA_ptr<Location> b; b = new NamedLoc(SymHandle((irhandle_t)'b'),true);
OA_ptr<Location> c; c = new NamedLoc(SymHandle((irhandle_t)'c'),true);	

//c->dump(std::cout, mIR);

//e = 0.0 //a = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//a = e //NODEP
mLM->putDepsSet(a,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //b = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//b = e //NODEP
mLM->putDepsSet(b,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //c = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//c = e //NODEP
mLM->putDepsSet(c,deps); //{<<v,w>,T>|v,w E V}

OA_ptr<LinearityDepsSet> depb, depc;
deps = new LinearityDepsSet(a,mLM->getDepsSet(a));
depb = new LinearityDepsSet(b,mLM->getDepsSet(b));
//a = b
mLM->putDepsSet(a,depb);

deps = new LinearityDepsSet(a,mLM->getDepsSet(a));
depc = new LinearityDepsSet(c,mLM->getDepsSet(c));
//c = a
mLM->putDepsSet(c,deps);

std::cout << "LM Contains:\n";
mLM->output();
*/
/*
 * END of TestCase1 
 */

return mLM;
}


OA_ptr<DataFlow::DataFlowSet> 
ManagerLinearity::meet (OA_ptr<DataFlow::DataFlowSet> set1orig, 
                       OA_ptr<DataFlow::DataFlowSet> set2orig)
{

    OA_ptr<DataFlow::DataFlowSet> set1clone = set1orig->clone(); 
    OA_ptr<DataFlow::DataFlowSet> set2clone = set2orig->clone(); 
    OA_ptr<LinearityMatrix> set1 = set1clone.convert<LinearityMatrix>();
    OA_ptr<LinearityMatrix> set2 = set2clone.convert<LinearityMatrix>();

    if (debug) {
        std::cout << "ManagerLinearityStandard::meet" << std::endl;
        std::cout << "\tset1 = ";
        set1->dump(std::cout,mIR);
        std::cout << ", set2 = ";
        set2->dump(std::cout,mIR);
    }
       
    OA_ptr<LinearityMatrix> retval; retval = new LinearityMatrix;
    retval = set1->meet(set2, *mIR);
    if (debug) {
        std::cout << std::endl << "\tretval set = ";
        retval->dump(std::cout,mIR);
        std::cout << std::endl;
    }
       
    return retval;
}

//! Transfer function for Linearity, LinearityMatrix in/out Sets
//
OA_ptr<DataFlow::DataFlowSet> 
ManagerLinearity::transfer(OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt) 
{
    OA_ptr<DataFlow::DataFlowSet> inclone = in->clone();
    OA_ptr<LinearityMatrix> inRecast = inclone.convert<LinearityMatrix>();

    if (debug) {
      std::cout << "Top of Transfer==\n";
      inRecast->output(*mIR);
    }

    OA_ptr<AssignPairIterator> espIterPtr
            = mIR->getAssignPairIterator(stmt);

    if(!espIterPtr.ptrEqual(0)) {

        for ( ; espIterPtr->isValid(); ++(*espIterPtr)) {
           // unbundle pair
           MemRefHandle mref = espIterPtr->currentTarget();
           ExprHandle expr = espIterPtr->currentSource();
        
           // getExpression Tree
           OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(expr);

           if (debug) {
             std::cout << "Expression Tree:\n";
             eTreePtr->output(*mIR);
           }
           // get deps by evaluating expression tree
           //LinearityLocsVisitor linearityVisitor(mIR,mLM,mAlias,mParamBind);
           LinearityLocsVisitor linearityVisitor(mIR,inRecast,mAlias,mParamBind);
           eTreePtr->acceptVisitor(linearityVisitor);

           //std::cout << "\n===============================================";
           //std::cout << "\nLinearityVisitor.getDepsSet() -> ExprTree Visit\n";
           //linearityVisitor.getDepsSet()->output(*mIR);
           //std::cout << "\n===============================================\n";           
           // for each MayLoc(MustLoc??), include into the inSet with
           // the new depsset.
           OA_ptr<LocIterator> lIterPtr = mAlias->getMayLocs(mref);
           for ( ; lIterPtr->isValid() ; ++(*lIterPtr)) {
             OA_ptr<Location> lPtr = lIterPtr->current();
             //mLM->putDepsSet(lPtr,linearityVisitor.getDepsSet());
             inRecast->putDepsSet(lPtr,linearityVisitor.getDepsSet());
           }
        }
    }

    if (debug) {
      std::cout << "Bottom of Transfer:::\n";
      inRecast->output(*mIR);
    }

    return inRecast;

}

OA_ptr<Linearity::LinearityMatrix> ManagerLinearity::performAnalysis2(SymHandle sym)
{

/* 
  TestCase2:
  
				  *********
				  * Entry *
				  *********
				      ;
				  *********
				  * a=0.0 *
				  * f=0.0 *
				  *********           
				      ;<------------------
				  *************          |
				  * if (i<=N) *          |
				  *************          |
			|-----------;                    |
			|	*********************    |
			|	* a = a + x[i]*x[i] *    |
			|	* t = sin(a)        *    |
			|	* f = f + t         *    |
			|	*********************    |
			|	      ;------------------|
                        |
                        |
			|	  *********
			--------->* Exit  *
				  *********  
  
 *
 * START of TestCase2: 
 */

std::cout <<"  TestCase2:\n";
std::cout <<"  \n";
std::cout <<"				  *********\n";
std::cout <<"				  * Entry *\n";
std::cout <<"				  *********\n";
std::cout <<"				      ;\n";
std::cout <<"				  *********\n";
std::cout <<"				  * a=0.0 *\n";
std::cout <<"				  * f=0.0 *\n";
std::cout <<"				  *********    \n";       
std::cout <<"				      ;<------------------\n";
std::cout <<"				  *************          |\n";
std::cout <<"				  * if (i<=N) *          |\n";
std::cout <<"				  *************          |\n";
std::cout <<"			|-----------;                    |\n";
std::cout <<"			|	*********************    |\n";
std::cout <<"			|	* a = a + x[i]*x[i] *    |\n";
std::cout <<"			|	* t = sin(a)        *    |\n";
std::cout <<"			|	* f = f + t         *    |\n";
std::cout <<"			|	*********************    |\n";
std::cout <<"			|	      ;------------------|\n";
std::cout <<"                        |\n";
std::cout <<"                        |\n";
std::cout <<"			|	  *********\n";
std::cout <<"			--------->* Exit  *\n";
std::cout <<"				  *********  \n";
  

std::cout <<"  START of TestCase2: NOTWORKING \n";
    
/*
OA_ptr<LinearityMatrix> LM; LM = new LinearityMatrix;
OA_ptr<LinearityDepsSet> deps;

//Variables ->
OA_ptr<AbstractVar> a; a = new SimpleAbstractVar('a');
OA_ptr<AbstractVar> f; f = new SimpleAbstractVar('f');
OA_ptr<AbstractVar> x; x = new SimpleAbstractVar('x');	
OA_ptr<AbstractVar> t; t = new SimpleAbstractVar('t');

//e = 0.0 //a = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//a = e //NODEP
LM->putDepsSet(a,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //f = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LM and its DEPS
//f = e //NODEP
LM->putDepsSet(f,deps); //{<<v,w>,T>|v,w E V}
    
    //TODO: Meet LM (IN) with (OUT) of This Block
    //LM->meet(LM2);
    //LinearityMatrix *LM2 = new LinearityMatrix(5);
    
    //e = a + x[i] * x[i] //a = e
    /* Expression Tree for e
               +
              / \
             a   *
                / \
             x[i] x[i] */
/*OA_ptr<LinearityDepsSet> depx, mult, add;
deps = new LinearityDepsSet(a,LM->getDepsSet(a));
depx = new LinearityDepsSet(x,LM->getDepsSet(x));
mult = depx->multdiv(depx);
add = deps->addsub(mult);
//Insert Assigment to LM and its DEPS
//a = e
LM->putDepsSet(a,add);


//e = sin(a) //t = sin(a)
OA_ptr<LinearityDepsSet> sin;
deps = new LinearityDepsSet(a,LM->getDepsSet(a));
sin = deps->func(deps);
//Insert Assignment to LM and its DEPS
//t = e
LM->putDepsSet(t,sin);
    
    //e = f + t //f = f + t
    /* Expression Tree for e
               +
              / \
             f   t */
/*OA_ptr<LinearityDepsSet> dept;
deps = new LinearityDepsSet(f,LM->getDepsSet(f));
dept = new LinearityDepsSet(t,LM->getDepsSet(t));
add = deps->addsub(dept);
//Insert Assignment to LM and its DEPS
//f = e
LM->putDepsSet(f,add);

std::cout << "LM Contains:\n";
LM->output();
*/
/*
 * END of TestCase2 
 */
}

OA_ptr<Linearity::LinearityMatrix> ManagerLinearity::performAnalysis3(ProcHandle proc,
                                                OA_ptr<CFG::CFGInterface> cfg,
                                                OA_ptr<Alias::Interface> alias,
                                                OA_ptr<DataFlow::ParamBindings> paramBind)
{

/* 
  TestCase3:
                                    *********
				    * Entry *
				    *********
				        |
                                        v
				  ***************
				  * a=0.0       *
				  * b=0.0       *
                                  * c=0.0       *
                                  * d=0.0       *
                                  * a = a + x*x *
				  ***************           
				        |
                                        v
                                  ***************          
				  * if (a > 10) *          
				  ***************          
                  ________true__________|__________false_______
                 |                                             |
                 v                                             v
             *********                                 *****************    
             * i = 0 *                                 * c = c + x + 2 *
             *********                                 *****************        
                 |                                             |
                 v<-------------                               |
	   **************      |                               |
	   * if (i<=10) *      |                               |
	   **************      |                               |
    --false------|             |                               |
    |            v             |                               |
    |   *********************  |                               |
    |   * a = a + y[i]      *  |                               |
    |   * b = b + y[i] + a  *  |                               |
    |   * c = c/(sin(y[i])) *  |                               |
    |   * i = i + 1         *  |                               |
    |   *********************  |                               |
    |           |______________|                               |
    |                                                          |
    |------------------------------->|<-------------------------
                                     v
                             ********************
                             * d = c + pow(x,1) *
                             ********************
                                     |
                                     v
                                 *********
			         * Exit  *
			         *********  
  
 *
 * START of TestCase3: 
 */

std::cout <<"  TestCase3:\n";
std::cout <<"                                    *********\n";
std::cout <<"				    * Entry *\n";
std::cout <<"				    *********\n";
std::cout <<"				        |\n";
std::cout <<"                                        v\n";
std::cout <<"				  ***************\n";
std::cout <<"				  * a=0.0       *\n";
std::cout <<"				  * b=0.0       *\n";
std::cout <<"                                  * c=0.0       *\n";
std::cout <<"                                  * d=0.0       *\n";
std::cout <<"                                  * a = a + x*x *\n";
std::cout <<"				  ***************           \n";
std::cout <<"				        |\n";
std::cout <<"                                        v\n";
std::cout <<"                                  ***************          \n";
std::cout <<"				  * if (a > 10) *          \n";
std::cout <<"				  ***************         \n"; 
std::cout <<"                  ________true__________|__________false_______\n";
std::cout <<"                 |                                             |\n";
std::cout <<"                 v                                             v\n";
std::cout <<"             *********                                 *****************    \n";
std::cout <<"             * i = 0 *                                 * c = c + x + 2 *\n";
std::cout <<"             *********                                 *****************        \n";
std::cout <<"                 |                                             |\n";
std::cout <<"                 v<-------------                               |\n";
std::cout <<"	   **************      |                               |\n";
std::cout <<"	   * if (i<=10) *      |                               |\n";
std::cout <<"	   **************      |                               |\n";
std::cout <<"    --false------|             |                               |\n";
std::cout <<"    |            v             |                               |\n";
std::cout <<"    |   *********************  |                               |\n";
std::cout <<"    |   * a = a + y[i]      *  |                               |\n";
std::cout <<"    |   * b = b + y[i] + a  *  |                               |\n";
std::cout <<"    |   * c = c/(sin(y[i])) *  |                               |\n";
std::cout <<"    |   * i = i + 1         *  |                               |\n";
std::cout <<"    |   *********************  |                               |\n";
std::cout <<"    |           |______________|                               |\n";
std::cout <<"    |                                                          |\n";
std::cout <<"    |------------------------------->|<-------------------------\n";
std::cout <<"                                     v\n";
std::cout <<"                             ********************\n";
std::cout <<"                             * d = c + pow(x,1) *\n";
std::cout <<"                             ********************\n";
std::cout <<"                                     |\n";
std::cout <<"                                     v\n";
std::cout <<"                                 *********\n";
std::cout <<"			         * Exit  *\n";
std::cout <<"			         *********  \n";

std::cout <<"\nSTART of TestCase3: WORKING\n\n";



OA_ptr<LinearityMatrix> LM1; LM1 = new LinearityMatrix;
OA_ptr<LinearityDepsSet> deps;

//Variables ->
OA_ptr<Location> a; a = new NamedLoc(SymHandle((irhandle_t)'a'),true);
OA_ptr<Location> b; b = new NamedLoc(SymHandle((irhandle_t)'b'),true);
OA_ptr<Location> c; c = new NamedLoc(SymHandle((irhandle_t)'c'),true);	
OA_ptr<Location> d; d = new NamedLoc(SymHandle((irhandle_t)'d'),true);
OA_ptr<Location> x; x = new NamedLoc(SymHandle((irhandle_t)'x'),true);
OA_ptr<Location> y; y = new NamedLoc(SymHandle((irhandle_t)'y'),true);	
OA_ptr<Location> i; i = new NamedLoc(SymHandle((irhandle_t)'i'),true);	

//e = 0.0 //a = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//a = e //NODEP
LM1->putDepsSet(a,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //b = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LM and its DEPS
//b = e //NODEP
LM1->putDepsSet(b,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //c = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LinearityMatrix and its DEPS
//c = e //NODEP
LM1->putDepsSet(c,deps); //{<<v,w>,T>|v,w E V}

//e = 0.0 //d = 0.0
deps = new LinearityDepsSet(0.0);
//Insert Assignment to LM and its DEPS
//d = e //NODEP
LM1->putDepsSet(d,deps); //{<<v,w>,T>|v,w E V}


//e = a + x * x //a = e
/* Expression Tree for e
               +
              / \
             a   *
                / \
               x   x */
OA_ptr<LinearityDepsSet> depx, mult, add;
deps = new LinearityDepsSet(a,LM1->getDepsSet(a));
depx = new LinearityDepsSet(x,LM1->getDepsSet(x));
mult = depx->multdiv(depx);
add = deps->addsub(mult);
//Insert Assignment to LM and its DEPS
//a = e
LM1->putDepsSet(a,add);


//if (a > 10) True:LM, False:LM2
OA_ptr<LinearityMatrix> LM2; LM2 = new LinearityMatrix;
LM2 = LM1;

//TRUE:
   //e = 0.0 //i = 0.0
   deps = new LinearityDepsSet(0.0);
   //Insert Assignment to LinearityMatrix and its DEPS
   //i = e //NODEP
   LM1->putDepsSet(i,deps); //{<<v,w>,T>|v,w E V}

   //e = a + y[i] //a = a + y[i]
   /* Expression Tree for e
              +
             / \
            a   y */
   OA_ptr<LinearityDepsSet> depy;
   deps = new LinearityDepsSet(a,LM1->getDepsSet(a));
   depy = new LinearityDepsSet(y,LM1->getDepsSet(y));
   add = deps->addsub(depy);
   //Insert Assignment to LM and its DEPS
   //a = e
   LM1->putDepsSet(a,add);
   //e = b + y[i] + a //b = b + y[i] + a
   /* Expression Tree for e
             +
            / \
           b   +
              / \
           y[i]  a */
   OA_ptr<LinearityDepsSet> depb, add2;
   depb = new LinearityDepsSet(b,LM1->getDepsSet(b));
   depy = new LinearityDepsSet(y,LM1->getDepsSet(y));
   deps = new LinearityDepsSet(a,LM1->getDepsSet(a));
   add = depy->addsub(deps);
   add2 = depb->addsub(add);
   //Insert Assignment to LM and its DEPS
   //b = e
   LM1->putDepsSet(b,add2);
   //e = c/(sin(y[i])) //c = c/(sin(y[i]))
   /* Expression Tree for e
              /
             / \ 
            c  sin()
                |
               y[i]*/
   OA_ptr<LinearityDepsSet> depc, div, sin;
   depy = new LinearityDepsSet(y,LM1->getDepsSet(y));
   depc = new LinearityDepsSet(c,LM1->getDepsSet(c));
   sin = depy->func(depy);
   div = depc->multdiv(sin);
   //Insert Assignment to LM and its DEPS
   //c = e
   LM1->putDepsSet(c,div);

   //e = i + 1 //i = i + 1
   OA_ptr<LinearityDepsSet> depi;
   depi = new LinearityDepsSet(i,LM1->getDepsSet(i));
   add = depi->anyopk(); //Constant Addition
   //Insert Assignment to LM and its DEPS
   //i = e
   LM1->putDepsSet(i,add);

//FALSE:
   //e = c + x + 2 //c = c + x + 2
   /* Expression Tree for e
              +
             / \
            c   +
               / \
              x   2 */
   depx = new LinearityDepsSet(x,LM2->getDepsSet(x));
   depc = new LinearityDepsSet(c,LM2->getDepsSet(c));
   add = depx->anyopk(); //Constant Addition
   add2 = depc->addsub(add);
   //Insert Assignment to LM and its DEPS
   //c = e
   LM2->putDepsSet(c,add2);

//MEET:
mLM = new LinearityMatrix;
//mLM = LM1->meet(LM2);

//e = c + pow(x,1) //d = c + pow(x,1)
/* Expression Tree for e
              +
             / \
            c   ^
               / \
              x   1 */
OA_ptr<LinearityDepsSet> depp;
depc = new LinearityDepsSet(c,mLM->getDepsSet(c));
depx = new LinearityDepsSet(x,mLM->getDepsSet(x));
depp = depx->power1(depx);
add = depc->addsub(depp);
//Insert Assignment to LM and its DEPS
//d = e
mLM->putDepsSet(d,add);

/*
 * END of TestCase3 
 */

return mLM;
}


  } // end of Linearity namespace
} // end of OA namespace

