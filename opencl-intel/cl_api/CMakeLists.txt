file (GLOB CL_API_HEADER_FILES ${CL_API_HEADERS}/CL/*.h ${CL_API_HEADERS}/CL/*.hpp)
install_to (${CL_API_HEADER_FILES} DESTINATION include/CL COMPONENT ocl-api-headers)

if (NOT WIN32)
  set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)
