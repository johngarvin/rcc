/*! \file
 
  \brief Declaration for DGraphInterface class.
 
  \authors Michelle Strout, Priyadarshini Malusare
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../../Copyright.txt for details. <br>
 
*/



#ifndef DGraphImplement_H
#define DGraphImplement_H

#include "DGraphInterface.hpp"
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <map>
#include <set>
#include <list>

namespace OA {
  namespace DGraph {
    
  class EdgesIteratorImplement : public virtual EdgesIteratorInterface {
      friend class DGraphImplement;
      friend class NodeImplement;

      public:
        OA_ptr<EdgeInterface> current() const { return *mIter; }
        void operator++() { ++mIter; }      // prefix
        void operator++(int) {++*this; }    // postfix
        bool isValid() const { return (mIter != mEdgeList->end()); }
        void reset() { mIter = mEdgeList->begin(); }

      private:
        OA_ptr<std::list<OA_ptr<EdgeInterface> > > mEdgeList;
        std::list<OA_ptr<EdgeInterface> >::iterator mIter;

      protected: 
        // only subclasses can call this constructor
        // it can not take a const OA_ptr because it is doing a convert
        EdgesIteratorImplement(OA_ptr<EdgesIteratorInterface> ni);
        EdgesIteratorImplement(OA_ptr<std::list<OA_ptr<EdgeInterface> > > elist);
    };


  class NodesIteratorImplement : public virtual NodesIteratorInterface {
      friend class DGraphImplement;
      friend class NodeImplement;

      public:
        OA_ptr<NodeInterface> current() const { return *mIter; }
        void operator++() { ++mIter; }
        void operator++(int) { ++*this; }
        bool isValid() const {  return (mIter != mNodeList->end()); } 
        void reset() { mIter = mNodeList->begin(); }

      private:
        OA_ptr<std::list<OA_ptr<NodeInterface> > > mNodeList;
        std::list<OA_ptr<NodeInterface> >::iterator mIter;

      protected: 
        // only subclasses can call this constructor
        // it can not take a const OA_ptr because it is doing a convert
        NodesIteratorImplement(OA_ptr<NodesIteratorInterface> ni);
        NodesIteratorImplement(OA_ptr<std::list<OA_ptr<NodeInterface> > > nlist);
      
    };




 class NodeImplement : public virtual NodeInterface {
      public:
        NodeImplement() ;
         
        //========================================================  
        // Information
        //========================================================
        unsigned int getId () const;
        int num_incoming () const;
        int num_outgoing () const;
      
        //! returns true if node is an entry node, IOW  has no incoming edges
        bool isAnEntry() const;
  
        //! returns true if node is an exit node, IOW  has no outgoing edges
        bool isAnExit() const;

        //========================================================  
        // Iterators
        //========================================================
        OA_ptr<EdgesIteratorInterface> getIncomingEdgesIterator() const;
        
        OA_ptr<EdgesIteratorInterface> getOutgoingEdgesIterator() const;

        OA_ptr<NodesIteratorInterface> getSourceNodesIterator() const;

        OA_ptr<NodesIteratorInterface> getSinkNodesIterator() const;

        //========================================================  
        // Comparison operators
        //========================================================
        bool operator== (NodeInterface& other);
        
        bool operator< (NodeInterface& other);

        //========================================================  
        // Output
        //========================================================
        void output(IRHandlesIRInterface& ir) const;
        
        void dump(std::ostream& os);

        //========================================================  
        // Construction
        //========================================================
        void addIncomingEdge(OA_ptr<EdgeInterface> e);

        void addOutgoingEdge(OA_ptr<EdgeInterface> e);

        void removeIncomingEdge(OA_ptr<EdgeInterface> e);

        void removeOutgoingEdge(OA_ptr<EdgeInterface> e);


      private:
        OA_ptr<std::list<OA_ptr<EdgeInterface> > > mIncomingEdges;
        OA_ptr<std::list<OA_ptr<EdgeInterface> > > mOutgoingEdges;
        unsigned int mId;
        static unsigned int sNextId;
    };
 

