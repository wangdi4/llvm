
__kernel void internalDivBranchMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    if (id+i == a[id+i]) {       // non uniform branch
      res[id] = 7; 
      break;
    }
  }
}

__kernel void externalDivBranchMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
       res[id] += 7;
       if (num%49 == 0)
         break; 
  }
}

__kernel void externalDivBranchNestedUnLoopsMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
       res[id] += 7;
       if (num%49 == 0)
         break; 
    }
  }
}

__kernel void externalDivBranchNestedLoopsMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (id == a[id]) {       // non uniform branch
    res[id] = 8;
  }

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < id + 3; ++j) { // non uniform loop
       res[id] += 7;
       if (num%49 == 0)
         break; 
    }
  }
}

__kernel void internalDivBranchNestedUnLoopsMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
      if (id == a[id]) {       // non uniform branch
        res[id] += 8;
        break;
      }
    }
  }
}

__kernel void internalDivBranchNestedLoopsMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < id + 3; ++j) { // non uniform loop
      if (id == a[id]) {       // non uniform branch
        res[id] += 8;
        break;
      }
    }
  }
}

__kernel void internalDivBranchThreeNestedUnLoopsMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < num; ++i) {  // uniform loop
    for (int j=0; j < num + 3; ++j) { // uniform loop
      for (int k=0; k < num + 7; ++k) { // uniform loop
        if (id == a[id]) {       // non uniform branch
          res[id] += 8;
          break;
        }
      }
    }
  }
}

__kernel void internalUnBranchDivLoopMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  for (int i=0; i < id; ++i) { // non uniform loop
    if (num == 7) {       // uniform branch
      res[id] += 8;
      break;
    }
  }
}

__kernel void externalUnBranchDivLoopMX (const __global int * a, 
                    __global int * res,
                    const int num) {

  int id = get_global_id(0);

  if (num == 7) {   // uniform branch
    res[id] += 8;
  }
  
  for (int i=0; i < id; ++i) { // non uniform loop
    res[id] += 6;  
    if (id%5 == 0)
         break; 
  }
}
