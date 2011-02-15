// LivenessStandard.cpp


#include "LivenessStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Liveness {

using namespace Alias;
using namespace std;

OA_ptr<LivenessIterator> LivenessStandard::getLivenessIterator(StmtHandle s) 
    const
{ 
    OA_ptr<LivenessIterator> retval;

    if (mLiveness.find(s) == mLiveness.end()) {
        OA_ptr<set<AliasTag> > emptySet;
        emptySet = new set<AliasTag>();
        retval = new LivenessStandardIterator(emptySet);
    } else {
        OA_ptr<set<AliasTag> > ptr = mLiveness.find(s)->second;
        retval = new LivenessStandardIterator(ptr);
    }
    return retval;
}

OA_ptr<LivenessIterator> LivenessStandard::getExitLivenessIterator() const
{ 
    OA_ptr<LivenessIterator> retval;
     if (mExitLiveness.ptrEqual(0)) {
        OA_ptr<set<AliasTag> > emptySet;
        emptySet = new set<AliasTag>();
        retval = new LivenessStandardIterator(emptySet);
    }
    else {
        retval = new LivenessStandardIterator(mExitLiveness);
    }
    return retval;
}

void LivenessStandard::output(IRHandlesIRInterface& ir) const
{    
    sOutBuild->objStart("LivenessStandard");

    sOutBuild->mapStart("mLiveness", "StmtHandle", "OA_ptr<set<AliasTag> >");
    std::map<StmtHandle,OA_ptr<set<AliasTag> > >::const_iterator mapIter;
    for(mapIter = mLiveness.begin(); mapIter != mLiveness.end(); mapIter++) {
        StmtHandle s = mapIter->first;
        OA_ptr<set<AliasTag> > rdset = mapIter->second;
        if ( rdset.ptrEqual(0) ) continue;

        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(s, ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
       
       
        sOutBuild->listStart();
        OA_ptr<LivenessIterator> setIter;
        setIter = getLivenessIterator(s);
        for ( ; setIter->isValid(); (*setIter)++ ) {
             sOutBuild->listItemStart(); {
             setIter->current().output(ir);  
            }
        sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();  
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mLiveness");
   
    sOutBuild->objEnd("LivenessStandard");
}



void LivenessStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    std::cout<< "======Liveness===="<< std::endl;
    OA_ptr<LivenessIterator> locIterPtr;
    std::map<StmtHandle,OA_ptr<set<AliasTag> > >::iterator mapIter;
    for (mapIter = mLiveness.begin(); mapIter != mLiveness.end(); mapIter++) {
        std::cout<< "\tstmt = " << ir->toString(mapIter->first) << std::endl; 
         locIterPtr = getLivenessIterator(mapIter->first);
        for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
            std::cout << "\t\t";
            locIterPtr->current().output(*ir);
            std::cout << std::endl;
        }

    }
}



  } // end of Liveness namespace
} // end of OA namespace
