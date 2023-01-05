__kernel void clIntelOfflineCompilerTest(__global int *i) {
  int tid = get_global_id(0);
  if (tid == (*i))
    (*i)++;
}