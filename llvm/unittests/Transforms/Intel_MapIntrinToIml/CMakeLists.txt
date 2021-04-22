set(LLVM_LINK_COMPONENTS
  Intel_MapIntrinToIml
  )

add_llvm_unittest(Intel_MapIntrinToImlTests
  ImlAttrTest.cpp)
