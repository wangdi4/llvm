if (NOT INCLUDE_MIC_DEVICE)
    message (FATAL_ERROR "Entered this file without INCLUDE_MIC_DEVICE ON")
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  message (FATAL_ERROR "knf is not supported by 32bit architechtures")	
else (CMAKE_SIZEOF_VOID_P EQUAL 4)
  create_target ( knf b1 __MIC__ " ")
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

