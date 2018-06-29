#include "faultinjection.h"
#include <string.h>
#include <assert.h>
#include "pin.H"
#include "fi_cjmp_map.h"

#include "utils.h"
#include "instselector.h"

//#define INCLUDEALLINST
#define NOBRANCHES //always set
//#define NOSTACKFRAMEOP
//#define ONLYFP

UINT64 fi_inject_instance = 0;
UINT64 fi_iterator = 0;
UINT64 total_num_inst = 0;
int activated = 0;


CJmpMap jmp_map;


VOID FI_InjectFault_FlagReg(VOID * ip, UINT32 reg_num, UINT32 jmp_num, CONTEXT* ctxt)
{
	if(fi_iterator == fi_inject_instance) {

    bool isvalid = false;

    const REG reg =  reg_map.findInjectReg(reg_num);
		if(REG_valid(reg)){

      isvalid = true;

			CJmpMap::JmpType jmptype = jmp_map.findJmpType(jmp_num);
		PRINT_MESSAGE(3, ("EXECUTING flag reg: Original Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
					(VOID*)PIN_GetContextReg( ctxt, reg )));
			if(jmptype == CJmpMap::DEFAULT) {
				ADDRINT temp = PIN_GetContextReg( ctxt, reg );
				UINT32 inject_bit = jmp_map.findInjectBit(jmp_num);
				temp = temp ^ (1UL << inject_bit);

				PIN_SetContextReg( ctxt, reg, temp);
	    } else if (jmptype == CJmpMap::USPECJMP) {
				ADDRINT temp = PIN_GetContextReg( ctxt, reg );
				UINT32 CF_val = (temp & (1UL << CF_BIT)) >> CF_BIT;
				UINT32 ZF_val = (temp & (1UL << ZF_BIT)) >> ZF_BIT;
				if(CF_val || ZF_val) {
					temp = temp & (~(1UL << CF_BIT));
					temp = temp & (~(1UL << ZF_BIT));
				}
				else {
					temp = temp | (1UL << ZF_BIT);
				}
				PIN_SetContextReg( ctxt, reg, temp);
	    }	else {
				ADDRINT temp = PIN_GetContextReg( ctxt, reg );
				UINT32 SF_val = (temp & (1UL << SF_BIT)) >> SF_BIT;
				UINT32 OF_val = (temp & (1UL << OF_BIT)) >> OF_BIT;
				UINT32 ZF_val = (temp & (1UL << ZF_BIT)) >> ZF_BIT;
				if(ZF_val || (SF_val != OF_val)) {
					temp = temp & (~(1UL << ZF_BIT));
					if(SF_val != OF_val) {
						temp = temp ^ (1UL << SF_BIT);
					}
				}
				else {
					temp = temp | (1UL << ZF_BIT);
				}
				PIN_SetContextReg( ctxt, reg, temp);
			}
			PRINT_MESSAGE(3, ("EXECUTING flag reg: Changed Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
					(VOID*)PIN_GetContextReg( ctxt, reg )));
			
			//FI_PrintActivationInfo();	
			//fi_iterator ++;
		}
		if(isvalid){
			fprintf(activationFile, "Activated: Valid Reg name %s in %p\n", REG_StringShort(reg).c_str(), ip);
			fclose(activationFile); // can crash after this!
			activated = 1;
			fi_iterator ++;
	
			PIN_ExecuteAt(ctxt);
				//PIN_ExecuteAt() will lead to reexecution of the function right after injection
		}



 else
      fi_inject_instance++;

		//fi_iterator ++;

	} 
	fi_iterator ++;
}