    // lt_Node: function object that compares by id.  Useful for sorting.
    class lt_Node : public DGraph::lt_NodeInterface {
      public:
      // return true if n1 < n2; false otherwise
      bool operator()(const OA_ptr<NodeInterface> n1,
                      const OA_ptr<NodeInterface> n2) const; 
    };


  class EdgeImplement : public virtual EdgeInterface {
      public:
        EdgeImplement(OA_ptr<NodeInterface> source, 
                      OA_ptr<NodeInterface> sink);

        //========================================================  
        // Information
        //========================================================
        virtual unsigned int getId () const { return mId; }

        
        virtual OA_ptr<NodeInterface> getSource() const;
        
        virtual OA_ptr<NodeInterface> getSink() const;

        //========================================================  
        // Comparison operators
        //========================================================
        bool operator== (EdgeInterface& other);
        
        bool operator< (EdgeInterface& other);

        //========================================================  
        // Output
        //========================================================
        void output(IRHandlesIRInterface& ir) const;

        void dump(std::ostream& os);

     //========================================================  
     // Construction
     //========================================================
     //! used by inheriters to gain write access to mType
        
      private:
        OA_ptr<NodeInterface> mSourceNode;
        OA_ptr<NodeInterface> mSinkNode;
        unsigned int mId;
        static unsigned int sNextId;
    };


  // lt_Edge: function object that compares by source and sink node
  // ids.  Useful for sorting.  Not used to put edges in sets or other
  // STL containers.
  class lt_Edge : public lt_EdgeInterface {
  public:
    // DO NOT change this to use edge Id because
    // code exists that assumes this compares by source and sink node ids
    // if another one is wanted then just make a new functor
    bool operator()(const OA_ptr<EdgeInterface> e1,
                    const OA_ptr<EdgeInterface> e2) const;
  };


  class DGraphImplement : public virtual DGraphInterface {
      public:

        DGraphImplement(); 
        
        ~DGraphImplement() { } 


        OA_ptr<NodesIteratorInterface> getNodesIterator() const;
        OA_ptr<NodesIteratorInterface> getEntryNodesIterator() const;
        OA_ptr<NodesIteratorInterface> getExitNodesIterator() const;
        OA_ptr<NodesIteratorInterface>
                 getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

        OA_ptr<NodesIteratorInterface>
                 getDFSIterator(OA_ptr<NodeInterface> n);
        
        OA_ptr<EdgesIteratorInterface> getEdgesIterator() const;
        void addNode(OA_ptr<NodeInterface> n);
        void addEdge(OA_ptr<EdgeInterface> e);
        void removeEdge(OA_ptr<EdgeInterface> e);
        void removeNode(OA_ptr<NodeInterface> n);

        int getNumNodes() { return mNodeSet->size(); }
        int getNumEdges() { return mEdgeSet->size(); }

        void output(IRHandlesIRInterface& ir) const;
       
        virtual std::string getGraphName() const;

        // temporary workaround until expression graphs have been 
        // properly factored out (specialized) from DGraphImplement
        OA_ptr<NodeInterface> getExprGraphRootNode() const;

      //========================================================  
      // Helper methods
      //========================================================
      private:
        std::map<OA_ptr<NodeInterface>,bool> mVisitMap;

        void createDFSList(OA_ptr<NodeInterface> pNode, OA_ptr<std::list<OA_ptr<NodeInterface> > > pList);
    
        //! returns a list of DGraph NodeInterface's that do not have incoming
        OA_ptr<std::list<OA_ptr<NodeInterface> > > create_entry_list() const;

        OA_ptr<std::list<OA_ptr<NodeInterface> > > create_exit_list() const;

        OA_ptr<std::list<OA_ptr<NodeInterface> > >
          create_reverse_post_order_list(DGraphEdgeDirection pOrient); 
        
        void reverse_postorder_recurse( OA_ptr<NodeInterface> pNode,
                                       DGraphEdgeDirection pOrient,
                                       OA_ptr<std::list<OA_ptr<NodeInterface> > > pList );

        friend class NodesIteratorImplement;
       
      private:
        OA_ptr<std::set<OA_ptr<NodeInterface> > > mNodeSet;
        OA_ptr<std::set<OA_ptr<EdgeInterface> > > mEdgeSet;
    };

  } //end of namespace DGraph
} //end of namespace OA

#endif
