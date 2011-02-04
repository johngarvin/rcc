// $Id: trtest.cpp,v 1.2 2004/11/19 19:21:53 mstrout Exp $
// -*-C++-*-
// * BeginRiceCopyright *****************************************************
/* 
  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/
// ******************************************************* EndRiceCopyright *


#include <iostream>

using std::endl;
using std::cout;
using std::cerr;

#include "Tree.hpp"


class myTree : public Tree {
public:
  class Node : public Tree::Node {
  public:
    Node (int v) : Tree::Node() { val = v; }
    virtual ~Node () {}
    int val;
  };
  class Edge : public Tree::Edge {
  public:
    Edge (int v, Node* parent, Node* child) : Tree::Edge (parent, child) { val = v; }
    int val;
  };
  myTree () : Tree () {}
  myTree (Node* n) : Tree (n) {}
};

static void print_preorder (myTree& t);


int
main (int argc, char* argv[])
{
  try {
    myTree::Node n1(1);
    myTree::Node n2(2);
    myTree::Node n3(3);
    myTree::Edge e1(1, &n1, &n2);
    myTree::Edge e2(2, &n1, &n3);

    myTree t(&n1);
    t.add_empty_edge(&n1);
    t.add(&e2);
    t.add(&e1);

    print_preorder(t);
  }
  catch (Exception& e) {
    e.report(cerr);
  }
}


void
print_preorder (myTree& t)
{
  myTree::PreOrderIterator iter(t);
  cout << "Tree nodes in pre-order:" << endl;
  while ((bool)iter) {
    cout << "\tnode " << (dynamic_cast<myTree::Node*>((Tree::Node*)iter))->val << endl;
    ++iter;
  }
}
