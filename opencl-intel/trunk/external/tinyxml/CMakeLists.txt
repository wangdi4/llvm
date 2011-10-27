set(TINYXML_HEADERS tinystr.h tinyxml.h)

add_definitions(-DTIXML_USE_STL)

add_llvm_library(tinyxml_STL
  ${TINYXML_HEADERS}
  tinystr.cpp 
  tinyxml.cpp 
  tinyxmlerror.cpp 
  tinyxmlparser.cpp 
)