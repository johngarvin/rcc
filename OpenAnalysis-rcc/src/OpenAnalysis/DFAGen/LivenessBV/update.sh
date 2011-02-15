#!/bin/bash
DFAGEN_DIR=/s/chopin/l/grad/stonea/dfagen/src
ANALYSIS=LivenessBV

cp $DFAGEN_DIR/$ANALYSIS/* .
sed -e "s/OpenAnalysis\/$ANALYSIS\/auto_$ANALYSIS.hpp/OpenAnalysis\/DFAGen\/$ANALYSIS\/auto_$ANALYSIS.hpp/" auto_Manager$ANALYSIS.hpp > tmp
mv tmp auto_Manager$ANALYSIS.hpp
