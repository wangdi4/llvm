add_llvm_library(LLVMWRegionInfoAnalysis
  WRegionNode.cpp
  WRegion.cpp
  WRegionCollection.cpp
  WRegionInfo.cpp
  WRegionUtils.cpp
  WRegionInfoAnalysis.cpp
)

add_dependencies(LLVMWRegionInfoAnalysis intrinsics_gen)
