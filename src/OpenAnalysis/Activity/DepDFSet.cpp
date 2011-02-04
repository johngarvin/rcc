/*! \file
  
  \brief Implementation of DepDFSet.

  \author Michelle Strout
  \version $Id: DepDFSet.cpp,v 1.2 2005/06/10 02:32:01 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "DepDFSet.hpp"
#include <Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilderText.hpp>

namespace OA {
  namespace Activity {

static bool debug = false;

//std::map<Alias::AliasTag, OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > >  DepDFSet::mUseToDefsMap;
//std::map<Alias::AliasTag, OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > >  DepDFSet::mDefToUsesMap;


//! default constructor
DepDFSet::DepDFSet() //: mMakeImplicitExplicitMemoized(false)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_DepDFSet:ALL", debug);
    mImplicitRemoves = new  DataFlow::DataFlowSetImpl<Alias::AliasTag>();
    mUses = new DataFlow::DataFlowSetImpl<Alias::AliasTag>(); 
    mDefs = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();
}

//! copy constructor
DepDFSet::DepDFSet(const DepDFSet &other) //: mMakeImplicitExplicitMemoized(false)
{ 
    mImplicitRemoves = other.mImplicitRemoves;
    mUseToDefsMap = other.mUseToDefsMap;
    mDefToUsesMap = other.mDefToUsesMap;
    mDefs = other.mDefs;
    mUses = other.mUses;
}

//! assignment
DepDFSet& DepDFSet::operator=(const DepDFSet &other)
{
    //mMakeImplicitExplicitMemoized = other.mMakeImplicitExplicitMemoized;
    mImplicitRemoves = other.mImplicitRemoves;
    mUseToDefsMap = other.mUseToDefsMap;
    mDefToUsesMap = other.mDefToUsesMap;
    mDefs = other.mDefs;
    mUses = other.mUses;
    return *this;
}

//*****************************************************************
// DataFlowSet Interface Implementation
//*****************************************************************
    
OA_ptr<DataFlow::DataFlowSet> DepDFSet::clone() const
{ 
    OA_ptr<DepDFSet> retval;
    retval = new DepDFSet(*this); 
    return retval; 
}

bool DepDFSet::operator==(DataFlow::DataFlowSet &other) const
{ 
    DepDFSet& recastOther = dynamic_cast<DepDFSet&>(other);
    bool retval = false;

    if (mImplicitRemoves == recastOther.mImplicitRemoves)
    {
        retval = true;
    }

    
    if (debug) {
        std::cout << "operator= returning " <<  retval << std::endl;
    }
    return retval;

}

bool DepDFSet::operator!=(DataFlow::DataFlowSet &other) const
{
    if (*this == other) {
        if (debug) { std::cout << "operator!= will return false" << std::endl; }
        return false;
    } else {
        if (debug) { std::cout << "operator!= will return true" << std::endl; }
        return true;
    }
}

//*****************************************************************
// Methods specific to DepDFSet
//*****************************************************************

/*!
    This routine generates a new DepDFSet based of the
    current one but with the implicit pairs included.
 */
OA_ptr<DepIterator> DepDFSet::getDepIterator() const
{

    // copy self and then create iterator for it
    OA_ptr<DepDFSet> setCopy; setCopy = new DepDFSet(*this);
    //setCopy->makeImplicitExplicit();
    OA_ptr<DepIterator> retval;
    retval = new DepIterator(setCopy);
    return retval;
}

 
OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
      DepDFSet::getDefsIterator(Alias::AliasTag use) const
{

    OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > retSet;
    retSet = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();

    // iterator over associated defs
    AliasTagToAliasTagDFSetMap::const_iterator pos = mUseToDefsMap.find(use);
    if (pos!=mUseToDefsMap.end()) {

       OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > rIter;
       rIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(pos->second);
        for( ; rIter->isValid(); ++(*rIter) ) { 
          retSet->insert(rIter->current()); 
        }      
    }


    // see if reflexive dep for use loc should be included,
    // only reason why it wouldn't is if use
    // is in the ImplicitRemoves
    // FIXME: ok could put hasMustOverlapLoc, but for now going with 
    // conservative assumption which means equivalenced symbols will get
    // short shrifted. insertDep if use==def makes same assumption.
    if (!mImplicitRemoves->contains(use)) {
        retSet->insert(use);
    }

    // construct the iterator
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > retval;
    retval = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(retSet);
    return retval;
}





OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > 
      DepDFSet::getUsesIterator(Alias::AliasTag def) const
{
    OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > retSet;
    retSet = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();

    // iterator over associated deps
    AliasTagToAliasTagDFSetMap::const_iterator pos = mDefToUsesMap.find(def);
    if (pos!=mDefToUsesMap.end()) {

        //retSet = new DataFlow::DataFlowSetImpl<Alias::AliasTag>(pos->second); 

       OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > rIter;
       rIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(pos->second);

        for( ; rIter->isValid(); ++(*rIter) ) {
          retSet->insert(rIter->current());
        }
    }


    // see if reflexive dep for def loc should be included,
    // only reason why it wouldn't is if def
    // is in the ImplicitRemoves
    // FIXME: ok could put hasMustOverlapLoc, but for now going with 
    // conservative assumption which means equivalenced symbols will get
    // short shrifted
    if (!mImplicitRemoves->contains(def)) {
        retSet->insert(def);
    }

    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > retval;
    retval = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(retSet);

    return retval;
}

/*!
   Does a union of all dep pairs and stores the result in this.
*/
DepDFSet& DepDFSet::setUnion(DataFlow::DataFlowSet &other)
{
    DepDFSet& recastOther = dynamic_cast<DepDFSet&>(other);
    if (debug) {
        std::cout << "DepDFSet::setUnion" << std::endl;
    }

    // for each use in other
    AliasTagToAliasTagDFSetMap::iterator mapIter;
    for (mapIter=recastOther.mUseToDefsMap.begin();
         mapIter!=recastOther.mUseToDefsMap.end(); mapIter++ )
    {
        Alias::AliasTag use = mapIter->first;
        mUseToDefsMap[use]->unionEqu(*(mapIter->second));
    }
    
    // for each def in other
    for (mapIter=recastOther.mDefToUsesMap.begin();
         mapIter!=recastOther.mDefToUsesMap.end(); mapIter++ )
    {
        Alias::AliasTag def = mapIter->first;
        mDefToUsesMap[def]->unionEqu(*(mapIter->second));
    }

    // take on all of other's implicitRemoves that we already have
    mImplicitRemoves->intersectEqu(*(recastOther.mImplicitRemoves));

    mUses->unionEqu(*(recastOther.mUses));
    mDefs->unionEqu(*(recastOther.mDefs));

    return *this;
}


//! helper function that determines if the given location 
//! has been killed due to a must def
bool DepDFSet::isImplicitRemoved(Alias::AliasTag aliasTag) 
{
    return mImplicitRemoves->contains(aliasTag);
}