//analysis code -- injection code
/*
VOID inject_SP_FP(VOID *ip, UINT32 reg_num, CONTEXT *ctxt){
	if(fi_iterator == fi_inject_instance) {
		const REG reg =  reg_map.findInjectReg(reg_num);
		if(!REG_valid(reg)){
			fprintf(stderr, "ERROR, Has to be one of SP or BP\n");
			exit(1);
		}	
		PRINT_MESSAGE(4, ("EXECUTING: Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)PIN_GetContextReg( ctxt, reg )));

		ADDRINT temp = PIN_GetContextReg( ctxt, reg );
		srand((unsigned)time(0)); 
		UINT32 low_bound_bit = reg_map.findLowBoundBit(reg_num);
		UINT32 high_bound_bit = reg_map.findHighBoundBit(reg_num);

		UINT32 inject_bit = (rand() % (high_bound_bit - low_bound_bit)) + low_bound_bit;

		temp = (ADDRINT) (temp ^ (1UL << inject_bit));

		PIN_SetContextReg( ctxt, reg, temp);
		PRINT_MESSAGE(4, ("EXECUTING: Changed Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
			(VOID*)PIN_GetContextReg( ctxt, reg )));

		FI_PrintActivationInfo();	
		fi_iterator ++;
		PIN_ExecuteAt(ctxt);
	}
	//PIN_ExecuteAt() will lead to reexecution of the function right after injection
	fi_iterator ++;
}
*/


VOID inject_CCS(VOID *ip, UINT32 reg_num, CONTEXT *ctxt){
	//need to consider FP regs and context
	if(fi_iterator == fi_inject_instance) {
		const REG reg =  reg_map.findInjectReg(reg_num);
		int isvalid = 0;
		if(REG_valid(reg)){
			isvalid = 1;
//PRINT_MESSAGE(4, ("Executing: Valid Reg name %s\n", REG_StringShort(reg).c_str()));

			if(reg_map.isFloatReg(reg_num)) {
				//PRINT_MESSAGE(4, ("Executing: Float Reg name %s\n", REG_StringShort(reg).c_str()));

        if (REG_is_xmm(reg)) {
          PRINT_MESSAGE(4, ("Executing: xmm: Reg name %s\n", REG_StringShort(reg).c_str()));

					FI_SetXMMContextReg(ctxt, reg, reg_num);
				}
				else if (REG_is_ymm(reg)) {
					
					PRINT_MESSAGE(4, ("Executing: ymm: Reg name %s\n", REG_StringShort(reg).c_str()));

					FI_SetYMMContextReg(ctxt, reg, reg_num);
				}
				//else if(REG_is_fr_or_x87(reg) || REG_is_mm(reg)) {
				else if(REG_is_fr(reg) || REG_is_mm(reg)) {
					PRINT_MESSAGE(4, ("Executing: mm or x87: Reg name %s\n", REG_StringShort(reg).c_str()));

					FI_SetSTContextReg(ctxt, reg, reg_num);
				}
				else {
					fprintf(stderr, "Register %s not covered!\n", REG_StringShort(reg).c_str());
					exit(3);
				}
			}
			else{
				//PRINT_MESSAGE(4, ("EXECUTING: Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
				//	(VOID*)PIN_GetContextReg( ctxt, reg )));

				ADDRINT temp = PIN_GetContextReg( ctxt, reg );
				srand((unsigned)time(0)); 
				UINT32 low_bound_bit = reg_map.findLowBoundBit(reg_num);
				UINT32 high_bound_bit = reg_map.findHighBoundBit(reg_num);

				UINT32 inject_bit = (rand() % (high_bound_bit - low_bound_bit)) + low_bound_bit;

				temp = (ADDRINT)(temp ^ (1UL << inject_bit));

				PIN_SetContextReg( ctxt, reg, temp);

				
				//PRINT_MESSAGE(4, ("EXECUTING: Changed Reg name %s value %p\n", REG_StringShort(reg).c_str(), 
				//	(VOID*)PIN_GetContextReg( ctxt, reg )));
			}

                        //FI_PrintActivationInfo();	
		}
		if(isvalid){
			fprintf(activationFile, "Activated: Valid Reg name %s in %p\n", REG_StringShort(reg).c_str(), ip);
			fclose(activationFile); // can crash after this!
			activated = 1;
			fi_iterator ++;
	
			PIN_ExecuteAt(ctxt);
				//PIN_ExecuteAt() will lead to reexecution of the function right after injection
		}
        else
			fi_inject_instance++;
	}
	fi_iterator ++;
}

