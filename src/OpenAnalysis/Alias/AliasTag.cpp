#include "AliasTag.hpp"

#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Alias {

void AliasTag::output(OA::IRHandlesIRInterface& ir) const
{
    sOutBuild->objStart("Tag");
    sOutBuild->field("mTagID", int2string(mTagID));
    sOutBuild->objEnd("Tag");
}

void AliasTag::output(
    OA::IRHandlesIRInterface& ir,
    const Interface& aliasResults) const
{
    sOutBuild->objStart("Tag");
    sOutBuild->field("mTagID", int2string(mTagID));

    // MMS, 8/19/08, do not want MRE for each alias tag
    /*
    // iterate and output all MREs asscociated with this tag as comments
    OA_ptr<MemRefExprIterator> mreIter;
    mreIter = aliasResults.getMemRefExprIterator(mTagID);
    for(; mreIter->isValid(); ++(*mreIter)) {
        sOutBuild->comment(mreIter->current()->comment(ir));
    }
    */

    sOutBuild->objEnd("Tag");
}

void AliasTag::dump(
    std::ostream& os,
    IRHandlesIRInterface& ir,
    Interface& aliasResults) const
{
    bool first = true;

    os << "<mTagID: " << int2string(mTagID) << ", MREs: \n";

    // iterate through each mre
    OA_ptr<MemRefExprIterator> mreIter;
    mreIter = aliasResults.getMemRefExprIterator(mTagID);
    for(; mreIter->isValid(); ++(*mreIter)) {
        if(!first) {
            os << ", \n";
        } else {
            first = false;
        }
	os << "\t\t" << mreIter->current()->comment(ir);
    }

    os << "\n>";
}

std::string AliasTag::toString(OA::IRHandlesIRInterface& ir) const
{
  std::ostringstream os;
  os << "tag(" << mTagID << ")";
  return os.str();
}


std::string AliasTag::toString(OA::IRHandlesIRInterface& ir,
                       const Interface& aliasResults) const
{
  std::ostringstream os;
  os << "tag<" << mTagID;

  //bool first = true;
  //
  //  // iterate through each mre
  //  OA_ptr<MemRefExprIterator> mreIter;
  //  mreIter = aliasResults.getMemRefExprIterator(mTagID);
  //  for(; mreIter->isValid(); ++(*mreIter)) {
  //      if(!first) {
  //          os << ", ";
  //      } else {
  //          first = false;
  //      }
  //      mreIter->current()->dump(os, ir);
  //  }

  os << ">";


  return os.str();

}



std::ostream &operator<<(
    std::ostream &os,
    const AliasTag &tag)
{
    os << "tag(" << tag.val() << ")";
    return os;
}

} }

