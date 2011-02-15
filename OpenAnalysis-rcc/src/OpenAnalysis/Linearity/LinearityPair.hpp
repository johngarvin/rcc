/*! \file
  
  \brief Linearity Pairs, holds the Datastructures for LinearityAnalysis.

  \authors Luis Ramos
  \version $Id: LinearityPair.hpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef _LINEARITYPAIR_H
#define _LINEARITYPAIR_H

//--------------------------------------------------------------------
// STL headers
#include <iostream>
#include <map>
#include <set>
#include <list>

//OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

class AbstractVar {
    public:
        AbstractVar() {}
        ~AbstractVar() {}
        
        virtual char getVar() = 0;

    virtual void operator=( AbstractVar &other) = 0;
        // comparison operators
        virtual bool operator==( AbstractVar &other ) = 0;
        virtual bool operator<( AbstractVar &other ) = 0;
};

class SimpleAbstractVar : public AbstractVar {
    private:
        char var;
    public:
    	SimpleAbstractVar() {}
        ~SimpleAbstractVar() {}
        SimpleAbstractVar(char v) { var = v; }
        char getVar() { return var; }
    	
    	
        void operator= ( AbstractVar &other ); 
        bool operator==( AbstractVar &other ); 
        bool operator<( AbstractVar &other ); 
};
//ampl interpretor -> capable of making C/Fortran Statements
//Talk to Tod About Ampl and TestCases
//***********************************************
//LinearityClass Starts
//***********************************************

class LinearityClass {
    public:
        typedef enum { 
            LCLASS_NODEP, LCLASS_LINEAR, LCLASS_NONLINEAR 
        } LClassLatticeVal;

    LinearityClass() {}
		
        LinearityClass( LClassLatticeVal init ) { mVal = init; }
        LinearityClass( int init )
        {
            if (init == 1) { mVal = LCLASS_LINEAR; }
            else if (init == 2) { mVal = LCLASS_NONLINEAR; }	
        }
        LClassLatticeVal getLClass();
        void putLClass(LClassLatticeVal init);
        void output() const { std::cout << "Meet = " << mVal << std::endl; }
		
        LinearityClass meet(LinearityClass other);
        bool operator==( LinearityClass other ); 
	bool operator!=( LinearityClass other );
        
    private:
        LClassLatticeVal mVal; //enum type
};

//***********************************************
//VarClass Starts
//***********************************************

class VarClassPair : public virtual Annotation{
    public:
        //<v,class>
        VarClassPair( OA_ptr<Location> v1, LinearityClass lclass2 )
        {
         //OA_ptr<SymHandle> empty; empty = new SymHandle();
         //v = new NamedLoc(SymHandle(),true);
         //*v = *v1;
         //v = v1->clone();
         v = v1;
         //if (v1->isaNamed()) {
         // OA_ptr<NamedLoc> nv1 = v1.convert<NamedLoc>();
         // v = new NamedLoc(*nv1);
         //}
         lclass = lclass2; 
        }
        
        OA_ptr<Location> getVar();
        LinearityClass getLClass();

        bool operator==( VarClassPair &other );
        bool operator<( VarClassPair &other );
        void output(IRHandlesIRInterface& ir) const;
        
    private:
        OA_ptr<Location> v;
        LinearityClass lclass;

};

// HINT: while implementing this, look up some of the iterator
// implementations in OpenAnalysis
class VarClassPairIterator {
private:
    OA_ptr<std::set<OA_ptr<VarClassPair> > > vcSet;
    std::set<OA_ptr<VarClassPair> >::iterator mIter;
	
public:
    VarClassPairIterator(OA_ptr<std::set<OA_ptr<VarClassPair> > > pSet) 
    { 
        vcSet = new std::set<OA_ptr<VarClassPair> >(*pSet);
        mIter = vcSet->begin(); 
        //std::cout << "(" << vcSet->size() << ")vc" << std::endl;
    }
    ~VarClassPairIterator() 
    { 
        vcSet = new std::set<OA_ptr<VarClassPair> >;
       mIter = vcSet->begin();
    }

    // Returns the current item. 
    //NOTE: Should VarClassPair Return be *?
    OA_ptr<VarClassPair> current() const { return (*mIter); }
    // False when all items are exhausted.
    bool isValid() const {return (mIter != vcSet->end()); };

    // Changes an Iter to Find()
    void findIter(OA_ptr<VarClassPair> vcp) {
        mIter = (vcSet)->find(vcp); 
    }

    void operator++() { if (mIter != vcSet->end()) mIter++; }
    void operator++(int) { ++*this; } ;

    void reset();
};

//***********************************************
//LinearityPair Starts
//***********************************************

class LinearityPair : public virtual Annotation {
    public:
        //<<v,w>,class>
        LinearityPair( OA_ptr<Location> v2, OA_ptr<Location> w2, 
                       LinearityClass lclass2 )
        { 
            //v = new NamedLoc(SymHandle(),true);
            //w = new NamedLoc(SymHandle(),true);
            //*v = *v2; *w = *w2;
            //if (v2->isaNamed() && w2->isaNamed()) {
            //  OA_ptr<NamedLoc> nv2 = v2.convert<NamedLoc>();
            //  OA_ptr<NamedLoc> nw2 = w2.convert<NamedLoc>();
            //  v = new NamedLoc(*nv2);
            //  w = new NamedLoc(*nw2);
            //}
            v = v2;
            w = w2;
            lclass = lclass2;  
        }
        
        OA_ptr<VarClassPair> getVarClassPair();  
        LinearityClass getLClass();
        OA_ptr<Location> getVar1();
        OA_ptr<Location> getVar2();

        void output(IRHandlesIRInterface& ir) const;
        void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
        
        bool operator==( LinearityPair &other );
	bool operator!=( LinearityPair &other );
        bool operator<( LinearityPair &other );
    private:
        OA_ptr<Location> v;
        OA_ptr<Location> w;
        LinearityClass lclass;
};


// NOTE: Will this actually iterate through VarClassPair's of a Specific
//       AbstractVar *v? and Not through just a LinearityPair?
//       So in reality, it would be <v,<w,class>> | <w,class> is in v??
class LinearityPairIterator {
private:
    OA_ptr<std::set<OA_ptr<LinearityPair> > > lpSet;
    std::set<OA_ptr<LinearityPair> >::iterator mIter;

public:
    LinearityPairIterator(OA_ptr<std::set<OA_ptr<LinearityPair> > > pSet)
    { 
        lpSet = new std::set<OA_ptr<LinearityPair> >(*pSet); 
        mIter = lpSet->begin(); 
        //std::cout << "(" << lpSet->size() << ")lp" << std::endl;
    }
    LinearityPairIterator()
    {
        lpSet = new std::set<OA_ptr<LinearityPair> >;	
        mIter = lpSet->begin();
    }
    ~LinearityPairIterator() { }

    // Returns the current item. 
    OA_ptr<LinearityPair> current() const { return (*mIter); }  
    // False when all items are exhausted.
    bool isValid() const {return (mIter != lpSet->end()); }
    
    // Changes an Iter to Find()
    void findIter(OA_ptr<LinearityPair> lp) {
        mIter = (lpSet)->find(lp); 
    }
    
    // Returns the Set of LinearityPairs
    OA_ptr<std::set<OA_ptr<LinearityPair> > > getSet() {
        return lpSet;
    }

    void operator ++ () { if (mIter != lpSet->end()) mIter++; }
    void operator++(int) { ++*this; } ;

    void reset() { mIter = lpSet->begin(); }

};

  } // end of namespace Linearity
} // end of namespace OA

#endif