VOID FI_InjectFault_Mem(VOID * ip, VOID *memp, UINT32 size)
{
		if(fi_iterator == fi_inject_instance) {
			if(size == 4) {
				PRINT_MESSAGE(4, ("Executing %p, memory %p, value %d, in hex %p\n", 
					ip, memp, * ((int*)memp), (VOID*)(*((int*)memp))));
			}

			UINT8* temp_p = (UINT8*) memp;
			srand((unsigned)time(0)); 	
			UINT32 inject_bit = rand() % (size * 8/* bits in one byte*/);
		
			UINT32 byte_num = inject_bit / 8;
			UINT32 offset_num = inject_bit % 8;
		
			*(temp_p + byte_num) = *(temp_p + byte_num) ^ (1U << offset_num);
		
			if(size == 4) {
				PRINT_MESSAGE(4, ("Executing %p, memory %p, value %d, in hex %p\n", 
					ip, memp, * ((int*)memp), (VOID*)(*((int*)memp))));
			}
			

      fprintf(activationFile, "Activated: Memory injection\n");
			fclose(activationFile); // can crash after this!
			activated = 1;


			fi_iterator ++; //This is because the inject_reg will mistakenly add 1 more time when injecting 
		}

		fi_iterator ++;
}

VOID FI_InjectFault_MEM_ECC(VOID * ip, VOID *memp, UINT32 size,UINT32 num, BOOL mode)
{
	if(fi_iterator == fi_inject_instance) {

		fprintf(activationFile, "Executing %p, memory %p, value %d, in hex %x\n",
					ip, memp, * ((int*)memp), (*((int*)memp)));


		UINT8* temp_p = (UINT8*) memp;
		srand((unsigned)time(0));
		UINT32 last_inject_bit = 0;
		for (UINT32 i = 0; i < num; i ++) {
			UINT32 inject_bit = rand() % (size * 8/* bits in one byte*/);

			if ((i == num-1) && mode)
			{
				inject_bit = last_inject_bit+1;
				if (inject_bit == size*8)
					inject_bit = 0; // just do this for now. This case should be rare.
			}
			UINT32 byte_num = inject_bit / 8;
			UINT32 offset_num = inject_bit % 8;

			*(temp_p + byte_num) = *(temp_p + byte_num) ^ (1U << offset_num);


			fprintf(activationFile, "Executing %p, memory %p, value %d, in hex %x\n",
					ip, memp, * ((int*)memp), (*((int*)memp)));
			last_inject_bit = inject_bit;
		}
		fprintf(activationFile, "Activated: Memory injection\n");
		fclose(activationFile); // can crash after this!
		activated = 1;


		fi_iterator ++; //This is because the inject_reg will mistakenly add 1 more time when injecting
	}

	fi_iterator ++;
}


VOID instruction_InstrumentationECC(INS ins, VOID *v)
{
	int num = multibits.Value();
	bool mode = consecutive.Value();
	if (!isValidInst(ins))
		return;
	// memory write, so the injection happens after
	/*if (INS_IsMemoryWrite(ins)){
		INS_InsertPredicatedCall(
				ins, IPOINT_AFTER, (AFUNPTR)FI_InjectFault_MEM_ECC,
				IARG_ADDRINT, INS_Address(ins),
				IARG_MEMORYREAD_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_UINT32,num,
				IARG_UINT32,mode,
				IARG_END);
	}*/

	if (INS_IsMemoryRead(ins)){
		INS_InsertPredicatedCall(
				ins, IPOINT_BEFORE, (AFUNPTR)FI_InjectFault_MEM_ECC,
				IARG_ADDRINT, INS_Address(ins),
				IARG_MEMORYREAD_EA,
				IARG_MEMORYREAD_SIZE,
				IARG_UINT32,num,
				IARG_BOOL,mode,
				IARG_END);
	}

}


