/*! \file
  
  \brief Implementation of LoopManager

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

    #include "LoopManager.hpp"
using namespace std;

namespace OA {
  namespace Loop {

using namespace AffineExpr;


class Statistics {
  public:
    enum LoopType {
        LOOP_LEGAL,
        LOOP_ILLEGAL,
        LOOP_NONE
    };

    enum Linearity {
        LINEARITY_FULL,
        LINEARITY_PARTIAL,
        LINEARITY_NONE
    };


    Statistics() :
        mnLinear(0),
        mnPartial(0),
        mnNonLinear(0),
        mnNotInLoop(0),
        mnInInvalidLoop(0),
        mnInValidLoop(0)
    { }

    int getNumLinear()    { return mnLinear; }
    int getNumPartial()   { return mnPartial; }
    int getNumNonLinear() { return mnNonLinear; }

    int getNumNotInLoop()   { return mnNotInLoop; }
    int getNumInvalidLoop() { return mnInValidLoop; }
    int getNumValidLoop()   { return mnInValidLoop; }

    void incrementAccessCounter(Linearity linearity, LoopType type);
    void output();

  private:
    int mnLinear;
    int mnPartial;
    int mnNonLinear;

    int mnNotInLoop;
    int mnInInvalidLoop;
    int mnInValidLoop;
};

void Statistics::incrementAccessCounter(Linearity linearity, LoopType type) {
    switch(linearity) {
        case LINEARITY_FULL:
            mnLinear++;
            break;

        case LINEARITY_PARTIAL:
            mnPartial++;
            break;

        case LINEARITY_NONE:
            mnNonLinear++;
            break;
    }

    switch(type) {
        case LOOP_LEGAL:
            mnInValidLoop++;
            break;

        case LOOP_ILLEGAL:
            mnInInvalidLoop++;
            break;

        case LOOP_NONE:
            mnNotInLoop++;
            break;
    }
}

void Statistics::output() {
    cout << "Array Access Statistics: " << endl;
    cout << " Linearity" << endl;
    cout << "   Linear:           " << getNumLinear() << endl;
    cout << "   Partially Linear: " << getNumPartial() << endl;
    cout << "   Nonlinear:        " << getNumNonLinear() << endl;
    cout << " Loops" << endl;
    cout << "   Not In Loop:      " << getNumNonLinear() << endl;
    cout << "   In Invalid Loop nest:  " << getNumInvalidLoop() << endl;
    cout << "   In Valid Loop nest:    " << getNumValidLoop() << endl;
    cout << "   In partial Loop nest:  "  /* !!! */ << "!!!" << endl;
}



OA_ptr<LoopResults> LoopManager::performLoopDetection(
        ProcHandle proc)
{
    OA_ptr<LoopResults> results;
    results = new LoopResults(*mIR);

    // For now call a method in the interface which performs a loop
    // detection analysis for us.  This is a temporary strategy.
    OA_ptr<list<OA_ptr<LoopAbstraction> > > loops;
    loops = mIR->gatherLoops(proc);

    // Encode results
    for(list<OA_ptr<LoopAbstraction> >::iterator i = loops->begin();
        i != loops->end(); i++)
    {
        results->addLoop(*i);
    }

    return results;
}

} } // end namespaces

