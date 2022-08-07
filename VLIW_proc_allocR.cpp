#include "VLIW_proc.h"
#include "utils.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <map> 
#include <ctype.h>
using namespace std;

vector<string> VLIW_proc::allocR(){
    // vector
    results = vector<vector<string> >(scheduleTable.size(), vector<string>(5, "nop"));
    cout << left << "  " << setw(15) <<right<<endl;
    map<string, string> renameTable;
    vector<string> newMovOP;
    for(int i=0;i<scheduleTable.size();i++){
        
        for(int j=0;j<5;j++){
            int ins = scheduleTable[i][j];
            string op0, op1;
            if(ins<0) ; // nop, skip
            // specital register for loop, no need to rename
            else if(dst[ins]=="LC" | action[ins]=="loop"){
                string final_ins = action[ins]+" "+dst[ins]+" "+ operator0[ins];
                results[i][j] = final_ins;
            } 
            else{
                if(!isNumeric(operator0[ins]) && operator0[ins]!=""){
                    op0 = "x" + findRegister(operator0[ins]);
                    operator0[ins] = renameTable[op0];
                }
                if(!isNumeric(operator1[ins]) && operator1[ins]!=""){
                    op1 = "x" + findRegister(operator1[ins]);
                    operator1[ins] = renameTable[op1];
                }
                if (findRegister(dst[ins])!=""){
                    string dstreg = "x" + findRegister(dst[ins]);
                    string oldReg = renameTable[dst[ins]];
                    renameTable[dst[ins]] = "x" + to_string(freeReg.back());
                    if(dstreg==op0||dstreg==op1){
                        string movOP = "mov " + oldReg + " " + renameTable[dst[ins]]+ " " ;
                        newMovOP.push_back(movOP);
                    }
                    dst[ins] = "x" + to_string(freeReg.back());
                }

                freeReg.pop_back();
                string final_ins = action[ins]+" "+dst[ins]+" "+ operator0[ins]+" "+ operator1[ins];
                results[i][j] = final_ins;
            }
            op0 = op1 = "";
            cout << left << results[i][j]<< setw(15) <<right;
        }
        cout << std::endl;
    }

    return newMovOP;
}