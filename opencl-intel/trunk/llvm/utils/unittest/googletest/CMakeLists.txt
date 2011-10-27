add_definitions(-I${CMAKE_CURRENT_SOURCE_DIR}/include -DGTEST_HAS_RTTI=0 -DGTEST_HAS_TR1_TUPLE=0)
if (CMAKE_COMPILER_IS_GNUCXX)
add_definitions(-Wno-missing-field-initializers -Wno-variadic-macros )
endif (CMAKE_COMPILER_IS_GNUCXX)

set (GT_INCLUDE_FILES
	"include/gtest/gtest.h"
	"include/gtest/gtest-death-test.h"
	"include/gtest/gtest-message.h"
	"include/gtest/gtest-param-test.h"
	"include/gtest/gtest-spi.h"
	"include/gtest/gtest-test-part.h"
	"include/gtest/gtest-typed-test.h"
	"include/gtest/gtest_pred_impl.h"
	"include/gtest/gtest_prod.h"
  )

add_llvm_library(GoogleTest
  ${GT_INCLUDE_FILES}
  gtest.cc 
  gtest-death-test.cc 
  gtest-filepath.cc 
  gtest-port.cc 
  gtest-test-part.cc 
  gtest-typed-test.cc 
  )
