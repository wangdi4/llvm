
__kernel void divBranchNestedUnLoop (__global int * res,
                    const int num) {
  int id = get_global_id(0);

  if (id%2 == 0) { // non uniform branch
    for (int i=0; i < num; ++i) {  // uniform loop
        res[id] = 7; 
    }
  }
  else {
    res[id] = 8;
  }
}

__kernel void divBranchNestedUnBranch (__global int * res,
                    const int num) {
  int id = get_global_id(0);

  if (id%2 == 0) {
    if (num > 10)
      res[id] = 7; 
  }
  else {
    res[id] = 8;
  }
}

__kernel void divBranchNestedUnBranchI(__global int * res,
                    const int num) {
  int id = get_global_id(0);

  if (id%2 == 0) {
    if (num > 10)
      res[id] = 7;
    else
      res[id] = 9;
  }
  else {
    res[id] = 8;
  }
}

__kernel void divBranchedUnBranch (__global int * res,
                    const int num) {
  int id = get_global_id(0);

  if (id%2 == 0) { // non uniform branch
    res[id] = 8;
  }
  
  if (num%2 == 0) { // uniform branch
    res[id] = 7;
  }
}

__kernel void nestedUnBranchedUnBranchDivBranch (__global int * res,
                    const int num) {
  int id = get_global_id(0);

  if (num%2 == 0) { // uniform branch
    if (num > 9) { // uniform branch
      if (id%2 == 0) { // non uniform branch
        res[id] = 8;
      }    
    }
  }
}

