; INTEL_FEATURE_CSA
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
; CHECK: store i32 1, i32* %dynamic
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
; CHECK: store i32 0, i32* %nested
  store i32 %0, i32* %nested, align 4
  ret void
}

declare dso_local void @omp_set_nested(i32)
declare dso_local i32 @omp_get_nested()

%struct.omp_lock_t = type { i8* }
declare void @omp_init_lock(%struct.omp_lock_t*)
declare void @omp_destroy_lock(%struct.omp_lock_t*)
declare void @omp_set_lock(%struct.omp_lock_t*)
declare void @omp_unset_lock(%struct.omp_lock_t*)
declare i32 @omp_test_lock(%struct.omp_lock_t*)

; CHECK-LABEL: @check.init.lock
define void @check.init.lock(%struct.omp_lock_t* %lock) {
entry:
; CHECK-NO: @omp_init_lock
  call void @omp_init_lock(%struct.omp_lock_t* %lock)
; CHECK:      [[CAST:%.*]] = bitcast %struct.omp_lock_t* %lock to i64*
; CHECK-NEXT: store atomic i64 0, i64* [[CAST]] seq_cst
  ret void
}

; CHECK-LABEL: @check.destroy.lock
define void @check.destroy.lock(%struct.omp_lock_t* %lock) {
entry:
; CHECK-NO: @omp_destroy_lock
  call void @omp_destroy_lock(%struct.omp_lock_t* %lock)
  ret void
}

; CHECK-LABEL: @check.set.lock
define void @check.set.lock(%struct.omp_lock_t* %lock) {
entry:
; CHECK-NO: @omp_set_lock
  call void @omp_set_lock(%struct.omp_lock_t* %lock)
; CHECK:      [[CAST:%.*]] = bitcast %struct.omp_lock_t* %lock to i64*
; CHECK-NEXT: br label %[[LOOP:.*]]
; CHECK:    [[LOOP]]:
; CHECK-NEXT: [[XCHG:%.*]] = cmpxchg i64* [[CAST]], i64 0, i64 1 seq_cst seq_cst
; CHECK-NEXT: [[COND:%.*]] = extractvalue { i64, i1 } [[XCHG]], 1
; CHECK-NEXT: br i1 [[COND]], label %[[EXIT:.*]], label %[[LOOP]]
; CHECK:    [[EXIT]]:
  ret void
}

; CHECK-LABEL: @check.unset.lock
define void @check.unset.lock(%struct.omp_lock_t* %lock) {
entry:
; CHECK-NO: @omp_unset_lock
  call void @omp_unset_lock(%struct.omp_lock_t* %lock)
; CHECK:      [[CAST:%.*]] = bitcast %struct.omp_lock_t* %lock to i64*
; CHECK-NEXT: store atomic i64 0, i64* [[CAST]] seq_cst
  ret void
}

; CHECK-LABEL: @check.test.lock
define i32 @check.test.lock(%struct.omp_lock_t* %lock) {
entry:
; CHECK-NO: @omp_test_lock
  %call = call i32 @omp_test_lock(%struct.omp_lock_t* %lock)
; CHECK:      [[CAST:%.*]] = bitcast %struct.omp_lock_t* %lock to i64*
; CHECK-NEXT: [[XCHG:%.*]] = cmpxchg i64* [[CAST]], i64 0, i64 1 seq_cst seq_cst
; CHECK-NEXT: [[COND:%.*]] = extractvalue { i64, i1 } [[XCHG]], 1
; CHECK-NEXT: [[RES:%.*]] = select i1 [[COND]], i32 1, i32 0
; CHECK-NEXT: ret i32 [[RES]]
  ret i32 %call
}
; end INTEL_FEATURE_CSA