/*!
   Composes dep pairs in this with other
   DepDFSet and stores result in this.

Examples: let <b,b> has been removed from set2 with removeDep
this        other        result
set1        set2
<use1,def1> <use2,def2>
<a,b>       <b,a>       <a,a> explicitly represented but marked as implicitly blocked
<a,b>                   shouldn't have <a,b> unless <b,b> is explicit in set 2   
Unions mImplicitRemoves for this and other

Assumes that getDefsIterator and getUsesIterator handles the mImplicitRemoves
correctly.
In other words those iterators will return reflexive uses and defs
unless the given parameters overlap with something in the implicitRemoves list.

*/
DepDFSet& DepDFSet::compose(DataFlow::DataFlowSet &other)
{
    DepDFSet& recastOther = dynamic_cast<DepDFSet&>(other);
    if (debug) {
        std::cout << "DepDFSet::compose" << std::endl;
        std::cout << "\t*this = ";
        dump(std::cout);
        std::cout << "\tother = ";
        recastOther.dump(std::cout);
    }
    
    OA_ptr<DepDFSet> tempDFSet;
    tempDFSet = new DepDFSet;
    
    // have to do these before do insertDeps because could be inserting
    // a dep that will take things out of the mImplicitRemoves set
    //tempDFSet->mImplicitRemoves->setUnion(*(mImplicitRemoves)); 
    //tempDFSet->mImplicitRemoves->setUnion(*(recastOther.mImplicitRemoves)); 

    (tempDFSet->mImplicitRemoves)->unionEqu(*mImplicitRemoves);
    (tempDFSet->mImplicitRemoves)->unionEqu(*(recastOther.mImplicitRemoves));
    
    // iterate over deps in this keying off the def
    // see if the def overlaps with anything in recastOther
    AliasTagToAliasTagDFSetMap::iterator mapIter;
    for (mapIter=mDefToUsesMap.begin();
         mapIter!=mDefToUsesMap.end(); mapIter++ )
    {
        Alias::AliasTag def1 = mapIter->first;
        if (debug) {
            //std::cout << "def1 = "; def1->dump(std::cout); 
            std::cout << "def1 = "; std::cout << def1 << std::endl;
        }
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> >use1Set;
        use1Set = mapIter->second;

        /*
        if (debug) {
            std::cout << "use1Set = "; use1Set->dump(std::cout);
        }
        */

        // get all the defs in use2,def2 where def1 overlaps with use2
        

        //OA_ptr<AliasTag::AliasTagIteratorImpl> def2Iter(dSet);
        OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > def2Iter;
        def2Iter = recastOther.getDefsIterator(def1); 

        // loop over all use1,def1 pairs
        //OA_ptr<AliasTag::AliasTagIteratorImpl> use1Iter;
        //use1Iter = use1Set->getLocIterator();
        //OA_ptr<AliasTag::AliasTagIteratorImpl> use1Iter(use1Set.clone());
       DataFlow::DataFlowSetImplIterator<Alias::AliasTag> use1Iter(use1Set); 
        for ( ; use1Iter.isValid(); ++use1Iter ) {

            Alias::AliasTag use1 = use1Iter.current();

            // get all the defs in use2, def2 where def1 overlaps with use2
            for (def2Iter->reset(); def2Iter->isValid(); ++(*def2Iter) ) {

                tempDFSet->insertDep(use1, def2Iter->current());

                /*
                if (debug) {
                    std::cout << "inserting <use1,def2Iter->current> = "; 
                    std::cout << use1; 
                    std::cout << " , ";
                    std::cout << def2Iter->current(); 
                    std::cout << std::endl;
                }
                */
            }
        }
    }

    // iterate over deps in recastOther keying off the use
    // see if the use overlaps with anything in this
    // even if it doesn't insert dep pairs for use,def 
    // as long as use,use hasn't been removed from this
    for (mapIter=recastOther.mUseToDefsMap.begin();
         mapIter!=recastOther.mUseToDefsMap.end(); mapIter++ )
    {
        Alias::AliasTag use2 = mapIter->first;
        if (debug) {
            std::cout << "use2 = "; std::cout << use2 << std::endl;
        }
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > def2Set;
        def2Set = mapIter->second;
        
        assert(!def2Set->isEmpty());
    
        /*
        if (debug) {
            std::cout << "def2Set = ";
            def2Set.dump(std::cout);
        }
        */

        OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > use1Iter;
        use1Iter = recastOther.getDefsIterator(use2);

        // loop over all use1,def1 pairs
        //OA_ptr<AliasTag::AliasTagIteratorImpl> use1Iter;
        //use1Iter = use1Set->getLocIterator();
        //OA_ptr<AliasTag::AliasTagIteratorImpl> use1Iter(use1Set.clone());
        DataFlow::DataFlowSetImplIterator<Alias::AliasTag> def2Iter(def2Set);

        //OA_ptr<AliasTag::AliasTagIteratorImpl> use1Iter;
        //use1Iter = getUsesIterator(use2);

        // loop over all use2,def2 pairs
        //OA_ptr<AliasTag::AliasTagIteratorImpl> def2Iter(def2Set.clone());


        for ( ; def2Iter.isValid(); ++def2Iter) {
            Alias::AliasTag def2 = def2Iter.current();

            // get all the uses in use1,def1 where def1 overlaps with use2
            for ( use1Iter->reset(); use1Iter->isValid(); ++(*use1Iter)) {
                tempDFSet->insertDep(use1Iter->current(), def2);
                /*
                if (debug) {
                    std::cout << "inserting <use1Iter->current,def2> = "; 
                    std::cout << use1Iter->current();
                    std::cout << " , ";
                    std::cout << def2; 
                    std::cout << std::endl;
                }
                */
            }
        }
    }

    (*this) = *tempDFSet;
    if (debug) {
        std::cout << "\t*this = ";
        dump(std::cout);
    }
    

    return (*this);
}

