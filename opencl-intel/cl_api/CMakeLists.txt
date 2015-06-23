file(GLOB EXTERNAL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CL/*.h*)
set( OCL_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CL)

install( DIRECTORY CL/ DESTINATION include/CL/ PATTERN *.h* )

if (NOT WIN32)
  # disable warnings inherited from external componennts
  add_definitions(-Wno-attributes)
  add_definitions(-Wno-missing-braces)
endif (NOT WIN32)
