/*! \file
  
  \brief Definitions for OutputBuilder for DOT output
  
  \authors Andy Stone, modified by Luis Ramos and John Garvin

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "OutputBuilderDOT.hpp"
#include <string>

std::string filter(const std::string &cpsource,const std::string &find,const std::string &replace )
{
  std::string source = cpsource;
  size_t i = source.find(find);
  while (i != std::string::npos) {
    source.replace(i, find.length(), replace);
    i = source.find(find, i + replace.length());
  }
  return source;
}

namespace OA {

OutputBuilderDOT::OutputBuilderDOT() :
    OutputBuilderText(std::cout)
{
}

OutputBuilderDOT::OutputBuilderDOT(std::ostream& stream) :
    OutputBuilderText(stream)
{
}

void OutputBuilderDOT::outputString(const std::string &str)
{
    mStream << filter(filter(str, "\n", "\\n"), "\"", "\\\"");
}

void OutputBuilderDOT::graphStart(const std::string &label)
{
    mStream << indt << "digraph " << filter(label, "\n", "\\n") 
            << " {" << pushIndt;
    mStream << indt << "node [shape=rectangle];";
}

void OutputBuilderDOT::graphEnd(const std::string &label)
{
  //mStream << popIndt << indt << "}" << std::endl;
  mStream << popIndt << "}" << std::endl;
}

void OutputBuilderDOT::graphSubStart(const std::string & label)
{
  // label now comes with a complete path: GrParent::Parent::ChildName
  // for the label of the subgraph, we can use label as is.
  // but for the cluster name, the "::"s get in the way. BK 4/17/07
  int len = label.length();
  // find last ';'
  int lastColonPos = -1;
  int i;
  for (i = len-1; i >= 0; i--) {
    if (label[i]==':') {
      lastColonPos = i;
      break; // exit loop
    }
  }
  if (lastColonPos == -1) {
    // do same-'ol-same-'ol
    mStream << indt << "subgraph cluster_" << label << " {" 
            << pushIndt;
  } else {
    // strip off the characters preceding the last :
    mStream << indt << "subgraph cluster_" << label.substr(i+1,len-i) << " {"
            << pushIndt;
  }

  // but the label can have the ::
  mStream << indt << "label=\"" << filter(filter(label, "\n", "\\n"), "\"", "\\\"")  << "\"";
}

void OutputBuilderDOT::graphSubEnd(const std::string &label)
{
  mStream << popIndt << indt << "}";
}

void OutputBuilderDOT::graphNodeStart(int id)
{
  mStream << indt << id << " [ ";
}

void OutputBuilderDOT::graphNodeLabel(const std::string &label)
{
    mStream << "label=\"" << filter(filter(label, "\n", "\\n"), "\"", "\\\"") << "\"";
}

void OutputBuilderDOT::graphNodeLabelStart()
{
    mStream << "label=\"";
}

void OutputBuilderDOT::graphNodeLabelEnd()
{
    mStream << "\"";
}

void OutputBuilderDOT::graphNodeEnd()
{
  mStream << " ];";
}

void OutputBuilderDOT::graphEdgeStart()
{
}

void OutputBuilderDOT::graphEdgeSourceNode(int id)
{
    mStream << indt << id << " -> ";
}

void OutputBuilderDOT::graphEdgeSinkNode(int id)
{
  mStream << id << " [";
}

void OutputBuilderDOT::graphEdgeLabelStart()
{
    mStream << " label=\"";
}

void OutputBuilderDOT::graphEdgeLabelEnd()
{
    mStream << "\"";
}

void OutputBuilderDOT::graphEdgeEnd()
{
  mStream << " ];";
}

} // end of OA namespace

