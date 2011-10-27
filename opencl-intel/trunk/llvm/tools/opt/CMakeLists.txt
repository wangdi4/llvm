set(LLVM_LINK_COMPONENTS linker bitreader asmparser bitwriter instrumentation scalaropts ipo vectorizer barrier hello)

add_llvm_tool(opt
  AnalysisWrappers.cpp
  GraphPrinters.cpp
  PrintSCC.cpp
  opt.cpp
  )
