/*! \file
  
  \brief Definitions for OutputBuilder for text output
  
  \authors Michelle Strout, Andy Stone
  \version $Id: OutputBuilderText.cpp,v 1.3 2005/01/18 21:13:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "OutputBuilderText.hpp"

namespace OA {

int gNestedObjects = 0;
int gIndentLevel = 0;
ostream &indt(ostream &os)
{
    os << '\n';
    os.flush();
    for(int i = 0; i < gIndentLevel; i++) { os << "    "; }
    return os;
}

ostream &pushIndt(ostream &os)
{
    gIndentLevel++;
    return os;
}

ostream &popIndt(ostream &os) 
{
    gIndentLevel--;
    return os;
}

ostream &black(ostream &os)     { os << "\033[30m"; return os; }
ostream &red(ostream &os)       { os << "\033[31m"; return os; }
ostream &green(ostream &os)     { os << "\033[32m"; return os; }
ostream &yellow(ostream &os)    { os << "\033[33m"; return os; }
ostream &blue(ostream &os)      { os << "\033[34m"; return os; }
ostream &magenta(ostream &os)   { os << "\033[35m"; return os; }
ostream &purple(ostream &os)    { os << "\033[35m"; return os; }
ostream &cyan(ostream &os)      { os << "\033[36m"; return os; }
ostream &white(ostream &os)     { os << "\033[37m"; return os; }
ostream &brBlack(ostream &os)   { os << "\033[90m"; return os; }
ostream &darkGray(ostream &os)  { os << "\033[90m"; return os; }
ostream &darkGrey(ostream &os)  { os << "\033[90m"; return os; }
ostream &brRed(ostream &os)     { os << "\033[91m"; return os; }
ostream &brGreen(ostream &os)   { os << "\033[92m"; return os; }
ostream &brYellow(ostream &os)  { os << "\033[93m"; return os; }
ostream &brBlue(ostream &os)    { os << "\033[94m"; return os; }
ostream &brMagenta(ostream &os) { os << "\033[95m"; return os; }
ostream &brPurple(ostream &os)  { os << "\033[95m"; return os; }
ostream &brCyan(ostream &os)    { os << "\033[96m"; return os; }
ostream &brWhite(ostream &os)   { os << "\033[97m"; return os; }

ostream &blackBG(ostream &os)     { os << "\033[40m"; return os; }
ostream &redBG(ostream &os)       { os << "\033[41m"; return os; }
ostream &greenBG(ostream &os)     { os << "\033[42m"; return os; }
ostream &yellowBG(ostream &os)    { os << "\033[43m"; return os; }
ostream &blueBG(ostream &os)      { os << "\033[44m"; return os; }
ostream &magentaBG(ostream &os)   { os << "\033[45m"; return os; }
ostream &purpleBG(ostream &os)    { os << "\033[45m"; return os; }
ostream &cyanBG(ostream &os)      { os << "\033[46m"; return os; }
ostream &whiteBG(ostream &os)     { os << "\033[47m"; return os; }
ostream &brBlackBG(ostream &os)   { os << "\033[100m"; return os; }
ostream &darkGrayBG(ostream &os)  { os << "\033[100m"; return os; }
ostream &darkGreyBG(ostream &os)  { os << "\033[100m"; return os; }
ostream &brRedBG(ostream &os)     { os << "\033[101m"; return os; }
ostream &brGreenBG(ostream &os)   { os << "\033[102m"; return os; }
ostream &brYellowBG(ostream &os)  { os << "\033[103m"; return os; }
ostream &brBlueBG(ostream &os)    { os << "\033[104m"; return os; }
ostream &brMagentaBG(ostream &os) { os << "\033[105m"; return os; }
ostream &brPurpleBG(ostream &os)  { os << "\033[105m"; return os; }
ostream &brCyanBG(ostream &os)    { os << "\033[106m"; return os; }
ostream &brWhiteBG(ostream &os)   { os << "\033[107m"; return os; }

ostream &boldOn(ostream &os)          { os << "\033[1m"; return os; }
ostream &underlineOn(ostream &os)     { os << "\033[4m"; return os; }
ostream &blinkOn(ostream &os)         { os << "\033[5m"; return os; }

ostream &boldOff(ostream &os)          { os << "\033[22m"; return os; }
ostream &underlineOff(ostream &os)     { os << "\033[24m"; return os; }
ostream &blinkOff(ostream &os)         { os << "\033[25m"; return os; }

ostream &resetColor(ostream &os) { os << "\033[0m"; return os; }


void OutputBuilderText::outputString(const std::string &str)
{
    mStream << str;
}

//*****************************************************************
// IRHandles
//*****************************************************************
void OutputBuilderText::outputIRHandle(ProcHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "ProcHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(StmtHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "StmtHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(ExprHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "ExprHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(CallHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "CallHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(OpHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "OpHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(MemRefHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "MemRefHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(SymHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "SymHandle(" << pIR.toString(h) << ")";
}

void OutputBuilderText::outputIRHandle(ConstSymHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "ConstSymHandle(" << pIR.toString(h) << ")";
}

void 
OutputBuilderText::outputIRHandle(ConstValHandle h,
    IRHandlesIRInterface& pIR)
{
    mStream << "ConstValHandle(" << pIR.toString(h) << ")";
}

//*****************************************************************
// Objects
//*****************************************************************
void OutputBuilderText::objStart(const std::string& objName)
/*!
 * \param objName   Name of object class
 *
 * The start of each object increased object depth.
 */
{ 
    gNestedObjects++;
    mStream << indt << objName << "(" << pushIndt;
}

