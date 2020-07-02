set(TARGET_NAME "ImathLibd")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(TARGET_SOURCE_FILES
  dllmain.cpp
  imathLibd.cpp)

add_library(${TARGET_NAME} SHARED ${TARGET_SOURCE_FILES})
target_compile_options(${TARGET_NAME} PUBLIC -U__MMX__)
target_link_options(${TARGET_NAME} PUBLIC "-Wl,-rpath=\\$$ORIGIN")
