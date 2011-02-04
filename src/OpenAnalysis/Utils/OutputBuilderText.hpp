/*! \file
  
  \brief An OutputBuilder for text output
  
  \authors Michelle Strout
  \version $Id: OutputBuilderText.hpp,v 1.4 2005/01/18 21:13:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef OutputBuilderText_H
#define OutputBuilderText_H

#include "OutputBuilder.hpp"
#include <iostream>
using namespace std;

namespace OA {

//! Output a new line and indent appropriatly
ostream &indt(ostream &os);

//! Increase the indentation on all following lines
ostream &pushIndt(ostream &os);

//! Decrease the indentation on all following lines
ostream &popIndt(ostream &os);

// colors are specific to UNIX terminals.
// use color sparingly
ostream &black(ostream &os);
ostream &red(ostream &os);
ostream &green(ostream &os);
ostream &yellow(ostream &os);
ostream &blue(ostream &os);
ostream &magenta(ostream &os);
ostream &purple(ostream &os);
ostream &cyan(ostream &os);
ostream &white(ostream &os);
ostream &brBlack(ostream &os);
ostream &darkGray(ostream &os);
ostream &darkGrey(ostream &os);
ostream &brRed(ostream &os);
ostream &brGreen(ostream &os);
ostream &brYellow(ostream &os);
ostream &brBlue(ostream &os);
ostream &brMagenta(ostream &os);
ostream &brPurple(ostream &os);
ostream &brCyan(ostream &os);
ostream &brWhite(ostream &os);


ostream &blackBG(ostream &os);
ostream &redBG(ostream &os);
ostream &greenBG(ostream &os);
ostream &yellowBG(ostream &os);
ostream &blueBG(ostream &os);
ostream &magentaBG(ostream &os);
ostream &purpleBG(ostream &os);
ostream &cyanBG(ostream &os);
ostream &whiteBG(ostream &os);
ostream &brBlackBG(ostream &os);
ostream &darkGrayBG(ostream &os);
ostream &darkGreyBG(ostream &os);
ostream &brRedBG(ostream &os);
ostream &brGreenBG(ostream &os);
ostream &brYellowBG(ostream &os);
ostream &brBlueBG(ostream &os);
ostream &brMagentaBG(ostream &os);
ostream &brPurpleBG(ostream &os);
ostream &brCyanBG(ostream &os);
ostream &brWhiteBG(ostream &os);


ostream &boldOn(ostream &os);
ostream &underlineOn(ostream &os);
ostream &blinkOn(ostream &os);

ostream &boldOff(ostream &os);
ostream &underlineOff(ostream &os);
ostream &blinkOff(ostream &os);

ostream &resetColor(ostream &os);



class OutputBuilderText : public OutputBuilder { 

  public:
    OutputBuilderText() : mObjDepth(0), mStream(std::cout) {}
    OutputBuilderText(std::ostream& s) : mObjDepth(0), mStream(s) {}

    //void configStream(std::ostream& s) : mStream(s) {}

  virtual void outputString(const std::string &str);

    //*****************************************************************
    // IRHandles
    //*****************************************************************
    void outputIRHandle(ProcHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(StmtHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(ExprHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(CallHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(OpHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(MemRefHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(SymHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(ConstSymHandle h, IRHandlesIRInterface& pIR);
    void outputIRHandle(ConstValHandle h, IRHandlesIRInterface& pIR);

    //*****************************************************************
    // Object
    //*****************************************************************
    void objStart(const std::string& objName); 
    void objEnd(const std::string& objName);

    //! short hand when specifying string for field value
    void field(const std::string& fieldname, const std::string& value);
 
    // for fields whose value requires further output calls
    void fieldStart(const std::string& fieldname);
    void fieldEnd(const std::string& fieldname);

    //*****************************************************************
    // Lists
    //*****************************************************************
    void listStart();
    void listEnd();

    //! short hand when specifying string for list item
    void listItem(const std::string& value);

    //! for list items that require more output calls
    void listItemStart();
    void listItemEnd();

    //*****************************************************************
    // Maps
    //*****************************************************************
    void mapStart(const std::string& label,
                  const std::string& keyLabel,
                  const std::string& valueLabel);
    void mapEnd(const std::string& label);

    //! short hand when all parts of entry are string
    void mapEntry(const std::string& key, const std::string& value);
    void mapKey(const std::string& key);
    void mapValue(const std::string& value);


    //! for map entries that require more output calls
    void mapEntryStart();
    void mapEntryEnd();
    void mapKeyStart();
    void mapKeyEnd();
    void mapValueStart();
    void mapValueEnd();

    //*****************************************************************
    // Graphs
    //*****************************************************************
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
    virtual void graphEdgeLabelStart();
    virtual void graphEdgeLabelEnd();
    virtual void graphEdgeSourceNode(int id);
    virtual void graphEdgeSinkNode(int id);
    virtual void graphEdgeEnd();
    
    //*****************************************************************
    // Comments
    //*****************************************************************
    virtual void comment(std::string s);

  protected:
    int mObjDepth;
    std::ostream& mStream;

  private:
    int mListItemCount;
};

} // end of OA namespace

#endif

