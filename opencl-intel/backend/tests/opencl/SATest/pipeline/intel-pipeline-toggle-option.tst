; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -enable-vec-clone=true" -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s
; RUN: CL_CONFIG_LLVM_OPTIONS=-enable-vec-clone=true SATest -BUILD -llvm-option=-debug-pass-manager -config=%S/pipeline_lto.tst.cfg 2>&1 | FileCheck %s

; Check that pipeline changes when -enable-vec-clone option is toggled.
; The option is defined as false in CompilerConfig.cpp.

; CHECK: Running pass: VecClonePass
