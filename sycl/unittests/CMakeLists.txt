add_custom_target(SYCLUnitTests)
set_target_properties(SYCLUnitTests PROPERTIES FOLDER "SYCL tests")

foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
# INTEL_CUSTOMIZATION
string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
# end INTEL_CUSTOMIZATION
endforeach()

include(AddSYCLUnitTest)

add_subdirectory(misc)
add_subdirectory(pi)
# INTEL_CUSTOMIZATION
# Disabled because of build failures from not passing --gcc-toolchain value
# in command lines. (CMPLRLLVM-21852)
#add_subdirectory(kernel-and-program)
# end INTEL_CUSTOMIZATION
add_subdirectory(queue)
add_subdirectory(scheduler)
add_subdirectory(thread_safety)
