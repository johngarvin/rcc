/*! \file
    
    \brief Declaration for DGraphInterface class. 

    \authors Michelle Strout, Priyadarshini Malusare

     Copyright (c) 2002-2005, Rice University <br>
     Copyright (c) 2004-2005, University of Chicago <br>
     Copyright (c) 2006, Contributors <br>
     All rights reserved. <br>
     See ../../../../Copyright.txt for details. <br>
             
 */

#ifndef DGraphInterface_H
#define DGraphInterface_H

#include <cassert>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <set>

namespace OA {
  namespace DGraph {

    typedef enum {DEdgeOrg = 0, DEdgeRev = 1} DGraphEdgeDirection;

    class EdgeInterface;
    class EdgesIteratorInterface;
    class NodeInterface;
    class NodesIteratorInterface;
    class DGraphInterface;

  class NodeInterface : public virtual Annotation {
      public:
        //========================================================  
        // Information
        //========================================================
        
        virtual ~NodeInterface() { }
        
        virtual unsigned int getId () const = 0;

        virtual int num_incoming () const = 0;
        
        virtual int num_outgoing () const = 0;
      
        //! returns true if node is an entry node, IOW  has no incoming edges
        virtual bool isAnEntry() const = 0;
  
        //! returns true if node is an exit node, IOW  has no outgoing edges
        virtual bool isAnExit() const = 0;


        //========================================================  
        // Iterators
        //========================================================
        virtual OA_ptr<EdgesIteratorInterface> 
            getIncomingEdgesIterator() const =0;
        
        virtual OA_ptr<EdgesIteratorInterface> 
            getOutgoingEdgesIterator() const =0;

        virtual OA_ptr<NodesIteratorInterface> 
            getSourceNodesIterator() const =0;

        virtual OA_ptr<NodesIteratorInterface> 
            getSinkNodesIterator() const =0;


        //========================================================  
        // Comparison operators
        //========================================================
        virtual bool operator== (NodeInterface& other) = 0;
        virtual bool operator< (NodeInterface& other) = 0;

        //========================================================  
        // Construction
        //========================================================
        virtual void addOutgoingEdge(OA_ptr<EdgeInterface>) = 0;
        virtual void addIncomingEdge(OA_ptr<EdgeInterface>) = 0;
        virtual void removeIncomingEdge(OA_ptr<EdgeInterface> e) = 0;
        virtual void removeOutgoingEdge(OA_ptr<EdgeInterface> e) = 0;

        
        //========================================================  
        // Output
        //========================================================
        virtual void dump(std::ostream& os) = 0;
    };

    // lt_Node: function object that compares by node id.  Useful for sorting.
  class lt_NodeInterface {
      public:
          // return true if n1 < n2; false otherwise
          virtual bool operator()(const OA_ptr<NodeInterface> n1,
                                  const OA_ptr<NodeInterface> n2) const = 0;
    };

 class EdgeInterface : public virtual Annotation {
      public:

        virtual ~EdgeInterface() { }
        //========================================================  
        // Information
        //========================================================
        virtual unsigned int getId () const = 0;
        virtual OA_ptr<NodeInterface> getSource() const = 0;
        virtual OA_ptr<NodeInterface> getSink() const = 0;

        //========================================================  
        // Comparison operators
        //========================================================
        virtual bool operator== (EdgeInterface& other) = 0;
        virtual bool operator< (EdgeInterface& other) = 0;
        //========================================================  
        // Output
        //========================================================
        virtual void dump(std::ostream& os) = 0;
    };

 

    // lt_Edge: function object that compares by id.  Useful for sorting.
 class lt_EdgeInterface {
     public:
        // return true if e1 < e2; false otherwise
        virtual bool operator()(
                                const OA_ptr<EdgeInterface> e1, 
                                const OA_ptr<EdgeInterface> e2) const = 0;
    };



 class NodesIteratorInterface {
      public:
        virtual ~NodesIteratorInterface() { }
        virtual OA_ptr<NodeInterface> current() const = 0;
        virtual void operator++() = 0;      // prefix
        virtual void operator++(int) = 0;   // postfix
        virtual bool isValid() const = 0;
        virtual void reset() = 0;
    };

    class EdgesIteratorInterface {
      public:
        virtual ~EdgesIteratorInterface() { }
        virtual OA_ptr<EdgeInterface> current() const = 0;
        virtual void operator++() = 0;      // prefix
        virtual void operator++(int) = 0;   // postfix
        virtual bool isValid() const = 0;
        virtual void reset() = 0;
    };


  class DGraphInterface : public virtual Annotation {
      public:

        virtual ~DGraphInterface() { }  

        virtual int getNumNodes () = 0;
        virtual int getNumEdges () = 0;


        //========================================================  
        // Iterators
        //========================================================
        virtual OA_ptr<NodesIteratorInterface> 
            getNodesIterator() const = 0;


        virtual OA_ptr<NodesIteratorInterface> 
            getEntryNodesIterator() const =0;
        
        virtual OA_ptr<NodesIteratorInterface> 
            getExitNodesIterator() const =0;

        virtual OA_ptr<NodesIteratorInterface>
            getReversePostDFSIterator(
                    DGraphEdgeDirection pOrient) =0;

        
        virtual OA_ptr<NodesIteratorInterface>
            getDFSIterator(OA_ptr<NodeInterface> n) = 0;


        virtual OA_ptr<EdgesIteratorInterface> 
            getEdgesIterator() const =0;



        //========================================================
        // Construction
        //========================================================
        virtual void addNode(OA_ptr<NodeInterface> n) = 0;
        virtual void addEdge(OA_ptr<EdgeInterface> e) = 0;

        
    };

 }// End of namespace DGraph
} // End of namespace OA

#endif
