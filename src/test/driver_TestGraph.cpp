/*! \file
    
  \brief This program reads in a restricted dot file format
         and tests the directed graph class.

  \usage ./driver_TestGraph file.dot

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//#include "DGraph.cpp"

#include "OpenAnalysis/Utils/DGraph/DGraphInterface.hpp"
#include "OpenAnalysis/Utils/DGraph/DGraphImplement.hpp"
#include "OpenAnalysis/Utils/OutputBuilderDOT.hpp"
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

namespace OA
{
static bool debug = false;

/*! The TestGraph is different than graphs such as the CFG and ICFG in
    that those graphs will have abstract interfaces for themselves
    to enable different implementations.  This guy doesn't have an
    abstract interface.
*/

namespace TestGraph {
    class NodesIterator;
    class EdgesIterator;
    class Node;
    class Edge;
    class TestGraph;

    class NodesIterator : public DGraph::NodesIteratorImplement 
    {
      public:
        NodesIterator(OA_ptr<DGraph::NodesIteratorInterface> ni)
            : DGraph::NodesIteratorImplement(ni) {}
        ~NodesIterator() { }
        
        OA_ptr<Node> currentTestGraphNode() const
            {
                return current().convert<Node>();
            }
    };

    class EdgesIterator : public DGraph::EdgesIteratorImplement 
    {
      public:
        EdgesIterator(OA_ptr<DGraph::EdgesIteratorInterface> ei)
            : DGraph::EdgesIteratorImplement(ei) {}
        ~EdgesIterator()    {}

        OA_ptr<Edge> currentTestGraphEdge() const
            {
                return current().convert<Edge>();
            }
    };
    class Node : public virtual DGraph::NodeInterface {
      public:
        Node(char c) : mChar(c), mDGNode() { }

        // will implement the DGraph::NodeInterface by making calls
        // to mDGNode, this is called delegation
        //========================================================  
        // Information
        //========================================================
        unsigned int getId () const { return mDGNode.getId(); }
        char getChar() const { return mChar; }
        OA_ptr<DGraph::EdgesIteratorInterface> 
            getIncomingEdgesIterator()
            {
                return mDGNode.getIncomingEdgesIterator();
            }
        int num_incoming () const { return mDGNode.num_incoming(); }
        int num_outgoing () const { return mDGNode.num_outgoing(); }
      
        //! returns true if node is an entry node, IOW  has no incoming edges
        virtual bool isAnEntry() const { return mDGNode.isAnEntry(); }
  
        //! returns true if node is an exit node, IOW  has no outgoing edges
        virtual bool isAnExit() const { return mDGNode.isAnExit(); }

        //========================================================  
        // Iterators
        //========================================================
        OA_ptr<DGraph::EdgesIteratorInterface> getIncomingEdgesIterator() const
            {
                return mDGNode.getIncomingEdgesIterator();
            }
        
        OA_ptr<DGraph::EdgesIteratorInterface> getOutgoingEdgesIterator() const
            {
                return mDGNode.getOutgoingEdgesIterator();
            }

        OA_ptr<DGraph::NodesIteratorInterface> getSourceNodesIterator() const
            {
                return mDGNode.getSourceNodesIterator();
            }

        OA_ptr<DGraph::NodesIteratorInterface> getSinkNodesIterator() const
            {
                return mDGNode.getSinkNodesIterator();
            }

        //========================================================  
        // Comparison operators
        //========================================================
        bool operator== (NodeInterface& other) 
            {
                if (debug) {
                    std::cout << "TestGraph::Node::operator==" << std::endl;
                    std::cout << "\tmChar = " << mChar;
                    std::cout << ", other.mChar = "
                              << dynamic_cast<Node&>(other).mChar << std::endl;
                }
                return mChar == dynamic_cast<Node&>(other).mChar;
            }
        bool operator< (NodeInterface& other)
            {
                if (debug) {
                    std::cout << "TestGraph::Node::operator<" << std::endl;
                    std::cout << "\tmChar = " << mChar;
                    std::cout << ", other.mChar = " 
                              << dynamic_cast<Node&>(other).mChar << std::endl;
                }
                return mChar < dynamic_cast<Node&>(other).mChar;
            }

        //========================================================  
        // Output
        //========================================================
        void output(IRHandlesIRInterface& ir) 
            {
                std::ostringstream label;
                label << getId() << ": " << mChar;
                sOutBuild->outputString(label.str());
            }

        void dump(std::ostream& os) 
            {
                os << "Node: getId() = " << getId();
                os << ", mChar = " << mChar << std::endl;
            }

        //========================================================  
        // Construction
        //========================================================
        void addIncomingEdge(OA_ptr<DGraph::EdgeInterface> e) 
            {
                mDGNode.addIncomingEdge(e);
            }
        void addOutgoingEdge(OA_ptr<DGraph::EdgeInterface> e) 
            {
                mDGNode.addOutgoingEdge(e);
            }
      private:
        DGraph::NodeImplement mDGNode;
        char mChar;
    };

