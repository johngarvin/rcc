/*! \file
  
  \brief Definition for UnionFind

  \authors This code was taken from the HPCVIEW system, which took it from the D system.
  \version $Id: UnionFindUniverse.cpp,v 1.2 2004/11/19 19:21:53 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  UNION-FIND algorithm from Aho, Hopcroft & Ullman,
  The Design and Analysis of Computer Algorithms, Addison Wesley 1974
*/


 
#include "UnionFindUniverse.hpp"


#define UF_NIL -1

namespace OA {

//***********************************************************************************************
// internal class UnionFindElement
//***********************************************************************************************

class UnionFindElement {
public:
  UnionFindElement() { };

  int &Count() { return count; };
  int &Root() { return root; };
  int &Parent() { return parent; }
  int &Name() { return name; }
  void Init(int i); 
private:
  int parent; 
  int name;
  int count;
  int root;
};


void UnionFindElement::Init(int i) 
{
  name = root = i; 
  parent = UF_NIL;
  count = 1;
}
	

//***********************************************************************************************
// class UnionFindUniverse interface operations
//***********************************************************************************************


UnionFindUniverse::UnionFindUniverse(unsigned int highWaterMark)
{
  e = new UnionFindElement[highWaterMark];

  for (unsigned int i = 0; i < highWaterMark; i++) {
    e[i].Init(i);
  }
}

UnionFindUniverse::~UnionFindUniverse()
{
  if (e)
    delete [] e;
}


int UnionFindUniverse::Find(int v)
{
  if (Parent(v) == UF_NIL)
    return Name(v);

  Parent(v) = do_FIND(Parent(v));

  return Name(Parent(v));
}


void UnionFindUniverse::Union(int i, int j, int k)
{
  if ((i == j) && (j == k))
    return;

  int large, small;
  if (Count(Root(i)) > Count(Root(j))) {
    large = Root(i);
    small = Root(j);
  } else {
    large = Root(j);
    small = Root(i);
  }

  Parent(small) = large;
  Count(large) += Count(small);
  Name(large) = k;
  Root(k) = large; 
}



//***********************************************************************************************
// class UnionFindUniverse private operations
//***********************************************************************************************

int UnionFindUniverse::do_FIND(int v)
{
  if (Parent(v) == UF_NIL)
    return v;

  Parent(v) = do_FIND(Parent(v));

  return Parent(v);
}

int &UnionFindUniverse::Count(int i) 
{
  return e[i].Count();
}

int &UnionFindUniverse::Root(int i) 
{
  return e[i].Root();
}
int &UnionFindUniverse::Parent(int i) 
{
  return e[i].Parent();
}

int &UnionFindUniverse::Name(int i) 
{
  return e[i].Name();
}

} // end of namespace OA
