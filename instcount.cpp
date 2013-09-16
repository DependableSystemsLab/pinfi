#include<iostream>
#include<fstream>

#include <set>
#include <map>
#include <string>

#include "pin.H"
#include "utils.h"
#include "instselector.h"

//#include "faultinjection.h"
//#include "commonvars.h"

//#define INCLUDEALLINST
#define NOBRANCHES
//#define NOSTACKFRAMEOP
//#define ONLYFP

KNOB<string> instcount_file(KNOB_MODE_WRITEONCE, "pintool",
    "o", "pin.instcount.txt", "specify instruction count file name");
	
static UINT64 fi_all = 0;
static UINT64 fi_ccs = 0;
static UINT64 fi_sp = 0;
static UINT64 fi_bp = 0;

VOID countAllInst() {fi_all++;}
VOID countCCSInst() {fi_ccs++;}
VOID countSPInst() {fi_sp++;}
VOID countBPInst() { fi_bp++;}


// Pin calls this function every time a new instruction is encountered
VOID CountInst(INS ins, VOID *v)
{
  if (!isValidInst(ins))
    return;

#ifdef INCLUDEALLINST
 	int numW = INS_MaxNumWRegs(ins), mayChangeControlFlow = 0;
   if(!INS_HasFallThrough(ins))
		mayChangeControlFlow = 1;
	for(int i =0; i < numW; i++){
		reg = INS_RegW(ins, i);
		if(reg == REG_RIP || reg == REG_EIP || reg == REG_IP) // conditional branches
		{	mayChangeControlFlow = 1; break;}
	}

	if(mayChangeControlFlow) {
		INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)countAllInst,
				IARG_END);	
		//LOG("No through\n");
	}
	else{
		INS_InsertPredicatedCall(
				ins, IPOINT_AFTER, (AFUNPTR)countAllInst,
				IARG_END);	
// 		LOG("ins SP:" + INS_Disassemble(ins) + "\n"); 
// 		LOG("reg:" + REG_StringShort(reg) +"\n");
		//LOG(numW+"\n"); 
	}
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
  for (int i = 0; i < numW; i++){
    if (reg_map.isFloatReg(reg)) {
      hasfp = true;
      break;
    }
  }
  if (!hasfp){
    return;  
  }
#endif

  
// select instruction based on instruction type
  if(!isInstFITarget(ins))
    return;


	INS_InsertPredicatedCall(
				ins, IPOINT_AFTER, (AFUNPTR)countAllInst,
				IARG_END);	
#endif


}

// bool mayChangeControlFlow(INS ins){
// 	REG reg;
// 	if(!INS_HasFallThrough(ins))
// 		return true;
// 	int numW = INS_MaxNumWRegs(ins);
// 	for(int i =0; i < numW; i++){
// 		if(reg == REG_RIP || reg == REG_EIP || reg == REG_IP) // conditional branches
// 			return true;
// 	}
// 	return false;
// }
// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    ofstream OutFile;
    OutFile.open(instcount_file.Value().c_str());
    OutFile.setf(ios::showbase);
    OutFile <<"AllInst:"<< fi_all << endl;
	OutFile <<"CCSavedInst:"<< fi_ccs  << endl;
	OutFile << "SPInst:"<< fi_sp << endl;
    OutFile << "FPInst:"<< fi_bp << endl;
    
	OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}



int main(int argc, char * argv[])
{
    PIN_InitSymbols();
	// Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

  
  
    configInstSelector();



    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(CountInst, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
