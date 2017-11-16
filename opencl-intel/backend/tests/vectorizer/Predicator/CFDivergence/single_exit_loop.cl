
__kernel void internalDivBranch (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    if (id+i == a[id+i]) {       // non uniform branch
      res[id] = 7; 
    }
  }
}

__kernel void externalDivBranch (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
       res[id] += 7; 
  }
}

__kernel void externalDivBranchNestedUnLoops (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
       res[id] += 7;
    }
  }
}

__kernel void externalDivBranchNestedLoops (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < id + 3; ++j) { // non uniform loop
       res[id] += 7;
    }
  }
}

__kernel void internalDivBranchNestedUnLoops (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
      if (id == a[id]) {       // non uniform branch
        res[id] += 8;
      }
    }
  }
}

__kernel void internalDivBranchNestedLoops (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < id + 3; ++j) { // non uniform loop
      if (id == a[id]) {       // non uniform branch
        res[id] += 8;
      }
    }
  }
}

__kernel void internalDivBranchThreeNestedUnLoops (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
      for (int k=0; k < num + 7; ++k) { // uniform loop
        if (id == a[id]) {       // non uniform branch
          res[id] += 8;
        }
      }
    }
  }
}

__kernel void internalUnBranchDivLoop (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < id; ++i) { // non uniform loop
    if (num == 7) {       // uniform branch
      res[id] += 8;
    }
  }
}

__kernel void externalUnBranchDivLoop (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (num == 7) {   // uniform branch
    res[id] += 8;
  }
  
  for (int i=0; i < id; ++i) { // non uniform loop
    res[id] += 6;  
  }
}

