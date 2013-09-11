#ifndef FI_CJMP_MAP_H
#define FI_CJMP_MAP_H

#include <map>
#include <assert.h>
#include "pin.H"
#include "stdio.h"

#define MAX_ST_NUM 8

#define CF_BIT 0
#define PF_BIT 2
#define ZF_BIT 6
#define SF_BIT 7
#define OF_BIT 11
#define NA_BIT 100

using namespace std;

class CJmpMap{
public:
	enum JmpType {DEFAULT = 0, USPECJMP = 1, SSPECJMP = 2};
	
	struct Jmp_Info {
		string name;
		enum JmpType type; //types other than DEFAULT needs special operations
		UINT32 inject_bit;
		
		Jmp_Info (string p_name, JmpType p_type, UINT32 p_inject) {
			name = p_name;
			type = p_type;
			inject_bit = p_inject;
		}
	};
	typedef map<UINT32, Jmp_Info*> JmpInfoMap;
public:
	
		CJmpMap(){
    		createMap();
    	}
    	
    UINT32 findJmpIndex(string jmp) {
    		JmpInfoMap::iterator jmp_iter;
			for(jmp_iter = jmp_map.begin() ; jmp_iter !=jmp_map.end() ; jmp_iter++ ) {
				if(jmp_iter -> second -> name == jmp)
					return jmp_iter -> first;
			}
			fprintf(stderr, "ERROR: Jump %s not in the list!\n", jmp.c_str());
			exit(2);
		}
		
		JmpType findJmpType(UINT32 index) {
    	JmpInfoMap::iterator jmp_iter;
      jmp_iter = jmp_map.find(index);
      if ( jmp_iter == jmp_map.end()) {
        fprintf(stderr, "ERROR: Jump index %u not in the list!\n", index);
			  exit(2);
      }
      return jmp_iter -> second -> type;
	
		}
		
		string& findJmpName(UINT32 index) {
    	JmpInfoMap::iterator jmp_iter;
      jmp_iter = jmp_map.find(index);
      if ( jmp_iter == jmp_map.end()) {
        fprintf(stderr, "ERROR: Jump index %u not in the list!\n", index);
			  exit(2);
      }
			return jmp_iter -> second -> name;
		}
				
		UINT32 findInjectBit (UINT32 index) {
      JmpInfoMap::iterator jmp_iter;
      jmp_iter = jmp_map.find(index);
      if ( jmp_iter == jmp_map.end()) {
        fprintf(stderr, "ERROR: Jump index %u not in the list!\n", index);
			  exit(2);
      }

			assert(jmp_iter->second->type == DEFAULT);
			return jmp_iter -> second -> inject_bit;
		}
		
		bool isParityJmp (string jmpname) {
			return (jmpname == "JP") || (jmpname == "JPE") || 
				(jmpname == "JNP") || (jmpname == "JPO");
		}
			
private:
    	void createMap() {
			jmp_map[0] = new Jmp_Info("JO", DEFAULT, OF_BIT);
			jmp_map[1] = new Jmp_Info("JNO", DEFAULT, OF_BIT);
			
			jmp_map[2] = new Jmp_Info("JS", DEFAULT, SF_BIT);
			jmp_map[3] = new Jmp_Info("JNS", DEFAULT, SF_BIT);
			
			jmp_map[4] = new Jmp_Info("JZ", DEFAULT, ZF_BIT);
			jmp_map[5] = new Jmp_Info("JE", DEFAULT, ZF_BIT);
			jmp_map[6] = new Jmp_Info("JNZ", DEFAULT, ZF_BIT);
			jmp_map[7] = new Jmp_Info("JNE", DEFAULT, ZF_BIT);
			
			jmp_map[8] = new Jmp_Info("JB", DEFAULT, CF_BIT);
			jmp_map[9] = new Jmp_Info("JNAE", DEFAULT, CF_BIT);
			jmp_map[10] = new Jmp_Info("JC", DEFAULT, CF_BIT);
			jmp_map[11] = new Jmp_Info("JNB", DEFAULT, CF_BIT);
			jmp_map[12] = new Jmp_Info("JAE", DEFAULT, CF_BIT);
			jmp_map[13] = new Jmp_Info("JNC", DEFAULT, CF_BIT);
			
			jmp_map[14] = new Jmp_Info("JBE", USPECJMP, NA_BIT);
			jmp_map[15] = new Jmp_Info("JNA", USPECJMP, NA_BIT);
			jmp_map[16] = new Jmp_Info("JA", USPECJMP, NA_BIT);
			jmp_map[17] = new Jmp_Info("JNBE", USPECJMP, NA_BIT);
			
			jmp_map[18] = new Jmp_Info("JL", DEFAULT, SF_BIT);
			jmp_map[19] = new Jmp_Info("JNGE", DEFAULT, SF_BIT);
			jmp_map[20] = new Jmp_Info("JGE", DEFAULT, SF_BIT);
			jmp_map[21] = new Jmp_Info("JNL", DEFAULT, SF_BIT);
			
			jmp_map[22] = new Jmp_Info("JLE", SSPECJMP, NA_BIT);
			jmp_map[23] = new Jmp_Info("JNG", SSPECJMP, NA_BIT);
			jmp_map[24] = new Jmp_Info("JG", SSPECJMP, NA_BIT);
			jmp_map[25] = new Jmp_Info("JNLE", SSPECJMP, NA_BIT);
			
			jmp_map[26] = new Jmp_Info("JP", DEFAULT, PF_BIT);
			jmp_map[27] = new Jmp_Info("JPE", DEFAULT, PF_BIT);		
			jmp_map[28] = new Jmp_Info("JNP", DEFAULT, PF_BIT);
			jmp_map[29] = new Jmp_Info("JPO", DEFAULT, PF_BIT);
			
			//DO NOT INCLUDE THE TWO BELOW FOR NOW
			//jmp_map[30] = new Jmp_Info("JCXZ", DEFAULT, NA_BIT);
			//jmp_map[31] = new Jmp_Info("JECXZ", DEFAULT, NA_BIT);
		}
  
    
private:

		JmpInfoMap jmp_map;
};

#endif
