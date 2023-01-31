; RUN: llvm-as -opaque-pointers=0 %p/work-group-builtins-64.ll -o %t.work-group-builtins-64.ll.bc
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.work-group-builtins-64.ll.bc -passes=sycl-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers=0 -sycl-kernel-builtin-lib=%t.work-group-builtins-64.ll.bc -passes=sycl-kernel-group-builtin -S < %s | FileCheck %s

;;****************************************************************************
; This test checks the GroupBuiltin pass
;;  - It checks that floating point accumulating result values for min and max
;;    built-ins are initialized with +INF and -INF as they must according to
;;    OpenCL C specification.
;;****************************************************************************

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; CHECK-DAG:  store float 0xFFF0000000000000, float* [[WGMAXS:%[a-zA-Z0-9]+]]
; CHECK-DAG:  call {{.*}}work_group_scan_exclusive_max{{.*}} [[WGMAXS]])

; CHECK-DAG:  store double 0xFFF0000000000000, double* [[WGMAXD:%[a-zA-Z0-9]+]]
; CHECK-DAG:  call {{.*}}work_group_scan_exclusive_max{{.*}} [[WGMAXD]])

; CHECK-DAG:  store float 0x7FF0000000000000, float* [[WGMINS:%[a-zA-Z0-9]+]]
; CHECK-DAG:  call {{.*}}work_group_scan_exclusive_min{{.*}} [[WGMINS]])

; CHECK-DAG:  store double 0x7FF0000000000000, double* [[WGMIND:%[a-zA-Z0-9]+]]
; CHECK-DAG:  call {{.*}}work_group_scan_exclusive_min{{.*}} [[WGMIND]])

; Function Attrs: nounwind
define spir_func void @test_work_group_scan_fp(float %float_in, double %double_in) {
entry:
  %0 = call spir_func float @_Z29work_group_scan_exclusive_maxf(float %float_in)
  %1 = call spir_func double @_Z29work_group_scan_exclusive_maxd(double %double_in)
  %2 = call spir_func float @_Z29work_group_scan_exclusive_minf(float %float_in)
  %3 = call spir_func double @_Z29work_group_scan_exclusive_mind(double %double_in)
  ret void
}

declare spir_func float @_Z29work_group_scan_exclusive_maxf(float) #0
declare spir_func double @_Z29work_group_scan_exclusive_maxd(double) #0
declare spir_func float @_Z29work_group_scan_exclusive_minf(float) #0
declare spir_func double @_Z29work_group_scan_exclusive_mind(double) #0

attributes #0 = { nounwind }

;; These are inserted by GroupBuiltin pass, should not have debug info
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- [[A1:%AllocaWGResult[0-9]*]] = alloca double, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store double 0x7FF0000000000000, double* [[A1]], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- [[A2:%AllocaWGResult[0-9]*]] = alloca float, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store float 0x7FF0000000000000, float* [[A2]], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- [[A3:%AllocaWGResult[0-9]*]] = alloca double, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store double 0xFFF0000000000000, double* [[A3]], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- [[A4:%AllocaWGResult[0-9]*]] = alloca float, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store float 0xFFF0000000000000, float* [[A4]], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- call void @dummy_barrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store float 0xFFF0000000000000, float* [[A4]], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- call void @dummy_barrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store double 0xFFF0000000000000, double* [[A3]], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- call void @dummy_barrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store float 0x7FF0000000000000, float* [[A2]], align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- call void @dummy_barrier.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- store double 0x7FF0000000000000, double* [[A1]], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp -- call void @dummy_barrier.()

; DEBUGIFY-NOT: WARNING
