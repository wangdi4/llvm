add_subdirectory( tbb )


#gtest
add_subdirectory( gtest )

if(WIN32)
add_subdirectory( dyn_gtest )
endif(WIN32)

