/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  if (strash_check)
    cerr << "Error: strash operation has already been performed!!" << endl;
  HashMap<HashKey, CirGate*> map(512); unsigned check;
  while (_nAIG != check) {
    check = _nAIG;
    for (int i = 0, s = _vAllGates.size(); i < s; ++i) {
      if (_vAllGates[i] != 0){
        if (_vAllGates[i]->isAig()) {
          HashKey temp(_vAllGates[i]); CirGate* data = _vAllGates[i];
          if (map.query(temp, data)) {
            cout << "Strashing: " << data->getVar() << " merging " << _vAllGates[i]->getVar() << "..." << endl;
            // fanout
            for (int j = 0, n = _vAllGates[i]->nFanouts(); j < n; ++j) {
              data->addFanout(_vAllGates[i]->fanout(j));
              if (_vAllGates[i]->fanout(j).gate()->fanin0_gate() == _vAllGates[i]) {
                CirGateV in(data, _vAllGates[i]->fanout_inv(j));
                _vAllGates[i]->fanout(j).gate()->setFanin0(in);
              }
              else {
                CirGateV in(data, _vAllGates[i]->fanout_inv(j));
                _vAllGates[i]->fanout(j).gate()->setFanin1(in);
              }
            }
            // fanin
            for (vector<CirGateV>::iterator k = _vAllGates[i]->fanin0_gate()->output().begin(); k != _vAllGates[i]->fanin0_gate()->output().end();) {
              if (k->gate() == _vAllGates[i]) {
                k = _vAllGates[i]->fanin0_gate()->output().erase(k);
                break;
              }
              else
                ++k;
            }
            for (vector<CirGateV>::iterator k = _vAllGates[i]->fanin1_gate()->output().begin(); k != _vAllGates[i]->fanin1_gate()->output().end();) {
              if (k->gate() == _vAllGates[i]) {
                k = _vAllGates[i]->fanin1_gate()->output().erase(k);
                break;
              }
              else
                ++k;
            }
            _vAllGates[i] = 0;
            _nAIG -= 1;
          }
          else {
            map.insert(temp, data);
          }
        }
      }
    }
    buildFloatingList();
    buildDfsList();
  }
  strash_check = true;
}

