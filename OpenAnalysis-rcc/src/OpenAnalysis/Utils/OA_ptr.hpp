/*! \file
  
  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef OAPTR_H
#define OAPTR_H

#include <iostream>
#include <cassert>
#include <set>
#include "Util.hpp"

static bool OA_ptr_debug = false;

namespace OA {

template <class T>
class OA_ptr {
  public:
    OA_ptr() : mPtr(NULL), mRefCountPtr(NULL)  { }
    /*
  private:
    OA_ptr(T* ptr) : mPtr(ptr), mRefCountPtr(NULL)
        { if (ptr!=NULL) {
              mRefCountPtr = new int;
              *mRefCountPtr = 1;
          }
        }
        */
  public:
    //! copy constructor
    OA_ptr(const OA_ptr<T> &other) 
        : mPtr(other.mPtr), mRefCountPtr(other.mRefCountPtr)
    {
        if (OA_ptr_debug) { std::cout << "OA_ptr copy constructor: other = ";
                     other.dump(std::cout);  std::cout << std::endl; }
        if (mRefCountPtr!=NULL) 
        { (*mRefCountPtr)++; }
    }

    //! getting a new ptr
    OA_ptr<T>& operator= (T* ptr)
    {
        // don't do anything for self-assignment
        if (mPtr != ptr) {
            decrementCurrPtr();
            mPtr = ptr;
            // do not need to reference count NULL, make new ref counter
            // if getting a ptr other than NULL and if getting NULL
            // set mRefCountPtr to NULL            
            if (mPtr != NULL) {              
                mRefCountPtr = new int;              
                *mRefCountPtr = 1;            
            } else {
                mRefCountPtr = NULL;
            }
        }
        return *this;
     }

    //! copy assignment
    OA_ptr<T>& operator= (const OA_ptr<T> &other)
    {
        // don't do anything for self-assignment
        if (mPtr != other.mPtr) {
            decrementCurrPtr();
            mPtr = other.mPtr;
            // do not need to reference count NULL, make new ref counter
            // if getting a ptr other than NULL and if getting NULL
            // set mRefCountPtr to NULL            
            mRefCountPtr = other.mRefCountPtr;
            if (mPtr != NULL) {              
                (*mRefCountPtr)++;            
            } 
        }
        return *this;
     }

  // this doesn't work properly:
//     //! copy assignment
//     template <class T2>
//     OA_ptr<T>& operator= (const OA_ptr<T2> &other)    
//     {
//         // don't do anything for self-assignment
//         if (this != dynamic_cast<const OA_ptr<T>* >(&other)) {
//             mPtr = (dynamic_cast<const OA_ptr<T>& >(other)).mPtr;
//             mRefCountPtr = (dynamic_cast<const OA_ptr<T>& >(other)).mRefCountPtr;
//             if (mRefCountPtr!=NULL) { (*mRefCountPtr)++; }
//         }
 
