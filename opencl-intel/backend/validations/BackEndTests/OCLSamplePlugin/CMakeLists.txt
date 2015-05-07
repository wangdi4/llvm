set(TARGET_NAME OCLSamplePlugin)

include_directories(${BACKEND_ROOT_DIR}/../cl_api)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend)
include_directories(${BACKEND_ROOT_DIR}/ocl_cpu_backend/export ) #for plugins interface


set (SAMPLEPLUGIN_INCLUDE_FILES
    SamplePlugin.h
)

set (SAMPLEPLUGIN_SOURCE_FILES
    dllmain.cpp
    SamplePlugin.cpp
    )

# Create DLL
add_library (${TARGET_NAME} SHARED 
    ${SAMPLEPLUGIN_INCLUDE_FILES}
    ${SAMPLEPLUGIN_SOURCE_FILES}
    )
    
target_link_libraries(${TARGET_NAME} OclBackendUtils)

install( TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION tests/BackEndTests
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION tests/BackEndTests)
    
    
    