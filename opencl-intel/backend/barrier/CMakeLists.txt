set( TARGET_NAME LLVMBarrier)

file(GLOB_RECURSE headers *.h)

include_directories(BEFORE "include")
include_directories(
    ${BACKEND_ROOT_DIR}/../cl_api 
    ${BACKEND_ROOT_DIR}/utils
    )
    
add_library( ${TARGET_NAME}
  BarrierInFunctionPass.cpp
  BarrierMain.cpp
  BarrierPass.cpp
  BarrierUtils.cpp
  DataPerBarrierPass.cpp
  DataPerInternalFunctionPass.cpp
  DataPerValuePass.cpp
  ImplicitGIDPass.cpp
  RedundantPhiNodePass.cpp
  RemoveDuplicationBarrierPass.cpp
  SplitBBonBarrierPass.cpp
  WIRelatedValuePass.cpp
  ${headers}
)