//        return *this;
//     } 

    virtual ~OA_ptr() 
    { 
        decrementCurrPtr();
    }

    T* operator->() const
    {
        assert(mPtr != NULL);
        return mPtr;
    }

    T& operator*() const
    {
        assert(mPtr != NULL);
        return *mPtr;
    }

    // constructor used by convert, can't be private because convert
    // calls it on the OA_ptr<T2> and T1=T2
    OA_ptr(T* ptr, int* countPtr) 
        : mPtr(ptr), mRefCountPtr(countPtr)
    {
        assert((ptr==0&&countPtr==0) || (ptr!=0&&countPtr!=0));
        if (OA_ptr_debug) { std::cout << "OA_ptr constructor ptr,cnt: ptr = ";
            //std::cout << mPtr << ", countPtr = " << countPtr << std::endl; 
        }
        if (mRefCountPtr!=NULL) { (*mRefCountPtr)++; }
        if (OA_ptr_debug) { std::cout << "OA_ptr constructor ptr,cnt: *this = ";
                     dump(std::cout);  std::cout << std::endl; }
     }
          
    // enables us to do dynamic casts
    template <class T2>
    OA_ptr<T2> convert() const
    {
        assert(mPtr != NULL);
        T2* retval = dynamic_cast<T2*>(mPtr);
        assert(retval != NULL);
        return OA_ptr<T2>(retval,mRefCountPtr);
    }
    
    // enables us to do some RTTI
    template <class T2>
    bool isa() const
    {
        assert(mPtr != NULL);
        T2* retval = dynamic_cast<T2*>(mPtr);
        if (retval==NULL) {
            return false;
        } else {
            return true;
        }
    }


    // so that can pass a subclass into base class
    // Stroustrup 349-350
    template <class T2> operator OA_ptr<T2> () const 
        { return OA_ptr<T2>(mPtr,mRefCountPtr); }
        
    bool ptrEqual(const T* rhs) const
        {
            return mPtr == rhs;
        }
    bool ptrEqual(const OA_ptr& other) const
        {
            return mPtr == other.mPtr;
        }


    bool operator==(const OA_ptr<T>& other) const 
        {
          bool retval;
          
          if (OA_ptr_debug) {
            std::cout << "In OA_ptr::operator==" << std::endl; 
            std::cout << "\t*this=";
            dump(std::cout);
            std::cout << ", other=";
            other.dump(std::cout);
            dump(std::cout);
          }

          if (mPtr==other.mPtr) {
            retval = true;
          } else if (mPtr==0 || other.mPtr==0) {
            retval = false;
          } else {
            retval = *mPtr==*(other.mPtr); 
          }
          if (OA_ptr_debug) {
            std::cout << "\nOA_ptr::operator== returning " << retval 
                      << std::endl;
          }
          return retval;
        }

    template <class T2>
    bool operator==(const OA_ptr<T2>& other) const 
        {
          bool retval;
          
          if (OA_ptr_debug) {
            std::cout << "In OA_ptr::operator==(T2)" << std::endl; 
            std::cout << "\t*this=";
            dump(std::cout);
            std::cout << ", other=";
            other.dump(std::cout);
            dump(std::cout);
          }

          if (ptrEqual(0) && other.ptrEqual(0)) {
            retval = true;
          } else if (ptrEqual(0) || other.ptrEqual(0)) {
            retval = false;
          } else {
            retval = *mPtr==*other; 
          }
          if (OA_ptr_debug) {
            std::cout << "\tOA_ptr::operator== returning " << retval 
                      << std::endl;
          }
          return retval;
        }


    bool operator!=(const OA_ptr<T>& other)  const 
        {
          return ! (*this==other); 
        }

    template <class T2>
    bool operator!=(const OA_ptr<T2>& other) const 
        {
          return ! (*this==other); 
        }


    /*! 
       Assuming that this will mostly be used by STL sorted associative
       containers.  They assume that two keys are equivalent if neither one
       is less than the other. 
    */
    bool operator<(const OA_ptr<T>& other)  const 
        { 
          bool retval;

          if (OA_ptr_debug) {
            std::cout << "In OA_ptr::operator<" << std::endl; 
            std::cout << "\t*this=";
            dump(std::cout);
            std::cout << ", other=";
            other.dump(std::cout);
            std::cout << std::endl;
          }

          // if both objects are equal then return false
          if ( mPtr==other.mPtr 
               || other.mPtr == 0
               || *mPtr==*(other.mPtr) ) 
          {
              retval = false;

          // if the first one is null and other wasn't then less than
          } else if ( mPtr==0 ) {
              retval = true;

          // if the second one is null and first one wasn't then greater
          } else if ( mPtr==0 ) {
              retval = false;

          // if the objects are not equal then call operator< on them
          } else {
              retval = *mPtr<*(other.mPtr); 
          }
          if (OA_ptr_debug) {
            std::cout << "\tOA_ptr::operator< returning " << retval 
                      << std::endl; 
          }
          return retval;
        }

    template <class T2>
    bool operator<(const OA_ptr<T2>& other)  const 
        { 
          bool retval;
          if (ptrEqual(0) && other.ptrEqual(0)) {
              retval = false;
          // if the first one is null and other wasn't then less than
          } else if (ptrEqual(0) ) {
              retval = true;
          // if the second one is null and first wasn't then greater than
          } else if (other.ptrEqual(0) ) {
              retval = false;

          } else { retval = *mPtr<*(other); }
          return retval;
        }


    void dump(std::ostream& os) const 
        { //os << "OA_ptr(mPtr = " << mPtr << ", *mRefCountPtr="; // IRIX compiler?
          os << "OA_ptr(mPtr = " << ", *mRefCountPtr=";
          os.flush();
          if (mRefCountPtr!=NULL) { os << *mRefCountPtr; }
          else { os << 0; }
          os << ")";
        }

    inline friend std::ostream& operator <<(std::ostream & out, 
                                            const OA_ptr<T>& oaptr)
        {
            oaptr.dump(out);
            return out;
        }

  private:
    T* mPtr;
    int* mRefCountPtr;

  private:
    //! I am giving up reference to current mPtr
    void decrementCurrPtr()
    {
        if (mPtr!=NULL) {
            //assert(mRefCountPtr!=NULL);
            //FIXME: sometimes can get here and have mRefCountPtr==NULL, how?
            //and mPtr be unitialized
            if (mRefCountPtr!=NULL) {
              (*mRefCountPtr)--;
              if (*mRefCountPtr == 0) {
                delete mPtr;
                delete mRefCountPtr;
              }
            }
        }
        // no matter what should not have mPtr or mRefCountPtr at this point
        mPtr = NULL;
        mRefCountPtr = NULL;
    }

};

} // end of OA namespace

#endif
