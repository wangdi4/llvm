if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    # ToDo: Do something with the following line.
	create_target ( knl d3 KNLni FALSE )
else ()
        create_target ( skx z0 SKXni TRUE )
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)
