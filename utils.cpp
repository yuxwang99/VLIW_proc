#include "utils.h"

using namespace std;
bool isNumeric(std::string const &str)
{
    bool isnum = !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    bool ishexnum = str[0]=='0'&str[1]=='x';
    return isnum|ishexnum;
}

string findRegister(std::string const &str)
{
    string register_id = "";
    bool start = false;
    for(int i=0; i < str.size(); i++){
        string character;
        character = str.substr(i,1);
        
        if(character[0]=='x'){
            start = true;
        
        }
        if (start & isNumeric(character)){
            register_id += str[i];
        }
    }
    return register_id;
}