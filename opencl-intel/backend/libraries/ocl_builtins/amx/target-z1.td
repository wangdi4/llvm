include "amx/amx.td"

def target : AMX {
  let Target = "z0"; // Use AVX512's ocl_svml lib for now
}
