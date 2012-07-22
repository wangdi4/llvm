set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_RTTI 1)

add_tablegen(ocl-tblgen cl_builtins
  OclBuiltinEmitter.cpp
  TableGen.cpp
  )
