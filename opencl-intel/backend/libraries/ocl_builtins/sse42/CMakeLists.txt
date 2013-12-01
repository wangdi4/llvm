if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  create_target ( corei7 n8 sse42 TRUE )
else ()
  create_target ( corei7 h8 sse42 TRUE )
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)


