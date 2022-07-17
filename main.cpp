#include "VLIW_proc.h"

int main(){
  char jsonfile_path[] = "instructions.json";
  string path_ = jsonfile_path;
  VLIW_proc proc(path_);
  proc.readFromStream();
  proc.scheduleVLIW();
  return 0;
}