    class Edge : public virtual DGraph::EdgeInterface {
      public:
        Edge(OA_ptr<DGraph::NodeInterface> source,
             OA_ptr<DGraph::NodeInterface> sink)
            : mDGEdge(source,sink) {}
        
        //========================================================  
        // Information
        //========================================================
        unsigned int getId () const { return mDGEdge.getId(); }
        virtual OA_ptr<DGraph::NodeInterface> source() const
            {
                return mDGEdge.source();
            }
        virtual OA_ptr<DGraph::NodeInterface> sink() const
            {
                return mDGEdge.sink();
            }

        //========================================================  
        // Comparison operators
        //========================================================
        bool operator== (EdgeInterface& other) 
            {
                return getId() == other.getId();
            }
        bool operator< (EdgeInterface& other)
            {
                return getId() < other.getId();
            }

        //========================================================  
        // Output
        //========================================================
        void output(IRHandlesIRInterface& ir) 
            {
                std::ostringstream label;
                label << getId(); 
                sOutBuild->outputString(label.str());
            }
        void dump(std::ostream& os) 
            {
                os << "Edge: getId() = " << getId();
                os << ", sourceId = " << source()->getId();
                os << ", sinkId = " << sink()->getId() << std::endl;
            }

        //========================================================  
        // Construction
        //========================================================

      private:
        DGraph::EdgeImplement mDGEdge;
    };


    /*! Since TestGraph does not have an abstract interface that
        in turn inherits from the DGraph abstract interface, we
        indicate that TestGraph implements the DGraph
        abstract interface by having it inherit from DGraph::DGraphInterface.
    */

    class TestGraph : public virtual DGraph::DGraphInterface
    {
      public:
        //========================================================  
        // Iterators
        //========================================================
        // the one required by DGraphInterface
        OA_ptr<DGraph::NodesIteratorInterface> getNodesIterator() const
            {
                return mDG.getNodesIterator();
            }
        // method to get a more specific iterator
        OA_ptr<NodesIterator> getTestGraphNodesIterator() const
            {
                OA_ptr<NodesIterator> retval;
                retval = new NodesIterator(mDG.getNodesIterator());
                return retval;
            }

        OA_ptr<DGraph::NodesIteratorInterface> getEntryNodesIterator() const
            {
                return mDG.getEntryNodesIterator();
            }

        OA_ptr<DGraph::NodesIteratorInterface> getExitNodesIterator() const
            {
                return mDG.getExitNodesIterator();
            }

        OA_ptr<DGraph::NodesIteratorInterface>
            getReversePostDFSIterator(
                    DGraph::DGraphEdgeDirection pOrient) 
            {
                return mDG.getReversePostDFSIterator(pOrient);
            }

        OA_ptr<DGraph::EdgesIteratorInterface> getEdgesIterator() const
            {
                return mDG.getEdgesIterator();
            }
        // method to get a more specific iterator
        OA_ptr<EdgesIterator> getTestGraphEdgesIterator() const
            {
                OA_ptr<EdgesIterator> retval;
                retval = new EdgesIterator(mDG.getEdgesIterator());
                return retval;
            }

        //========================================================  
        // Construction
        //========================================================
        void addNode(OA_ptr<DGraph::NodeInterface> n)
            {
                mDG.addNode(n);
            }
        void addEdge(OA_ptr<DGraph::EdgeInterface> e)
            {
                mDG.addEdge(e);
            }

        //========================================================  
        // Output
        //========================================================
        void output(IRHandlesIRInterface& ir)
            {
                mDG.output(ir);
            }
        
        //! not part of the normal interface, purely for testing
        //! will read in a dot file that uses restricted syntax
        //! Specifically:
        //!     digraph whatever {
        //!         a -> c;
        //!         b -> c;
        //!         ...
        //!     }
        //! Each node is assumed to be designated with only one character.
        //! There should be no labeling.
        void input(std::ifstream& in)
{
    std::string digraph, graphname, lcurly;
    in >> digraph >> graphname >> lcurly;
    assert(lcurly=="{");

    std::string nextchar;
    std::string node1, arrow, node2;
    in >> nextchar;
    while (nextchar!="}") {
        node1 = nextchar;
        in >> arrow >> node2;
        //std::cout << "node1 = " << node1 << std::endl;
        //std::cout << "arrow = " << arrow << std::endl;
        //std::cout << "node2 = " << node2 << std::endl;

        // create the two nodes and create an edge 
        // between them
        OA_ptr<Node> n1 = findNode(node1.c_str()[0]); 
        if (n1.ptrEqual(0)) { n1 = new Node(node1.c_str()[0]); }
        OA_ptr<Node> n2 = findNode(node2.c_str()[0]); 
        if (n2.ptrEqual(0)) { n2 = new Node(node2.c_str()[0]); }
        OA_ptr<Edge> e; e = new Edge(n1,n2);
        addEdge(e);

        in >> nextchar;
    }
}

