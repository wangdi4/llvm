if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  create_target (corei7-avx g9 avx TRUE)
else ()
  create_target (corei7-avx e9 avx TRUE)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

