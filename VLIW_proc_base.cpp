#include "VLIW_proc.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <map> 
#include <ctype.h>
using namespace std;
 
VLIW_proc::VLIW_proc(string jsonPath1){
      jsonPath = jsonPath1;
      int reg = 32;
      while(reg--){
        freeReg.push_back(reg);
      }
      mapOP2HW["add"] = "ALU";
      mapOP2HW["addi"] = "ALU";
      mapOP2HW["sub"] = "ALU";
      mapOP2HW["mov"] = "ALU";
      mapOP2HW["mulu"] = "Mul";
      mapOP2HW["ld"] = "Mem";
      mapOP2HW["st"] = "Mem";
      mapOP2HW["loop"] = "Branch";
    }

void VLIW_proc::scheduleVLIW(){
      
      analysizeCode();
      splitBB();
      dependencyAnalysize();
      scheduleCode();
      int validII = computeII(II);
      cout << "success!" << endl;
      if(II == validII){
        cout << "II is valid, no need for rescheduling" << endl;
      }
      else{
        rescheduleCode(validII);
        II = validII;
      }
      vector<insertIns> movOP = allocR();
      reArrangeR(movOP);
    }

vector<string> VLIW_proc::decodeIns(string instruction){
      vector<string> operation(4);
      string word;
      instruction = instruction + ' ';
      int ind = 0;
      for (auto s : instruction){
        if (s==' '){
          operation[ind] = word;
          ind += 1;
          word = "";
        }
        else{
          if(isdigit(s)|isalpha(s)|s=='('|s==')'){
            word = word + s;
          }
        }
      }
      return operation;
    }

void VLIW_proc::readFromStream(){
        ifstream ifs(jsonPath.c_str());
        Json::Reader reader;

        bool ret = reader.parse(ifs,insArray);
        if(ret == false){
          std::cout<<"Parse ReadConfig() Json data error."<<std::endl;
          ifs.close();
          return;
        }
        ifs.close();
        std::cout<<"READING INSTRUCTIONS:"<<std::endl;
        for (int i = 0; i < insArray.size(); i++) {
            std::cout << insArray[i].asString() << std::endl;
          }
          cout<<'\n';
        std::cout << "----------------" << std::endl;
        numIns = insArray.size();
    }

void VLIW_proc::analysizeCode(){
      vector<string> insCode(4);
      int ins_id;
      string ins_id_str;
      for (int i=0; i < insArray.size(); i++){
        insCode = decodeIns(insArray[i].asString());
        action.push_back(insCode[0]);
        dst.push_back(insCode[1]);
        operator0.push_back(insCode[2]);
        operator1.push_back(insCode[3]);
        ins_id =i;
        Id.push_back(ins_id);
      }
      cout << "----------------" << std::endl;
      cout<<"COMPILING CODE..."<<'\n';
      cout << "ins_id" << setw(10) 
      << "operation" << setw(10)
      << "reg_dst" << setw(15)
      << "operator_0" << setw(15) 
      << "operator_1"<< setw(10)<< "\n";
      for (int i = 0; i < insArray.size(); i++) {
            std::cout << Id[i] << setw(10)
            << action[i] << setw(10) 
            << dst[i] << setw(15)
            << operator0[i] << setw(15) 
            << operator1[i]<<setw(10) << "\n";
          }
    }

void VLIW_proc::splitBB(){
      for(int i=0; i < numIns; i++){
        if (action[i]=="loop"){
          BB0 = stoi(dst[i]);
          BB1 = i+1;
        }
      }
      std::cout << "----------------" << std::endl;
      cout<<"ANALYZING BRANCH..."<<'\n';
      cout<<" Init branch index:"<<'\n';
      cout<<" 0 to "<<BB0-1<<'\n';
      cout<<" Loop branch index:"<<'\n';
      cout<<" "<<BB0<<" to "<<BB1-1<<'\n';
      cout<<" Finalize branch index:"<<'\n';
      cout<<" "<<BB1<<" to "<<insArray.size()<<'\n';
    }

int VLIW_proc::computeII(int initII){
      int exam_ins_pos, dep_ins_pos;
      bool incrementII = false;
      cout << "----------------" << std::endl;
      cout << "Test and check initial interval(II): " << initII  << std::endl;
      for (auto interdep : depInter){
        int exam_ins = interdep.first;
        exam_ins_pos = timeSchedule[exam_ins];
        
        for (auto dep_ins: interdep.second){
          int latency = (action[dep_ins]=="mulu")? 3:1;
          dep_ins_pos = timeSchedule[dep_ins];
          // If dependent instruction is namly after current examing instruction, 
          // it is the last loop results that brought dependency
          if(dep_ins_pos>=exam_ins_pos){
            if(dep_ins_pos+latency>exam_ins_pos+initII){
              initII++;
              cout << "Invalid" << std::endl;
              initII = computeII(initII);
              incrementII = true;
              break;
            }
          }
          // If interloop dependency is caused by instruction in BB0, check the latency directly
          else{
            if(dep_ins_pos+latency>exam_ins_pos){
              initII++;
              cout << "Invalid" << std::endl;
              initII = computeII(initII);
              incrementII = true;
              break;
            }
          }
          
        }
        if(incrementII) break;
      }

      
      return initII;
    }

void VLIW_proc::reArrangeR(vector<insertIns> newIns){
  
  int numIns = newIns.size()-1;
  // Check whether there is enough ALU units 
  // for register renaming
  int arrangeLoc = VLIW_BB1-1;
  int validII = II;
  while(numIns>=0){
    struct insertIns scheduleins=newIns[numIns];
    if(arrangeLoc>scheduleins.loc){
      if(results[arrangeLoc][0]=="nop"){
        results[arrangeLoc][0] = scheduleins.ins;
      }
      else if(results[arrangeLoc][1]=="nop"){
        results[arrangeLoc][1] = scheduleins.ins;
        arrangeLoc--;
      }
      else{
        validII++;
      }
      numIns--;
    }
  }
  if(validII!=II){
    rescheduleCode(validII);
    reArrangeR(newIns);
  }
  // for(int )

}
void VLIW_proc::printResult(){
  cout << left << "  " << setw(15) <<right<<endl;
  for(int i=0;i<results.size();i++){
    for(int j=0; j<5; j++){
       cout << left << results[i][j]<< setw(15) <<right;
    }
    cout << std::endl;
  }
}