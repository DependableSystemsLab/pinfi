#include "utils.h"


bool isValidInst(INS ins) {
/**
 * IMPORTANT: This is to make sure fault injections are done at the .text 
 * of the compiled code, instead of at libraries or .init/.fini sections
 */
  if (!RTN_Valid(INS_Rtn(ins))) { // some library instructions do not have rtn !?
    LOG("Invalid RTN " + INS_Disassemble(ins) + "\n");
    return false;
  }
  
  if (!IMG_IsMainExecutable(SEC_Img(RTN_Sec(INS_Rtn(ins))))) {
    //LOG("Libraries " + IMG_Name(SEC_Img(RTN_Sec(INS_Rtn(ins)))) + "\n");
    return false;
  }
  if (SEC_Name(RTN_Sec(INS_Rtn(ins))) != ".text") {
    //LOG("Section: " + SEC_Name(RTN_Sec(INS_Rtn(ins))) + "\n");
    return false;
  }
  std::string rtnname = RTN_Name(INS_Rtn(ins));
  if (rtnname.find("__libc") == 0 || rtnname.find("_start") == 0 ||
      rtnname.find("call_gmon_start") == 0 || rtnname.find("frame_dummy") == 0 ||
      rtnname.find("__do_global") == 0 || rtnname.find("__stat") == 0) {
    return false;
  }
  LOG("Exe " + RTN_Name(INS_Rtn(ins)) + "\n");

	REG reg = INS_RegW(ins, 0);
	if(!REG_valid(reg))
		return false;
  
  return true;
}
