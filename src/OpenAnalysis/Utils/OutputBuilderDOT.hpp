/*! \file
  
  \brief An OutputBuilder for DOT output
  
  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef OutputBuilderDOT_H
#define OutputBuilderDOT_H

#include "OutputBuilderText.hpp"
#include <iostream>

namespace OA {

class OutputBuilderDOT : public OutputBuilderText { 
  public:
    OutputBuilderDOT();
    OutputBuilderDOT(std::ostream& stream);

    virtual void outputString(const std::string &str);

    virtual void graphStart(const std::string &label);
    virtual void graphEnd(const std::string &label);
    virtual void graphSubStart(const std::string &label);
    virtual void graphSubEnd(const std::string &label);

    virtual void graphNodeStart(int id);
    virtual void graphNodeLabel(const std::string &label);
    virtual void graphNodeLabelStart();
    virtual void graphNodeLabelEnd();
    virtual void graphNodeEnd();

    virtual void graphEdgeStart();
    virtual void graphEdgeSourceNode(int id);
    virtual void graphEdgeSinkNode(int id);
    virtual void graphEdgeLabelStart();
    virtual void graphEdgeLabelEnd();
    virtual void graphEdgeEnd();
};

} // end of OA namespace

#endif

