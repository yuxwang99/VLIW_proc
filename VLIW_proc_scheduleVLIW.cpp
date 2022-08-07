#include "VLIW_proc.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <map> 
#include <ctype.h>

void VLIW_proc::scheduleCode(){
      int scheduleIns = 0;
      string op, desireHW;
      
      
      // table used to record the available hardware 
      map<string, int> availHW;
      availHW["ALU0"] = 0;
      availHW["ALU1"] = 0;
      availHW["Mul"] = 0;
      availHW["Mem"] = 0;
      availHW["Branch"] = 0;
      // fence every basic branch
      while(scheduleIns<numIns){
        op = action[scheduleIns];
        desireHW = mapOP2HW[op];
        if(scheduleIns==BB0 | scheduleIns==BB1){
          auto fence = max_element(availHW.begin(), availHW.end(), [](const auto &x, const auto &y) {
                    return x.second < y.second;
                });
          for (auto HW : availHW){
            availHW[HW.first] = fence->second;
          }
        }
        if(scheduleIns==BB1-1) availHW["Branch"] = scheduleTable.size()-1;
      // assign available hardware that solve dependency
      if(depLocal.count(scheduleIns)){
          vector<int> scheduled_ins = depLocal[scheduleIns];
          int latency;
          for (int ins : scheduled_ins){
            latency = (action[ins]=="mulu")? 3:1;
            // the needable hardware resource
            
            availHW[desireHW] = (availHW[desireHW]>=timeSchedule[ins]+latency) ? 
                            availHW[desireHW]: latency+timeSchedule[ins];
          }
        }

        if(depInvar.count(scheduleIns)){
          vector<int> scheduled_ins = depLocal[scheduleIns];
          int latency;
          for (int ins : scheduled_ins){
            latency = (action[ins]=="mulu")? 3:1;
            // the needable hardware resource
            availHW[desireHW] = (availHW[desireHW]>=timeSchedule[ins]+latency) ? 
                            availHW[desireHW]: latency+timeSchedule[ins];
          }
        }
        if(depPost.count(scheduleIns)){
          vector<int> scheduled_ins = depPost[scheduleIns];
          int latency;
          for (int ins : scheduled_ins){
            latency = (action[ins]=="mulu")? 3:1;
            // the needable hardware resource
            availHW[desireHW] = (availHW[desireHW]>=timeSchedule[ins]+latency) ? 
                            availHW[desireHW]: latency+timeSchedule[ins];
          }
        }
      // assign available hardware ASAP
        if(desireHW=="ALU"){
          // If instruction can be execuated by ALU1 early, then 
          // assign ALU1 for this instruction
          if(availHW["ALU1"]<availHW["ALU0"]){
            while(scheduleTable.size()<availHW["ALU1"]+1){
              scheduleTable.push_back(emptyHW);
            }
            scheduleTable[availHW["ALU1"]][1] = scheduleIns;
            timeSchedule[scheduleIns] = availHW["ALU1"];
            availHW["ALU1"]++;
          }
          else{
            // Assign earliest ALU0 for this instruction
            while(scheduleTable.size()<availHW["ALU0"]+1){
              scheduleTable.push_back(emptyHW);
            }
            scheduleTable[availHW["ALU0"]][0] = scheduleIns;
            timeSchedule[scheduleIns] = availHW["ALU0"];
            availHW["ALU0"]++;
          }
          
        }
        else{
          // Assign other hardware resource
          while(scheduleTable.size()<availHW[desireHW]+1){
              scheduleTable.push_back(emptyHW);
            }
          int index;
          if (desireHW=="Mul") index = 2;
          if (desireHW=="Mem") index = 3;
          if (desireHW=="Branch") index = 4;

          scheduleTable[availHW[desireHW]][index] = scheduleIns;
          timeSchedule[scheduleIns] = availHW[desireHW];
          availHW[desireHW]++;
        }

        // loop start address of VLIW changed
        if (scheduleIns==BB0) {
          VLIW_BB0 = timeSchedule[scheduleIns];
          dst[BB1-1]= to_string(VLIW_BB0);
        }
        if(scheduleIns==BB1){
          VLIW_BB1 = timeSchedule[scheduleIns];
          II = VLIW_BB1-VLIW_BB0;
        }
        // next instruction
        scheduleIns++;

      }
      cout<< "CODE SCHEDULING..." << '\n';
      cout<< " " << left << "|ALU0" << setw(11) << right 
          <<  "|ALU1" << setw(11)
          <<  "|Mult" << setw(10)
          <<  "|Mem" << setw(14)
          <<  "|Branch" << setw(5)  << "|" << '\n';
      int i = 0;
      for (auto time : scheduleTable){
        cout<< i <<"|" << setw(10) ;
        for(int used: time){
          if(used>=0) {
            cout << left<< setw(10)<< used << "|" << right ;
          }
          else{
            cout << left<< setw(10)<< " " << "|" << right;
          }
          
        }
        cout << "\n";
        i++;
      }
    }

void VLIW_proc::rescheduleCode(int validII){
  int gap = validII - II;
  while(gap--){
    scheduleTable.push_back(emptyHW);
  }
  gap = validII - II;
  for(int t=scheduleTable.size()-1;t>=VLIW_BB1+gap;t--){
    scheduleTable[t] = scheduleTable[t-gap];
    scheduleTable[t-gap] = emptyHW;
  }
  scheduleTable[VLIW_BB1-1+gap][4] = scheduleTable[VLIW_BB1-1][4];
  scheduleTable[VLIW_BB1-1][4] = -1;
  VLIW_BB1 = VLIW_BB1 + gap;

  cout<< "NEW SCHEDULING..." << '\n';
  cout<< " " << left << "|ALU0" << setw(11) << right 
      <<  "|ALU1" << setw(11)
      <<  "|Mult" << setw(10)
      <<  "|Mem" << setw(14)
      <<  "|Branch" << setw(5)  << "|" << '\n';
  int i = 0;
  for (auto time : scheduleTable){
    cout<< left << i <<"|" << setw(10)<<right ;
    for(int used: time){
      if(used>=0) {
        cout << left<< setw(10)<< used << "|" << right ;
      }
      else{
        cout << left<< setw(10)<< " " << "|" << right;
      }
      
    }
    cout << std::endl;
    i++;
  }

}