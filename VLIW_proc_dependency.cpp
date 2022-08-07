#include "VLIW_proc.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <map> 
#include <ctype.h>
#include "utils.h" 

void VLIW_proc::dependencyAnalysize(){
      for(int i=0; i < numIns; i++){
        // analysize dependency on consumer

        // const number; no dependency
        if(!isNumeric(operator0[i])){
          if(operator0[i]=="") continue;
           // determine which brands the instruction is in
          int branchStart, branchEnd;
          bool loopIns = false;
          if(i>0 & i<BB0) {
            branchStart=0;
            branchEnd=BB0;
            loopIns = false;
          }
          else if(i>=BB0 & i<BB1){
            branchStart=BB0;
            branchEnd=BB1;
            loopIns = true;
          }
          else{
            branchStart=BB1;
            branchEnd=numIns;
            loopIns = false;
          }
          // find the dependency between register
          string reg_dst = findRegister(dst[i]);
          string reg_op0 = findRegister(operator0[i]);
          string reg_op1 = findRegister(operator1[i]);

          // determine local dependency, find dependent register in local branch
           string producer;
           vector<int> depreg;
          for(int j=branchStart;j<i;j++){
            if (action[j]!="st"){
              producer =findRegister(dst[j]);
            }
            else{
              producer =findRegister(operator0[j]);
            }
            // register that cause dependency cannot be empty
            if(producer!=""){
              if(producer==reg_op0 | producer==reg_op1 | producer==reg_dst){
              depreg.push_back(j);
              }
            }
          }
          if (!depreg.empty() ) {
            depLocal[i]=depreg;
            depreg.clear();
          }
          // determine loop invariant dependency, find dependent register in init branch
          if(loopIns){
            for(int j=0;j<BB0;j++){
              producer =findRegister(dst[j]);
              if(producer!=""){
                if(producer==reg_op0 | producer==reg_op1| producer==reg_dst){
                  // register is used in initial branch, 
                  // determine whether it is changed in following branch
                  bool used = false;
                  for(int reg1=BB0;reg1<BB1;reg1++){
                    if(findRegister(dst[reg1])==producer) used=true;
                  }
                  if(!used){
                    depreg.push_back(j);
                    used=false;
                  }
                }
              }
            }
            if (!depreg.empty() ) {
              depInvar[i]=depreg;
              depreg.clear();
            }
          }

          // determine interloop dependency, find dependent register that cross the branch
          if(loopIns){
            // find interloop dependency between instructions in loop branch
            for(int j=i;j<BB1;j++){
              producer =findRegister(dst[j]);
              // register that cause dependency cannot be empty
              if(producer!=""){
                  if(producer==reg_op0 | producer==reg_op1){
                    depreg.push_back(j);
                  }
              }
            }
            // find interloop dependency in initial branch
            for(int j=0;j<BB0;j++){
              producer =findRegister(dst[j]);
              if(producer!=""){
                if(producer==reg_op0 | producer==reg_op1){
                  // check such dependency is not invariant
                  bool invariant=false;
                  if(depInvar.count(i) > 0){
                    for(auto depins : depInvar[i]){
                      if (j==depins) invariant=true;
                    }
                  }
                  if(!invariant){
                    depreg.push_back(j);
                    invariant=false;
                  }
                }
              }
            }
            if (!depreg.empty() ) {
              depInter[i]=depreg;
              depreg.clear();
              }
          }
          // determine post loop dependency
          if(i>=BB1){
            for(int j=BB0;j<BB1;j++){
              producer =findRegister(dst[j]);
              if(producer!=""){
                if(producer==reg_op0 | producer==reg_op1 | producer==reg_dst){
                  depreg.push_back(j);
                }
              }
            }
            if(!depreg.empty()){
              depPost[i]=depreg;
              depreg.clear();
            }
          }
          
        }
        
      }
      std::cout << "----------------" << std::endl;
      cout<<"ANALYZING DEPENDENCY..."<<'\n';
      
      cout<< "Local dependency analysis" << '\n';
      for (auto ins : depLocal){
        for (auto dep : depLocal[ins.first]){
          std::cout << setw(2) << ins.first << " is locally dependent with " <<  dep  << " in register " << dst[dep] << "\n";
        }
      }
      cout<<"Local dependency analysis done!" << '\n';

      cout<< "Loop invariant dependency analysis" << '\n';
      for (auto ins : depInvar){
        for (auto dep : depInvar[ins.first]){
          std::cout << setw(2)<< ins.first << " is invariantly dependent with " <<  dep  << " in register " << dst[dep] << "\n";
        }
      }
      cout<<"Loop invariant analysis done!" << '\n';

      cout<< "Interloop dependency analysis" << '\n';
      for (auto ins : depInter){
        for (auto dep : depInter[ins.first]){
          std::cout << setw(2)<< ins.first << " is interloop dependent with " <<  dep  << " in register " << dst[dep] << "\n";
        }
      }
      cout<<"Loop invariant analysis done!" << '\n';

      cout<< "Post loop dependency analysis" << '\n';
      for (auto ins : depPost){
        for (auto dep : depPost[ins.first]){
          std::cout << setw(2)<< ins.first << " is post dependent with " <<  dep  << " in register " << dst[dep] << "\n";
        }
      }
      cout<<"Post loop analysis done!" << '\n';
    }