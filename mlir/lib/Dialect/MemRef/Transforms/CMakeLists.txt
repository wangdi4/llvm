add_mlir_dialect_library(MLIRMemRefTransforms
  ComposeSubView.cpp
  ExpandOps.cpp
  FoldMemRefAliasOps.cpp
  MultiBuffer.cpp
  NormalizeMemRefs.cpp
  ResolveShapedTypeResultDims.cpp
  SimplifyExtractStridedMetadata.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/MemRef

  DEPENDS
  MLIRMemRefPassIncGen

  LINK_LIBS PUBLIC
  MLIRAffineDialect
  MLIRAffineUtils
  MLIRArithDialect
  MLIRFuncDialect
  MLIRInferTypeOpInterface
  MLIRLoopLikeInterface
  MLIRMemRefDialect
  MLIRPass
  MLIRTensorDialect
  MLIRTransforms
  MLIRVectorDialect
)

