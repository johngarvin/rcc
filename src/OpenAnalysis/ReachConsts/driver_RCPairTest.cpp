//! ===========================================================
//! 
//! $Id: driver_RCPairTest, 05/20/2008 PLM, 07/09/08 BK $
//!
//! Testing RCPairs:
//!
//! - Test for
//!   - operator == 
//!   - operator !=
//!   - operator <
//!
//! - Test includes
//!   - RCPairs with same AliasTag and different constant values
//!   - RCPairs with same AliasTag and same constant values
//!   - RCPairs with Different AliasTags. 
//!   - RCPairSet::Intersection()
//! ===========================================================


#include "RCPair.hpp"
#include "RCPairSet.hpp"
#include <set>
#include <vector>

using namespace OA::ReachConsts;

//! =======================================================
//! Constant Class equivalent to Open64IRIntegerConstant
//! =======================================================
class IntegerConstVal : public OA::ConstValBasicInterface {

  public:

    //! ===== Constructor/Destructor =====
  IntegerConstVal() { }
  IntegerConstVal(int aVal) : mVal(aVal) { }
  ~IntegerConstVal() { }

    //! ===== Access Methods =====
  bool isaInteger() const { return true; }
  int getIntegerVal() const { return mVal; }

    
    //! ===== Constant Value comparision =====
  bool operator<(OA::ConstValBasicInterface& other) {
    IntegerConstVal& otherRecast = dynamic_cast<IntegerConstVal&>(other);
    return (mVal < otherRecast.getIntegerVal());
  }

  bool operator==(OA::ConstValBasicInterface& other) {
    IntegerConstVal& otherRecast = dynamic_cast<IntegerConstVal&>(other);
    return (otherRecast.getIntegerVal() == mVal);
  }

  bool operator!=(OA::ConstValBasicInterface& other) {
    IntegerConstVal& otherRecast = dynamic_cast<IntegerConstVal&>(other);
    return (mVal != otherRecast.getIntegerVal());
  }


    //! ===== Output =====
  std::string toString() {
    std::ostringstream oss; oss << mVal; return oss.str(); 
  }



  private:

    int mVal;
};

void printTagSet(OA::OA_ptr<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> > tagset) 
{
  std::cout << "{ ";
  OA::OA_ptr<OA::DataFlow::DataFlowSetImplIterator<OA::Alias::AliasTag> > pIter;
  pIter = new OA::DataFlow::DataFlowSetImplIterator<OA::Alias::AliasTag>(tagset);
  for (; pIter->isValid(); (*pIter)++) {
    OA::Alias::AliasTag tag = pIter->current();
    std::cout << tag << ", ";
  }
  std::cout << " }";
}



