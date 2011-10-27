file(GLOB_RECURSE headers *.h)
include_directories(BEFORE "include")
include_directories( ../cl_api )

add_llvm_library( LLVMBarrier
  BarrierInFunctionPass.cpp
  BarrierMain.cpp
  BarrierPass.cpp
  BarrierUtils.cpp
  DataPerBarrierPass.cpp
  DataPerInternalFunctionPass.cpp
  DataPerValuePass.cpp
  RedundantPhiNodePass.cpp
  RemoveDuplicationBarrierPass.cpp
  SplitBBonBarrierPass.cpp
  WIRelatedValuePass.cpp
  ${headers}
)
