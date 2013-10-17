set(TARGET_NAME BuiltinLibInfo)

include_directories(BEFORE ".")
include_directories(BEFORE "Driver")
include_directories(BEFORE "Driver/OpenCL")

include_directories(
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/utils
  ${CMAKE_SOURCE_DIR}/cl_api
)

if(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  Driver/Apple/AppleOCLRuntime.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES 
  Driver/Apple/AppleOCLRuntime.h
)
else(APPLE)
set(TARGET_EXTRA_SOURCE_FILES 
  Driver/Volcano/VolcanoOpenclRuntime.cpp
#These files are needed only for building opt
  Driver/Apple/AppleOCLRuntime.cpp
#These files are needed only for building opt (to run dx LIT tests)
  Driver/DX/DXData.cpp
  Driver/DX/DXRuntime.cpp
  Driver/DX/DXWrapperParamsCheck.cpp
  Driver/Functions.cpp
)
set(TARGET_EXTRA_INCLUDE_FILES 
  Driver/Volcano/VolcanoOpenclRuntime.h
#These files are needed only for building opt
  Driver/Apple/AppleOCLRuntime.h
#These files are needed only for building opt (to run dx LIT tests)
  Driver/DX/DXRuntime.h
  Driver/DX/DXWrapperParamsCheck.h
  Driver/Functions.h
)
endif(APPLE)

set(TARGET_INCLUDE_FILES
  BuiltinLibInfo.h
  RuntimeServices.h
  VectorizerFunction.h
  Driver/OpenCL/OpenclRuntime.h
  ${TARGET_EXTRA_INCLUDE_FILES}
)

set(TARGET_SOURCE_FILES
  BuiltinLibInfo.cpp
  Driver/OpenCL/OpenclRuntime.cpp
  ${TARGET_EXTRA_SOURCE_FILES}
)


add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
