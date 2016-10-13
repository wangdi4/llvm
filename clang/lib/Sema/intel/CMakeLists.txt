include_directories(..)
set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangSemaIntel
  SemaCilk.cpp
  SemaIntelAttr.cpp
  )

# INTEL_CUSTOMIZATION
if(MSVC)
  # /bigobj is required for windows builds because of the size of
  # some object files (CastImage for instance)
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj" )
  set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj" )
endif()
# end INTEL_CUSTOMIZATION
