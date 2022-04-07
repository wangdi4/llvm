set(TINYXML_HEADERS tinystr.h tinyxml.h)

add_definitions(-DTIXML_USE_STL)

# disable "implicit-fallthrough" warning in TiXmlBase::ConvertUTF32ToUTF8()
if (NOT WIN32)
  add_compile_options(-Wno-implicit-fallthrough -Wno-suggest-override)
endif (NOT WIN32)

add_opencl_library(tinyxml_STL
  ${TINYXML_HEADERS}
  tinystr.cpp 
  tinyxml.cpp 
  tinyxmlerror.cpp 
  tinyxmlparser.cpp 
)
