/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
unsigned globalRef = 0;
ifstream fin;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
errorHandle(CirParseType type, string s = "")
{
   size_t idx;
   char   c;
   int    i;
   switch(type) {
      case AAG:
         if(fin.eof() && s == "") {
            errMsg = "aag";
            return parseError(MISSING_IDENTIFIER);
         }
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(s != "aag") {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "aag";
                  return parseError(MISSING_IDENTIFIER);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "number of variables";
                  return parseError(MISSING_NUM);
               }
               if(isdigit(s[idx]) && s.substr(0, 3) == "aag") {
                  return parseError(MISSING_SPACE);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_IDENTIFIER);
         }
         break;
      case M:
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "number of variables";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "number of PIs";
                  return parseError(MISSING_NUM);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "number of variables(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
         }
         break;
      case I:
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "number of PIs";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "number of latches";
                  return parseError(MISSING_NUM);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "number of variables(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
         }
         break;
      case L:
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "number of latches";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "number of POs";
                  return parseError(MISSING_NUM);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "number of variables(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
         }
         break;
      case O:
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "number of POs";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "number of AIGs";
                  return parseError(MISSING_NUM);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "number of variables(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
         }
         break;
      case A:
         if(s == "") {
            errMsg = "number of AIGs";
            return parseError(MISSING_NUM);
         }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == ' ' && idx == 0){
                  return parseError(EXTRA_SPACE);
               }
               if(s[idx] == ' '){
                  return parseError(MISSING_NEWLINE);
               }
               if((s[idx] <= 47 || s[idx] >= 58)) {
                  errMsg = "number of AIGs(" + s.substr(0, idx+1) + ")";
                  return parseError(ILLEGAL_NUM);
               }
               colNo += 1;
            }
            return parseError(MISSING_NEWLINE);
         }
         else {
            if(i < 0) {
               errMsg = "number of variables(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
         }
         break;
      case LIMIT:
         // Make sure that M >= I + O + A
         if(cirMgr->_maxIdx < (cirMgr->_nPI + cirMgr->_nLATCH + cirMgr->_nAIG)) {
            errMsg = "Number of variables";
            errInt = cirMgr->_maxIdx;
            return parseError(NUM_TOO_SMALL);
         }
         // LATCH must equal to 0 in this homework
         if(cirMgr->_nLATCH > 0) {
            errMsg = "latches";
            return parseError(ILLEGAL_NUM);
         }
         break;
      case PI:
         if(s == "DEF") {
            errMsg = "PI";
            return parseError(MISSING_DEF);
         }
         else if(s == "LIT") {
            errMsg = "PI literal ID";
            return parseError(MISSING_NUM);
         }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == ' ' && idx == 0){
                  return parseError(EXTRA_SPACE);
               }
               if(s[idx] == ' '){
                  return parseError(MISSING_NEWLINE);
               }
               if((s[idx] <= 47 || s[idx] >= 58)) {
                  errMsg = "PI literal ID(" + s + ")";
                  return parseError(ILLEGAL_NUM);
               }
               colNo += 1;
            }
         }
         else {
            if(i < 0) {
               errMsg = "PI literal ID(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
            if(i == 1 || i == 0) {
               errInt = i;
               return parseError(REDEF_CONST);
            }
            if(i % 2) {
               errMsg = "PI";
               errInt = i;
               return parseError(CANNOT_INVERTED);
            }
            if(VAR(unsigned(i)) > cirMgr->_maxIdx) {
               errInt = i;
               return parseError(MAX_LIT_ID);
            }
            if(cirMgr->_vAllGates[VAR(i)]) {
               errInt   = i;
               errGate  = cirMgr->_vAllGates[VAR(i)];
               return parseError(REDEF_GATE);
            }
         }
         break;
      case PO:
         if(s == "DEF") {
            errMsg = "PO";
            return parseError(MISSING_DEF);
         }
         else if(s == "LIT") {
            errMsg = "PO literal ID";
            return parseError(MISSING_NUM);
         }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == ' ' && idx == 0){
                  return parseError(EXTRA_SPACE);
               }
               if(s[idx] == ' '){
                  return parseError(MISSING_NEWLINE);
               }
               if((s[idx] <= 47 || s[idx] >= 58)) {
                  errMsg = "PI literal ID(" + s + ")";
                  return parseError(ILLEGAL_NUM);
               }
               colNo += 1;
            }
         }
         else {
            if(i < 0) {
               errMsg = "PO literal ID(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
            if(VAR(unsigned(i)) > cirMgr->_maxIdx) {
               errInt = i;
               return parseError(MAX_LIT_ID);
            }
         }
         break;
      case AIG_G:
         if(s == "DEF") {
            errMsg = "AIG";
            return parseError(MISSING_DEF);
         }
         else if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "AIG gate literal ID";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9 || s[idx] == '\n') {
                  return parseError(MISSING_SPACE);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "AIG gate literal ID(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
            if(i == 1) {
               errInt = i;
               return parseError(REDEF_CONST);
            }
            if(i % 2) {
               errMsg = "AIG gate";
               errInt = i;
               return parseError(CANNOT_INVERTED);
            }
            if(VAR(unsigned(i)) > cirMgr->_maxIdx) {
               errInt = i;
               return parseError(MAX_LIT_ID);
            }
            if(cirMgr->_vAllGates[VAR(i)]) {
               if(cirMgr->_vAllGates[VAR(i)]->getTypeStr() == "PI" 
               || cirMgr->_vAllGates[VAR(i)]->getTypeStr() == "AIG") {
               errInt   = i;
               errGate  = cirMgr->_vAllGates[VAR(i)];
               return parseError(REDEF_GATE);
               }
            }
         }
         break;
      case AIG_F0:
         if(s == "") { return parseError(EXTRA_SPACE); }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "AIG input literal ID";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9 || s[idx] == '\n') {
                  return parseError(MISSING_SPACE);
               }
               colNo += 1;
            }
            errMsg = s;
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(i < 0) {
               errMsg = "AIG input literal ID(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
            if(VAR(unsigned(i)) > cirMgr->_maxIdx) {
               errInt = i;
               return parseError(MAX_LIT_ID);
            }
         }
         break;
      case AIG_F1:
         if(s == "") {
            errMsg = "AIG input literal ID";
            return parseError(MISSING_NUM);
         }
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == ' ' && idx == 0){
                  return parseError(EXTRA_SPACE);
               }
               if(s[idx] == ' '){
                  return parseError(MISSING_NEWLINE);
               }
               if((s[idx] <= 47 || s[idx] >= 58)) {
                  errMsg = "AIG input literal ID(" + s.substr(0, idx+1) + ")";
                  return parseError(ILLEGAL_NUM);
               }
               colNo += 1;
            }
            return parseError(MISSING_NEWLINE);
         }
         else {
            if(i < 0) {
               errMsg = "AIG input literal ID(" + s + ")";
               return parseError(ILLEGAL_NUM);
            }
            if(VAR(unsigned(i)) > cirMgr->_maxIdx) {
               errInt = i;
               return parseError(MAX_LIT_ID);
            }
         }
         break;
      case SYMBOL_ID:
         c = s[0];
         s = s.substr(1);
         ++colNo;
         if(s == "") return parseError(EXTRA_SPACE);
         else if(!myStr2Int(s, i)) {
            for(idx = 0; idx < s.size(); ++idx) {
               if(s[idx] == 9 && idx == 0) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_WSPACE);
               }
               if(s[idx] == '\n' && idx == 0) {
                  errMsg = "symbol index";
                  return parseError(MISSING_NUM);
               }
               if(s[idx] == 9) {
                  return parseError(MISSING_SPACE);
               }
               if(s[idx] == '\n') {
                  errMsg = "symbolic name";
                  return parseError(MISSING_IDENTIFIER);
               }
               colNo += 1;
            }
            errMsg = "symbol index(" + s + ")";
            return parseError(ILLEGAL_NUM);
         }
         else {
            if(unsigned(i) >= cirMgr->_nPI) {
               errMsg = "PI index";
               errInt = i;
               return parseError(NUM_TOO_BIG);
            }
            if(c == 'i' && cirMgr->pi(i)->symbol() != "") {
               errMsg = "i";
               errInt = i;
               return parseError(REDEF_SYMBOLIC_NAME);
            }
            if(c == 'o' && cirMgr->po(i)->symbol() != "") {
               errMsg = "o";
               errInt = i;
               return parseError(REDEF_SYMBOLIC_NAME);
            }
         }
         colNo += s.size();
         break;
      case SYMBOL_NAME:
         if(s == "") {
            errMsg = "symbolic name";
            return parseError(MISSING_IDENTIFIER);
         }
         else {
            for(idx = 0; idx < s.size(); ++idx) {
               if((s[idx] <= 31 || s[idx] == 127)) {
                  errInt = s[idx];
                  return parseError(ILLEGAL_SYMBOL_NAME);
               } 
               colNo += 1;
            }
         }
         break;
      case COMMENT:
         if(s.size() == 1) return parseError(MISSING_NEWLINE);
         else if(s[1] != '\n') return parseError(MISSING_NEWLINE);
         break;
      default: break;
   }
   return true;
}

