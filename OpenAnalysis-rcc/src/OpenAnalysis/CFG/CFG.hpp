/*! \file
  
  \brief Declaration for standard CFGStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent,
           Michelle Strout, Andy Stone, Priyadarshini Malusare
  \version $Id: CFGStandard.hpp,v 1.48 2005/08/18 14:06:57 johnmc Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CFG_H
#define CFG_H

//--------------------------------------------------------------------
// standard headers
#ifdef NO_STD_CHEADERS
# include <string.h>
#else
# include <cstring>
#endif

// STL headers
#include <list>
#include <set>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphImplement.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace CFG {


//--------------------------------------------------------------------
/*! CFG implements the CFG::CFGInterface using the DGraph class.
*/
//--------------------------------------------------------------------

  class Node;
  class Edge;
  class CFG;
  //------------------------------------------------------------------
  class NodeStatementsIterator;
  class NodeStatementsRevIterator;
  class NodesIterator;
  class EdgesIterator;

  //! used to be in cfg debugging section, needee for ICFG/MPI_ICFG output
    //  char* edgeTypeToString(EdgeType et);
    std::string edgeTypeToString(EdgeType et);

  //------------------------------------------------------------------
  /*! The NodesIterator is just an extension of DGraphImplement::NodesIteratorImplement and CFGInterface::NodesIteratorInterface to provide access to CFG nodes. 
  */
  class NodesIterator : 
                       public DGraph::NodesIteratorImplement, 
                       public virtual NodesIteratorInterface 
  {
     public:
        NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni);
    
        OA_ptr<NodeInterface> currentCFGNode() const;
    
  };

  //------------------------------------------------------------------
  class EdgesIterator : 
                       public DGraph::EdgesIteratorImplement, 
                       public virtual EdgesIteratorInterface 
  {
     public:
        EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ni)
            : DGraph::EdgesIteratorImplement(ni) {}

        OA_ptr<EdgeInterface> currentCFGEdge() const;

  };
   
  //--------------------------------------------------------------------
  /*! CFG::Node implements the CFG::NodeInterface
  */
  //--------------------------------------------------------------------
  class Node : public virtual NodeInterface, 
               public virtual DGraph::NodeImplement
  {
  public:
      
        Node();
        Node(StmtHandle n);
       ~Node();
    
       //=========================================================
       // Information
       //=========================================================
    
       //! An id unique within instances of CFG::Node is taken 
       //from DGraph::NodeImplement
   
        /*
        unsigned int getId() const;

        int num_incoming () const ; 
        int num_outgoing () const ;

        //! returns true if node is an entry node, IOW  has no incoming edges
        bool isAnEntry() const;

        //! returns true if node is an exit node, IOW  has no outgoing edges
        bool isAnExit() const;
        */
        
        //! return number of statements in block
        unsigned int size() const;

        //! remove all statements from block
         bool empty() const;

        /* 
        //========================================================
        // Comparison operators
        //========================================================
        bool operator== (DGraph::NodeInterface& other);

        bool operator< (DGraph::NodeInterface& other);


        //========================================================
        // Construction
        //========================================================
        void addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e);
        
        void addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e);

        void removeIncomingEdge(OA_ptr<DGraph::EdgeInterface> e);

        void removeOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e);
        */

        //! add statement to block
        void add(StmtHandle h);

        void add_front(StmtHandle h);

        //! erase statement from block, Careful: linear time!
        StmtHandle erase(StmtHandle h); 

        void split(StmtHandle splitPoint, OA_ptr<Node> newBlock);

        //=======================================================
        // Output
        //=======================================================

        void dump(std::ostream& os);

        void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);
       
       /*! commented out by PLM 08/17/06
       void longdump(CFG::CFG&, std::ostream& os, 
                  OA_ptr<IRHandlesIRInterface> ir);
       */           
    
       void dumpdot(CFG &currcfg, std::ostream& os, 
                 OA_ptr<IRHandlesIRInterface> ir);

       virtual void output(OA::IRHandlesIRInterface& ir) const;

       //================================================================
       // Iterators
       //================================================================ 
        
       friend class CFG;
       friend class NodeStatementsIterator;
       friend class NodeStatementsRevIterator;

       OA_ptr<NodeStatementsIteratorInterface> 
          getNodeStatementsIterator() const;

       OA_ptr<NodeStatementsRevIteratorInterface> 
        getNodeStatementsRevIterator()  const;

       //! DGraph Iterators
  
       /*
       OA_ptr<DGraph::EdgesIteratorInterface> 
           getIncomingEdgesIterator() const;

       OA_ptr<DGraph::EdgesIteratorInterface> 
           getOutgoingEdgesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getSourceNodesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getPredNodesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getSinkNodesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getSuccNodesIterator() const;
       */    

       OA_ptr<EdgesIteratorInterface> 
           getCFGIncomingEdgesIterator() const;

       //! CFG Specific Iterators  

       OA_ptr<EdgesIteratorInterface> 
           getCFGOutgoingEdgesIterator() const;

       OA_ptr<NodesIteratorInterface> 
           getCFGSourceNodesIterator() const;

       OA_ptr<NodesIteratorInterface> 
           getCFGPredNodesIterator() const;

       OA_ptr<NodesIteratorInterface> 
           getCFGSinkNodesIterator() const;

       OA_ptr<NodesIteratorInterface> 
           getCFGSuccNodesIterator() const;

  protected:
       //! gives inheriters read access to mStmt_list (a)
       std::list<StmtHandle>::iterator getStmtListBegin ();
    
       //! gives inheriters read access to mStmt_list (b)
       std::list<StmtHandle>::iterator getStmtListEnd ();
    
       //! gives inheriters read access to mStmt_list (c)
       std::list<StmtHandle>::reverse_iterator getStmtListRBegin ();

       //! gives inheriters read access to mStmt_list (d)
       std::list<StmtHandle>::reverse_iterator getStmtListREnd ();


  private:
    
        void Ctor();
    
        // OA_ptr because want a reference to it in NodeStatementsIterator
        OA_ptr<std::list<StmtHandle> > mStmt_list;
    
        //std::list<StmtHandle> mStmt_list;
        ExprHandle mEnd;
    
        //DGraph::NodeImplement mdgNode;
 
  };

  //--------------------------------------------------------------------
  /*! CFG::Edge implements the CFG::EdgeInterface
  */
  //--------------------------------------------------------------------
  class Edge : public virtual EdgeInterface,
               public virtual DGraph::EdgeImplement
  {
  public:
  
         Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink, EdgeType _type, 
             OA::ExprHandle _expr);

         ~Edge();
    
         friend class CFG;

         //========================================================
         // Iterators
         //=======================================================

         /*
         //! DGraph Iterators

         OA_ptr<DGraph::NodeInterface> getSource() const;

         OA_ptr<DGraph::NodeInterface> getSink() const;
         */

         //! CFG Specific Iterators 

         OA_ptr<NodeInterface> getCFGSource() const; 

         OA_ptr<NodeInterface> getCFGSink() const; 

         /*
         //========================================================
         // Comparison operators
         //========================================================

         bool operator== (DGraph::EdgeInterface& other);

         bool operator< (DGraph::EdgeInterface& other);
         */
         
         //=============================================================
         // Output   
         //==============================================================
         void dumpdot(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir);

          void dump(std::ostream& os);
         //void dump () { dump(std::cout); }

         void output(IRHandlesIRInterface& ir) const;

         //=================================================================
         // Information
         //=================================================================

         //! get an id unique within instances of CFG::Edge taken from DGraph::EdgeImplement
        // virtual unsigned int getId() const;
    
         //! get type of edge: FALLTHROUGH, etc.
         EdgeType getType() const;


         //! get expression associated with edge, specifies branching val
         ExprHandle getExpr() const;
    

  protected:

         //! used by inheriters to gain write access to mType
         void setType(EdgeType type);

         //! used by inheriters to gain write access to mExpr
         void setExpr(ExprHandle expr);

    
  private:
         EdgeType mType;
         ExprHandle mExpr;
         static const char* sEdgeTypeToString[];
         //DGraph::EdgeImplement mdgEdge;
  };  

