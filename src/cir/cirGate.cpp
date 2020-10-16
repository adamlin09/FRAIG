/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
extern unsigned globalRef;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   // Information string
   string infoStr = getTypeStr() + "(" + to_string(_var) + ")";
   if (isPi()) {
      if (((CirPiGate*)this)->symbol() != "")
         infoStr += "\"" + ((CirPiGate*)this)->symbol() + "\"";
   }
   if (isPo()) {
      if (((CirPoGate*)this)->symbol() != "")
         infoStr += "\"" + ((CirPoGate*)this)->symbol() + "\"";
   }
   infoStr += ", line " + to_string(_lineNo);
            
   cout << "================================================================================\n";
   cout << "= " << setw(78) << left << infoStr << endl;
   cout << "= FECs:";
   if (getFec() != 0) {   
      for (int i = 0, s = getFec()->size(); i < s; ++i) {
         if (getVar() != getFec()->at(i)->getVar()) {
            if (getFec()->at(i)->getFecInv())
               cout << " !" << getFec()->at(i)->getVar();
            else
               cout << " " << getFec()->at(i)->getVar();
         }
      }
   }
   cout << endl;
   cout << "= value: ";
   size_t s = 0x8000000000000000;
   for ( int i = 0 ; i < 64 ; ++i ) {
      if (i % 8 == 0 && i != 0)
         cout << "_";
      if ( s & getPattern() ) cout << "1";
      else                    cout << "0";
      s = s >> 1;
   }
   cout << endl;
   cout << "================================================================================\n";
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   ++globalRef;
   rec_rptFanin(this, 0, level, 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   ++globalRef;
   rec_rptFanout(this, 0, level, 0);
}

void
CirGate::rec_rptFanin(const CirGate* g, bool inv, int level, int nSpace) const
{
   if(g == 0) return;
   if(level < 0) return;
   for(int i = 0; i < nSpace; ++i) cout << ' ';
   cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->getVar();
   if(g->ref() == globalRef && level > 0) {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if (level > 0 && g->isAig()) g->setRef(globalRef);
   rec_rptFanin(g->fanin0_gate(), g->fanin0_inv(), level - 1, nSpace + 2);
   rec_rptFanin(g->fanin1_gate(), g->fanin1_inv(), level - 1, nSpace + 2);
}

void
CirGate::rec_rptFanout(const CirGate* g, bool inv, int level, int nSpace) const
{
   if(g == 0) return;
   if(level < 0) return;
   for(int i = 0; i < nSpace; ++i) cout << ' ';
   cout << (inv ? "!" : "") << g->getTypeStr() << " " << g->getVar();
   if (g->ref() == globalRef && level > 0) {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if (level > 0 && g->isAig()) g->setRef(globalRef);
   for(unsigned j = 0, n = g->nFanouts(); j < n; ++j) {
      rec_rptFanout(g->fanout_gate(j), g->fanout_inv(j), level - 1, nSpace + 2);
   }
}
/**************************************/
/*   class CirGate sorting functions  */
/**************************************/
void 
CirGate::sortFanout() 
{ 
   sort(_fanouts.begin(), _fanouts.end(), 
        [] (const CirGateV& g1, const CirGateV& g2) {
           return g1.gate()->getVar() < g2.gate()->getVar();
        });
}
