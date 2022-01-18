// INTEL_COLLAB
#if VALUE==2
// expected-error@+2 {{multiple target regions at same the location is not supported}}
#endif
#pragma omp target
{
  i = i + VALUE;
}
// end INTEL_COLLAB