void OutputBuilderText::objEnd(const std::string& objName)
/*!
 * The end of each object decreases object depth.
 */
{ 
    gNestedObjects--;
    mStream << popIndt << indt << ")";
    
    // We want to output a newline character after the object is done.
    // We must insure that nested objects (caused by two or more calls to
    //  objStart without a call to objEnd) do not outline this newline char.
    if(gNestedObjects <= 0) {
        mStream << endl;
    }
}

void OutputBuilderText::field(const std::string& fieldname,
    const std::string& value)
{
    mStream << indt << fieldname << ": " << value; 
}

void OutputBuilderText::fieldStart(const std::string& fieldname) 
{
    mStream << indt << fieldname + ": "; 
}

void OutputBuilderText::fieldEnd(const std::string& fieldname) 
{
}
    
/*
void OutputBuilderText::fieldValue(const std::string& fieldvalue) 
{ 
    mStream << fieldvalue; 
}

void OutputBuilderText::fieldValueBegin() 
{}
    
void OutputBuilderText::fieldValueEnd() 
{}

void OutputBuilderText::fieldDelimit() 
{ mStream << ", "; }
*/

void OutputBuilderText::listStart() 
{ mStream << "[ " << pushIndt;  mListItemCount = 0; }

void OutputBuilderText::listEnd()   
{ mStream << " ] " << popIndt; }

void OutputBuilderText::listItem(const std::string& value) 
{ 
  if(mListItemCount>0) {mStream << "    "; }
  mStream << value; 
  mListItemCount++;
} 

void OutputBuilderText::listItemStart() 
{
  if(mListItemCount>0) {mStream << "    "; }
}

void OutputBuilderText::listItemEnd() 
{
  mListItemCount++;
}

//*****************************************************************
// Maps
//*****************************************************************
void OutputBuilderText::mapStart(const std::string& label,
                  const std::string& keyLabel,
                  const std::string& valueLabel)
{ 
  mStream << indt << label << ": " << keyLabel << "\t => " << valueLabel;
  mStream << pushIndt;
}

void OutputBuilderText::mapEnd(const std::string& label)   
{ 
//  mStream << popIndt;
    mStream << popIndt << std::endl;
}

void OutputBuilderText::mapEntry(const std::string& key, const std::string& value)
{ 
  mapEntryStart();
  mapKey(key);
  mapValue(value);
  mapEntryEnd();
} 

void OutputBuilderText::mapKey(const std::string& key)
{ 
  mapKeyStart();
  mStream << key;
  mapKeyEnd();
} 

void OutputBuilderText::mapValue(const std::string& value)
{ 
  mapValueStart();
  mStream << value;
  mapValueEnd();
} 


void OutputBuilderText::mapEntryStart() 
{
  mStream << pushIndt;
}

void OutputBuilderText::mapEntryEnd() 
{
  mStream << popIndt;
}

void OutputBuilderText::mapKeyStart() 
{
  mStream << indt;
}

void OutputBuilderText::mapKeyEnd() 
{
  mStream << "\t => ";
}

void OutputBuilderText::mapValueStart() 
{
}

void OutputBuilderText::mapValueEnd() 
{
}

//*****************************************************************
// Graphs
//*****************************************************************
void OutputBuilderText::graphStart(const std::string &label)
{
    mStream << indt << "graph " << label << ":" << std::endl << pushIndt;
}

void OutputBuilderText::graphEnd(const std::string &label)
{
    mStream << popIndt << std::endl;
}

void OutputBuilderText::graphSubStart(const std::string &label)
{
    mStream << indt << "subgraph " << label << ":" << std::endl << pushIndt;
}

void OutputBuilderText::graphSubEnd(const std::string &label)
{
    mStream << popIndt << std::endl;
}

void OutputBuilderText::graphNodeStart(int id)
{
    mStream << indt << "Node " << id << ": ";
}

void OutputBuilderText::graphNodeLabel(const std::string &label)
{
    mStream << label;
}

void OutputBuilderText::graphNodeLabelStart()
{
    mStream << pushIndt << indt;
}

void OutputBuilderText::graphNodeLabelEnd()
{
    mStream << popIndt;
}

void OutputBuilderText::graphNodeEnd()
{
    mStream << std::endl;
}

void OutputBuilderText::graphEdgeStart()
{
    mStream << indt << "Edge: ";
}

void OutputBuilderText::graphEdgeLabelStart()
{
    mStream << pushIndt << indt;
}

void OutputBuilderText::graphEdgeLabelEnd()
{
    mStream << popIndt;
}

void OutputBuilderText::graphEdgeSourceNode(int id)
{
    mStream << id << " => ";
}

void OutputBuilderText::graphEdgeSinkNode(int id)
{
    mStream << id;
}

void OutputBuilderText::graphEdgeEnd()
{
    mStream << std::endl;
}

//*****************************************************************
// Comments
//*****************************************************************
void OutputBuilderText::comment(std::string s) {
    mStream << indt << "# " << s;
}

} // end of OA namespace

