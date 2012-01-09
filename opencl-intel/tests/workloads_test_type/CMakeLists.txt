# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

add_definitions( -D_INTEL_OCL )

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET workloads_test_type )
                
if (WIN32)
	set( UTILS easyBMP AdobePixelBender_CPURuntime loadbalancer tccace_common threads yuv)
else (WIN32)
	set( UTILS easyBMP loadbalancer tccace_common threads yuv)
endif(WIN32)

#compiling workloads list
if (WIN32)
	set( WORKLOADS  ati_binary_search ati_dct ati_mersenne_twister ati_nbody ati_bitonic_sort checkerboard 
                ati_convolution framework_overhead intel_histogram intel_nbody tcc )
else(WIN32)
	set( WORKLOADS  ati_binary_search ati_dct ati_mersenne_twister ati_nbody ati_bitonic_sort  
                ati_convolution framework_overhead intel_histogram tcc )
endif(WIN32)
                
               
foreach( DIR ${UTILS})
	file(GLOB_RECURSE CURR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} wolf/utils/${DIR}/*.cpp wolf/utils/${DIR}/*.h)
	list(APPEND SRCS ${CURR_SRCS})
	list(APPEND HEADERS_DIRECTORIES wolf/utils/${DIR})
endforeach( DIR )

foreach( DIR ${WORKLOADS})
	file(GLOB_RECURSE CURR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} wolf/Workloads/${DIR}/*.cpp wolf/Workloads/${DIR}/*.h)
	list(APPEND SRCS ${CURR_SRCS})
	list(APPEND HEADERS_DIRECTORIES wolf/Workloads/${DIR})
endforeach( DIR )

file(GLOB CURR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} wolf/*.cpp wolf/*.h)
list(APPEND SRCS ${CURR_SRCS})
list(APPEND HEADERS_DIRECTORIES wolf)

list(APPEND HEADERS_DIRECTORIES ../../cl_api   wolf/Workloads/tcc/common)

if (WIN32)
list(APPEND HEADERS_DIRECTORIES wolf/external/vtune/include )
endif (WIN32)

include_directories( ${HEADERS_DIRECTORIES} )
link_directories ( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

add_executable( ${TARGET} ${SRCS} )

target_link_libraries( ${TARGET} intelocl )

if ( WIN32 )
target_link_libraries( ${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/wolf/external/vtune/lib/VtuneApi.lib )
else (WIN32)
target_link_libraries( ${TARGET} rt )
endif( WIN32 )

set( CREATE_DIR_NAME bin/validation/workloads_test_type)

if ( WIN32 )
file(GLOB_RECURSE DLL_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} VtuneApi.dll )
endif( WIN32 )

file(GLOB_RECURSE CP_FILES RELATIVE  ${CMAKE_CURRENT_SOURCE_DIR} *.py *.xml *.xsd )
                                                                 
set( CP_FILES ${CP_FILES} wolf/Workloads/tcc/tcc.cl
                          wolf/Workloads/intel_histogram/wlHistogram.cl
                          wolf/Workloads/ati_nbody/wlATINBody.cl
                          wolf/Workloads/ati_matrix_transpose/wlATIMatrixTrans.cl
                          )
                                                                 
install(FILES ${DLL_FILES} DESTINATION bin)                                                                 
# Copy *.py *.xml *.cl and *.xsd from current directory to appropriate test directory in 'Release' area.
install(FILES ${CP_FILES} DESTINATION ${CREATE_DIR_NAME})
# Copy *.py from ../GeneralScripts directory to appropriate test directory in 'Release' area.
install(FILES ../GeneralScripts/cmk_test_type_runner.py ../GeneralScripts/cmk_xml_entities.py DESTINATION ${CREATE_DIR_NAME})
install(TARGETS ${TARGET} DESTINATION ${CREATE_DIR_NAME})
# Copy *.xml cfg files which defines the options of the test to appropriate test directory in 'Release' area.
install(FILES cfg_win.xml cfg_linux.xml DESTINATION ${CREATE_DIR_NAME}/workloads_benchmarks)
set_target_properties( ${TARGET} PROPERTIES FOLDER "validation/workloads" )
