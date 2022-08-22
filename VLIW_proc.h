// Rectangle.hpp
#ifndef VLIW_PROC_H
#define VLIW_PROC_H
#include "json/json.h"
#include <iostream>
#include <map> 
// #include <ctype.h>

using namespace std;

struct insertIns{
    int loc;
    string ins;
};

class VLIW_proc
{
  private:
    Json::Value insArray; 
    int numIns;
    int BB0;
    int BB1;

    int VLIW_BB0;
    int VLIW_BB1;
    int II; 
    vector<int> Id;
    vector<string> action;
    vector<string> dst;
    vector<string> operator0;
    vector<string> operator1;

    map<int,vector<int> > depLocal;
    map<int,vector<int> > depInter;
    map<int,vector<int> > depInvar;
    map<int,vector<int> > depPost;

    vector<vector<int> > scheduleTable;
    // table used to record when the scheduled instruction will be executed
    // for avoiding dependency 
    vector<vector<string> >results;
    map<int, int> timeSchedule; 
    vector<int> freeReg;
    vector<int> emptyHW = vector<int> (5,-1);

public:
    string jsonPath;
    map<string, string> mapOP2HW;

    VLIW_proc(string jsonPath1);
    vector<string> decodeIns(string instruction);//
    void readFromStream(); //
    void scheduleVLIW();
    void analysizeCode();//
    void splitBB();//
    void scheduleCode();
    void dependencyAnalysize();
    int computeII(int initII);
    void rescheduleCode(int validII);
    vector<insertIns> allocR();
    void reArrangeR(vector<insertIns>);
    void printResult();

};


#endif