//*****************************************************************
// Output
//*****************************************************************
void DepDFSet::dump(std::ostream &os) 
{
    /*
    os << "\tuses" << std::endl;
    // iterate over all uses
    std::map<OA_ptr<Location>,OA_ptr<DataFlow::LocDFSet> >::const_iterator 
        mapIter;
    for (mapIter=mUseToDefsMap.begin();
         mapIter!=mUseToDefsMap.end(); mapIter++ )
    {
        OA_ptr<int> use = mapIter->first;
        OA_ptr<DataFlow::DataFlowSetImpl<int> > aliasset = mapIter->second;

        os << "\t\t";
        os << use;
        os << " ==> ";
        aliasset->dump(os);
        os << std::endl;
    }
    
    os << "\tdefs" << std::endl;
    // iterate over all defs
    for (mapIter=mDefToUsesMap.begin();
         mapIter!=mDefToUsesMap.end(); mapIter++ )
    {
        OA_ptr<int> def = mapIter->first;
        OA_ptr<DataFlow::DataFlowSetImpl<int> > aliasset = mapIter->second;

        os << "\t\t";
        os << def;
        os << " ==> ";
        aliasset->dump(os);
        os << std::endl;
    }
   
    os << "\tmImplicitRemoves = ";
    mImplicitRemoves->dump(os);
  
    os << "\tmUses = ";
    mUses->dump(os);
  
    os << "\tmDefs = ";
    mDefs->dump(os);

    */
}

void DepDFSet::dump(std::ostream &os) const
{
    /*
    os << "\tuses" << std::endl;
    // iterate over all uses
    std::map<OA_ptr<Location>,OA_ptr<DataFlow::LocDFSet> >::const_iterator 
        mapIter;
    for (mapIter=mUseToDefsMap.begin();
         mapIter!=mUseToDefsMap.end(); mapIter++ )
    {
        OA_ptr<Location> use = mapIter->first;
        OA_ptr<DataFlow::LocDFSet> locset = mapIter->second;

        os << "\t\t";
        use->dump(os);
        os << " ==> ";
        locset->dump(os);
        os << std::endl;
    }
    
    os << "\tdefs" << std::endl;
    // iterate over all defs
    for (mapIter=mDefToUsesMap.begin();
         mapIter!=mDefToUsesMap.end(); mapIter++ )
    {
        OA_ptr<Location> def = mapIter->first;
        OA_ptr<DataFlow::LocDFSet> locset = mapIter->second;

        os << "\t\t";
        def->dump(os);
        os << " ==> ";
        locset->dump(os);
        os << std::endl;
    }
   
    os << "\tmImplicitRemoves = ";
    mImplicitRemoves->dump(os);
  
    os << "\tmUses = ";
    mUses->dump(os);
  
    os << "\tmDefs = ";
    mDefs->dump(os);
    */
 }


void DepDFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir) 
{
    /*
    os << "\tuses" << std::endl;
    // iterate over all uses
    std::map<OA_ptr<Location>,OA_ptr<DataFlow::LocDFSet> >::const_iterator mapIter;
    for (mapIter=mUseToDefsMap.begin();
         mapIter!=mUseToDefsMap.end(); mapIter++ )
    {
        OA_ptr<Location> use = mapIter->first;
        OA_ptr<DataFlow::LocDFSet> locset = mapIter->second;

        os << "\t\t";
        use->dump(os,ir);
        os << " ==> ";
        locset->dump(os,ir);
        os << std::endl;
    }
    
    os << "\tdefs" << std::endl;
    // iterate over all defs
    for (mapIter=mDefToUsesMap.begin();
         mapIter!=mDefToUsesMap.end(); mapIter++ )
    {
        OA_ptr<Location> def = mapIter->first;
        OA_ptr<DataFlow::LocDFSet> locset = mapIter->second;

        os << "\t\t";
        def->dump(os,ir);
        os << " ==> ";
        locset->dump(os,ir);
        os << std::endl;
    }
    
    // dump implicitRemoves
    os << "\tmImplicitRemoves = ";
    mImplicitRemoves->dump(os,ir); 
  
    os << "\tmUses = ";
    mUses->dump(os,ir);
  
    os << "\tmDefs = ";
    mDefs->dump(os,ir);
    
    */
}

