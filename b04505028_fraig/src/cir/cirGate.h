/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGateV;
class CirGate;
class CirPiGate;
class CirPoGate;
class CirAigGate;
class CirConstGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateV
{
   #define NEG 0x1
public:
   CirGateV(CirGate* g = 0, size_t phase = 0): _gateV(size_t(g) + phase) {}
   ~CirGateV() {}

   CirGate* gate()   const { return (CirGate*)(_gateV & ~size_t(NEG)); }
   bool     isInv()  const { return (_gateV & NEG);                    }
   bool     null()   const { return _gateV == 0;                       }
   bool operator == (const CirGateV& k) const { return (gate() == k.gate() && isInv() == k.isInv()); }

private:
   size_t _gateV;
};

class CirGate
{
public:
   CirGate(unsigned l = 0, unsigned v = 0): _lineNo(l), _var(v), _ref(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   virtual string symbol()     const = 0;
   
   unsigned getLineNo()    const { return _lineNo;                   }
   unsigned getVar()       const { return _var;                      }
   unsigned ref()          const { return _ref;                      }
   size_t   getPattern()   const { return _simPattern;               }
   bool     getFecInv()    const { return fec_inv;                   }
   vector<CirGate*>* getFec() const { return _fec;                   }
   Var      getSatVar()    const {return sat_var;                    }

         
   // Fanin       
   CirGateV fanin0()       const { return _fanin0;                   }
   CirGateV fanin1()       const { return _fanin1;                   }
   CirGate* fanin0_gate()  const { return _fanin0.gate();            }
   CirGate* fanin1_gate()  const { return _fanin1.gate();            }
   bool     fanin0_inv()   const { return _fanin0.isInv();           }
   bool     fanin1_inv()   const { return _fanin1.isInv();           }
   unsigned fanin0_var()   const { return _fanin0.gate()->getVar();  }
   unsigned fanin1_var()   const { return _fanin1.gate()->getVar();  }

   // Fanout
   CirGateV fanout(const unsigned i)      const { assert(i < _fanouts.size()); return _fanouts[i];          }
   CirGate* fanout_gate(const unsigned i) const { assert(i < _fanouts.size()); return _fanouts[i].gate();   }
   bool     fanout_inv(const unsigned i)  const { assert(i < _fanouts.size()); return _fanouts[i].isInv();  }
   unsigned nFanouts()                    const { return _fanouts.size();                                   }
   CirGateV& fanout_opt(const unsigned i)       { assert(i < _fanouts.size()); return _fanouts[i];          }
   vector<CirGateV>&  output()                  { return _fanouts;                                          }

   // Fanout sorting
   void sortFanout();

   // Type query
   virtual bool isPi()       const = 0;
   virtual bool isPo()       const = 0;
   virtual bool isAig()      const = 0;
   virtual bool isConst()    const = 0;
   virtual bool isUndef()    const = 0;
   virtual bool isFloating() const = 0;

   // Basic setting methods
   void setLineNo(const unsigned l)     { _lineNo = l;                        }
   void setVar(const unsigned v)        { _var = v;                           }
   void setRef(const unsigned r)  const { _ref = r; /* const method orz... */ }
   void setFanin0(const CirGateV& g)    { _fanin0 = g;                        } 
   void setFanin1(const CirGateV& g)    { _fanin1 = g;                        } 
   void addFanout(const CirGateV& g)    { _fanouts.push_back(g);              } 
   void setFanin0(CirGate* g, size_t i) { _fanin0 = CirGateV(g, i);           } 
   void setFanin1(CirGate* g, size_t i) { _fanin1 = CirGateV(g, i);           } 
   void addFanout(CirGate* g, size_t i) { _fanouts.push_back(CirGateV(g, i)); }
   void setPattern(const size_t& a)     { _simPattern = a;                    } 
   void setFecInv(const bool a)         { fec_inv = a;                        }
   void setFec(vector<CirGate*>* a)     { _fec = a;                           }
   void setSatVar(const Var v)          { sat_var = v;                        }

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate()                                  const;
   void reportFanin(int level)                        const;
   void reportFanout(int level)                       const;
   void rec_rptFanin(const CirGate*, bool, int, int)  const;
   void rec_rptFanout(const CirGate*, bool, int, int) const;

private:
   unsigned          _lineNo;
   unsigned          _var;
   mutable unsigned  _ref;
   size_t            _simPattern;
   vector<CirGate*>* _fec;
   Var               sat_var;

protected:
   CirGateV          _fanin0;
   CirGateV          _fanin1;
   vector<CirGateV>  _fanouts;
   bool              fec_inv = false;

};

class CirPiGate : public CirGate
{
public:
   CirPiGate(unsigned l = 0, unsigned v = 0): CirGate(l, v) { _symbol = ""; }
   ~CirPiGate() {}

