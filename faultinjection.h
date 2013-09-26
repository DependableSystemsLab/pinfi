#ifndef FAULT_INJECTION_H
#define FAULT_INJECTION_H
#include <map>
#include "pin.H"
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <time.h>

KNOB<string> instcount_file(KNOB_MODE_WRITEONCE, "pintool",
    "o", "pin.instcount.txt", "specify instruction count file name");

KNOB<string> fioption(KNOB_MODE_WRITEONCE, "pintool", "fioption", "", "specify fault injection option: all, sp, fp, ccs");

KNOB<string> fi_activation_file (KNOB_MODE_WRITEONCE, "pintool",
    "fi_activation", "activate", "specify fault injection activation file");
//typedef uint64_t UINT64;
//typedef uint32_t UINT32;

FILE *activationFile;
#define ALL_INST "AllInst"
#define CCS_INST "CCSavedInst"
#define FP_INST "FPInst" //EBP reg
#define SP_INST "SPInst"
#define FI_MAX_CHAR_PER_LINE 1000

//modified version of Jiesheng's code for Reg Map and floating point registers, We need this for FI into CCS 
#define MAX_ST_NUM 8

//#define DEBUG

#ifdef DEBUG
#define _logLevel 2

#define PRINT_MESSAGE(l, x)\
	if(l > _logLevel) {printf x; fflush(stdout);}
#else 

#define PRINT_MESSAGE(l, x) 

#endif // DEBUG

using namespace std;

class RegMap{

	struct Reg_Info {
		REG reg;
		REG inject_reg;
		UINT32 low_bit;
		UINT32 high_bit;
		bool fp_flag;
		
		Reg_Info (REG p_reg, REG p_inject, UINT32 p_low, UINT32 p_high, bool p_flag) {
			reg = p_reg;
			inject_reg = p_inject;
			low_bit = p_low;
			high_bit = p_high;
			fp_flag = p_flag;
		}
	};
	typedef map<UINT32,Reg_Info*> RegInfoMap;
	
	RegInfoMap reg_map;
		
	public:
	
		RegMap(){
    		createMap();
    	}
    	
    UINT32 findRegIndex(REG reg) {
    		RegInfoMap::iterator reg_iter;
			for(reg_iter = reg_map.begin() ; reg_iter !=reg_map.end() ; reg_iter++ ) {
				if(reg_iter -> second -> reg == reg)
					return reg_iter -> first;
			}
			fprintf(stderr, "Register %s not in the list!\n", REG_StringShort(reg).c_str());
			exit(2);
		}
		
    // Jiesheng: made changes to the find process for performance improvement
		REG findInjectReg (UINT32 index) {
			RegInfoMap::iterator reg_iter;
      reg_iter = reg_map.find(index);
      if ( reg_iter == reg_map.end()) {
        fprintf(stderr, "Register index %u not in the list!\n", index);
			  exit(2);
      }
			return reg_iter -> second -> inject_reg;
		}
		
		UINT32 findLowBoundBit (UINT32 index) {
			RegInfoMap::iterator reg_iter;
      reg_iter = reg_map.find(index);
      if ( reg_iter == reg_map.end()) {
        fprintf(stderr, "Register index %u not in the list!\n", index);
			  exit(2);
      }	
			return reg_iter -> second -> low_bit;

		}
		
		UINT32 findHighBoundBit (UINT32 index) {
			RegInfoMap::iterator reg_iter;
			reg_iter = reg_map.find(index);
      if ( reg_iter == reg_map.end()) {
        fprintf(stderr, "Register index %u not in the list!\n", index);
			  exit(2);
      }
			return reg_iter -> second -> high_bit;
			
		}
		