int main()
{
  int debug = true;

   OA::OA_ptr<OA::ConstValBasicInterface> 
     constInter3, constInter5, constInter7, constInter8,
     constInter1, constInter0, constInter6,
     constInter9, constInter2, constInter4, nullVal;


   //! ===== Creating Constants =====  
   constInter0 = new IntegerConstVal(0);
   constInter1 = new IntegerConstVal(1);
   constInter2 = new IntegerConstVal(2);
   constInter3 = new IntegerConstVal(3);
   constInter4 = new IntegerConstVal(4);
   constInter5 = new IntegerConstVal(5);
   constInter6 = new IntegerConstVal(6);
   constInter7 = new IntegerConstVal(7);
   constInter8 = new IntegerConstVal(8);
   constInter9 = new IntegerConstVal(9);
   nullVal     = 0;


   //! ===== Creating RCPairs =====
   RCPair rc1a(1, constInter3);
   RCPair rc1b(1, constInter5);
   RCPair rc1c(1, constInter3);
   RCPair rc2a(2, constInter3);
   RCPair rc2c(2, constInter7);

   // FIXME
   //rc2b = new RCPair(2, nullVal); // Hmmm, how should these be handled?

   //! ===== Creating two sets with the same pairs ====
   /*
   OA::OA_ptr<RCPairSet> set1, set2;

   set1 = new RCPairSet();
   set2= new RCPairSet();

   set1->insert(rc1a);
   set2->insert(rc1a);

   set1->insert(rc2a);
   set2->insert(rc2a);

   std::cout << "testing\n";
   std::cout.flush();

   std::cout << "set1: " << set1->toString() << std::endl;
   std::cout.flush();

   std::cout << "set2: " << set2->toString() << std::endl;
   std::cout.flush();

   assert( *set1 == *set2);
   */


   //! ===== Creating two static sets with the same pairs =====
   /*
   RCPairSet set3;
   RCPairSet set4;

   set3.insert(rc1a);
   set4.insert(rc1a);

   set3.insert(rc2a);
   set4.insert(rc2a);

   std::cout << "testing\n";
   std::cout.flush();

   std::cout << "set3: " << set3.toString() << std::endl;
   std::cout.flush();

   std::cout << "set4: " << set4.toString() << std::endl;
   std::cout.flush();

   assert(set1==set2);
   */


   //! ===== Output RCPair =====
   std::cout << "================================" << std::endl;
   std::cout << "              RCPair            " << std::endl;
   std::cout << "rc1a : " << rc1a << std::endl;
   std::cout << "rc1b : " << rc1b << std::endl;
   std::cout << "rc1c : " << rc1c << std::endl;
   std::cout << "rc2a : " << rc2a << std::endl;
   std::cout << "rc2c : " << rc2c << std::endl;

   //! ===== Testing begins here =====

   std::cout << "================================" << std::endl;
   std::cout << "Testing RCPair::operator==" << std::endl;

   assert(rc1a == rc1a);
   assert((rc1a == rc1b) == 0); // diff const values
   assert((rc1a == rc1c) == 1);
   assert((rc1a == rc2a) == 0); // diff tag values (same const value)


   std::cout << "================================" << std::endl;
   std::cout << "Testing RCPair::operator!=" << std::endl;

   assert((rc1a != rc1a) == 0);
   assert((rc2c != rc1c) == 1);
   assert((rc1a != rc1c) == 0);


   std::cout << "================================" << std::endl;
   std::cout << "Testing RCPair::operator<" << std::endl;


   assert((rc1a < rc1a) == 0);
   assert((rc1a < rc1b) == 1); // same tags, cI3 < cI5
   assert((rc1a < rc1c) == 0);
   assert((rc1a < rc2a) == 1); // tag1 < tag2 
   assert((rc2c < rc1a) == 0); // tag1 > tag2 

   //! ====================== RCPairSet Test ===========================

   std::cout << "========== Testing RCPairSet ==========" << std::endl;

   OA::OA_ptr<RCPairSet> rSet1;
   rSet1 = new RCPairSet;
   rSet1->insert(rc1a);
   rSet1->insert(rc2a);
   std::cout << "RCPairSet 1 : " << rSet1->toString() << std::endl; 
   std::cout << std::endl;

   OA::OA_ptr<RCPairSet> rSet2;
   rSet2 = new RCPairSet;
   rSet2->insert(rc1c);
   std::cout << "RCPairSet 2 : " << rSet2->toString() << std::endl; 

   OA::OA_ptr<RCPairSet> rSet3;
   rSet3 = new RCPairSet;
   rSet3->insert(rc1a);
   rSet3->insert(rc1b);
   //rSet3->insert(rc1c);
   rSet3->insert(rc2c);
   std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 

   OA::OA_ptr<RCPairSet> rSet4;
   rSet4 = new RCPairSet;
   rSet4->setUniversal();
   std::cout << "RCPairSet 4 : " << rSet4->toString() << std::endl; 

   OA::OA_ptr<RCPairSet> rSet5;
   rSet5 = new RCPairSet;
   rSet5->setUniversal();
   std::cout << "RCPairSet 5 : " << rSet5->toString() << std::endl; 

   std::cout << "Meet::RCPairSet1 - Intersection - RCPairSet1" 
             << std::endl
             << rSet1->intersectEqu(*rSet1).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet1 - Intersection - RCPairSet2" 
             << std::endl
             << rSet1->intersectEqu(*rSet2).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet1 - Intersection - RCPairSet3" 
             << std::endl
             << rSet1->intersectEqu(*rSet3).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet3 - Intersection - RCPairSet1" 
             << std::endl
             << rSet3->intersectEqu(*rSet1).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet1 - Intersection - RCPairSet4" 
             << std::endl
             << rSet1->intersectEqu(*rSet4).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet4 - Intersection - RCPairSet1" 
             << std::endl
             << rSet1->intersectEqu(*rSet4).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet2 - Intersection - RCPairSet3" 
             << std::endl
             << rSet2->intersectEqu(*rSet3).toString()
             << std::endl;

   std::cout << "Meet::RCPairSet4 - Intersection - RCPairSet5" 
             << std::endl
             << rSet4->intersectEqu(*rSet5).toString()
             << std::endl;
  
   // ====== Testing with Partial Universal sets ======

   {
     // IntersectEqu
     std::cout << "======= Testing RCPairSet IntersectEqu ==" << std::endl;


     OA::OA_ptr<RCPairSet> rSet1;
     rSet1 = new RCPairSet;
     rSet1->insert(rc1a);
     rSet1->insert(rc2a);
     std::cout << "RCPairSet 1 : " << rSet1->toString() << std::endl; 
     std::cout << std::endl;
     
     OA::OA_ptr<RCPairSet> rSet2;
     rSet2 = new RCPairSet;
     rSet2->insert(rc1c);
     std::cout << "RCPairSet 2 : " << rSet2->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet3;
     rSet3 = new RCPairSet;
     rSet3->insert(rc1a);
     rSet3->insert(rc1b);
     //rSet3->insert(rc1c);
     rSet3->insert(rc2c);
     std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet5;
     rSet5 = new RCPairSet;
     rSet5->setUniversal();
     std::cout << "RCPairSet 5 : " << rSet5->toString() << std::endl; 

     OA::OA_ptr<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> > tagSet;
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(5));
     tagSet->insert(OA::Alias::AliasTag(7));
     OA::OA_ptr<RCPairSet> pSet6;
     pSet6 = new RCPairSet(tagSet);
     std::cout << "RCPairSet 6 : " << pSet6->toString() << std::endl;

     OA::OA_ptr<RCPairSet> pSet7;
     pSet7 = new RCPairSet(tagSet);
     pSet7->insert(RCPair(2,constInter5));
     pSet7->insert(RCPair(6,constInter7));
     pSet7->insert(RCPair(8,constInter9));
     std::cout << "RCPairSet 7 : " << pSet7->toString() << std::endl;
     
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(2));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(4));
     OA::OA_ptr<RCPairSet> pSet8;
     pSet8 = new RCPairSet(tagSet);
     pSet8->insert(RCPair(5,constInter3));
     pSet8->insert(RCPair(6,constInter8));
     pSet8->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 8 : " << pSet8->toString() << std::endl;

     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(3));
     OA::OA_ptr<RCPairSet> pSet7i8;
     pSet7i8 = new RCPairSet(tagSet);
     pSet7i8->insert(RCPair(5,constInter3));
     pSet7i8->insert(RCPair(2,constInter5));
     pSet7i8->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 7i8 : " << pSet7i8->toString() << std::endl;

     // Explicit intEqu Partial-only
     // rSet1 has one pair with tag in pSet6,
     //       and one pair with tag not in pSet6
     // only the first pair should come through the intersection

     std::cout << "Meet::RCPairSet1 - Intersection - RCPairSet6" 
               << std::endl;
     std::cout << rSet1->intersectEqu(*pSet6).toString()
               << std::endl;
     assert(rSet1==rSet2);

     // Universal intEqu Partial-Only
     // rSet5 is Universal, pSet6 is Partial-Universal

     std::cout << "Meet::RCPairSet5 - Intersection - RCPairSet6" 
               << std::endl;
     std::cout << rSet5->intersectEqu(*pSet6).toString()
               << std::endl;
     assert(rSet5==pSet6);

     // Partial-Pairs intEqu Partial-Pairs
     // pSet7 and pSet8 have same and different tags
     // pSet7 and pSet8 have same and different pairs
     // pSet7 has a tag that pSet8 has in a pair
     // pSet8 has a tag that pSet7 has in a pair

     std::cout << "Meet::RCPairSet7 - Intersection - RCPairSet8" 
               << std::endl;
     std::cout << pSet7->intersectEqu(*pSet8).toString()
               << std::endl;
     assert(pSet7==pSet7i8);

     // Partial-Pair intEqu Explicit
     
     std::cout << "Meet::RCPairSet7i8 - Intersection - RCPairSet3" 
               << std::endl;
     std::cout << pSet7i8->intersectEqu(*rSet3).toString()
               << std::endl;
     rSet3->remove(RCPair(2,constInter7));
     assert(pSet7i8==rSet3);

   }

   {
     // unionEqu
     std::cout << "======= Testing RCPairSet UnionEqu ==" << std::endl;

     OA::OA_ptr<RCPairSet> rSet1;
     rSet1 = new RCPairSet;
     rSet1->insert(rc1a);
     rSet1->insert(rc2a);
     std::cout << "RCPairSet 1 : " << rSet1->toString() << std::endl; 
     std::cout << std::endl;
     
     OA::OA_ptr<RCPairSet> rSet2;
     rSet2 = new RCPairSet;
     rSet2->insert(rc1c);
     std::cout << "RCPairSet 2 : " << rSet2->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet3;
     rSet3 = new RCPairSet;
     rSet3->insert(rc1a);
     rSet3->insert(rc1b);
     //rSet3->insert(rc1c);
     rSet3->insert(rc2c);
     rSet3->insert(RCPair(9,constInter9));
     std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet4;
     rSet4 = new RCPairSet;
     rSet4->setUniversal();
     std::cout << "RCPairSet 4 : " << rSet4->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet5;
     rSet5 = new RCPairSet;
     rSet5->setUniversal();
     std::cout << "RCPairSet 5 : " << rSet5->toString() << std::endl; 

     OA::OA_ptr<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> > tagSet;
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(5));
     tagSet->insert(OA::Alias::AliasTag(7));
     OA::OA_ptr<RCPairSet> pSet6;
     pSet6 = new RCPairSet(tagSet);
     std::cout << "RCPairSet 6 : " << pSet6->toString() << std::endl;

     OA::OA_ptr<RCPairSet> pSet7;
     pSet7 = new RCPairSet(tagSet);
     pSet7->insert(RCPair(2,constInter5));
     pSet7->insert(RCPair(6,constInter7));
     pSet7->insert(RCPair(8,constInter9));
     std::cout << "RCPairSet 7 : " << pSet7->toString() << std::endl;
     
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(2));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(4));
     OA::OA_ptr<RCPairSet> pSet8;
     pSet8 = new RCPairSet(tagSet);
     pSet8->insert(RCPair(5,constInter3));
     pSet8->insert(RCPair(6,constInter8));
     pSet8->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 8 : " << pSet8->toString() << std::endl;

     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(2));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(4));
     tagSet->insert(OA::Alias::AliasTag(5));
     tagSet->insert(OA::Alias::AliasTag(7));
     OA::OA_ptr<RCPairSet> pSet7u8;
     pSet7u8 = new RCPairSet(tagSet);
     pSet7u8->insert(RCPair(6,constInter7));
     pSet7u8->insert(RCPair(6,constInter8));
     pSet7u8->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 7u8 : " << pSet7u8->toString() << std::endl;

     // Explicit unionEqu Partial-only
     // rSet1 has one pair with tag in pSet6,
     //       and one pair with tag not in pSet6
     // only the second pair should come through the union
     // along with all tags from partial

     std::cout << "Meet::RCPairSet1 - Union - RCPairSet6" 
               << std::endl;
     std::cout << rSet1->unionEqu(*pSet6).toString()
               << std::endl;
     // assert();

     // Partial unionEqu Universal and v.v.
     // rSet5 is Universal, pSet6 is Partial-Universal

     std::cout << "Meet::RCPairSet5 - Union - RCPairSet6" 
               << std::endl;
     std::cout << rSet5->unionEqu(*pSet6).toString()
               << std::endl;
     assert(rSet5->isUniversalSet());

     std::cout << "Meet::RCPairSet6 - Union - RCPairSet5" 
               << std::endl;
     std::cout << pSet6->unionEqu(*rSet5).toString()
               << std::endl;
     assert(pSet6->isUniversalSet());

     // Partial-Pairs unionEqu Partial-Pairs
     // pSet7 and pSet8 have same and different tags
     // pSet7 and pSet8 have same and different pairs
     // pSet7 has a tag that pSet8 has in a pair
     // pSet8 has a tag that pSet7 has in a pair

     std::cout << "Meet::RCPairSet7 - Union - RCPairSet8" 
               << std::endl;
     std::cout << pSet7->unionEqu(*pSet8).toString()
               << std::endl;
     assert(pSet7==pSet7u8);

     // Partial-Pair unionEqu Explicit
     
     std::cout << "Meet::RCPairSet7u8 - Union - RCPairSet3" 
               << std::endl;
     std::cout << pSet7u8->unionEqu(*rSet3).toString()
               << std::endl;
     //assert();
   }

   {
     // minusEqu
     std::cout << "======= Testing RCPairSet MinusEqu ==" << std::endl;

     OA::OA_ptr<RCPairSet> rSet1;
     rSet1 = new RCPairSet;
     rSet1->insert(rc1a);
     rSet1->insert(rc2a);
     std::cout << "RCPairSet 1 : " << rSet1->toString() << std::endl; 
     std::cout << std::endl;
     
     OA::OA_ptr<RCPairSet> rSet1copy;
     rSet1copy = new RCPairSet;
     rSet1copy->insert(rc1a);
     rSet1copy->insert(rc2a);
     std::cout << "RCPairSet 1copy : " << rSet1copy->toString() << std::endl; 
     std::cout << std::endl;
     
     OA::OA_ptr<RCPairSet> rSet1m2;
     rSet1m2 = new RCPairSet;
     rSet1m2->insert(rc2a);
     std::cout << "RCPairSet 1m2: " << rSet1m2->toString() << std::endl; 
     std::cout << std::endl;
     
     OA::OA_ptr<RCPairSet> rSet2;
     rSet2 = new RCPairSet;
     rSet2->insert(rc1c);
     std::cout << "RCPairSet 2 : " << rSet2->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet3;
     rSet3 = new RCPairSet;
     rSet3->insert(rc1a);
     rSet3->insert(rc1b);
     //rSet3->insert(rc1c);
     rSet3->insert(rc2c);
     rSet3->insert(RCPair(9,constInter9));
     std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet4;
     rSet4 = new RCPairSet;
     rSet4->setUniversal();
     std::cout << "RCPairSet 4 : " << rSet4->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet5;
     rSet5 = new RCPairSet;
     rSet5->setUniversal();
     std::cout << "RCPairSet 5 : " << rSet5->toString() << std::endl; 

     OA::OA_ptr<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> > tagSet;
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(5));
     tagSet->insert(OA::Alias::AliasTag(7));
     OA::OA_ptr<RCPairSet> pSet6;
     pSet6 = new RCPairSet(tagSet);
     std::cout << "RCPairSet 6 : " << pSet6->toString() << std::endl;

     OA::OA_ptr<RCPairSet> pSet7;
     pSet7 = new RCPairSet(tagSet);
     pSet7->insert(RCPair(2,constInter5));
     pSet7->insert(RCPair(6,constInter7));
     pSet7->insert(RCPair(8,constInter9));
     std::cout << "RCPairSet 7 : " << pSet7->toString() << std::endl;
     
     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(1));
     tagSet->insert(OA::Alias::AliasTag(2));
     tagSet->insert(OA::Alias::AliasTag(3));
     tagSet->insert(OA::Alias::AliasTag(4));
     OA::OA_ptr<RCPairSet> pSet8;
     pSet8 = new RCPairSet(tagSet);
     pSet8->insert(RCPair(5,constInter3));
     pSet8->insert(RCPair(6,constInter8));
     pSet8->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 8 : " << pSet8->toString() << std::endl;

     tagSet = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>;
     tagSet->insert(OA::Alias::AliasTag(2));
     tagSet->insert(OA::Alias::AliasTag(4));
     OA::OA_ptr<RCPairSet> pSet8m6;
     pSet8m6 = new RCPairSet(tagSet);
     pSet8m6->insert(RCPair(6,constInter8));
     pSet8m6->insert(RCPair(8,constInter9));
     
     std::cout << "RCPairSet 8m6 : " << pSet8m6->toString() << std::endl;

     // Explicit minus Explicit
     // rSet1 and rSet2 have one pair in common

     std::cout << "Meet::RCPairSet1 - Minus - RCPairSet2" 
               << std::endl;
     std::cout << rSet1->minusEqu(*rSet2).toString()
               << std::endl;
     assert(rSet1==rSet1m2);

     rSet1 = rSet1copy;
     std::cout << "Meet::RCPairSet2 - Minus - RCPairSet1" 
               << std::endl;
     std::cout << rSet2->minusEqu(*rSet1).toString()
               << std::endl;
     assert(rSet2->isEmpty());

     // Explicit minus Partial-with-no-pairs
     // rSet1 has a pair with a tag in pSet6
     // result should no longer have that pair

     rSet1 = rSet1copy;
     std::cout << "Meet::RCPairSet1 - Minus - RCPairSet6" 
               << std::endl;
     std::cout << rSet1->minusEqu(*pSet6).toString()
               << std::endl;
     assert(rSet1==rSet1m2);

     // Explicit minus Partial-with-pairs
     // rSet1 has a pair with a tag in pSet7
     // result should no longer have that pair
     // rSet1 has a pair with same tag as a pair in pSet7 (diff const)
     // result should still contain the original pair

     rSet1 = rSet1copy;
     std::cout << "Meet::RCPairSet1 - Minus - RCPairSet7" 
               << std::endl;
     std::cout << rSet1->minusEqu(*pSet7).toString()
               << std::endl;
     assert(rSet1==rSet1m2);

     //Partial minus Partial-with-no-pairs

     std::cout << "Meet::RCPairSet8 - Minus - RCPairSet6" 
               << std::endl;
     std::cout << pSet8->minusEqu(*pSet6).toString()
               << std::endl;
     assert(pSet8==pSet8m6);

     // Explicit minus Universal
     // result should be empty

     rSet1 = rSet1copy;
     std::cout << "Meet::RCPairSet1 - Minus - RCPairSet5" 
               << std::endl;
     std::cout << rSet1->minusEqu(*rSet5).toString()
               << std::endl;
     assert(rSet1->isEmpty());

     // Partial minus Universal
     // result should be empty

     std::cout << "Meet::RCPairSet7 - Minus - RCPairSet5" 
               << std::endl;
     std::cout << pSet7->minusEqu(*rSet5).toString()
               << std::endl;
     assert(pSet7->isEmpty());

     // Universal minus Universal
     // result should be empty

     std::cout << "Meet::RCPairSet4 - Minus - RCPairSet5" 
               << std::endl;
     std::cout << rSet4->minusEqu(*rSet5).toString()
               << std::endl;
     assert(rSet4->isEmpty());

   }

   {
     // multiple AssignPair transfer proposal
     std::cout << "======= Testing Proposal #5 ==" << std::endl;

     int A,B,C,D,E,F,G,H,derefC,derefG;
     A=1;B=2;C=3;D=4;E=5;F=6;G=7;H=8;derefC=9;derefG=0;

     // make inSet: {<A,2>,<B,3>,<*C,4>}
     OA::OA_ptr<RCPairSet> inRecast;
     inRecast = new RCPairSet;
     inRecast->insert(RCPair(A,constInter2));
     inRecast->insert(RCPair(B,constInter3));
     inRecast->insert(RCPair(derefC,constInter4));
     std::cout << "inRecast : " << inRecast->toString() << std::endl; 
     std::cout << std::endl;
     
     // make wantedOutSet: {<A.2>,<B,3>,<E.4>,<F,7>}
     OA::OA_ptr<RCPairSet> wantedOutSet;
     wantedOutSet = new RCPairSet;
     wantedOutSet->insert(RCPair(A,constInter2));
     wantedOutSet->insert(RCPair(B,constInter3));
     wantedOutSet->insert(RCPair(E,constInter4));
     wantedOutSet->insert(RCPair(F,constInter7));
     std::cout << "wantedOutSet : " << wantedOutSet->toString() << std::endl; 
     
     // AssignPairs:
     // [0]   F = B + 4   must def, must use
     // [1]   H = F + 2   must def, must use
     // [2]   B = A + 1   must def, must use
     // [3]   E = B + 1   must def, must use
     // [4]   G = &C      may def, no use
     // [5]   *C = 5       may def, no use
     // [6]   D = *C + 1  must def, may use

     vector<OA::OA_ptr<OA::Alias::AliasTagSet> > aliasDefSet(7);
     vector<OA::OA_ptr<OA::Alias::AliasTagSet> > aliasUseSet(7);
     
     OA::OA_ptr<std::set<OA::Alias::AliasTag> > tags;
     
     // AssignPair [0]:  F = B + 4 : must def, must use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(F);
     aliasDefSet[0] = new OA::Alias::AliasTagSet(tags,true); // ({F},isMUST)

     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(B);
     aliasUseSet[0] = new OA::Alias::AliasTagSet(tags,true); // ({B},isMUST)

     // AssignPair [1]:  H = F + 2 : must def, must use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(H);
     aliasDefSet[1] = new OA::Alias::AliasTagSet(tags,true); // ({H},isMUST)

     aliasUseSet[1] = (aliasDefSet[0])->clone(); // ({F},isMUST)

     // AssignPair [2]:  B = A + 1 : must def, must use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(B);
     aliasDefSet[2] = new OA::Alias::AliasTagSet(tags,true); // ({B},isMUST)

     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(A);
     aliasUseSet[2] = new OA::Alias::AliasTagSet(tags,true); // ({A},isMUST)

     // AssignPair [3]:  E = B + 1 : must def, must use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(E);
     aliasDefSet[3] = new OA::Alias::AliasTagSet(tags,true); // ({E},isMUST)

     aliasUseSet[3] = (aliasDefSet[2])->clone(); // ({B},isMUST)

     // AssignPair [4]:  G = &C : must def, no use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(G);
     aliasDefSet[4] = new OA::Alias::AliasTagSet(tags,true); // ({G},isMUST)

     aliasUseSet[4] = new OA::Alias::AliasTagSet(false); // (empty, isNOTmust)

     // AssignPair [5]:  *C = 5 : may def, no use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(derefC);
     tags->insert(derefG);
     aliasDefSet[5] = new OA::Alias::AliasTagSet(tags,false); // ({*C,*G},isNOTmust)

     aliasUseSet[5] = new OA::Alias::AliasTagSet(false); // (empty, isNOTmust)

     // AssignPair [6]:  D = *C + 1 : must def, may use
     tags = new std::set<OA::Alias::AliasTag>;
     tags->insert(D);
     aliasDefSet[6] = new OA::Alias::AliasTagSet(tags,true); // ({E},isMUST)

     aliasUseSet[6] = (aliasDefSet[5])->clone(); // ({*C,*G}, isNOTmust)

     //==== transfer across AssignPairs
     int count = 7;
     OA::OA_ptr<OA::ConstValBasicInterface> cvbiPtr;

     // copied from ManagerICFGReachConsts.cpp:351++
     vector<OA::OA_ptr<RCPairSet> > apOut(count);
     vector<OA::OA_ptr<RCPairSet> > apKill(count);
     vector<OA::OA_ptr<RCPairSet> > apGenTop(count); // for speedup?
     vector<bool> isMustDef(count); // for speedup?
     OA::OA_ptr<RCPairSet> apDefTags; // needed?
     OA::OA_ptr<RCPairSet> genTOP;
     OA::OA_ptr<RCPairSet> mayDefKills;
     OA::OA_ptr<RCPairSet> apInAll;
     OA::OA_ptr<RCPairSet> oldApInAll;
     OA::OA_ptr<OA::DataFlow::DataFlowSetImplIterator<OA::Alias::AliasTag> > aIter;

     // 1. loop thru APs and union all of the def tags (must or may)
     // and 2. create Partial with all def tags
     genTOP = new RCPairSet();
     mayDefKills = new RCPairSet();
     int apCnt; 
     /*
       for (espIterPtr->reset(), apCnt=0; 
       espIterPtr->isValid(); 
       (*espIterPtr)++, apCnt++) {
     */
     for (apCnt = 0; apCnt<count; apCnt++) {
       //MemRefHandle mref = espIterPtr->currentTarget();
       //OA_ptr<Alias::AliasTagSet> defSet = mAlias->getAliasTags(mref);
       OA::OA_ptr<OA::Alias::AliasTagSet> defSet = aliasDefSet[apCnt]->clone();
       isMustDef[apCnt] = defSet->isMust();
       apKill[apCnt] = new RCPairSet(defSet); // makes partial with defSet tags
       genTOP->unionEqu(*(apKill[apCnt]));
       if (!defSet->isMust()) {
         mayDefKills->unionEqu(*(apKill[apCnt]));
       }
     }
     
     if (debug) {
       std::cout << "genTOP : " << genTOP->toString() << std::endl; 
     }

     // 3. set-up for convergence iterations
     /*
     for (espIterPtr->reset(), apCnt=0; 
          espIterPtr->isValid(); 
          (*espIterPtr)++, apCnt++) {
     */
     for (apCnt=0; apCnt<count; apCnt++) {
       apGenTop[apCnt] = (genTOP->clone()).convert<RCPairSet>();
       apGenTop[apCnt]->minusEqu(*(apKill[apCnt]));
       if (debug) {
         std::cout << "apGenTop[" << apCnt << "] : " 
                   << apGenTop[apCnt]->toString()
                   << std::endl;
         std::cout << "genTOP ==> " << genTOP->toString() << std::endl;
       }
     }
     apInAll = inRecast->clone().convert<RCPairSet>();

     // subtract out the mayDefs to reduce iterations
     //apInAll->minusEqu(*mayDefKills);

     bool changed = true;
     int iterationNum = -1;
     // 4. iterate: compute apOut sets and apInAll until convervence
     while (changed) {
       changed = false;
       iterationNum++;
       if (debug) {
         std::cout << "\n==== starting iteration number " << iterationNum
                   << std::endl << std::endl; 
       }

       oldApInAll = (apInAll->clone()).convert<RCPairSet>();
       
       /*
       for (espIterPtr->reset(), apCnt=0; 
            espIterPtr->isValid(); 
            (*espIterPtr)++, apCnt++) {
         */
       for (apCnt=0; apCnt<count; apCnt++) {
         if (debug) {
           std::cout << "apCnt = " << apCnt << std::endl;
         }

         // kills
         if (debug) {
           std::cout << "\t apInAll : " << apInAll->toString()
                     << std::endl;
           std::cout << "\t apKill[] : " << apKill[apCnt]->toString()
                     << std::endl;
           std::cout << "\t apGenTop[] : " << apGenTop[apCnt]->toString()
                     << std::endl;
         }
         apOut[apCnt] = (apInAll->clone()).convert<RCPairSet>();
         if (debug) {
           std::cout << "\t apInAll : " << apOut[apCnt]->toString()
                     << std::endl;
         }
         apOut[apCnt]->minusEqu(*(apKill[apCnt]));
         if (debug) {
           std::cout << "\t minus apKill : " << apOut[apCnt]->toString()
                     << std::endl;
         }
         apOut[apCnt]->unionEqu(*(apGenTop[apCnt]));
         if (debug) {
           std::cout << "\t union apGenTop[] : " << apOut[apCnt]->toString()
                     << std::endl;
         }
         
         // gens
         if (isMustDef[apCnt]) {
           
           if (debug) {
             std::cout << "\tdefSet is MUST\n";
           }
           
           cvbiPtr = NULL;
           
           /*
           ExprHandle expr = espIterPtr->currentSource();
           
           OA_ptr<ExprTree> eTreePtr;
           OA_ptr<ConstValBasicInterface> cvbiPtr;
           
           //! ===== get the expression tree for RHS =====    
           eTreePtr = mIR->getExprTree(expr);
           
           //! ===== Evaluate RHS expression =====
           EvalToConstVisitor evalVisitor(mIR, apInAll, mAlias);
           eTreePtr->acceptVisitor(evalVisitor);
           cvbiPtr = 0;
           cvbiPtr = evalVisitor.getConstVal();
           
           */
           // equivalent for testing
           switch (apCnt) {

           case 0: // F = B + 4
             {
             // from EvalToConstVisitor::visitMemRefNode():
             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
             aIter = aliasUseSet[apCnt]->getIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               RCPairSetIterator set1Iter(*apInAll);
               for (; set1Iter.isValid(); ++(set1Iter)) {
                 RCPair rc = set1Iter.current();
                 OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                 if(sTag == tag) {
                   cvbiPtr = (set1Iter.current()).getConstPtr();
                   //break;
                 }
               }
             }
             // from EvalToConstVisitor::visitOpNode(): // so to speak
             if (!(cvbiPtr.ptrEqual(0))) {
               
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal() + 4);
               
             }
             }
             break;


           case 1: //  H = F + 2
             {
               cvbiPtr = NULL;
               // from EvalToConstVisitor::visitMemRefNode():
               OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
               aIter = aliasUseSet[apCnt]->getIterator();
               for ( ; aIter->isValid() ; ++(*aIter)) {
                 OA::Alias::AliasTag tag = aIter->current();
                 RCPairSetIterator set1Iter(*apInAll);
                 for (; set1Iter.isValid(); ++(set1Iter)) {
                   RCPair rc = set1Iter.current();
                   OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                   if(sTag == tag) {
                     cvbiPtr = (set1Iter.current()).getConstPtr();
                     //break;
                   }
                 }
               }
               // from EvalToConstVisitor::visitOpNode(): // so to speak
               if (!(cvbiPtr.ptrEqual(0))) {
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal() + 2);
                 
               }
             }
             break;

           case 2: //  B = A + 1
             {
             cvbiPtr = NULL;
             // from EvalToConstVisitor::visitMemRefNode():
             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
             aIter = aliasUseSet[apCnt]->getIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               RCPairSetIterator set1Iter(*apInAll);
               for (; set1Iter.isValid(); ++(set1Iter)) {
                 RCPair rc = set1Iter.current();
                 OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                 if(sTag == tag) {
                   cvbiPtr = (set1Iter.current()).getConstPtr();
                   //break;
                 }
               }
             }
             // from EvalToConstVisitor::visitOpNode(): // so to speak
             if (!(cvbiPtr.ptrEqual(0))) {
               
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal() + 1);
               
             }
             }
             break;


           case 3: //  E = B + 1
             {
             cvbiPtr = NULL;
             // from EvalToConstVisitor::visitMemRefNode():
             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
             aIter = aliasUseSet[apCnt]->getIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               RCPairSetIterator set1Iter(*apInAll);
               for (; set1Iter.isValid(); ++(set1Iter)) {
                 RCPair rc = set1Iter.current();
                 OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                 if(sTag == tag) {
                   cvbiPtr = (set1Iter.current()).getConstPtr();
                   //break;
                 }
               }
             }
             // from EvalToConstVisitor::visitOpNode(): // so to speak
             if (!(cvbiPtr.ptrEqual(0))) {
               
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal() + 1);
               
             }
             }
             break;


           case 4: //  G = &C
             {
             cvbiPtr = NULL;
             // from EvalToConstVisitor::visitMemRefNode():
             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
             aIter = aliasUseSet[apCnt]->getIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               RCPairSetIterator set1Iter(*apInAll);
               for (; set1Iter.isValid(); ++(set1Iter)) {
                 RCPair rc = set1Iter.current();
                 OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                 if(sTag == tag) {
                   cvbiPtr = (set1Iter.current()).getConstPtr();
                   //break;
                 }
               }
             }
             // from EvalToConstVisitor::visitOpNode(): // so to speak
             if (!(cvbiPtr.ptrEqual(0))) {
               
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal());
               
             }
             }
             break;


           case 5: //  C = 5
             assert(true); // C is not a MUST def
             break;


           case 6: //  D = *C + 1
             {
             cvbiPtr = NULL;
             // from EvalToConstVisitor::visitMemRefNode():
             OA::OA_ptr<OA::Alias::AliasTagIterator> aIter;
             aIter = aliasUseSet[apCnt]->getIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               RCPairSetIterator set1Iter(*apInAll);
               for (; set1Iter.isValid(); ++(set1Iter)) {
                 RCPair rc = set1Iter.current();
                 OA::Alias::AliasTag sTag = (set1Iter.current()).getTag();
                 if(sTag == tag) {
                   cvbiPtr = (set1Iter.current()).getConstPtr();
                   //break;
                 }
               }
             }
             // from EvalToConstVisitor::visitOpNode(): // so to speak
             if (!(cvbiPtr.ptrEqual(0))) {
              
                 OA::OA_ptr<IntegerConstVal> icvPtr;
                 icvPtr = cvbiPtr.convert<IntegerConstVal>();
                 cvbiPtr = new IntegerConstVal(icvPtr->getIntegerVal() + 1);
             
             }
             }
             break;


           default:
             assert(true);
             break;
           }
           
           if (!cvbiPtr.ptrEqual(NULL)) {
             if (debug) {
               std::cout << "\texpr == (" << cvbiPtr->toString() << ")\n";
             }
             
             //! ===== Generate RCPairs directly into out set =====
             aIter = apKill[apCnt]->getTagSetIterator();
             for ( ; aIter->isValid() ; ++(*aIter)) {
               OA::Alias::AliasTag tag = aIter->current();
               if (debug) {
                 std::cout << "\t\tgen'ing RCPair<" << tag << ",VALUE="
                           << cvbiPtr->toString() << ">\n";
               }
               // none of these tags should be in apOut[apCnt] because
               // we just minusEqu'd them above
               apOut[apCnt]->insert(RCPair(tag,cvbiPtr)); 
             }
             
           } else {
             if (debug) {
               std::cout << "\texpr != const val\n";
             }
           }
         } else {
           if (debug) {
             std::cout << "\tdefSet isNOT MUST\n";
           }
         }
         
         //! update inset for next iteration
         apInAll->intersectEqu(*(apOut[apCnt]));
         
       } // end of assign pairs loop
       
       // check for convergence
       if (oldApInAll != apInAll) {
         changed = true;
       }
       
     } // end of while(changed) convergence loop
     
     // 5. calculate out set for stmt
     
     OA::OA_ptr<RCPairSet> outRecast;     
     outRecast = (apOut[0]->clone()).convert<RCPairSet>();
     for (apCnt=1; apCnt<count; apCnt++) {
       outRecast->intersectEqu(*(apOut[apCnt]));
     }
     std::cout << "outRecast : " << outRecast->toString() << std::endl; 
     assert(outRecast==wantedOutSet);
     


   }

   {
     std::cout << "===== Testing DataFlowSetImpl set routines\n";

     OA::OA_ptr<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> > set1,set2,set3,set4;
     set1 = new OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag>();
     set1->insert(1);
     set1->insert(2);
     set1->insert(3);

     std::cout << "\nSet1 : ";
     printTagSet(set1);
     std::cout << std::endl;

     set2 = set1;  
     // just assigns the OA_ptr set2 to have the address of OA_ptr set1

     std::cout << "\nSet2 : ";
     printTagSet(set2);
     std::cout << std::endl;

     set1->remove(1);
     std::cout << "\nSet1 : ";
     printTagSet(set1);
     std::cout << std::endl;

     std::cout << "\nSet2 : ";
     printTagSet(set2);
     std::cout << std::endl;

     set2 = (set1->clone()).convert<OA::DataFlow::DataFlowSetImpl<OA::Alias::AliasTag> >();
     // set2 has no connection to set1 (true clone)

     set1->remove(2);
     std::cout << "\nSet1 : ";
     printTagSet(set1);
     std::cout << std::endl;

     std::cout << "\nSet2 : ";
     printTagSet(set2);
     std::cout << std::endl;


     OA::OA_ptr<RCPairSet> rSet3;
     rSet3 = new RCPairSet;
     rSet3->insert(rc1a);
     rSet3->insert(rc1b);
     //rSet3->insert(rc1c);
     rSet3->insert(rc2c);
     std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 
     
     OA::OA_ptr<RCPairSet> rSet4;
     //rSet4 = rSet3->clone().convert<RCPairSet>();
     rSet4 = new RCPairSet(*rSet3);
     std::cout << "RCPairSet 4 : " << rSet4->toString() << std::endl; 

     rSet3->remove(rc1a);
     std::cout << "RCPairSet 3 : " << rSet3->toString() << std::endl; 
     
     std::cout << "RCPairSet 4 : " << rSet4->toString() << std::endl; 



   }
     

   return 0;

} // end of main()

