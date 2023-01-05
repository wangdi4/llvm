__kernel void copy(__global float *a, __global float *b) {
#ifdef MyNameIsGladiator
  printf("Gladiator\n");
#else
  printf("Comodus\n");
#endif
}
