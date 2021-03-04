; Make sure that llvm-as/llvm-dis properly assemble/disassemble the
; source_filename.

; INTEL_CUSTOMIZATION
; Use extra option to allow full source filename path in module
; RUN: llvm-as -strip-module-src-path=false < %s | llvm-dis | FileCheck %s
; end INTEL_CUSTOMIZATION

; CHECK: source_filename = "C:\\path\\with\\backslashes\\test.cc"
source_filename = "C:\\path\\with\5Cbackslashes\\test.cc"
