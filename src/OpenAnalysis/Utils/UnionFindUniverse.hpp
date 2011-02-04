/*! \file
  
  \brief Declarations for UnionFind

 \authors This code was taken from the HPCVIEW system, which took it from the D system.
  \version $Id: UnionFindUniverse.hpp,v 1.2.10.1 2005/08/25 22:36:23 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UnionFindUniverse_H
#define UnionFindUniverse_H

namespace OA {

class UnionFindElement;

/*!
   Here is example usage of this class:

   OA::UnionFindUniverse ufset(10);

   ufset.Union( ufset.Find(1), ufset.Find(5), ufset.Find(5) )

*/

class UnionFindUniverse {
public:
  UnionFindUniverse(unsigned int highWaterMark);
  ~UnionFindUniverse();
  //! merge set i with set j and name it k
  //! if you want to merge the sets that a and b are in then
  //! you need to call Union( Find(a), Find(b), ...)
  void Union(int i, int j, int k);
  //! returns what set i is in
  int Find(int i);

private: // methods
  int &Count(int i);
  int &Root(int i);
  int &Parent(int i);
  int &Name(int i);
  int do_FIND(int i);

private: // data
  UnionFindElement *e;
};
 
} // end of OA namespace

#endif