//-------------------------------------------------------------------
// CFG methods
//-------------------------------------------------------------------

class CFG : public virtual CFGInterface,
            public virtual DGraph::DGraphImplement  
{
public:
        CFG();
        ~CFG();

        //-------------------------------------
        // CFG information access
        //-------------------------------------
        //! get ptr to unique entry node
       OA_ptr<NodeInterface> getEntry() const;

       OA_ptr<NodeInterface> getExit() const;

       SymHandle getName() const;

       /*
       int getNumNodes();
       int getNumEdges();
       */
  

       //========================================================
       // Output
       //========================================================
       //void output(IRHandlesIRInterface& ir);
 
       void dump(std::ostream&, OA_ptr<IRHandlesIRInterface> ); 

       void dumpdot(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

       //void output(IRHandlesIRInterface &ir) const; 
  
       //-------------------------------------
       // Iterators
       // Calling functions from DGraphImplement.hpp
       //-------------------------------------
       

       /*! DGraph Iterators */
 
       /*
       OA_ptr<DGraph::NodesIteratorInterface> 
           getNodesIterator() const;

       OA_ptr<DGraph::EdgesIteratorInterface> 
           getEdgesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getEntryNodesIterator() const;

       OA_ptr<DGraph::NodesIteratorInterface> 
           getExitNodesIterator() const;
  
       OA_ptr<DGraph::NodesIteratorInterface> 
           getReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

       OA_ptr<DGraph::NodesIteratorInterface> 
           getDFSIterator(OA_ptr<DGraph::NodeInterface> n);
       */    

       /*! CFG Specific Iterators */

       OA_ptr<NodesIteratorInterface> 
           getCFGNodesIterator() const;

       // method to get a more specific iterator
       OA_ptr<EdgesIteratorInterface>
           getCFGEdgesIterator() const;

       OA_ptr<NodesIteratorInterface>
           getCFGEntryNodesIterator() const;

       OA_ptr<NodesIteratorInterface>
           getCFGExitNodesIterator() const;

       OA_ptr<NodesIteratorInterface>
         getCFGReversePostDFSIterator(DGraph::DGraphEdgeDirection pOrient);

       OA_ptr<NodesIteratorInterface> 
           getCFGDFSIterator(OA_ptr<NodeInterface> n);

       
       /*
       //========================================================
       // Construction
       //========================================================
       void addNode(OA_ptr<DGraph::NodeInterface> n);
   
       void addEdge(OA_ptr<DGraph::EdgeInterface> e);

       void removeEdge(OA_ptr<DGraph::EdgeInterface> e)
       {

          mDG.removeEdge(e);
       }

      void removeNode(OA_ptr<DGraph::NodeInterface> n)
      {

        mDG.removeNode(n);
      }
      */
 
public: 
      //-------------------------------------
      // CFG building methods
      //-------------------------------------
      void setEntry(OA_ptr<Node> n);
      void setExit(OA_ptr<Node> n);

      OA_ptr<Node> splitBlock(OA_ptr<Node>, StmtHandle);

      OA_ptr<Edge> connect(OA_ptr<NodeInterface> src,
                       OA_ptr<NodeInterface> dst,
                       EdgeType type);

  
      OA_ptr<Edge> connect(OA_ptr<NodeInterface> src,
                       OA_ptr<NodeInterface> dst,
                       EdgeType type, ExprHandle expr) ;


      class NodeLabel;
      typedef std::list<NodeLabel> NodeLabelList;
      void connect(OA_ptr<NodeInterface>, NodeLabelList&);
      void connect(NodeLabelList&, OA_ptr<NodeInterface>);
     
      void disconnect(OA_ptr<EdgeInterface> e); 
      void disconnect(OA_ptr<NodeInterface> n);
  
      //-------------------------------------
      // CFG attaching labels to nodes
      //-------------------------------------
      OA_ptr<Node> node_from_label(OA::StmtLabel);
      OA_ptr<Node> getLabelBlock(StmtLabel lab); 
      //! associate given label with given node
      void mapLabelToNode(OA::StmtLabel lab, OA_ptr<Node> n);
      //! determine if the given label is currently mapped to a node
      bool isLabelMappedToNode(StmtLabel lab);

      // members for CFG
private:
      //static const char* sEdgeTypeToString[]; //moved to edge class
      SymHandle mName;
      OA_ptr<Node> mEntry;
      OA_ptr<Node> mExit;
      std::map<OA::StmtLabel, OA_ptr<Node> > mlabel_to_node_map;

//      DGraph::DGraphImplement mDG;

public: 

      class NodeLabelListIterator;
      friend class NodeLabelListIterator;
      /*----------------------------------------------------------------*/
      /*! A label for a node that contains information about edgetype?,
       * and an expression for each node?
     *//*-------------------------------------------------------------*/
      class NodeLabel {
        public:
          NodeLabel(OA_ptr<Node> _n, EdgeType _et, ExprHandle _eh = 0) :
                    mN(_n), mEh(_eh), mEt(_et) {}
          OA_ptr<Node> getNode();
          EdgeType getEdgeType();
          OA::ExprHandle getExpr();

        private:
          OA_ptr<Node> mN;
          OA::ExprHandle mEh;
          EdgeType mEt;
       };

       /*----------------------------------------------------------------*/
       /*! An iterator over a list of Node labels.
        * and an expression for each node?
       *//*-------------------------------------------------------------*/
       class NodeLabelListIterator {
         public:
            NodeLabelListIterator(NodeLabelList& nl)  
               : mList(nl), mIter(mList.begin()) {}
           ~NodeLabelListIterator() {}
           void operator++();
           void operator++(int);
           bool isValid() const;
           NodeLabel current() const;
         private:
           NodeLabelList& mList;
           std::list<NodeLabel>::iterator mIter;
        };
};

 //--------------------------------------------------------------------
  /*! An iterator for the statements in a node.
  */
  //--------------------------------------------------------------------
  class NodeStatementsIterator : public virtual
      NodeStatementsIteratorInterface {
  public:
    NodeStatementsIterator(const Node& node)
        { mList = node.mStmt_list;
          mIter = mList->begin();
        }
    ~NodeStatementsIterator() {}
    void operator++();
    void operator++(int);
    //! returns true if we are not past the end of the list
    bool isValid() const;
    //! returns the statement handle that the iterator is currently pointing
    StmtHandle current() const;
    void reset();
  private:
    OA_ptr<std::list<StmtHandle> > mList;
    std::list<StmtHandle>::iterator mIter;
  };

  //--------------------------------------------------------------------
  /*! An iterator for the statements in a node.
  */
  //--------------------------------------------------------------------
  class NodeStatementsRevIterator : public virtual
      NodeStatementsRevIteratorInterface {
  public:
    NodeStatementsRevIterator(const Node& node)
        { mList = node.mStmt_list;
          mRevIter = mList->rbegin();
        }
    ~NodeStatementsRevIterator() {}
    void operator++();
    void operator++(int);
    //! returns true if we are not past the end of the list
    bool isValid() const;
    //! returns the statement handle that the iterator is currently pointing
    StmtHandle current() const;
    void reset();
  private:
    OA_ptr<std::list<StmtHandle> > mList;
    std::list<StmtHandle>::reverse_iterator mRevIter;
  };


  } // end of CFG namespace
} // end of OA namespace

#endif