		bool isFloatReg (UINT32 index) {
			RegInfoMap::iterator reg_iter;
			reg_iter = reg_map.find(index);
      if ( reg_iter == reg_map.end()) {
        fprintf(stderr, "Register index %u not in the list!\n", index);
			  exit(2);
      }
			return reg_iter -> second -> fp_flag;
			
		}

	
	private:
    	void createMap() {
			reg_map[0] = new Reg_Info(REG_EDI, REG_RDI, 0, 32, false);
			reg_map[1] = new Reg_Info(REG_ESI, REG_RSI, 0, 32, false);
			reg_map[2] = new Reg_Info(REG_EBP, REG_RBP, 0, 32, false);
			reg_map[3] = new Reg_Info(REG_ESP, REG_RSP, 0, 32, false);
			reg_map[4] = new Reg_Info(REG_EBX, REG_RBX, 0, 32, false);
			reg_map[5] = new Reg_Info(REG_EDX, REG_RDX, 0, 32, false);
			reg_map[6] = new Reg_Info(REG_ECX, REG_RCX, 0, 32, false);
			reg_map[7] = new Reg_Info(REG_EAX, REG_RAX, 0, 32, false);
			
			reg_map[8] = new Reg_Info(REG_SEG_CS, REG_SEG_CS, 0, 32, false);
			reg_map[9] = new Reg_Info(REG_SEG_SS, REG_SEG_SS, 0, 32, false);
			reg_map[10] = new Reg_Info(REG_SEG_DS, REG_SEG_DS, 0, 32, false);
			reg_map[11] = new Reg_Info(REG_SEG_ES, REG_SEG_ES, 0, 32, false);
			reg_map[12] = new Reg_Info(REG_SEG_FS, REG_SEG_FS, 0, 32, false);
			reg_map[13] = new Reg_Info(REG_SEG_GS, REG_SEG_GS, 0, 32, false);
			
			reg_map[14] = new Reg_Info(REG_EFLAGS, REG_RFLAGS, 0, 32, false);
			reg_map[15] = new Reg_Info(REG_EIP, REG_RIP, 0, 32, false);
			
			reg_map[16] = new Reg_Info(REG_BX, REG_RBX, 0, 16, false);
			reg_map[17] = new Reg_Info(REG_DX, REG_RDX, 0, 16, false);
			reg_map[18] = new Reg_Info(REG_CX, REG_RCX, 0, 16, false);
			reg_map[19] = new Reg_Info(REG_AX, REG_RAX, 0, 16, false);
			reg_map[20] = new Reg_Info(REG_BL, REG_RBX, 0, 8, false);
			reg_map[21] = new Reg_Info(REG_DL, REG_RDX, 0, 8, false);
			reg_map[22] = new Reg_Info(REG_CL, REG_RCX, 0, 8, false);
			reg_map[23] = new Reg_Info(REG_AL, REG_RAX, 0, 8, false);
			reg_map[24] = new Reg_Info(REG_BH, REG_RBX, 8, 16, false);
			reg_map[25] = new Reg_Info(REG_DH, REG_RDX, 8, 16, false);
			reg_map[26] = new Reg_Info(REG_CH, REG_RCX, 8, 16, false);
			reg_map[27] = new Reg_Info(REG_AH, REG_RAX, 8, 16, false);
			
			reg_map[28] = new Reg_Info(REG_DI, REG_RDI, 0, 16, false);
			reg_map[29] = new Reg_Info(REG_SI, REG_RSI, 0, 16, false);
			reg_map[30] = new Reg_Info(REG_BP, REG_RBP, 0, 16, false);
			reg_map[31] = new Reg_Info(REG_SP, REG_RSP, 0, 16, false);
			
			reg_map[32] = new Reg_Info(REG_FLAGS, REG_RFLAGS, 0, 16, false);
			reg_map[33] = new Reg_Info(REG_IP, REG_RIP, 0, 16, false);
			
			reg_map[34] = new Reg_Info(REG_MM0, REG_MM0, 0, 64, true);
			reg_map[35] = new Reg_Info(REG_MM1, REG_MM1, 0, 64, true);
			reg_map[36] = new Reg_Info(REG_MM2, REG_MM2, 0, 64, true);
			reg_map[37] = new Reg_Info(REG_MM3, REG_MM3, 0, 64, true);
			reg_map[38] = new Reg_Info(REG_MM4, REG_MM4, 0, 64, true);
			reg_map[39] = new Reg_Info(REG_MM5, REG_MM5, 0, 64, true);
			reg_map[40] = new Reg_Info(REG_MM6, REG_MM6, 0, 64, true);
			reg_map[41] = new Reg_Info(REG_MM7, REG_MM7, 0, 64, true);
			
			reg_map[42] = new Reg_Info(REG_ST0, REG_ST0, 0, 80, true);
			reg_map[43] = new Reg_Info(REG_ST1, REG_ST1, 0, 80, true);
			reg_map[44] = new Reg_Info(REG_ST2, REG_ST2, 0, 80, true);
			reg_map[45] = new Reg_Info(REG_ST3, REG_ST3, 0, 80, true);
			reg_map[46] = new Reg_Info(REG_ST4, REG_ST4, 0, 80, true);
			reg_map[47] = new Reg_Info(REG_ST5, REG_ST5, 0, 80, true);
			reg_map[48] = new Reg_Info(REG_ST6, REG_ST6, 0, 80, true);
			reg_map[49] = new Reg_Info(REG_ST7, REG_ST7, 0, 80, true);
			
			reg_map[50] = new Reg_Info(REG_XMM0, REG_XMM0, 0, 128, true);
			reg_map[51] = new Reg_Info(REG_XMM1, REG_XMM1, 0, 128, true);
			reg_map[52] = new Reg_Info(REG_XMM2, REG_XMM2, 0, 128, true);
			reg_map[53] = new Reg_Info(REG_XMM3, REG_XMM3, 0, 128, true);
			reg_map[54] = new Reg_Info(REG_XMM4, REG_XMM4, 0, 128, true);
			reg_map[55] = new Reg_Info(REG_XMM5, REG_XMM5, 0, 128, true);
			reg_map[56] = new Reg_Info(REG_XMM6, REG_XMM6, 0, 128, true);
			reg_map[57] = new Reg_Info(REG_XMM7, REG_XMM7, 0, 128, true);
			
			reg_map[58] = new Reg_Info(REG_X87, REG_X87, 0, 80, true);
			
			reg_map[59] = new Reg_Info(REG_YMM0, REG_YMM0, 0, 128, true);
			reg_map[60] = new Reg_Info(REG_YMM1, REG_YMM1, 0, 128, true);
			reg_map[61] = new Reg_Info(REG_YMM2, REG_YMM2, 0, 128, true);
			reg_map[62] = new Reg_Info(REG_YMM3, REG_YMM3, 0, 128, true);
			reg_map[63] = new Reg_Info(REG_YMM4, REG_YMM4, 0, 128, true);
			reg_map[64] = new Reg_Info(REG_YMM5, REG_YMM5, 0, 128, true);
			reg_map[65] = new Reg_Info(REG_YMM6, REG_YMM6, 0, 128, true);
			reg_map[66] = new Reg_Info(REG_YMM7, REG_YMM7, 0, 128, true);
			
			//the R regs for 64 bit
			reg_map[67] = new Reg_Info(REG_RDI, REG_RDI, 0, 64, false);
			reg_map[68] = new Reg_Info(REG_RSI, REG_RSI, 0, 64, false);
			reg_map[69] = new Reg_Info(REG_RBP, REG_RBP, 0, 64, false);
			reg_map[70] = new Reg_Info(REG_RSP, REG_RSP, 0, 64, false);
			reg_map[71] = new Reg_Info(REG_RBX, REG_RBX, 0, 64, false);
			reg_map[72] = new Reg_Info(REG_RDX, REG_RDX, 0, 64, false);
			reg_map[73] = new Reg_Info(REG_RCX, REG_RCX, 0, 64, false);
			reg_map[74] = new Reg_Info(REG_RAX, REG_RAX, 0, 64, false);
			
			reg_map[75] = new Reg_Info(REG_RIP, REG_RIP, 0, 64, false);
			
			//the R8-R16 regs
			reg_map[76] = new Reg_Info(REG_R8, REG_R8, 0, 64, false);
			reg_map[77] = new Reg_Info(REG_R9, REG_R9, 0, 64, false);
			reg_map[78] = new Reg_Info(REG_R10, REG_R10, 0, 64, false);
			reg_map[79] = new Reg_Info(REG_R11, REG_R11, 0, 64, false);
			reg_map[80] = new Reg_Info(REG_R12, REG_R12, 0, 64, false);
			reg_map[81] = new Reg_Info(REG_R13, REG_R13, 0, 64, false);
			reg_map[82] = new Reg_Info(REG_R14, REG_R14, 0, 64, false);
			reg_map[83] = new Reg_Info(REG_R15, REG_R15, 0, 64, false);
			//reg_map[84] = new Reg_Info(REG_R16, REG_R16, 0, 64, false);
			
			reg_map[85] = new Reg_Info(REG_R8D, REG_R8, 0, 32, false);
			reg_map[86] = new Reg_Info(REG_R9D, REG_R9, 0, 32, false);
			reg_map[87] = new Reg_Info(REG_R10D, REG_R10, 0, 32, false);
			reg_map[88] = new Reg_Info(REG_R11D, REG_R11, 0, 32, false);
			reg_map[89] = new Reg_Info(REG_R12D, REG_R12, 0, 32, false);
			reg_map[90] = new Reg_Info(REG_R13D, REG_R13, 0, 32, false);
			reg_map[91] = new Reg_Info(REG_R14D, REG_R14, 0, 32, false);
			reg_map[92] = new Reg_Info(REG_R15D, REG_R15, 0, 32, false);
			//reg_map[93] = new Reg_Info(REG_R16D, REG_R16, 0, 32, false);
			
			reg_map[94] = new Reg_Info(REG_R8W, REG_R8, 0, 16, false);
			reg_map[95] = new Reg_Info(REG_R9W, REG_R9, 0, 16, false);
			reg_map[96] = new Reg_Info(REG_R10W, REG_R10, 0, 16, false);
			reg_map[97] = new Reg_Info(REG_R11W, REG_R11, 0, 16, false);
			reg_map[98] = new Reg_Info(REG_R12W, REG_R12, 0, 16, false);
			reg_map[99] = new Reg_Info(REG_R13W, REG_R13, 0, 16, false);
			reg_map[100] = new Reg_Info(REG_R14W, REG_R14, 0, 16, false);
			reg_map[101] = new Reg_Info(REG_R15W, REG_R15, 0, 16, false);
			//reg_map[102] = new Reg_Info(REG_R16W, REG_R16, 0, 16, false);
			
			reg_map[103] = new Reg_Info(REG_R8B, REG_R8, 0, 8, false);
			reg_map[104] = new Reg_Info(REG_R9B, REG_R9, 0, 8, false);
			reg_map[105] = new Reg_Info(REG_R10B, REG_R10, 0, 8, false);
			reg_map[106] = new Reg_Info(REG_R11B, REG_R11, 0, 8, false);
			reg_map[107] = new Reg_Info(REG_R12B, REG_R12, 0, 8, false);
			reg_map[108] = new Reg_Info(REG_R13B, REG_R13, 0, 8, false);
			reg_map[109] = new Reg_Info(REG_R14B, REG_R14, 0, 8, false);
			reg_map[110] = new Reg_Info(REG_R15B, REG_R15, 0, 8, false);
			//reg_map[111] = new Reg_Info(REG_R16B, REG_R16, 0, 8, false);		
			
			reg_map[111] = new Reg_Info(REG_RFLAGS, REG_RFLAGS, 0, 64, false);
			
			//low 8 bits
			reg_map[112] = new Reg_Info(REG_SIL, REG_RSI, 0, 8, false);
			reg_map[113] = new Reg_Info(REG_DIL, REG_RDI, 0, 8, false);
			reg_map[114] = new Reg_Info(REG_BPL, REG_RBP, 0, 8, false);
			reg_map[115] = new Reg_Info(REG_SPL, REG_RSP, 0, 8, false);
			
			//additional xmm8-15
			reg_map[116] = new Reg_Info(REG_XMM8, REG_XMM8, 0, 128, true);
			reg_map[117] = new Reg_Info(REG_XMM9, REG_XMM9, 0, 128, true);
			reg_map[118] = new Reg_Info(REG_XMM10, REG_XMM10, 0, 128, true);
			reg_map[119] = new Reg_Info(REG_XMM11, REG_XMM11, 0, 128, true);
			reg_map[120] = new Reg_Info(REG_XMM12, REG_XMM12, 0, 128, true);
			reg_map[121] = new Reg_Info(REG_XMM13, REG_XMM13, 0, 128, true);
			reg_map[122] = new Reg_Info(REG_XMM14, REG_XMM14, 0, 128, true);
			reg_map[123] = new Reg_Info(REG_XMM15, REG_XMM15, 0, 128, true);

			//additional ymm8-15
			reg_map[124] = new Reg_Info(REG_YMM8, REG_YMM8, 0, 128, true);
			reg_map[125] = new Reg_Info(REG_YMM9, REG_YMM9, 0, 128, true);
			reg_map[126] = new Reg_Info(REG_YMM10, REG_YMM10, 0, 128, true);
			reg_map[127] = new Reg_Info(REG_YMM11, REG_YMM11, 0, 128, true);
			reg_map[128] = new Reg_Info(REG_YMM12, REG_YMM12, 0, 128, true);
			reg_map[129] = new Reg_Info(REG_YMM13, REG_YMM13, 0, 128, true);
			reg_map[130] = new Reg_Info(REG_YMM14, REG_YMM14, 0, 128, true);
			reg_map[131] = new Reg_Info(REG_YMM15, REG_YMM15, 0, 128, true);			
		}

};

