// -*- Mode: C++ -*-

#ifndef BINDING_ANALYSIS_H
#define BINDING_ANALYSIS_H

#include <analysis/Annotation.h>

class BindingAnalysis {
public:
  BindingAnalysis(RAnnot::FuncInfo * root);

  void perform_analysis();
private:
  void assign_scopes();
  void fill_in_symbol_tables();
  RAnnot::FuncInfo * const m_root;
};

#endif // BINDING_ANALYSIS_H