void DepDFSet::output(OA::IRHandlesIRInterface& ir,
                      OA::Alias::Interface& alias) const
{
    std::cout << "Inside DepDFSet output" << std::endl;
    // iterate over all uses
    sOutBuild->mapStart("mUseToDefsMap","UseLoc","DefLocSet");
    std::map<Alias::AliasTag, OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > >::const_iterator mapIter;

    for (mapIter=mUseToDefsMap.begin();
         mapIter!=mUseToDefsMap.end(); mapIter++ )
    {
        Alias::AliasTag use = mapIter->first;
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > dset;
        dset = mapIter->second;

        OA::OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > dIter;
        dIter = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(dset);
        
        sOutBuild->mapEntryStart();
          sOutBuild->mapKeyStart();
            use.output(ir,alias);
          sOutBuild->mapKeyEnd();

          sOutBuild->mapValueStart();
          for( ; dIter->isValid(); ++(*dIter)) {
                 dIter->current().output(ir,alias);
          }
          sOutBuild->mapValueEnd();
          
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mUseToDefsMap");
    
    // iterate over all defs
    sOutBuild->mapStart("mDefToUsesMap","DefLoc","UseLocSet");
    for (mapIter=mDefToUsesMap.begin();
         mapIter!=mDefToUsesMap.end(); mapIter++ )
    {
        Alias::AliasTag def = mapIter->first;
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> >dset;
        dset = mapIter->second;
        sOutBuild->mapEntryStart();
          sOutBuild->mapKeyStart();
            def.output(ir,alias);
          sOutBuild->mapKeyEnd();

          OA::OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > dIter;
          dIter 
            = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(dset);

          sOutBuild->mapValueStart();
          for( ; dIter->isValid(); ++(*dIter)) {
                 dIter->current().output(ir,alias);
          }
          sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mDefToUsesMap");

  
    
    OA::OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > dIter;
    dIter 
     = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mImplicitRemoves);

    
    // dump implicitRemoves
    std::ostringstream label1;
    label1 << indt << "ImplicitRemoves:";
    sOutBuild->outputString( label1.str() );
    //mImplicitRemoves.output(ir); 
    for( ; dIter->isValid(); ++(*dIter)) {
           dIter->current().output(ir,alias);
    }


    dIter = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mUses);

    std::ostringstream label2;
    label2 << indt << "Uses:";
    sOutBuild->outputString( label2.str() );
    //mUses.output(ir); 
    for( ; dIter->isValid(); ++(*dIter)) {
           dIter->current().output(ir,alias);
    }



    dIter= new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mDefs);

    
    std::ostringstream label3;
    label3 << indt << "Defs:";
    sOutBuild->outputString( label3.str() );
    //mDefs.output(ir); 
    for( ; dIter->isValid(); ++(*dIter)) {
           dIter->current().output(ir,alias);
    }

}


//*****************************************************************
// Construction
//*****************************************************************
  
/*! 
    Inserts a dep pair <use,def> into the set of Deps.

    The reflexive pairs (eg. <a,a>) are implicitly assumed
    in the datastructure unless they have been explicitly
    removed with removeImplicitDep.  If they have,
    calling this routine will take them out of removeImplicitDep.
    This comes about when a statement
    such as (a = a + b) occurs.  a still depends on itself
    because it is on the rhs and lhs.

*/
void DepDFSet::insertDep(Alias::AliasTag use, Alias::AliasTag def)
{
    /* I dont think we will need this 01/03/07
    if (use==def) {
        mImplicitRemoves->minusEqu(use);
    } else {
    */
    /*
    if (mUseToDefsMap[use].ptrEqual(0)) {
        mUseToDefsMap[use] = new DataFlow::DataFlowSetImpl<int>(); 
    }
    if (mDefToUsesMap[def].ptrEqual(0)) {
        mDefToUsesMap[def] = new DataFlow::DataFlowSetImpl<int>();
    }
    */


    if (use==def) {
        mImplicitRemoves->remove(use);
    } else {

       if (mUseToDefsMap[use].ptrEqual(0)) {
          mUseToDefsMap[use] = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();
       }
       if (mDefToUsesMap[def].ptrEqual(0)) {
         mDefToUsesMap[def] = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();
       }

        mUseToDefsMap[use]->insert(def);
        mDefToUsesMap[def]->insert(use);

        mUses->insert(use);
        mDefs->insert(def);
    }
}

/*!
   Must allow removeImplicitDep and insertDep to be commutative on
   DepDFSet.  Made possible by taking things out of implicitRemoves
   if it gets added explicitly.
*/
void DepDFSet::removeImplicitDep(Alias::AliasTag use, Alias::AliasTag def)
{
//    mMakeImplicitExplicitMemoized = false;
    // to be conservative only insert locations when the use and def must
    // overlap each other, sublocs that only have partial accuracy will
    // only may loc
    if (use == def) {
        mImplicitRemoves->insert(use);
    }
}

  } // end of Activity namespace
} // end of OA namespace
