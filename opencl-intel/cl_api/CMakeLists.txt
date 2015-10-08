message( STATUS ${CL_API_HEADERS} )
install( DIRECTORY ${CL_API_HEADERS}/CL/ DESTINATION include/CL/ PATTERN ".git" EXCLUDE PATTERN *.h )

if (NOT WIN32)
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)