   virtual string getTypeStr() const { return "PI";      }
   virtual string symbol()     const { return _symbol;   }
   virtual bool isPi()         const { return true;      }
   virtual bool isPo()         const { return false;     }
   virtual bool isAig()        const { return false;     }
   virtual bool isConst()      const { return false;     }
   virtual bool isUndef()      const { return false;     }
   virtual bool isFloating()   const { return false;     }

   virtual void printGate() const {
      cout << "PI  " << getVar();
      if(_symbol != "") cout << " (" << _symbol << ")";
      cout << endl;
   }

   void setSymbol(const string& s) { _symbol = s; }

private:
   string _symbol;
};


class CirPoGate : public CirGate
{
public:
   CirPoGate(unsigned l = 0, unsigned v = 0): CirGate(l, v) { _symbol = ""; }
   ~CirPoGate() {}

   virtual string getTypeStr() const { return "PO";                      }
   virtual string symbol()     const { return _symbol;                   }
   virtual bool isPi()         const { return false;                     }
   virtual bool isPo()         const { return true;                      }
   virtual bool isAig()        const { return false;                     }
   virtual bool isConst()      const { return false;                     }
   virtual bool isUndef()      const { return false;                     }
   virtual bool isFloating()   const { return _fanin0.gate()->isUndef(); }

   virtual void printGate() const {
      cout << "PO  " << getVar() << " "
           << (_fanin0.gate()->isUndef() ? "*" : "") << (_fanin0.isInv() ? "!" : "")
           << _fanin0.gate()->getVar();
      if(_symbol != "") cout << " (" << _symbol << ")";
      cout << endl;
   }

   void setSymbol(const string& s) { _symbol = s; }

private:
   string _symbol;
};


class CirAigGate : public CirGate
{
public:
   CirAigGate(unsigned l = 0, unsigned v = 0): CirGate(l, v) {}
   ~CirAigGate() {}

   virtual string getTypeStr() const { return isUndef() ? "UNDEF" : "AIG";      }
   virtual string symbol()     const { return "";                               }
   virtual bool isPi()         const { return false;                            }
   virtual bool isPo()         const { return false;                            }
   virtual bool isAig()        const { return !isUndef();                       }
   virtual bool isConst()      const { return false;                            }
   virtual bool isUndef()      const { return _fanin0.null() || _fanin1.null(); }
   virtual bool isFloating()   const {
      if(isUndef()) return false;
      return _fanin0.gate()->isUndef() || _fanin1.gate()->isUndef();
   }
   virtual void printGate() const {
      assert(!isUndef());
      cout << "AIG " << getVar() << " "
           << (_fanin0.gate()->isUndef() ? "*" : "") << (_fanin0.isInv() ? "!" : "") 
           << _fanin0.gate()->getVar() << " " 
           << (_fanin1.gate()->isUndef() ? "*" : "") << (_fanin1.isInv() ? "!" : "") 
           << _fanin1.gate()->getVar() << endl;
   }
private:
};

class CirConstGate : public CirGate
{
public:
   CirConstGate(): CirGate(0, 0) {}
   ~CirConstGate() {}

   virtual string getTypeStr() const { return "CONST"; }
   virtual string symbol()     const { return "";   }
   virtual bool isPi()         const { return false;     }
   virtual bool isPo()         const { return false;     }
   virtual bool isAig()        const { return false;     }
   virtual bool isConst()      const { return true;      }
   virtual bool isUndef()      const { return false;     }
   virtual bool isFloating()   const { return false;     }

   virtual void printGate() const {
      cout << "CONST0" << endl;
   }
private:
};

#endif // CIR_GATE_H
