; RUN: opt -passes=sycl-kernel-spec-constant \
; RUN:   -sycl-spec-constant='invalid:format' \
; RUN:   -sycl-spec-constant='2.0:i8:0' \
; RUN:   -sycl-spec-constant='2:i32*:0' \
; RUN:   -sycl-spec-constant='1:i8:42' \
; RUN:   -sycl-spec-constant='1:i8:-42' \
; RUN:   -sycl-spec-constant='2:i32:-42' \
; RUN:   -sycl-spec-constant='3:i32:-42' \
; RUN:   -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-spec-constant \
; RUN:   -sycl-spec-constant='invalid:format' \
; RUN:   -sycl-spec-constant='2.0:i8:0' \
; RUN:   -sycl-spec-constant='2:i32*:0' \
; RUN:   -sycl-spec-constant='1:i8:42' \
; RUN:   -sycl-spec-constant='1:i8:-42' \
; RUN:   -sycl-spec-constant='2:i32:-42' \
; RUN:   -sycl-spec-constant='3:i32:-42' \
; RUN:   -S < %s 2>&1 | FileCheck %s

; CHECK-DAG: warning: Option --sycl-spec-constant=invalid:format is ignored because the format is invalid. The expected format is id:type:value
; CHECK-DAG: warning: Option --sycl-spec-constant=2.0:i8:0 is ignored because the id must be an unsigned int
; CHECK-DAG: warning: Option --sycl-spec-constant=2:i32*:0 is ignored because the type is invalid. The expected type is one of i1, i8, i16, i32, i64, f16, f32 and f64
; CHECK-DAG: warning: Option --sycl-spec-constant=1:i8:42 is ignored because it is overrided by the later option with same id (1:i8:-42)
; CHECK-DAG: warning: Option --sycl-spec-constant=2:i32:-42 is ignored because the specified type doesn't match with the spec constant type defined in the module
; CHECK-DAG: warning: Option --sycl-spec-constant=3:i32:-42 is ignored because there's no matched spec constant in the module of the given id

define void @foo() {
entry:
; CHECK-NOT: __spirv_SpecConstant
; CHECK: %cmp.1 = icmp eq i8 -42, 0
; CHECK: %cmp.2 = icmp eq i8 0, 0
  %call.1 = tail call spir_func signext i8 @__spirv_SpecConstant(i32 noundef 1, i8 noundef signext 0)
  %call.2 = tail call spir_func signext i8 @__spirv_SpecConstant(i32 noundef 2, i8 noundef signext 0)
  %cmp.1 = icmp eq i8 %call.1, 0
  %cmp.2 = icmp eq i8 %call.2, 0
  ret void
}

declare extern_weak dso_local spir_func signext i8 @__spirv_SpecConstant(i32 noundef, i8 noundef signext)

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY-NOT: WARNING
