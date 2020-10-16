/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <math.h>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  // initialize
  if ( _fecGrpList.size() == 0 ) {
    vector<CirGate*>* fecGrp = new vector<CirGate*>;
    fecGrp->push_back(_vAllGates[0]);
    for (int i = 0, s = _vAllGates.size() ; i < s ; ++i) {
      if (_vAllGates[i]->isAig()) 
        fecGrp->push_back(_vAllGates[i]);
    }
    _fecGrpList.push_back(fecGrp);
  }
  // random value
  int max = (_vDfsList.size() < 100) ? _vDfsList.size() : sqrt(_vDfsList.size()) * 5;
  for (int i = 0; i < max; ++i) {
    for (int j = 0, n = _vPi.size(); j < n; ++j) {
      size_t temp = (size_t)(rnGen(INT_MAX) << 32) | (size_t)rnGen(INT_MAX);
      _vPi[j]->setPattern(temp);
    }
    for (int j = 0, n = _vDfsList.size(); j < n; ++j) {
      if (_vDfsList[j]->isAig()) {
        size_t in0 = (_vDfsList[j]->fanin0_inv()) ? ~_vDfsList[j]->fanin0_gate()->getPattern() : _vDfsList[j]->fanin0_gate()->getPattern();
        size_t in1 = (_vDfsList[j]->fanin1_inv()) ? ~_vDfsList[j]->fanin1_gate()->getPattern() : _vDfsList[j]->fanin1_gate()->getPattern();
        size_t value = in0 & in1;
        _vDfsList[j]->setPattern(value);
      }
      else if (_vDfsList[j]->isPo()) {
        size_t value = (_vDfsList[j]->fanin0_inv()) ? ~_vDfsList[j]->fanin0_gate()->getPattern() : _vDfsList[j]->fanin0_gate()->getPattern();
        _vDfsList[j]->setPattern(value);
      }
    }
    // FEC
    size_t c = _fecGrpList.size();
    for (int j = 0, n = _fecGrpList.size(); j < n; ++j) {
      if (!_fecGrpList[j]->empty()) {
        HashMap< SimKey, vector<CirGate*>* > newfeclist(_fecGrpList[j]->size());
        for (int k = 0, l = _fecGrpList[j]->size(); k < l; ++k) {
          SimKey newkey(_fecGrpList[j]->at(k)->getPattern()); SimKey i_newkey(~_fecGrpList[j]->at(k)->getPattern());
          vector<CirGate*>* temp = 0;
          if (newfeclist.query(newkey, temp)) {
            _fecGrpList[j]->at(k)->setFecInv(false);
            _fecGrpList[j]->at(k)->setFec(temp);
            temp->push_back(_fecGrpList[j]->at(k));
          }
          else if (newfeclist.query(i_newkey, temp)) {
            _fecGrpList[j]->at(k)->setFecInv(true);
            _fecGrpList[j]->at(k)->setFec(temp);
            temp->push_back(_fecGrpList[j]->at(k));
          }
          else {
            temp = new vector<CirGate*>;
            _fecGrpList[j]->at(k)->setFecInv(false);
            _fecGrpList[j]->at(k)->setFec(temp);
            temp->push_back(_fecGrpList[j]->at(k));
            _fecGrpList.push_back(temp);
            newfeclist.insert(newkey, temp);
          }
        }
      }
    }
    vector<vector<CirGate*>*> x;
    for ( size_t k = c, l = _fecGrpList.size() ; k < l ; ++k ) {
      if ( _fecGrpList[k]->size() > 1 )  
        x.push_back(_fecGrpList[k]);
    }
    _fecGrpList.clear();
    _fecGrpList = x;

    size_t w = 0;
    for (int j = 0, n = _fecGrpList.size(); j < n; ++j) {
      if (_fecGrpList[j]->size() > 1)
        w += 1;
    }
    cout << "Total #FEC Group = " << w << "\r" ;
  }
  cout << max*64 << " patterns simulated." << endl;
  
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  if ( _fecGrpList.empty() ) {
    vector<CirGate*>* fecGrp = new vector<CirGate*>;
    fecGrp->push_back(_vAllGates[0]);
    for (int i = 0, s = _vAllGates.size() ; i < s ; ++i) {
      if (_vAllGates[i]->isAig()) 
        fecGrp->push_back(_vAllGates[i]);
    }
    _fecGrpList.push_back(fecGrp);
  }
  size_t p = 0;
  string temp; vector<string> input;
  while (patternFile >> temp) {
    if (temp.size() != _nPI)
      cerr << "Error: Pattern(" << temp << ") length(" << temp.size() << ") does not match the number of inputs(" << _nPI << ") in a circuit!!" << endl;
    input.resize(_nPI);
    for (int i = 0, s = temp.size(); i < s; ++i) {
      if (temp[i] != '0' && temp[i] != '1')
        cerr << "Error: Pattern(" << temp << ") contains a non-0/1 character(‘" << temp[i] << "’)." << endl;
      string t = temp.substr(i,1);
      input[i] += t;
    }
  }

  for (int u = 0, v = input[0].size(); u < v; u += 64)
  {
    vector<string> in; in.resize(_nPI);
    for (int i = 0, s = _nPI; i < s; ++i)
    {
      in[i] = (input[i].size() - u < 64) ? input[i].substr(u, input[i].size() - u) : input[i].substr(u, 64);
      size_t x = 0;
      for (int j = 0, n = in[i].size(); j < n; ++j)
      {
        x += (size_t) stoi(in[i].substr(j,1)) * (size_t) pow(2, j);
      }
      _vPi[i]->setPattern(x);
    }

    for (int j = 0, n = _vDfsList.size(); j < n; ++j)
    {
      if (_vDfsList[j]->isAig())
      {
        size_t in0 = (_vDfsList[j]->fanin0_inv()) ? ~_vDfsList[j]->fanin0_gate()->getPattern() : _vDfsList[j]->fanin0_gate()->getPattern();
        size_t in1 = (_vDfsList[j]->fanin1_inv()) ? ~_vDfsList[j]->fanin1_gate()->getPattern() : _vDfsList[j]->fanin1_gate()->getPattern();
        size_t value = in0 & in1;
        _vDfsList[j]->setPattern(value);
      }
      else if (_vDfsList[j]->isPo())
      {
        size_t value = (_vDfsList[j]->fanin0_inv()) ? ~_vDfsList[j]->fanin0_gate()->getPattern() : _vDfsList[j]->fanin0_gate()->getPattern();
        _vDfsList[j]->setPattern(value);
      }
    }

    if (_simLog)
    {
      size_t g = 0x1;
      for (int i = 0, m = in[0].size(); i < m; ++i)
      {
        for (int j = 0; j < _nPI; ++j)
        {
          *_simLog << in[j][i];
        }
        *_simLog << " ";
        for (int j = 0, n = _vAllGates.size(); j < n; ++j)
        {
          if (_vAllGates[j]->isPo())
          {
            if (g & _vAllGates[j]->getPattern())  *_simLog << "1";
            else                                  *_simLog << "0";
          }
        }
        g *= 2;
        *_simLog << endl;
      }
    }

    /////
    size_t c = _fecGrpList.size();
    for (int j = 0, n = _fecGrpList.size(); j < n; ++j)
    {
      if (_fecGrpList[j]->size() > 1)
      {
        HashMap<SimKey, vector<CirGate *> *> newfeclist(_fecGrpList[j]->size());
        for (int k = 0, l = _fecGrpList[j]->size(); k < l; ++k)
        {
          if (_fecGrpList[j]->at(k)->isAig() || _fecGrpList[j]->at(k)->isConst())
          {
            SimKey newkey(_fecGrpList[j]->at(k)->getPattern());
            SimKey i_newkey(~_fecGrpList[j]->at(k)->getPattern());
            vector<CirGate *> *temp = 0;
            if (newfeclist.query(newkey, temp))
            {
              _fecGrpList[j]->at(k)->setFec(temp);
              _fecGrpList[j]->at(k)->setFecInv(false);
              temp->push_back(_fecGrpList[j]->at(k));
            }
            else if (newfeclist.query(i_newkey, temp))
            {
              _fecGrpList[j]->at(k)->setFecInv(true);
              _fecGrpList[j]->at(k)->setFec(temp);
              temp->push_back(_fecGrpList[j]->at(k));
            }
            else
            {
              _fecGrpList[j]->at(k)->setFecInv(false);
              temp = new vector<CirGate *>;
              _fecGrpList[j]->at(k)->setFec(temp);
              temp->push_back(_fecGrpList[j]->at(k));
              _fecGrpList.push_back(temp);
              newfeclist.insert(newkey, temp);
            }
          }
        }
      }
    }
    vector<vector<CirGate *> *> x;
    for (size_t k = c, l = _fecGrpList.size(); k < l; ++k)
    {
      if (_fecGrpList[k]->size() > 1)
        x.push_back(_fecGrpList[k]);
    }
    _fecGrpList.clear();
    _fecGrpList = x;

    p += in[0].size();

    size_t size = 0;
    for (int j = 0, n = _fecGrpList.size(); j < n; ++j)
    {
      if (_fecGrpList[j]->size() > 1)
        size += 1;
    }
    cout << "Total #FEC Group = " << size << "\r";
  }

  sort(_fecGrpList.begin(), _fecGrpList.end(), 
        [] (const vector<CirGate*>* g1, const vector<CirGate*>* g2) {
           return g1->at(0)->getVar() < g2->at(0)->getVar();
        });
  cout << p << " patterns simulated." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

