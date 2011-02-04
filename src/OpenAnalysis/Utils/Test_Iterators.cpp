/** Driver program to test the Iterator classes */

#include "Iterator.hpp"
#include "SetIterator.hpp"
#include "OA_ptr.hpp"
#include "ListIterator.hpp"
#include <iostream>
using namespace std;
using namespace OA;

class Test {
  public:
    int mVal;
    Test(int val) { mVal = val; }
    Test(Test &t) : mVal(t.mVal) { }
    Test(const Test &t) : mVal(t.mVal) { }

    bool operator==(const Test &rhs) const {
        return mVal == rhs.mVal;
    }

    bool operator<(const Test &rhs) const {
        return mVal < rhs.mVal;
    }
};

int main(char argc, char *argv[]) {
    // Test set iterator, native
    OA_ptr<set<Test> > testSet;
    testSet = new set<Test>();
    for(int i = 0; i < 10; i++) {
        Test newTest(i);
        testSet->insert(newTest);
        testSet->insert(newTest);
    }

    SetIterator<Test> iter(testSet);

    cout << "Set of Tests: ";
    while(iter.isValid()) {
        cout << iter.current().mVal << " ";
        ++iter;
    }
    cout << endl;

    // Test set iterator, pointers
    OA_ptr<set<OA_ptr<Test> > > testSet2;
    testSet2 = new set<OA_ptr<Test> >();
    for(int i = 0; i < 10; i++) {
        OA_ptr<Test> newTest1;
        OA_ptr<Test> newTest2;
        newTest1 = new Test(i);
        newTest2 = new Test(i);
        testSet2->insert(newTest1);
        testSet2->insert(newTest2);
    }

    SetIterator<OA_ptr<Test> > iter2(testSet2);

    cout << "Set of Test pointers: ";
    while(iter2.isValid()) {
        cout << iter2.current()->mVal << " ";
        ++iter2;
    }
    cout << endl;

    // Test list iterator, native
    OA_ptr<list<Test> > testList;
    testList = new list<Test>();
    for(int i = 0; i < 10; i++) {
        Test newTest(i);
        testList->push_back(newTest);
        testList->push_back(newTest);
    }

    ListIterator<Test> iter3(testList);

    cout << "List of Tests: ";
    while(iter3.isValid()) {
        cout << iter3.current().mVal << " ";
        ++iter3;
    }
    cout << endl;

    // Test list iterator, pointers
    OA_ptr<list<OA_ptr<Test> > > testList2;
    testList2 = new list<OA_ptr<Test> >();
    for(int i = 0; i < 10; i++) {
        OA_ptr<Test> newTest1;
        OA_ptr<Test> newTest2;
        newTest1 = new Test(i);
        newTest2 = new Test(i);
        testList2->push_back(newTest1);
        testList2->push_back(newTest2);
    }

    ListIterator<OA_ptr<Test> > iter4(testList2);

    cout << "List of Test pointers: ";
    while(iter4.isValid()) {
        cout << iter4.current()->mVal << " ";
        ++iter4;
    }
    cout << endl;
}