RegMap reg_map;

// FI: set the X87 ST[0-7] or MM[0-7] context register
VOID FI_SetSTContextReg (CONTEXT* ctxt, REG reg, UINT32 reg_num)
{
	//INPUT: x87 or st0-7 or MM[0-7]
	//choose st[i] to inject
	UINT32 i = 0;
	if(reg == REG_X87) {
		srand((unsigned)time(0)); 
		i = rand() % MAX_ST_NUM;
	}
	else {
		string reg_name = REG_StringShort(reg);
		i = reg_name[reg_name.size() - 1] - '0';
	}
	
	CHAR fpContextSpace[FPSTATE_SIZE];
    FPSTATE *fpContext = reinterpret_cast<FPSTATE *>(fpContextSpace);
        
    PIN_GetContextFPState(ctxt, fpContext);

	
	UINT32 low_bound_bit = reg_map.findLowBoundBit(reg_num);
	UINT32 high_bound_bit = reg_map.findHighBoundBit(reg_num);

	UINT32 inject_bit = (rand() % (high_bound_bit - low_bound_bit)) + low_bound_bit;
	
	if(inject_bit < 64) {
		PRINT_MESSAGE(3, ("EXECUTING: Reg name %s Low value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._sts[i]._raw._lo));
		
		fpContext->fxsave_legacy._sts[i]._raw._lo ^= (1UL << inject_bit);

		PRINT_MESSAGE(3, ("EXECUTING: Changed Reg name %s Low value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._sts[i]._raw._lo));

	}
	else {
		PRINT_MESSAGE(3, ("EXECUTING: Reg name %s High value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._sts[i]._raw._hi));
		
		fpContext->fxsave_legacy._sts[i]._raw._hi ^= (1UL << (inject_bit - 64));

		PRINT_MESSAGE(3, ("EXECUTING: Changed Reg name %s High value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._sts[i]._raw._hi));

	}

	PIN_SetContextFPState(ctxt, fpContext);
}

// FI: set the XMM[0-7] context register
VOID FI_SetXMMContextReg (CONTEXT* ctxt, REG reg, UINT32 reg_num)
{
	//choose XMM[i] to inject
	string reg_name = REG_StringShort(reg);
	UINT32 i = reg_name[reg_name.size() - 1] - '0';
	
	CHAR fpContextSpace[FPSTATE_SIZE];
    FPSTATE *fpContext = reinterpret_cast<FPSTATE *>(fpContextSpace);
        
    PIN_GetContextFPState(ctxt, fpContext);

	
	UINT32 low_bound_bit = reg_map.findLowBoundBit(reg_num);
	UINT32 high_bound_bit = reg_map.findHighBoundBit(reg_num);

	UINT32 inject_bit = (rand() % (high_bound_bit - low_bound_bit)) + low_bound_bit;
  
   // JIESHENG: this is not a right change from the hardware perspective, but it
  // is to improve the activated faults. 
  // xmm is used for double, so only the lower 64 bits are used
  if (inject_bit >= 64)
    inject_bit -= 64;	

  // JIESHNEG: something wrong, just to test whether the xmm injection is correct or not
  //
  srand(time(NULL));
  inject_bit = (rand() % 64);
  std::cerr << "Inject into bit " << inject_bit << std::endl;

	if(inject_bit < 64) {
		PRINT_MESSAGE(3, ("EXECUTING: Reg name %s Low value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._xmms[i]._vec64[0]));
		
		fpContext->fxsave_legacy._xmms[i]._vec64[0] ^= (1UL << inject_bit);

		PRINT_MESSAGE(3, ("EXECUTING: Changed Reg name %s Low value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._xmms[i]._vec64[0]));

	}
	else {
		PRINT_MESSAGE(3, ("EXECUTING: Reg name %s High value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._xmms[i]._vec64[1]));
		
		fpContext->fxsave_legacy._xmms[i]._vec64[1] ^= (1UL << (inject_bit - 64));

		PRINT_MESSAGE(3, ("EXECUTING: Changed Reg name %s High value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)fpContext->fxsave_legacy._xmms[i]._vec64[1]));

	}

	PIN_SetContextFPState(ctxt, fpContext);
}

// FI: set the YMM[0-7] context register
VOID FI_SetYMMContextReg (CONTEXT* ctxt, REG reg, UINT32 reg_num)
{
	//choose YMM[i] to inject
	string reg_name = REG_StringShort(reg);
	UINT32 i = reg_name[reg_name.size() - 1] - '0';
	
	CHAR fpContextSpace[FPSTATE_SIZE];
    FPSTATE *fpContext = reinterpret_cast<FPSTATE *>(fpContextSpace);
        
    PIN_GetContextFPState(ctxt, fpContext);

	
	UINT32 low_bound_bit = reg_map.findLowBoundBit(reg_num);
	UINT32 high_bound_bit = reg_map.findHighBoundBit(reg_num);

	UINT32 inject_bit = (rand() % (high_bound_bit - low_bound_bit)) + low_bound_bit;
	
	//FIXME: change number below to parameter
	UINT32 index = (i * 128 + inject_bit) / (sizeof(UINT8) * 8);
	UINT32 bit = (i * 128 + inject_bit) % (sizeof(UINT8) * 8);

	PRINT_MESSAGE(3, ("EXECUTING: Reg name %s Low value %u\n", REG_StringShort(reg).c_str(), 
		fpContext->_xstate._ymmUpper[index]));
		
	fpContext->_xstate._ymmUpper[index] ^= (1UL << bit);

	PRINT_MESSAGE(3, ("EXECUTING: Changed Reg name %s Low value %u\n", REG_StringShort(reg).c_str(), 
		fpContext->_xstate._ymmUpper[index]));

	PIN_SetContextFPState(ctxt, fpContext);
}

VOID FI_PrintActivationInfo()
{
	if(fi_activation_file.Value() != "") {
		FILE* fi_act_FILE = fopen(fi_activation_file.Value().c_str(), "w");
					
		if(fi_act_FILE != NULL) {
			fprintf(fi_act_FILE, "activated");
			fclose(fi_act_FILE);
		}
	}

}

bool is_stackptrReg(REG reg){
	if(reg == REG_RSP || reg == REG_ESP || reg == REG_SP)
		return true;
	return false;
}

bool is_frameptrReg(REG reg){
	if(reg == REG_RBP || reg == REG_EBP || reg == REG_BP)
		return true;
	return false;
}
#endif