void
CirMgr::fraig()
{
  strash_check = false;
  SatSolver solver;
  solver.initialize();
  genProofModel(solver);
  size_t count = 0;
  for (int i = 0, n = _fecGrpList.size(); i < n; ++i) {
    vector<CirGate*>* fecGrp = _fecGrpList[i];
    for (int j = 0, s =fecGrp->size(); j < s; ++j) {
      for (int k = j+1; k < s; ++k) {
        Var newV = solver.newVar(); bool result;
        solver.addXorCNF(newV, fecGrp->at(j)->getSatVar(), fecGrp->at(j)->getFecInv(), fecGrp->at(k)->getSatVar(), fecGrp->at(k)->getFecInv());
        solver.assumeRelease();
        solver.assumeProperty(newV, true);
        result = solver.assumpSolve();
        if (!result) {  // equivalent
          // merge
          // fanout
          string str1 = (fecGrp->at(j)->getFecInv()) ? "!" + to_string(fecGrp->at(j)->getSatVar()) : to_string(fecGrp->at(j)->getSatVar());
          string str2 = (fecGrp->at(k)->getFecInv()) ? "!" + to_string(fecGrp->at(k)->getSatVar()) : to_string(fecGrp->at(k)->getSatVar());
          cout << "Proving (" << str1 << ", " << str2 << ")...UNSAT!!\r";
          vector<CirGateV> out = fecGrp->at(k)->output(); CirGate* g = fecGrp->at(k);
          for (int u = 0, v = out.size(); u < v; ++u) {
            if (out[u].gate()->fanin0_gate() == g) {
              if (!out[u].isInv()){
                if (fecGrp->at(j)->getFecInv() == g->getFecInv()){ 
                  CirGateV temp(fecGrp->at(j), 0);
                  out[u].gate()->setFanin0(temp);
                }
                else{
                  CirGateV temp(fecGrp->at(j), 1);
                  out[u].gate()->setFanin0(temp);
                }
              }
              else {
                if (fecGrp->at(j)->getFecInv() == g->getFecInv()) {
                  CirGateV temp(fecGrp->at(j), 1);
                  out[u].gate()->setFanin0(temp);
                }
                else{
                  CirGateV temp(fecGrp->at(j), 0);
                  out[j].gate()->setFanin0(temp);
                }
              }
              
            }
            else {
              if (!out[u].isInv()){
                if (fecGrp->at(j)->getFecInv() == g->getFecInv()) {
                  CirGateV temp(fecGrp->at(j), 0);
                  out[u].gate()->setFanin0(temp);
                }
                else {
                  CirGateV temp(fecGrp->at(j), 1);
                  out[u].gate()->setFanin1(temp);
                }
              }
              else {
                if (fecGrp->at(j)->getFecInv() == g->getFecInv()) {
                  CirGateV temp(fecGrp->at(j), 1);
                  out[u].gate()->setFanin0(temp);
                }
                else {
                  CirGateV temp(fecGrp->at(j), 0);
                  out[j].gate()->setFanin1(temp);
                }
              }
            }
            if (fecGrp->at(j)->getFecInv() == g->getFecInv()){
              fecGrp->at(j)->addFanout(out[u]);
            }
            else {
              CirGateV a(out[u].gate(), !out[u].isInv());
              fecGrp->at(j)->addFanout(a);
            }
          }
          // fanin
          for (vector<CirGateV>::iterator a = g->fanin0_gate()->output().begin(); a != g->fanin0_gate()->output().end();) {
            if (a->gate() == g){
              a = g->fanin0_gate()->output().erase(a);
              break;
            }
            else
              ++a;
          }
          for (vector<CirGateV>::iterator a = g->fanin1_gate()->output().begin(); a != g->fanin1_gate()->output().end();) {
            if (a->gate() == g){
              a = g->fanin1_gate()->output().erase(a);
              break;
            }
            else
              ++a;
          }
          if (fecGrp->at(j)->getFecInv() == g->getFecInv())
            cout << "Fraig: " << fecGrp->at(j)->getVar() << " merging " << fecGrp->at(k)->getVar() << "..." << endl;
          else
            cout << "Fraig: " << fecGrp->at(j)->getVar() << " merging !" << fecGrp->at(k)->getVar() << "..." << endl;
          _vAllGates[g->getVar()] = 0;
          _nAIG -= 1;
          fecGrp->erase(fecGrp->begin()+k);
          --s; --k;
          cout << "Updating by UNSAT... Total #FEC Group = " << _fecGrpList.size() << endl;
        }
        else {
          cout << "Proving " << newV << " = " << "1...SAT!!\r";
          ++count;
          if (count == 64) {
            count = 0;
          }
        }
      }
    }
  }
  _fecGrpList.clear();
  for (size_t i = 0; i < _vDfsList.size(); ++i) {
    if (_vDfsList[i]->getFec())  _vDfsList[i]->setFec(0);
  }
  buildDfsList();
  optimize();
  strash();
  buildDfsList();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::genProofModel(SatSolver& s)
{
  Var v = s.newVar();
  _vAllGates[0]->setSatVar(v);
  for (size_t i = 0, n = _vDfsList.size(); i < n; ++i) {
      if (!_vDfsList[i]->isPo()) {
        v = s.newVar();
        _vDfsList[i]->setSatVar(v);
        if (_vDfsList[i]->isAig()) {
          CirGate* g = _vDfsList[i];
          s.addAigCNF( g->getSatVar(), g->fanin0_gate()->getSatVar(), g->fanin0_inv(), g->fanin1_gate()->getSatVar(), g->fanin1_inv());
        }
      }
      else {
        v = _vDfsList[i]->fanin0_gate()->getSatVar();
        _vDfsList[i]->setSatVar(v);
      }
   }
}
