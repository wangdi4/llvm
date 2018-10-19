; RUN: opt < %s -vpo-paropt-prepare -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -S | FileCheck %s
; REQUIRES: csa-registered-target
;
; Check paropt lowering of OpenMP RTL calls for CSA.

target triple = "csa"

; CHECK-LABEL: @check.threads
define void @check.threads(i32* %num_threads, i32* %max_threads, i32* %thread_num) {
entry:
; CHECK-NO: @omp_set_num_threads
  call void @omp_set_num_threads(i32 10)
; CHECK-NO: @omp_get_num_threads
  %0 = call i32 @omp_get_num_threads()
; CHECK: store i32 1, i32* %num_threads
  store i32 %0, i32* %num_threads, align 4
; CHECK-NO: @omp_get_max_threads
  %1 = call i32 @omp_get_max_threads()
; CHECK: store i32 1, i32* %max_threads
  store i32 %1, i32* %max_threads, align 4
; CHECK-NO: @omp_get_thread_num
  %2 = call i32 @omp_get_thread_num()
; CHECK: store i32 0, i32* %thread_num
  store i32 %2, i32* %thread_num, align 4
  ret void
}

declare dso_local void @omp_set_num_threads(i32)
declare dso_local i32 @omp_get_num_threads()
declare dso_local i32 @omp_get_max_threads()
declare dso_local i32 @omp_get_thread_num()

; CHECK-LABEL: @check.dynamic
define void @check.dynamic(i32* %dynamic) {
entry:
; CHECK-NO: @omp_set_dynamic
  call void @omp_set_dynamic(i32 0)
; CHECK-NO: @omp_get_dynamic
  %0 = call i32 @omp_get_dynamic()
; CHECK: store i32 0, i32* %dynamic
  store i32 %0, i32* %dynamic, align 4
  ret void
}

declare dso_local void @omp_set_dynamic(i32)
declare dso_local i32 @omp_get_dynamic()

; CHECK-LABEL: @check.nested
define void @check.nested(i32* %nested) {
entry:
; CHECK-NO: @omp_set_nested
  call void @omp_set_nested(i32 0)
; CHECK-NO: @omp_get_nested
  %0 = call i32 @omp_get_nested()
; CHECK: store i32 1, i32* %nested
  store i32 %0, i32* %nested, align 4
  ret void
}

declare dso_local void @omp_set_nested(i32)
declare dso_local i32 @omp_get_nested()
