/*! \file
    
  \brief Definition of StrToHandle class

  \authors Michelle Strout, Andy Stone
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <string>
#include <map>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#ifndef STRTOHANDLE_H
#define STRTOHANDLE_H

//! Class that maps each unique string to a unique handle value.
class StrToHandle 
{
public:
    StrToHandle() : mCount(1) {}
    ~StrToHandle() {}

    OA::ProcHandle getProcHandle(std::string str)
    {
        // see if we need to make one
        if (mStrToProcHandle.find(str)==mStrToProcHandle.end()) { 
            mStrToProcHandle[str] = OA::ProcHandle(mCount++);
        }
        return mStrToProcHandle[str];
    }

    OA::SymHandle getSymHandle(std::string str)
    {
        // see if we need to make one
        if (mStrToSymHandle.find(str)==mStrToSymHandle.end()) { 
            mStrToSymHandle[str] = OA::SymHandle(mCount++);
        }
        return mStrToSymHandle[str];
    }

    OA::StmtHandle getStmtHandle(std::string str)
    {
        // see if we need to make one
        if (mStrToStmtHandle.find(str)==mStrToStmtHandle.end()) {
            mStrToStmtHandle[str] = OA::StmtHandle(mCount++);
        }
        return mStrToStmtHandle[str];
    }

    OA::MemRefHandle getMemRefHandle(std::string str)
    {
        // see if we need to make one
        if (mStrToMemRefHandle.find(str)==mStrToMemRefHandle.end()) {
            mStrToMemRefHandle[str] = OA::MemRefHandle(mCount++);
        }
        return mStrToMemRefHandle[str];
    }

    OA::CallHandle getCallHandle(std::string str)
    {
        // see if we need to make one
        if (mStrToCallHandle.find(str)==mStrToCallHandle.end()) {
            mStrToCallHandle[str] = OA::CallHandle(mCount++);
        }
        return mStrToCallHandle[str];
    }


private:
    int mCount;
    std::map<std::string,OA::ProcHandle>    mStrToProcHandle;
    std::map<std::string,OA::SymHandle>     mStrToSymHandle;
    std::map<std::string,OA::StmtHandle>    mStrToStmtHandle;
    std::map<std::string,OA::MemRefHandle>  mStrToMemRefHandle;
    std::map<std::string,OA::CallHandle>    mStrToCallHandle;
};

#endif
