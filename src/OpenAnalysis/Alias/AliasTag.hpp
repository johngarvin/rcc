/*! \file
  
  \brief Represents a unique location

  \authors Andy Stone
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasTag_H
#define AliasTag_H

#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Alias {

class Interface;

//! Abstraction used by OA's alias analyses to represent a unique location.
class AliasTag : public Annotation {
  public:
    //************************************************************
    // Constructors
    //************************************************************
    AliasTag() { mTagID = -1; }

    AliasTag(int tagID) { mTagID = tagID; }

    //! Return the integer value which is used to encode this tag
    int val() const {
        return mTagID;
    }

    //************************************************************
    // Comparison
    //************************************************************
    //! an ordering for tags, needed for use within STL containers
    bool operator<(const AliasTag &other) const {
        // just compare by tag value
        return val() < other.val();
    }

    //! check if two tags are equal
    bool operator==(const AliasTag &other) const {
        // just compare by tag value
        return val() == other.val();
    }

    //************************************************************
    // Output
    //************************************************************
    virtual void output(OA::IRHandlesIRInterface& ir) const;

    virtual void output(OA::IRHandlesIRInterface& ir,
                        const Interface& aliasResults) const;

    void dump(
        std::ostream& os,
        IRHandlesIRInterface& ir,
        Interface& aliasResults) const;

  std::string toString(OA::IRHandlesIRInterface& ir) const;
  std::string toString(OA::IRHandlesIRInterface& ir,
                       const Interface& aliasResults) const;

  private:
    int mTagID;

};
std::ostream &operator<<(
    std::ostream &os,
    const AliasTag &tag);

  }
}

#endif