      protected:
        std::string getGraphName() 
            {
                return std::string("TestGraph");
            }
        // returns the node with the given char label if one exists
        // I think we only need this functionality in TestGraph
        // because of the input routine
        OA_ptr<Node> findNode(char c)
            {
                OA_ptr<Node> retval;
                OA_ptr<NodesIterator> nodeIter = getTestGraphNodesIterator();
                for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
                    OA_ptr<Node> node = nodeIter->currentTestGraphNode();
                    if (node->getChar() == c) {
                        retval = node;
                        break;
                    }
                }
                return retval;
            }
      private:
        DGraph::DGraphImplement mDG;
    };
} // end of TestGraph namespace

class BogusIRInterface : public virtual IRHandlesIRInterface {
public:
  //--------------------------------------------------------
  // create a string for the given handle, should be succinct
  // and there should be no newlines
  //--------------------------------------------------------  
  virtual std::string toString(const ProcHandle h) {}
  virtual std::string toString(const StmtHandle h) {}
  virtual std::string toString(const ExprHandle h) {}
  virtual std::string toString(const OpHandle h) {}
  virtual std::string toString(const MemRefHandle h) {}
  virtual std::string toString(const CallHandle h) {}
  virtual std::string toString(const SymHandle h) {}
  virtual std::string toString(const ConstSymHandle h) {}
  virtual std::string toString(const ConstValHandle h) {}

  //--------------------------------------------------------
  // dumps that do pretty prints
  //--------------------------------------------------------  

  //! Given a memory reference, pretty-print it to the output stream os.
  virtual void dump(MemRefHandle h, std::ostream& os) {}

  //! Given a statement, pretty-print it to the output stream os.
  virtual void dump(StmtHandle stmt, std::ostream& os) {}

};

} // end of namespace OA


using namespace std;

//int main ( unsigned argc,  char * argv[] )
int main(int argc, char **argv)
{
    // parse the input file name from the command line
    std::string inFileStr;

    if(argc > 1) {
        inFileStr = argv[1];
    }
    else {
        std::cout << "usage: driver_TestGraph infile.dot" << std::endl;
        return -1;
    }

    std::ifstream inFile(inFileStr.c_str());

    // create a TestGraph and read its structure in from the input file
    OA::TestGraph::TestGraph tg;
    tg.input(inFile);

    // creating a bogus IRHandlesIRInterface
    OA::BogusIRInterface bogus;
    OA::OA_ptr<OA::OutputBuilder> ob; 
    ob = new OA::OutputBuilderDOT();
    tg.configOutput(ob);
    tg.output(bogus);

    // testing the various iterators
    OA::OA_ptr<OA::DGraph::NodesIteratorInterface> nodeIter;
    std::cout << std::endl << "NodesIterator: " << std::endl;
    nodeIter = tg.getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA::OA_ptr<OA::DGraph::NodeInterface> node = nodeIter->current();
        node->dump(std::cout);
    }

    OA::OA_ptr<OA::DGraph::EdgesIteratorInterface> edgeIter;
    std::cout << std::endl << "getOutgoingEdgesIterator: " << std::endl;
    nodeIter = tg.getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA::OA_ptr<OA::DGraph::NodeInterface> node = nodeIter->current();
        std::cout << "Starting Node:" << std::endl;
        node->dump(std::cout);
        std::cout << "\tOutgoing edges:" << std::endl;
        OA::OA_ptr<OA::DGraph::EdgesIteratorInterface> outIter
            = node->getOutgoingEdgesIterator();
        for ( ; outIter->isValid(); (*outIter)++ ) {
            OA::OA_ptr<OA::DGraph::EdgeInterface> out = outIter->current();
            out->dump(std::cout);
        }
        std::cout << std::endl;
    }

    std::cout << std::endl << "getSinkNodesIterator: " << std::endl;
    nodeIter = tg.getNodesIterator();
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA::OA_ptr<OA::DGraph::NodeInterface> node = nodeIter->current();
        std::cout << "Starting Node:" << std::endl;
        node->dump(std::cout);
        std::cout << "\tSinks:" << std::endl;
        OA::OA_ptr<OA::DGraph::NodesIteratorInterface> sinkIter
            = node->getSinkNodesIterator();
        for ( ; sinkIter->isValid(); (*sinkIter)++ ) {
            OA::OA_ptr<OA::DGraph::NodeInterface> sink = sinkIter->current();
            sink->dump(std::cout);
        }
        std::cout << std::endl;
    }


    // testing the reverse post dfs ordering
    std::cout << std::endl << "ReversePostDFSIterator: " << std::endl;
    nodeIter = tg.getReversePostDFSIterator(OA::DGraph::DEdgeOrg);
    for ( ; nodeIter->isValid(); (*nodeIter)++ ) {
        OA::OA_ptr<OA::DGraph::NodeInterface> node = nodeIter->current();
        node->dump(std::cout);
    }  
    return 0;
}




