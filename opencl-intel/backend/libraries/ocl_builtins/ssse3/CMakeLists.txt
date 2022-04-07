if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  create_target ( core2 v8 ssse3 TRUE )
else ()
  create_target ( core2 u8 ssse3 TRUE )
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)


