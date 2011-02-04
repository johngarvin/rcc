/*! \file
  
  \brief Declarations for the AliasMap implementation of interprocedural 
         Alias results.

  \authors Michelle Strout
  \version $Id: InterAliasMap.hpp,v 1.2.6.1 2006/01/18 23:28:42 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterAliasMap_H
#define InterAliasMap_H

#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <map>

namespace OA {
  namespace Alias {

class InterAliasMap : public virtual InterAliasInterface,
                      public virtual Annotation {
  public:
    InterAliasMap() {}
    virtual ~InterAliasMap() {}

    //! Returns alias analysis results for the given procedure
    OA_ptr<Alias::Interface> getAliasResults(ProcHandle proc)
      { assert(!mProcToAliasMap[proc].ptrEqual(0));
        return mProcToAliasMap[proc];
      }

    //! Returns alias analysis results for the given procedure
    //! that are specifically of type AliasMap.
    //! Oh if only covariant returns worked.
    OA_ptr<Alias::AliasMap> getAliasMapResults(ProcHandle proc)
      { assert(!mProcToAliasMap[proc].ptrEqual(0));
        return mProcToAliasMap[proc];
      }

    //*****************************************************************
    // Output
    //*****************************************************************
    //! regression output
    void output(IRHandlesIRInterface& ir) const
      {
          sOutBuild->objStart("InterAliasMap");
          std::map<ProcHandle,OA_ptr<AliasMap> >::iterator mapIter;
          for (mapIter=mProcToAliasMap.begin();
               mapIter!=mProcToAliasMap.end();
               mapIter++) 
          {
              OA_ptr<AliasMap> alias = mapIter->second;
              alias->output(ir);
          }
          sOutBuild->objEnd("InterAliasMap");

      }
    //! debugging output
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
      {
        os << "====================== InterAliasMap" << std::endl;
        std::map<ProcHandle,OA_ptr<AliasMap> >::iterator mapIter;
        for (mapIter=mProcToAliasMap.begin();
             mapIter!=mProcToAliasMap.end();
             mapIter++) 
        {
            OA_ptr<AliasMap> alias = mapIter->second;
            alias->dump(os,ir);
            os << "-------------------------------" << std::endl;
        }

      }
    
    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! attach AliasMap to procedure
    void mapProcToAliasMap(ProcHandle proc, OA_ptr<AliasMap> alias)
      { mProcToAliasMap[proc] = alias; }

  private:
    OA_ptr<ManagerAliasMapBasic> mAliasMapMan;
    std::map<ProcHandle,OA_ptr<AliasMap> > mProcToAliasMap;


};

  } // end of Alias namespace
} // end of OA namespace

#endif

