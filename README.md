# FRAIG
Data Structure and Programming Final Project (Fall, 2018) - Functionally Reduced And-Inverter Graph (FRAIG)

Instructor: Prof. Chung-Yang (Ric) Huang, National Taiwan University

# Description
In this final project, we are going to implement a special circuit representation, FRAIG (Functionally Reduced And-Inverter Graph), from a circuit description file. 

This fraig program should provide the following functionalities:
1. Parsing the circuit description file in the AIGER format.

2. Sweeping out the gates that cannot be reached from POs (primary outputs).

3. Performing trivial circuit optimizations including: 

    (a) If one of the fanins of an AND gate is a constant 1, this AND gate can be removed and replaced by the other fanin.
  
    (b) If one of the fanins of an AND gate is a constatnt 0, this AND gate can be removed and replaced by the constant 0.
  
    (c) If both fanins of an AND gate are the same, this AND gate can be removed and replaced by its fanin.
  
    (d) If one of the fanins of an AND gate is inverse to the other fanin, this AND gate can be removed and replaced by a constant 0.
  
4. Providing a function to perform “structural hash” (strash) on the circuit netlist. The strash operation is to merge the structurally equivalent signals by comparing their gate types and permuting their inputs.

5. When performing “merge” operation on a pair of gates, pick any of them to retain. That is, you can remove either one.

6. Boolean logic simulation is to explore the similarity of signals in the circuit netlist.

7. The program is capable of generating random patterns as well as taking simulation patterns from files.

8. The signals in a functionally equivalent candidate(FEC) pair can be formally proved or disproved by a Boolean Satisfiability (SAT) solver. 

9. If two signals are proven to be (inverse) functionally equivalent, merge them (with inverted phase) in the circuit netlist.
