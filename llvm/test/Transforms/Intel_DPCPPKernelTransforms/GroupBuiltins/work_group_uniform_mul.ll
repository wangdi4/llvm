; RUN: llvm-as %p/work-group-builtins-64.ll -o %t.lib.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin -S < %s | FileCheck %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin -S < %s | FileCheck %s

declare i32 @_Z21work_group_reduce_muli(i32)
declare float @_Z21work_group_reduce_mulf(float)
declare <16 x i32> @_Z21work_group_reduce_mulDv16_i(<16 x i32>)
declare <16 x float> @_Z21work_group_reduce_mulDv16_f(<16 x float>)

define void @test() {
; CHECK-LABEL: @test(
; CHECK-NEXT:    [[ALLOCAWGRESULT5:%.*]] = alloca <16 x float>, align 64
; CHECK-NEXT:    store <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <16 x float>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    [[ALLOCAWGRESULT3:%.*]] = alloca <16 x i32>, align 64
; CHECK-NEXT:    store <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, <16 x i32>* [[ALLOCAWGRESULT3]], align 64
; CHECK-NEXT:    [[ALLOCAWGRESULT1:%.*]] = alloca float, align 4
; CHECK-NEXT:    store float 1.000000e+00, float* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    [[ALLOCAWGRESULT:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 1, i32* [[ALLOCAWGRESULT]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM:%.*]] = call i32 @_Z21work_group_reduce_muliPi(i32 noundef 2, i32* [[ALLOCAWGRESULT]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load i32, i32* %AllocaWGResult, align 4
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call i32 @_Z30__finalize_work_group_identityi(i32 [[LOADWGFINALRESULT:%.*]])
; CHECK-NEXT:    store i32 1, i32* [[ALLOCAWGRESULT]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM2:%.*]] = call float @_Z21work_group_reduce_mulfPf(float noundef 2.000000e+00, float* [[ALLOCAWGRESULT1]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load float, float* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call float @_Z30__finalize_work_group_identityf(float [[LOADWGFINALRESULT:%.*]])
; CHECK-NEXT:    store float 1.000000e+00, float* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM4:%.*]] = call <16 x i32> @_Z21work_group_reduce_mulDv16_iPS_(<16 x i32> noundef <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>, <16 x i32>* [[ALLOCAWGRESULT3]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load <16 x i32>, <16 x i32>* [[ALLOCAWGRESULT3]], align 64
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call <16 x i32> @_Z32__finalize_work_group_reduce_mulDv16_i(<16 x i32> [[LOADWGFINALRESULT]])
; CHECK-NEXT:    store <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, <16 x i32>* [[ALLOCAWGRESULT3]], align 64
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM6:%.*]] = call <16 x float> @_Z21work_group_reduce_mulDv16_fPS_(<16 x float> noundef <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>, <16 x float>* [[ALLOCAWGRESULT5]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT7:%.*]] = load <16 x float>, <16 x float>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    [[CALLFINALIZEWG8:%.*]] = call <16 x float> @_Z32__finalize_work_group_reduce_mulDv16_f(<16 x float> [[LOADWGFINALRESULT7]])
; CHECK-NEXT:    store <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <16 x float>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    ret void

  %call2 = call i32 @_Z21work_group_reduce_muli(i32 noundef 2)
  %call3 = call float @_Z21work_group_reduce_mulf(float noundef 2.000000e+00)

  %call4 = call <16 x i32> @_Z21work_group_reduce_mulDv16_i(<16 x i32> noundef <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>)
  %call5 = call <16 x float> @_Z21work_group_reduce_mulDv16_f(<16 x float> noundef <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>)
  ret void
}

;; Instructions inserted by GroupBuiltin should not have debug info
; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-25: WARNING
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS


