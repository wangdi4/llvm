if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  create_target ( core2 v8 ssse3 TRUE )
else ()
  #create_target ( core2 u8 ssse3 TRUE )
  message (FATAL_ERROR "No support for Atom on x64!")
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)


