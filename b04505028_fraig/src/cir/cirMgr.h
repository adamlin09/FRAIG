/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;
enum CirParseType {
   AAG,
   M,
   I,
   L,
   O,
   A,
   LIMIT,
   PI,
   PO,
   AIG_G,
   AIG_F0,
   AIG_F1,
   SYMBOL_ID,
   SYMBOL_NAME,
   COMMENT
};

#define VAR(x) (x / 2)
#define INV(x) (x % 2)
#define LTI(x, y) ((x * 2) + y)

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      if(gid < _vAllGates.size()) {
         /*if (_vAllGates[gid] != 0) {
            if(_vAllGates[gid]->isUndef())
               return 0;
         }*/
         return _vAllGates[gid];
      }
      return 0;
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void sweepGate(CirGate* g);
   void opt(CirGate* g);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*);

   // Friend function for error handle
   friend bool errorHandle(CirParseType, string);

private:
   ofstream           *_simLog;
   vector<string>     comment;
   bool               strash_check = false;

   // Basic Info (M I L O A)
   unsigned _maxIdx;    // M (Max gate index)
   unsigned _nPI;       // I (Number of PIs)
   unsigned _nLATCH;    // L (Number of LATCHes)
   unsigned _nPO;       // O (Number of POs)
   unsigned _nAIG;      // A (Number of AIGs)
   unsigned _nDfsAIG;   // number of Aig in dfs list
   unsigned wg_max;

   // Gate Lists
   vector<CirGate*> _vPi;
   vector<CirGate*> _vAllGates;
   vector<CirGate*> _vDfsList;
   vector<CirGate*> _vFloatingList;
   vector<CirGate*> _vUnusedList;
   vector<CirGate*> _vUndefList;
   vector<CirGate*> wg_list;
   vector<CirGate*> sweep_removed;
   vector<vector<CirGate*>*> _fecGrpList;

   // Private access functions
   unsigned   nPi()                const { return _nPI; }
   unsigned   nPo()                const { return _nPO; }
   CirPiGate* pi(const int i)      const { assert(0 <= i && i < (int)_nPI); return (CirPiGate*)_vPi[i];                     }
   CirPoGate* po(const int i)      const { assert(0 <= i && i < (int)_nPO); return (CirPoGate*)_vAllGates[_maxIdx + i + 1]; }
   CirPiGate* pi(const unsigned i) const { assert(i < _nPI); return (CirPiGate*)_vPi[i];                     }
   CirPoGate* po(const unsigned i) const { assert(i < _nPO); return (CirPoGate*)_vAllGates[_maxIdx + i + 1]; }
   CirGate*   constGate()          const { return _vAllGates[0]; } 

   // Private function about parsing AAG file
   bool parseAag(ifstream&);
   bool parsePi(ifstream&);
   bool parsePo(ifstream&);
   bool parseAig(ifstream&);
   bool parseSymbol(ifstream&);
   bool parseComment(ifstream&);
   void preProcess();
   CirGate* queryGate(const unsigned);

   // Private functions about gate lists
   void buildDfsList();
   void buildFloatingList();
   void buildUnusedList();
   void buildUndefList();
   void countAig();
   void rec_dfs(CirGate* g);
   void wg_dfs(CirGate *g);

   // Private common functions
   void clear();
   void sortAllGateFanout();

   //SAT
   void genProofModel(SatSolver& s);

};

class HashKey
{
public:
   HashKey(CirGate* g) {
     assert(g->isAig());
     a = g->fanin0(); b = g->fanin1();
   }
   HashKey() { a = 0; b = 0;}
   ~HashKey() {}

   size_t operator() () const { 
     size_t i1 = LTI(a.gate()->getVar(), a.isInv());
     size_t i2 = LTI(b.gate()->getVar(), b.isInv());
     size_t k = i1 ^ i2;
     return k;
   }

   bool operator == (const HashKey& k) const { return (a == k.a && b == k.b) || (a == k.b && b == k.a); }

private:
  CirGateV a;
  CirGateV b;
};

class SimKey
{
  public:
    SimKey() { _key = 0; }
    SimKey(const size_t &i) { _key = i; }
    SimKey(const SimKey& k) { _key = k._key; }
    size_t operator () () const { return _key; }
    bool operator == (const SimKey& k) const { return _key == k._key; }
  private:
    size_t _key;
};

#endif // CIR_MGR_H