bool
CirMgr::readCircuit(const string& fileName)
{
   fin.clear();
   fin.close();
   fin.open(fileName, ios::in);
   if(!fin) {
      cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
      return false;
   }

   // Parse
   if (!parseAag(fin))    return false;
   if (!parsePi(fin))     return false;
   if (!parsePo(fin))     return false;
   if (!parseAig(fin))    return false;
   if (!parseSymbol(fin)) return false;

   parseComment(fin); // if any

   // Build Lists
   buildDfsList();
   buildFloatingList();
   buildUnusedList();
   buildUndefList();
   countAig();
   
   sortAllGateFanout();
   strash_check = false;

   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   static const int FRONTWIDTH = 7;
   static const int BACKWIDTH  = 9;
   cout << endl;
   cout << "Circuit Statistics\n"
        << "==================\n"
        << setw(FRONTWIDTH) << left    << "  PI"
        << setw(BACKWIDTH)  << right   << _nPI  << endl;
   cout << setw(FRONTWIDTH) << left    << "  PO"
        << setw(BACKWIDTH)  << right   << _nPO  << endl;
   cout << setw(FRONTWIDTH) << left    << "  AIG"
        << setw(BACKWIDTH)  << right   << _nAIG  << endl;
   cout << "------------------\n"
        << setw(FRONTWIDTH) << left    << "  Total"
        << setw(BACKWIDTH)  << right   << (_nPI + _nPO + _nAIG)
        << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for(unsigned i = 0, n = _vDfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _vDfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(unsigned i = 0; i < _nPI; ++i) {
      cout << " " << pi(i)->getVar();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(unsigned i = 0; i < _nPO; ++i) {
      cout << " " << po(i)->getVar();
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   // Print floating gates, if any
   if(!_vFloatingList.empty()) {
      cout << "Gates with floating fanin(s):";
      for(unsigned i = 0, n = _vFloatingList.size(); i < n; ++i)
         cout << " " << _vFloatingList[i]->getVar();
      cout << endl;
   }
   // Print unused gates, if any
   if(!_vUnusedList.empty()) {
      cout << "Gates defined but not used  :";
      for(unsigned i = 0, n = _vUnusedList.size(); i < n; ++i)
         cout << " " << _vUnusedList[i]->getVar();
      cout << endl;
   }
}

void
CirMgr::printFECPairs() const
{
   for (int i = 0, s = _fecGrpList.size(); i < s; ++i) {
      cout << "[" << i << "]";
      for (int j = 0, n = _fecGrpList[i]->size(); j < n; ++j) {
         if (_fecGrpList[i]->at(j)->getFecInv())
            cout << " !" << _fecGrpList[i]->at(j)->getVar();
         else
            cout << " " << _fecGrpList[i]->at(j)->getVar();
      }
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   unsigned i, n;
   // First line      
   outfile << "aag"   << " " << _maxIdx << " "  << _nPI   << " "
           << _nLATCH << " " << _nPO    << " "  << _nDfsAIG << endl;
   // PIs
   for(i = 0; i < _nPI; ++i) {
      outfile << LTI(pi(i)->getVar(), 0) << endl;
   }
   // POs
   for(i = 0; i < _nPO; ++i) {
      outfile << LTI(po(i)->fanin0_var(), po(i)->fanin0_inv()) << endl;
   }
   // AIGs
   for(i = 0, n = _vDfsList.size(); i < n; ++i) {
      if(_vDfsList[i]->isAig()) {
         outfile << LTI(_vDfsList[i]->getVar(), 0) << " "
                 << LTI(_vDfsList[i]->fanin0_var(), _vDfsList[i]->fanin0_inv()) << " "
                 << LTI(_vDfsList[i]->fanin1_var(), _vDfsList[i]->fanin1_inv()) << endl;
      }
   }
   // Symbols
   for(i = 0; i < _nPI; ++i) {
      if(pi(i)->symbol() != "")
         outfile << "i" << i  << " " << pi(i)->symbol() << endl;
   }
   for(i = 0; i < _nPO; ++i) {
      if(po(i)->symbol() != "")
         outfile << "o" << i  << " " << po(i)->symbol() << endl;
   }
   // Comments (optional)
   outfile << "c" << endl;
   outfile << "AAG output by Adam Lin" << endl;
   /*int size = comment.size();
   if (size != 0) {
      for (int i = 0; i < size; ++i) {
         outfile << comment[i] << endl;
      }
   }
   else {
      outfile << "c" << endl;
      outfile << "AAG output by Adam Lin" << endl;
   }*/
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) //const
{
   ++globalRef; wg_max = 0;
   vector<int> in; int sym = 0;
   wg_dfs(g);
   for(int i = 0, s = _nPI; i < s; ++i) {
      for (int j = 0, n = wg_list.size(); j < n; ++j){
         if (wg_list[j]->isAig()){
            if (pi(i)->getVar() == wg_list[j]->fanin0_var() || pi(i)->getVar() == wg_list[j]->fanin1_var()) {
               in.push_back(LTI(pi(i)->getVar(), 0));
               break;
            }
         }
      }
   }
   outfile << "aag " << wg_max << " " << in.size() << " " << 0 << " " << 1 << " " << wg_list.size() << endl;
   for (int i = 0, n = in.size(); i < n; ++i)
      outfile << in[i] << endl;
   outfile << LTI(g->getVar(), 0) << endl;
   for(int i = 0, n = wg_list.size(); i < n; ++i) {
      if(wg_list[i]->isAig()) {
         outfile << LTI(wg_list[i]->getVar(), 0) << " "
                 << LTI(wg_list[i]->fanin0_var(), wg_list[i]->fanin0_inv()) << " "
                 << LTI(wg_list[i]->fanin1_var(), wg_list[i]->fanin1_inv()) << endl;
      }
   }
   for (int i = 0, s = in.size(); i < s; ++i) {
      if (_vAllGates[VAR(in[i])]->symbol() != "") {
         outfile << "i" << sym << " " << _vAllGates[VAR(in[i])]->symbol() << endl;
         ++sym;
      }
   }
   outfile << "o0 Gate_" << g->getVar() << endl;
   outfile << "c" << endl;
   outfile << "Write gate (" << g->getVar() << ") by Adam Lin" << endl;
   wg_list.clear();
}

void
CirMgr::wg_dfs(CirGate *g)
{
   if (g == 0)                return;
   if (g->ref() == globalRef) return;
   if (g->isUndef())          return;
   g->setRef(globalRef);
   wg_dfs(g->fanin0_gate());
   wg_dfs(g->fanin1_gate());
   wg_max = (g->getVar() > wg_max) ? g->getVar() : wg_max;
   if (g->isAig())            wg_list.push_back(g);
}

/*********************************************************/
/*   class CirMgr member functions for circuit parsing   */
/*********************************************************/
bool
CirMgr::parseAag(ifstream& fin)
{
   lineNo = 0;
   colNo  = 0;
   string aagStr, tmp;
   // Read AAG
   getline(fin, aagStr, ' ');
   if(!errorHandle(AAG, aagStr)) return false;
   colNo += 4;
   // Read M
   getline(fin, tmp, ' ');
   if(!errorHandle(M, tmp)) return false;
   _maxIdx = stoul(tmp);
   colNo = colNo + tmp.size() + 1;
   // Read I
   getline(fin, tmp, ' ');
   if(!errorHandle(I, tmp)) return false;
   _nPI = stoul(tmp);
   colNo = colNo + tmp.size() + 1;
   // Read L
   getline(fin, tmp, ' ');
   if(!errorHandle(L, tmp)) return false;
   _nLATCH = stoul(tmp);
   colNo = colNo + tmp.size() + 1;
   // Read O
   getline(fin, tmp, ' ');
   if(!errorHandle(O, tmp)) return false;
   _nPO = stoul(tmp);
   colNo = colNo + tmp.size() + 1;
   // Read A
   getline(fin, tmp, '\n');
   if(!errorHandle(A, tmp)) return false;
   _nAIG = stoul(tmp);

   // Check latch number and M I L A relation
   if(!errorHandle(LIMIT)) return false;

   ++lineNo;
   preProcess();
   return true;
}

void
CirMgr::preProcess()
{
   // Resize _vAllGates
   _vAllGates.resize(1 + _maxIdx + _nPO, 0);
   // Create CONST gate
   CirConstGate* newGate = new CirConstGate;
   _vAllGates[0] = newGate;
}

bool
CirMgr::parsePi(ifstream& fin)
{
   CirPiGate* newPiGate = 0;
   string     tmp;
   unsigned   lit       = 0;
   
   for(unsigned i = 0; i < _nPI; ++i) {
      colNo = 0;                                   // Reset column number
      getline(fin, tmp, '\n');                     // Read in PI literal number
      if(tmp == "" && fin.eof()) { tmp = "DEF"; }
      else if(tmp == "")         { tmp = "LIT"; }
      if(!errorHandle(PI, tmp)) return false;
      lit = stoul(tmp);
      newPiGate = new CirPiGate(++lineNo, VAR(lit));
      _vAllGates[VAR(lit)] = newPiGate;
      _vPi.push_back(newPiGate);
   }
   return true;
}

bool
CirMgr::parsePo(ifstream& fin)
{
   CirPoGate* newPoGate = 0;
   CirGate*   fanin     = 0;
   string     tmp;
   unsigned lit = 0;
   for(unsigned i = 0; i < _nPO; ++i) {
      colNo = 0;                                   // Reset column number
      getline(fin, tmp, '\n');                     // Read in PO literal number
      if(tmp == "" && fin.eof()) { tmp = "DEF"; }
      else if(tmp == "")         { tmp = "LIT"; }
      if(!errorHandle(PO, tmp)) return false;
      lit = stoul(tmp);
      newPoGate = new CirPoGate(++lineNo, _maxIdx + 1 + i);
      fanin = queryGate(VAR(lit));
      newPoGate->setFanin0(fanin, INV(lit));
      fanin->addFanout(newPoGate, INV(lit));
      _vAllGates[_maxIdx + 1 + i] = newPoGate;
   }
   return true;
}

bool
CirMgr::parseAig(ifstream& fin)
{
   unsigned g_lit, f0_lit, f1_lit;
   CirGate* g, *f0, *f1;
   string tmp;
   for(unsigned i = 0; i < _nAIG; ++i) {
      // Reset column number
      colNo = 0;
      // Read in AIG'S G
      getline(fin, tmp, ' ');
      if(fin.eof() && (tmp == "" || tmp == "\n")) { tmp = "DEF"; }
      if(!errorHandle(AIG_G, tmp)) return false;
      g_lit = stoul(tmp);
      colNo = colNo + tmp.size() + 1;
      // Read in AIG'S F0
      getline(fin, tmp, ' ');
      if(!errorHandle(AIG_F0, tmp)) return false;
      f0_lit = stoul(tmp);
      colNo = colNo + tmp.size() + 1;
      // Read in AIG'S F1
      getline(fin, tmp, '\n');
      if(!errorHandle(AIG_F1, tmp)) return false;
      f1_lit = stoul(tmp);
      colNo = colNo + tmp.size() + 1;
      g  = queryGate(VAR(g_lit));
      f0 = queryGate(VAR(f0_lit));
      f1 = queryGate(VAR(f1_lit));
      g->setFanin0(f0, INV(f0_lit));
      g->setFanin1(f1, INV(f1_lit));
      f0->addFanout(g, INV(f0_lit));
      f1->addFanout(g, INV(f1_lit));
      g->setLineNo(++lineNo);
   }
   return true;
}

bool
CirMgr::parseSymbol(ifstream& fin)
{
   string str = "", temp;
   int    idx = 0;
   while(getline(fin, str, ' ')) {
      colNo = 0;
      if(str == "") return parseError(EXTRA_SPACE);
      else if(str[0] == 9) {
         errInt = str[0];
         return parseError(ILLEGAL_WSPACE);
      }
      else if(str[0] == 'c') {
         ++colNo;
         //if(fin.eof()) return parseError(MISSING_NEWLINE);
         if(!errorHandle(COMMENT, str)) return false;
         int s = str.size();
         temp = str[0];
         comment.push_back(temp);
         str = str.substr(2, s - 1);
         comment.push_back(str);
         break;
      }
      else if (str[0] == 'i') {
         if(!errorHandle(SYMBOL_ID, str)) return false;
         str = str.substr(1, str.size()-1);
         myStr2Int(str, idx);
         ++colNo;
         getline(fin, str, '\n');
         if(!errorHandle(SYMBOL_NAME, str)) return false;
         pi(idx)->setSymbol(str);
      }
      else if (str[0] == 'o') {
         if(!errorHandle(SYMBOL_ID, str)) return false;
         str = str.substr(1, str.size()-1);
         myStr2Int(str, idx);
         ++colNo;
         getline(fin, str, '\n');
         if(!errorHandle(SYMBOL_NAME, str)) return false;
         po(idx)->setSymbol(str);
      }
      else {
         errMsg = str[0];
         return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      ++lineNo;
   }
   
   return true;
}

bool
CirMgr::parseComment(ifstream& fin)
{
   string str; 
   if (getline(fin, str))
      comment.back() = comment.back() + " " + str; 
   while (getline(fin, str)) {
      comment.push_back(str);
   }
   return true;
}

CirGate*
CirMgr::queryGate(const unsigned gid) {
   assert(gid < _vAllGates.size());
   if(_vAllGates[gid] != 0) return _vAllGates[gid];

   // Create new aig gate
   CirAigGate* newGate = new CirAigGate(0, gid);
   _vAllGates[gid] = newGate;
   return newGate;
}

/**********************************************************/
/*   class CirMgr member functions for building lists     */
/**********************************************************/
void
CirMgr::rec_dfs(CirGate* g) 
{
   if (g == 0)                return;
   if (g->ref() == globalRef) return;
   if (g->isUndef())          return;
   g->setRef(globalRef);
   rec_dfs(g->fanin0_gate());
   rec_dfs(g->fanin1_gate());
   _vDfsList.push_back(g);
   if (g->isAig()) ++_nDfsAIG;
}

void
CirMgr::buildDfsList()
{
   _nDfsAIG = 0;
   ++globalRef;
   _vDfsList.clear();
   for(unsigned i = 0; i < _nPO; ++i)
      rec_dfs(po(i));
}

void
CirMgr::buildFloatingList()
{
   _vFloatingList.clear();
   for(unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
      if(_vAllGates[i])
         if(_vAllGates[i]->isFloating())
            _vFloatingList.push_back(_vAllGates[i]);
   }
}

void
CirMgr::buildUnusedList()
{
   _vUnusedList.clear();
   for(unsigned i = 0, n = _vAllGates.size(); i < n; ++i) {
      if(_vAllGates[i])
         if(_vAllGates[i]->nFanouts() == 0)
            if(_vAllGates[i]->isAig() || _vAllGates[i]->isPi())
               _vUnusedList.push_back(_vAllGates[i]);
   }
}

void
CirMgr::buildUndefList()
{
   _vUndefList.clear();
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->isUndef())
            _vUndefList.push_back(_vAllGates[i]);
}

void
CirMgr::countAig()
{
   _nAIG = 0;
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         if (_vAllGates[i]->isAig())
            ++_nAIG;
}

void 
CirMgr::sortAllGateFanout()
{
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         _vAllGates[i]->sortFanout();
}

/**********************************************************/
/*   class CirMgr member functions about freeing pointers */
/**********************************************************/
void
CirMgr::clear()
{
   // Delete gates
   for (unsigned i = 0, n = _vAllGates.size(); i < n; ++i)
      if (_vAllGates[i])
         delete _vAllGates[i];
   _vAllGates.clear();
   // Reset col & line number
   lineNo = 0;
   colNo  = 0;
   if(fin.is_open()) fin.close();
}