VOID instruction_Instrumentation(INS ins, VOID *v){
	// decides where to insert the injection calls and what calls to inject
  if (!isValidInst(ins))
    return;
	
	int numW = INS_MaxNumWRegs(ins), randW = 0;
	UINT32 index = 0;
	REG reg;
  
#ifdef INCLUDEALLINST	
  int mayChangeControlFlow = 0;
        if(!INS_HasFallThrough(ins))
			mayChangeControlFlow = 1;
		for(int i =0; i < numW; i++){
			reg = INS_RegW(ins, i);
			if(reg == REG_RIP || reg == REG_EIP || reg == REG_IP) // conditional branches
			{	mayChangeControlFlow = 1; break;}
		}
        if(numW > 1)
			randW = random() % numW;
        if(numW > 1 && (reg == REG_RFLAGS || reg == REG_FLAGS || reg == REG_EFLAGS))
           randW = (randW + 1) % numW; 
		if(numW > 1 && REG_valid(INS_RegW(ins, randW)))
            reg = INS_RegW(ins, randW);
        else
            reg = INS_RegW(ins, 0);
        if(!REG_valid(reg))
            return;
        index = reg_map.findRegIndex(reg);
        LOG("ins:" + INS_Disassemble(ins) + "\n"); 
		LOG("reg:" + REG_StringShort(reg) + "\n");
		
    // FIXME: INCLUDEINST is not used now. However, if you enable this option
    // in the future, you need to change the code below. If it changes the 
    // control flow, you need to inject fault in the read register rather than
    // write register
        if(mayChangeControlFlow)
			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)inject_CCS,
					IARG_ADDRINT, INS_Address(ins),
					IARG_UINT32, index,	
					IARG_CONTEXT,
					IARG_END);		
		else
			INS_InsertPredicatedCall(
					ins, IPOINT_AFTER, (AFUNPTR)inject_CCS,
					IARG_ADDRINT, INS_Address(ins),
					IARG_UINT32, index,	
					IARG_CONTEXT,
					IARG_END);
#else


#ifdef NOBRANCHES
  if(INS_IsBranch(ins) || !INS_HasFallThrough(ins)) {
    //LOG("faultinject: branch/ret inst: " + INS_Disassemble(ins) + "\n");
		return;
  }
#endif

// NOSTACKFRAMEOP must be used together with NOBRANCHES, IsStackWrite 
// has a bug that does not put pop into the list
#ifdef NOSTACKFRAMEOP
  if(INS_IsStackWrite(ins) || OPCODE_StringShort(INS_Opcode(ins)) == "POP") {
    //LOG("faultinject: stack frame change inst: " + INS_Disassemble(ins) + "\n");    
    return;
  }
#endif

#ifdef ONLYFP
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




      if(numW > 1)
			  randW = random() % numW;
      else
        randW = 0;

// Jiesheng
      reg = INS_RegW(ins, randW);
#ifdef ONLYFP
    while (!reg_map.isFloatReg(reg)) {
      randW = (randW + 1) % numW;
      reg = INS_RegW(ins, randW);
    }
#endif

  if(numW > 1 && (reg == REG_RFLAGS || reg == REG_FLAGS || reg == REG_EFLAGS))
           randW = (randW + 1) % numW; 
		if(numW > 1 && REG_valid(INS_RegW(ins, randW)))
            reg = INS_RegW(ins, randW);
        else
            reg = INS_RegW(ins, 0);
        if(!REG_valid(reg)) {

            LOG("REGNOTVALID: inst " + INS_Disassemble(ins) + "\n");
            return;
      }
        index = reg_map.findRegIndex(reg);
        LOG("ins:" + INS_Disassemble(ins) + "\n"); 
		LOG("reg:" + REG_StringShort(reg) + "\n");

