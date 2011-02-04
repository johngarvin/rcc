/*! \file
  
  \brief Tarjan's algorithm to determine strongly connected components
         of a directed graph. This was the first algorithm that solved
         the problem in linear time.

  \authors Original DSystem code by John Mellor-Crummey, Lei Zhou;
           Ported to OpenAnalysis by Nathan Tallent
  \version $Id: SCC.hpp,v 1.3 2005/03/08 18:54:45 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef StronglyConnectedComponents_H
#define StronglyConnectedComponents_H

#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/Interface.hpp>
#include <OpenAnalysis/Utils/RIFG.hpp>

//***************************************************************************

namespace OA {

typedef std::set<OA_ptr<DGraph::Interface::Node> > SCCNodeSet;

 
//***************************************************************************
// Class: SCCSet
//
// Description:
//    SCCSet is a set of SCCs; each SCC is set of directed graph nodes
//    (SCCNodeSet)
//
//***************************************************************************

class SCCSet : public std::set<OA_ptr<SCCNodeSet> > {
public:
  // Given a directed-graph, compute SCC-sets.  If available, pass a
  // corresponding RIFG, to avoid extra computation.
  SCCSet(OA_ptr<DGraph::Interface> graph, OA_ptr<RIFG> rifg = OA_ptr<RIFG>());
  
  virtual ~SCCSet();
    
  // NodeToSCC: Given a dgraph node, locate which SCCNodeSet it belongs to. 
  OA_ptr<SCCNodeSet> NodeToSCC(const OA_ptr<DGraph::Interface::Node> node);
  
  // dump: dump text output useful for debugging
  void dump(std::ostream& os);

protected:  
  typedef std::set<OA_ptr<SCCNodeSet> > self_t;
  
  SCCSet();  
  
  void Create(OA_ptr<DGraph::Interface> graph, OA_ptr<RIFG> rifg);
  void Destroy();

private:

};
  
  
} // end of namespace OA

#endif
