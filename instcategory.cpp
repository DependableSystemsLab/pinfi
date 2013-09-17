#include<iostream>
#include<fstream>

#include <set>
#include <map>
#include <string>

#include "pin.H"
#include "utils.h"

//#define INCLUDEALLINST
#define NOBRANCHES
//#define NOSTACKFRAMEOP
//#define ONLYFP

KNOB<string> instcategory_file(KNOB_MODE_WRITEONCE, "pintool",
    "o", "pin.instcategory.txt", "output file to store instruction categories");

static std::map<string, std::set<string>* > category_opcode_map;

// Pin calls this function every time a new instruction is encountered
VOID CountInst(INS ins, VOID *v)
{
  if (!isValidInst(ins))
    return;

    //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)countAllInst, IARG_END);
#ifdef INCLUDEALLINST
#else

#ifdef NOBRANCHES
  if(INS_IsBranch(ins) || !INS_HasFallThrough(ins)) {
    LOG("instcount: branch/ret inst: " + INS_Disassemble(ins) + "\n");
		return;
  }
#endif

// NOSTACKFRAMEOP must be used together with NOBRANCHES, IsStackWrite 
// has a bug that does not put pop into the list
#ifdef NOSTACKFRAMEOP
  if(INS_IsStackWrite(ins) || OPCODE_StringShort(INS_Opcode(ins)) == "POP") {
    LOG("instcount: stack frame change inst: " + INS_Disassemble(ins) + "\n");    
    return;
  }
#endif

#ifdef ONLYFP
 	int numW = INS_MaxNumWRegs(ins);
  bool hasfp = false;
  for (int i = 0; i < numW; i++) {
    if (reg_map.isFloatReg(reg)) {
      hasfp = true;
      break;
    }
  }
  if (!hasfp){
    return;  
  }
#endif

#endif

  string cate = CATEGORY_StringShort(INS_Category(ins));
  if (category_opcode_map.find(cate) == category_opcode_map.end()) {
    category_opcode_map.insert(std::pair<string, std::set<string>* >(cate, new std::set<string>));  
  }
  category_opcode_map[cate]->insert(INS_Mnemonic(ins));
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    ofstream OutFile;
    OutFile.open(instcategory_file.Value().c_str());
    OutFile.setf(ios::showbase);
    
    for (std::map<string, std::set<string>* >::iterator it = category_opcode_map.begin();
      it != category_opcode_map.end(); ++it) {
    OutFile << it->first << std::endl;  
    for (std::set<string>::iterator it2 = it->second->begin();
         it2 != it->second->end(); ++it2) {
      OutFile << "\t" << *it2 << endl;  
    }
  }

	OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() {
    cerr << "This tool collects the instruction categories/opcode in the program" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}


int main(int argc, char * argv[])
{
    PIN_InitSymbols();
	// Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(CountInst, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
