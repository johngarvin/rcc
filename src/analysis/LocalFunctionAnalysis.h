// -*- Mode:C++ -*-
#ifndef LOCAL_FUNCTION_ANALYSIS_H
#define LOCAL_FUNCTION_ANALYSIS_H

#include <include/R/R_RInternals.h>

//! LocalFunctionAnalysis
//!
//! Discovers local information about the given function
//! definition. Adds annotations to the global AnalysisResults.
class LocalFunctionAnalysis {
public:
  LocalFunctionAnalysis(const SEXP fundef);
  void perform_analysis();
private:
  void analyze_args();
  void collect_mentions_and_call_sites();

  const SEXP m_fundef;
};

#endif
