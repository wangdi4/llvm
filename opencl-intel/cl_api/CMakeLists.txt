install( DIRECTORY ${CL_API_HEADERS}/CL/ DESTINATION include/CL/ PATTERN ".git" EXCLUDE PATTERN *.h )

if (NOT WIN32)
  set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)

