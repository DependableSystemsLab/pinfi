#include<iostream>
#include<fstream>
#include "pin.H"

//#include "faultinjection.h"
//#include "commonvars.h"

//#define INCLUDEALLINST
#define NOBRANCHES
#define NOSTACKFRAMEOP
//#define ONLYFP

KNOB<string> instcount_file(KNOB_MODE_WRITEONCE, "pintool",
    "o", "instcount", "specify instruction count file name");
	
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


/**
 * IMPORTANT: This is to make sure fault injections are done at the .text 
 * of the compiled code, instead of at libraries or .init/.fini sections
 */
  if (!RTN_Valid(INS_Rtn(ins))) { // some library instructions do not have rtn !?
    LOG("Invalid RTN " + INS_Disassemble(ins) + "\n");
    return;
  }
  
  if (!IMG_IsMainExecutable(SEC_Img(RTN_Sec(INS_Rtn(ins))))) {
    //LOG("Libraries " + IMG_Name(SEC_Img(RTN_Sec(INS_Rtn(ins)))) + "\n");
    return;
  }
  if (SEC_Name(RTN_Sec(INS_Rtn(ins))) != ".text") {
    //LOG("Section: " + SEC_Name(RTN_Sec(INS_Rtn(ins))) + "\n");
    return;
  }
  std::string rtnname = RTN_Name(INS_Rtn(ins));
  if (rtnname.find("__libc") == 0 || rtnname.find("_start") == 0 ||
      rtnname.find("call_gmon_start") == 0 || rtnname.find("frame_dummy") == 0 ||
      rtnname.find("__do_global") == 0 || rtnname.find("__stat") == 0) {
    return;
  }
  LOG("Exe " + RTN_Name(INS_Rtn(ins)) + "\n");

    // Insert a call to docount before every instruction, no arguments are passed
	REG reg = INS_RegW(ins, 0);
	if(!REG_valid(reg))
		return;

    //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)countAllInst, IARG_END);
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




	INS_InsertPredicatedCall(
				ins, IPOINT_AFTER, (AFUNPTR)countAllInst,
				IARG_END);	
#endif

#if 0
	if (INS_Category(ins) == XED_CATEGORY_POP)
		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)countCCSInst, IARG_END);
	
	for(int i=0; i<numW; i++){
		const REG reg =  INS_RegW(ins, i );
		if(reg == REG_RSP || reg == REG_ESP || reg == REG_SP)
		{	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)countSPInst, IARG_END);
// 			LOG("ins SP:" + INS_Disassemble(ins) + "\n"); 
		}
		if(reg == REG_RBP || reg == REG_EBP || reg == REG_BP)
			INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)countBPInst, IARG_END);
	}
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

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(CountInst, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
