/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "algorithm"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  for (int i = 0, n = _vUnusedList.size(); i < n; ++i) {
    sweepGate(_vUnusedList[i]);
  }
  buildFloatingList();
  buildUnusedList();
  buildUndefList();
  sort(sweep_removed.begin(), sweep_removed.end(), 
        [] (const CirGate* g1, const CirGate* g2) {
           return g1->getVar() < g2->getVar();
        });
  for (int i = 0, n = sweep_removed.size(); i < n; ++i) {
    cout << "Sweeping: " << sweep_removed[i]->getTypeStr() << "(" << sweep_removed[i]->getVar() << ") removed..." << endl;
  }
}

void 
CirMgr::sweepGate(CirGate* g)
{
  if (!g->isPi() && !g->isConst()) {
    if (g->nFanouts() == 0) {
      unsigned thisGate = g->getVar();
      _vAllGates[thisGate] = 0; 
      if (g->isAig())
      {
        CirGate* in0 = g->fanin0_gate();
        for (vector<CirGateV>::iterator i = in0->output().begin(); i != in0->output().end();) {
          if (i->gate() == g) {
            i = in0->output().erase(i);
            break;
          }
          else
            ++i;
        }
        CirGate *in1 = g->fanin1_gate();
        for (vector<CirGateV>::iterator i = in1->output().begin(); i != in1->output().end();) {
          if (i->gate() == g) {
            i = in1->output().erase(i);
            break;
          }
          else
            ++i;
        }
        _nAIG -= 1;
        sweepGate(g->fanin0_gate());
        sweepGate(g->fanin1_gate());
      }     
      sweep_removed.push_back(g);
    }
  }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  unsigned check = 0;
  while (_nDfsAIG != check){
    check = _nDfsAIG;
    for (int i = 0, s = _vDfsList.size(); i < s; ++i) {
      opt(_vDfsList[i]);
      }
      buildDfsList();
      buildUndefList();
      buildUnusedList();
  }
  strash_check = false;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::opt(CirGate* g)
{
  if (g->isAig()) {
    unsigned gid = g->getVar();
    vector<CirGateV> out = g->output();
    if (g->fanin0_var() == g->fanin1_var() && g->fanin0_inv() == g->fanin1_inv()) {     // in1 = in2
      unsigned lit0 = g->fanin0_var();
      unsigned lit1 = g->getVar();
      string var0 = to_string(lit0);
      string var1 = (g->fanin0_inv()) ? "!" + to_string(lit1) : to_string(lit1);
      cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
      // fanout
      for (int i = 0, s = out.size(); i < s; ++i) {
        if (out[i].gate()->fanin0_gate() == g){
          if (!out[i].isInv()) {
            CirGateV temp = g->fanin0();
            out[i].gate()->setFanin0(temp);
          }
          else {
            CirGateV temp(g->fanin0_gate(), !g->fanin0_inv());
            out[i].gate()->setFanin0(temp);
          }
        }
        else {
          if (!out[i].isInv()) {
            CirGateV temp = g->fanin0();
            out[i].gate()->setFanin1(temp);
          }
          else {
            CirGateV temp(g->fanin0_gate(), !g->fanin0_inv());
            out[i].gate()->setFanin1(temp);
          }
        }
      }
      // fanin
      for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
        if (i->gate() == g) {
          i = g->fanin1_gate()->output().erase(i);
          break;
        }
        else
          ++i;
      }
      for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
        if (i->gate() == g) {
          i = g->fanin0_gate()->output().erase(i);
          for (int j = 0, n = g->nFanouts(); j < n; ++j) {
            if (!i->isInv()) {
              CirGateV temp = out[j];
              g->fanin1_gate()->addFanout(temp);
            }
            else {
              CirGateV temp(out[j].gate(), !out[j].isInv());
              g->fanin1_gate()->addFanout(temp);
            }
          }
          break;
        }
        else
          ++i;
      }
      _vAllGates[gid] = 0;
      _nAIG -= 1;
    }
    else if (g->fanin0_var() == g->fanin1_var() && g->fanin0_inv() != g->fanin1_inv()) {   // in1 = !in2
      unsigned lit0 = 0;
      unsigned lit1 = g->getVar();
      string var0 = to_string(lit0);
      string var1 = to_string(lit1);
      cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
      // fanout
      for (int i = 0, s = out.size(); i < s; ++i) {
        if (out[i].gate()->fanin0_gate() == g){
          if (!out[i].isInv()) {
            CirGateV temp(_vAllGates[0], 0);
            out[i].gate()->setFanin0(temp);
          }
          else {
            CirGateV temp(_vAllGates[0], 1);
            out[i].gate()->setFanin0(temp);
          }
        }
        else {
          if (!out[i].isInv()) {
            CirGateV temp(_vAllGates[0], 0);
            out[i].gate()->setFanin1(temp);
          }
          else {
            CirGateV temp(_vAllGates[0], 1);
            out[i].gate()->setFanin1(temp);
          }
        }
      }
      // fanin
      for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
        if (i->gate() == g) {
          i = g->fanin0_gate()->output().erase(i);
          break;
        }
        else
          ++i;
      }
      for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
        if (i->gate() == g) {
          i = g->fanin1_gate()->output().erase(i);
          break;
        }
        else
          ++i;
      }
      for (int i = 0, s = g->nFanouts(); i < s; ++i) {
        _vAllGates[0]->addFanout(out[i]);
      }
      _vAllGates[gid] = 0;
      _nAIG -= 1;
    }
    else if (g->fanin0_var() == 0 || g->fanin1_var() == 0) {     // one of the inputs is 1 or 0
      if (g->fanin0_var() == 0) {    // fanin0
        if (g->fanin0_inv()) {      // fanin0 = const 1
          unsigned lit0 = g->fanin1_var();
          unsigned lit1 = g->getVar();
          string var0 = to_string(lit0);
          string var1 = (g->fanin0_inv()) ? "!" + to_string(lit1) : to_string(lit1);
          cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
          // fanouts of the gate      
          for (int j = 0, n = out.size(); j < n; ++j) {
            if (out[j].gate()->fanin0_gate() == g) {
              if (!out[j].isInv()){ 
                CirGateV temp = g->fanin1();
                out[j].gate()->setFanin0(temp);
              }
              else {
                CirGateV temp(g->fanin1_gate(), !g->fanin1_inv());
                out[j].gate()->setFanin0(temp);
              }
            }
            else {
              if (!out[j].isInv()){ 
                CirGateV temp = g->fanin1();
                out[j].gate()->setFanin1(temp);
                }
              else {
                CirGateV temp(g->fanin1_gate(), !g->fanin1_inv());
                out[j].gate()->setFanin1(temp);
              } 
            }
          }
          // fanin
          for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin0_gate()->output().erase(i);
              break;
            }
            else
              ++i;
          }
          for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin1_gate()->output().erase(i);
              for (int j = 0, n = g->nFanouts(); j < n; ++j) {
                if (!i->isInv()) {
                  CirGateV temp  = out[j];
                  g->fanin1_gate()->addFanout(temp);
                }
                else {
                  CirGateV temp(out[j].gate(), !out[j].isInv());
                  g->fanin1_gate()->addFanout(temp);
                }
              }
              break;
            }
            else
              ++i;
          }
        }
        else {      // fanin0 = const 0
          unsigned lit0 = 0;
          unsigned lit1 = g->getVar();
          string var0 = to_string(lit0);
          string var1 = to_string(lit1);
          cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
          // fanout
          vector<CirGateV> out = g->output();      
          for (int j = 0, n = out.size(); j < n; ++j) {
            if (out[j].gate()->fanin0_gate() == g) {
              if (!out[j].isInv()){ 
                CirGateV temp(_vAllGates[0], 0);
                out[j].gate()->setFanin0(temp);
              }
              else {
                CirGateV temp(_vAllGates[0], 1);
                out[j].gate()->setFanin0(temp);
              }
            }
            else {
              if (!out[j].isInv()){ 
                CirGateV temp(_vAllGates[0], 0);
                out[j].gate()->setFanin1(temp);
              }
              else {
                CirGateV temp(_vAllGates[0], 1);
                out[j].gate()->setFanin1(temp);
              } 
            }
          }
          // fanin
          for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin1_gate()->output().erase(i);
              break;
            }
            else
              ++i;
          }
          for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin0_gate()->output().erase(i);
              for (int j = 0, n = g->nFanouts(); j < n; ++j) {
                CirGateV temp  = out[j];
                g->fanin0_gate()->addFanout(temp);
              }
              break;
            }
            else
              ++i;
          }
        }
      }
      else {                         // fanin1
        if (g->fanin1_inv()) {      // fanin1 = const 1
          unsigned lit0 = g->fanin0_var();
          unsigned lit1 = g->getVar();
          string var0 = to_string(lit0);
          string var1 = (g->fanin0_inv()) ? "!" + to_string(lit1) : to_string(lit1);
          cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
          // fanouts of the gate
          vector<CirGateV> out = g->output();      
          for (int j = 0, n = out.size(); j < n; ++j) {
            if (out[j].gate()->fanin0_gate() == g) {
              if (!out[j].isInv()){ 
                CirGateV temp = g->fanin0();
                out[j].gate()->setFanin0(temp);
              }
              else {
                CirGateV temp(g->fanin0_gate(), !g->fanin0_inv());
                out[j].gate()->setFanin0(temp);
              }
            }
            else {
              if (!out[j].isInv()){ 
                CirGateV temp = g->fanin0();
                out[j].gate()->setFanin1(temp);
                }
              else {
                CirGateV temp(g->fanin0_gate(), !g->fanin0_inv());
                out[j].gate()->setFanin1(temp);
              } 
            }
          }
          // fanin
          for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin1_gate()->output().erase(i);
              break;
            }
            else
              ++i;
          }
          for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin0_gate()->output().erase(i);
              for (int j = 0, n = g->nFanouts(); j < n; ++j) {
                if (!i->isInv()) {
                  CirGateV temp  = out[j];
                  g->fanin0_gate()->addFanout(temp);
                }
                else {
                  CirGateV temp(out[j].gate(), !out[j].isInv());
                  g->fanin0_gate()->addFanout(temp);
                }
              }
              break;
            }
            else
              ++i;
          }
        }
        else {      // fanin1 = const 0
          unsigned lit0 = 0;
          unsigned lit1 = g->getVar();
          string var0 = to_string(lit0);
          string var1 = to_string(lit1);
          cout << "Simplifying: " << var0 << " merging " << var1 << "..." << endl;
          // fanout
          vector<CirGateV> out = g->output();      
          for (int j = 0, n = out.size(); j < n; ++j) {
            if (out[j].gate()->fanin0_gate() == g) {
              if (!out[j].isInv()){ 
                CirGateV temp(_vAllGates[0], 0);
                out[j].gate()->setFanin0(temp);
              }
              else {
                CirGateV temp(_vAllGates[0], 1);
                out[j].gate()->setFanin0(temp);
              }
            }
            else {
              if (!out[j].isInv()){ 
                CirGateV temp(_vAllGates[0], 0);
                out[j].gate()->setFanin1(temp);
              }
              else {
                CirGateV temp(_vAllGates[0], 1);
                out[j].gate()->setFanin1(temp);
              } 
            }
          }
          // fanin
          for (vector<CirGateV>::iterator i = g->fanin0_gate()->output().begin(); i != g->fanin0_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin0_gate()->output().erase(i);
              break;
            }
            else
              ++i;
          }
          for (vector<CirGateV>::iterator i = g->fanin1_gate()->output().begin(); i != g->fanin1_gate()->output().end();) {
            if (i->gate() == g){
              i = g->fanin1_gate()->output().erase(i);
              for (int j = 0, n = g->nFanouts(); j < n; ++j) {
                CirGateV temp  = out[j];
                g->fanin1_gate()->addFanout(temp);
              }
              break;
            }
            else
              ++i;
          }
        }
      }
      _vAllGates[gid] = 0;
      _nAIG -= 1;
    }
  }
}