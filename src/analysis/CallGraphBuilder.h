// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_BUILDER_H
#define CALL_GRAPH_BUILDER_H

#include <analysis/Annotation.h>

class CallGraphBuilder {
public:
  CallGraphBuilder(RAnnot::FuncInfo* root);

  void perform_analysis();
private:
  RAnnot::FuncInfo* const m_root;  
};

#endif // CALL_GRAPH_BUILDER_H