// Jiesheng Wei
	if (reg == REG_RFLAGS || reg == REG_FLAGS || reg == REG_EFLAGS) {
		INS next_ins = INS_Next(ins);
		if (INS_Valid(next_ins) && INS_Category(next_ins) == XED_CATEGORY_COND_BR) {
      //LOG("inject flag bit:" + REG_StringShort(reg) + "\n");
			
      UINT32 jmpindex = jmp_map.findJmpIndex(OPCODE_StringShort(INS_Opcode(next_ins)));
			INS_InsertPredicatedCall(ins, IPOINT_AFTER, (AFUNPTR)FI_InjectFault_FlagReg,
						IARG_INST_PTR,
						IARG_UINT32, index,
						IARG_UINT32, jmpindex,
						IARG_CONTEXT,
						IARG_END);
			return;
		} else if (INS_IsMemoryWrite(ins)) {
        LOG("COMP2MEM: inst " + INS_Disassemble(ins) + "\n");
				
        INS_InsertPredicatedCall(
								ins, IPOINT_BEFORE, (AFUNPTR)FI_InjectFault_Mem,
								IARG_ADDRINT, INS_Address(ins),
								IARG_MEMORYREAD_EA,							
								IARG_MEMORYREAD_SIZE,
								IARG_END);
        return;
    
    } else {
      LOG("NORMAL FLAG REG: inst " + INS_Disassemble(ins) + "\n");
    }

	}



	    INS_InsertPredicatedCall(
					ins, IPOINT_AFTER, (AFUNPTR)inject_CCS,
					IARG_ADDRINT, INS_Address(ins),
					IARG_UINT32, index,	
					IARG_CONTEXT,
					IARG_END);		
#endif        

}

VOID get_instance_number(const char* fi_instcount_file)
{
	FILE *fi_input_FILE = fopen(fi_instcount_file, "r");
    activationFile = fopen(fi_activation_file.Value().c_str(), "a");
    char line_buffer[FI_MAX_CHAR_PER_LINE];
	char *word = NULL;
	char *brnode = NULL;
	char *temp = NULL;
	UINT32 index = 0;
	if(fi_input_FILE == NULL) {
		fprintf(stderr, "ERROR, can not open Instruction count file %s, use -fi_function to specify a valid one\n", 
		fi_instcount_file);
		exit(1);
	}
	if(!(fioption.Value() == CCS_INST || fioption.Value() == FP_INST || fioption.Value() == SP_INST || fioption.Value() == ALL_INST)){
		fprintf(stderr, "ERROR, Specify one of valid options\n");
		exit(1);
	}
		
	while(fgets(line_buffer,FI_MAX_CHAR_PER_LINE,fi_input_FILE) != NULL){
		if(line_buffer[0] == '#') //only accept comments that start a new line
			continue;
		
		word=strtok_r(line_buffer,":", &brnode);
		index=0;
		while(word != NULL){
			switch(index){
			case 0:
				temp = word;
				break;
			case 1:
				if(strcmp(temp, fioption.Value().c_str()) == 0) 
					total_num_inst = atol (word);
				break;
			default:
				fprintf(stderr, "Too many argument in the line\n");
				exit(1);
			}
			index++;		
			word=strtok_r(NULL,":",&brnode);	
		}
				
		//assert((index == 2 || index == 0) && "Too few arguments in the line");
	}
	//PRINT_MESSAGE(4, ("Num Insts:%llu\n",total_num_inst)); 
	//srand((unsigned)time(0)); 
	unsigned int seed;
	FILE* urandom = fopen("/dev/urandom", "r");
	fread(&seed, sizeof(int), 1, urandom);
	fclose(urandom);
	srand(seed);
	fi_inject_instance = random() / (RAND_MAX * 1.0) * total_num_inst;
	//PRINT_MESSAGE(4, ("Instance:%llu\n",fi_inject_instance));
	fclose(fi_input_FILE);	
}

VOID Fini(INT32 code, VOID *v)
{
	if(!activated){
		fprintf(activationFile, "Not Activated!\n");
		fclose(activationFile);
	}
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool does fault injection\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[])
{
	PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();
  


  configInstSelector();


	get_instance_number(instcount_file.Value().c_str());

	if (!fiecc.Value())
	INS_AddInstrumentFunction(instruction_Instrumentation, 0);

	else
		INS_AddInstrumentFunction(instruction_InstrumentationECC, 0);

	PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}

