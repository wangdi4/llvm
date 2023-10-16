; RUN: opt -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/work-group-builtins-64.rtl -passes=sycl-kernel-group-builtin -S < %s | FileCheck %s

;;****************************************************************************
; This test checks the GroupBuiltin pass
;;  - It checks that floating point accumulating result values for min and max
;;    built-ins are initialized with +INF and -INF as they must according to
;;    OpenCL C specification.
;;****************************************************************************

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; CHECK-DAG: store double 0x7FF0000000000000,{{.*}} ptr [[D_POS_INF:%.*]],
; CHECK-DAG: store double 0xFFF0000000000000,{{.*}} ptr [[D_NEG_INF:%.*]],
; CHECK-DAG: store float 0x7FF0000000000000,{{.*}} ptr [[F_POS_INF:%.*]],
; CHECK-DAG: store float 0xFFF0000000000000,{{.*}} ptr [[F_NEG_INF:%.*]],

; CHECK-DAG: store <16 x double> <double 0x7FF0000000000000,{{.*}} ptr [[D_POS_INF16:%.*]],
; CHECK-DAG: store <16 x double> <double 0xFFF0000000000000,{{.*}} ptr [[D_NEG_INF16:%.*]],
; CHECK-DAG: store <16 x float> <float 0x7FF0000000000000,{{.*}} ptr [[F_POS_INF16:%.*]],
; CHECK-DAG: store <16 x float> <float 0xFFF0000000000000,{{.*}} ptr [[F_NEG_INF16:%.*]],

define spir_func void @test_work_group_scan_fp(float %float_in, double %double_in) {
; CHECK: call spir_func float @_Z29work_group_scan_exclusive_maxfPf({{.*}} ptr [[F_NEG_INF]])
; CHECK: call spir_func double @_Z29work_group_scan_exclusive_maxdPd({{.*}} ptr [[D_NEG_INF]])
; CHECK: call spir_func float @_Z29work_group_scan_exclusive_minfPf({{.*}} ptr [[F_POS_INF]])
; CHECK: call spir_func double @_Z29work_group_scan_exclusive_mindPd({{.*}} ptr [[D_POS_INF]])
; CHECK: call spir_func <16 x float> @_Z29work_group_scan_exclusive_maxDv16_fPS_({{.*}} ptr [[F_NEG_INF16]])
; CHECK: call spir_func <16 x double> @_Z29work_group_scan_exclusive_maxDv16_dPS_({{.*}} ptr [[D_NEG_INF16]])
; CHECK: call spir_func <16 x float> @_Z29work_group_scan_exclusive_minDv16_fPS_({{.*}} ptr [[F_POS_INF16]])
; CHECK: call spir_func <16 x double> @_Z29work_group_scan_exclusive_minDv16_dPS_({{.*}} ptr [[D_POS_INF16]])
entry:
  %0 = call spir_func float @_Z29work_group_scan_exclusive_maxf(float %float_in)
  %1 = call spir_func double @_Z29work_group_scan_exclusive_maxd(double %double_in)
  %2 = call spir_func float @_Z29work_group_scan_exclusive_minf(float %float_in)
  %3 = call spir_func double @_Z29work_group_scan_exclusive_mind(double %double_in)
  %4 = call spir_func <16 x float> @_Z29work_group_scan_exclusive_maxDv16_f(<16 x float> zeroinitializer)
  %5 = call spir_func <16 x double> @_Z29work_group_scan_exclusive_maxDv16_d(<16 x double> zeroinitializer)
  %6 = call spir_func <16 x float> @_Z29work_group_scan_exclusive_minDv16_f(<16 x float> zeroinitializer)
  %7 = call spir_func <16 x double> @_Z29work_group_scan_exclusive_minDv16_d(<16 x double> zeroinitializer)
  ret void
}

declare spir_func float @_Z29work_group_scan_exclusive_maxf(float)
declare spir_func double @_Z29work_group_scan_exclusive_maxd(double)
declare spir_func float @_Z29work_group_scan_exclusive_minf(float)
declare spir_func double @_Z29work_group_scan_exclusive_mind(double)
declare spir_func <16 x float> @_Z29work_group_scan_exclusive_maxDv16_f(<16 x float>)
declare spir_func <16 x double> @_Z29work_group_scan_exclusive_maxDv16_d(<16 x double>)
declare spir_func <16 x float> @_Z29work_group_scan_exclusive_minDv16_f(<16 x float>)
declare spir_func <16 x double> @_Z29work_group_scan_exclusive_minDv16_d(<16 x double>)

;; These are inserted by GroupBuiltin pass, should not have debug info
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  %AllocaWGResult{{.*}} = alloca
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  store {{.*}} %AllocaWGResult
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test_work_group_scan_fp --  call void @dummy_barrier.()

; DEBUGIFY-NOT: WARNING
