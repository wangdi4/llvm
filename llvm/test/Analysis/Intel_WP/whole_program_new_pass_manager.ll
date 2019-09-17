; This test checks that the whole program analysis is marked as required
; in LTO for the new pass manager.

; RUN: opt < %s -passes='lto<O3>' -S -debug-pass-manager 2>&1 | FileCheck %s

define i32 @main() {
  ret i32 1
}

; CHECK:  RequireAnalysisPass<{{.*}}WholeProgramAnalysis
