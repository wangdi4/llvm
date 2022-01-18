; RUN: llvm-as < %s | llvm-dis | llvm-as > /dev/null
; RUN: verify-uselistorder %s

declare intel_features_init_cc i32 @__intel_cpu_features_init_x()

define i32 @f() {
  %res = call intel_features_init_cc i32 @__intel_cpu_features_init_x()
  ret i32 %res
}
