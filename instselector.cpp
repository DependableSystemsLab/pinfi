#include "pin.H"

static std::set<string> includeinst;
static std::set<string> excludeinst;
static const string CMP = "cmp";
static const string ALL = "all";
static const string LOAD = "load";
static const string STORE = "store";

static string configfile = "pin.instselector.config.txt"


bool _isLoadInst(INS ins) {
  return INS_IsMemoryRead(ins);  
}

bool _isStoreInst(INS ins) {
  return INS_IsMemoryWrite(ins);  
}

bool _isCmpInst(INS ins) {
  return INS_Valid(INS_Next(ins)) &&
         INS_Category(INS_Next(ins)) == XED_CATEGORY_COND_BR;
}

bool _isCmpIncluded() {
  return includeinst.find(CMP) != includeinst.end() ||
         (includeinst.find(ALL) != includeinst.end() &&
          excludeinst.find(CMP) != excludeinst.end())
}

void configInstSelector() {
  std::ifstream ifs;
  ifs.open(configfile, std::ifstream::in);

  if (!ifs.bad()) {
    unsigned line_num = 0;

    // first line include
    while (!ifs.eof()) {
      char line[1000];
      ifs.getline(line, 1000);
      // starting with '#' means comment
      if (line[0] == '#')
        continue;

      string line_str(line);
      string inst_arr[100];
      UINT32 ret = Tokenize(line_str, line_arr, 100);
      std::err << "ret of Tokenize " << ret << endl;

      if (line_num == 0) {
        for (int i = 0; i < 100; i++) {
          if (line_arr[i] != "") {
            includeinst.insert(line_arr[i]);
          }
        }
      } else if (line_num == 1) {
        for (int i = 0; i < 100; i++) {
          if (line_arr[i] != "") {
            excludeinst.insert(line_arr[i]);
          }
        }
      }

      line_num++;
    }

    ifs.close();
  } else {
    includeinst.insert(ALL);
  }

// for debug
  std::cerr << "include " << endl;
  for (std::set<string>::const_iterator it = includeinst.begin();
       it != includeinst.end(); ++it) {
    std::cerr << *it << endl;  
  }
  std::cerr << "exclude " << endl;
  for (std::set<string>::const_iterator it = excludeinst.begin();
       it != excludeinst.end(); ++it) {
    std::cerr << *it << endl;  
  }
}


bool isInstFITarget(INS ins) {
  bool ret = false;

  // check include
  if (includeinst.find(ALL) != includeinst.end()) {
    ret = true;  
  } else {
    if (includeinst.find(LOAD) != includeinst.end() &&
        _isLoadInst(ins)) {
      ret = true;
    } else if (includeinst.find(STORE) != includeinst.end() &&
              _isStoreInst(ins)) {
      ret = true; 
    } else if (includeinst.find(CATEGORY_StringShort(INS_Category(ins))) != includeinst.end() ||
              includeinst.find(INS_Mnemonic(ins) != includeinst.end())) {
      ret = true;  
    }
  }

  if (excludeinst.find(LOAD) != excludeinst.end() &&
      _isLoadInst(ins)) {
    ret = false;  
  } else if (excludeinst.find(STORE) != excludeinst.end() &&
             _isStoreInst(ins)) {
    ret = false;  
  } else if (excludeinst.find(CATEGORY_StringShort(INS_Category(ins))) != excludeinst.end() ||
            excludeinst.find(INS_Mnemonic(ins)) != excludeinst.end()) {
    ret = false;  
  }

  // cmp inst is treated differently because cmp inst and other categories are not mutually exclusive
  if (_isCmpIncluded()) {
    if (_isCmpInst(ins))
      ret = true;
  } else {
    if (_isCmpInst(ins))
      ret = false;
  }

// debug
  if (ret) {
    std::cerr << "instruction to be included " << INS_Disassemble(ins) << endl;
  }

  return ret;
}
