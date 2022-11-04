; RUN: llvm-as %p/work-group-builtins-64.ll -o %t.lib.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -passes=dpcpp-kernel-group-builtin -S < %s | FileCheck %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.lib.bc -dpcpp-kernel-group-builtin -S < %s | FileCheck %s

declare signext i16 @_Z29work_group_reduce_bitwise_ands(i16)
declare i32 @_Z28work_group_reduce_logical_ori(i32)
declare signext <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_s(<16 x i16>)
declare <16 x i32> @_Z28work_group_reduce_logical_orDv16_i(<16 x i32>)

define void @test() {
; CHECK-LABEL: @test(
; CHECK-NEXT:    [[ALLOCAWGRESULT5:%.*]] = alloca <16 x i32>, align 64
; CHECK-NEXT:    store <16 x i32> zeroinitializer, <16 x i32>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    [[ALLOCAWGRESULT3:%.*]] = alloca <16 x i16>, align 32
; CHECK-NEXT:    store <16 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <16 x i16>* [[ALLOCAWGRESULT3]], align 32
; CHECK-NEXT:    [[ALLOCAWGRESULT1:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 0, i32* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    [[ALLOCAWGRESULT:%.*]] = alloca i16, align 2
; CHECK-NEXT:    store i16 -1, i16* [[ALLOCAWGRESULT]], align 2
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM:%.*]] = call i16 @_Z29work_group_reduce_bitwise_andsPs(i16 noundef signext 4, i16* [[ALLOCAWGRESULT]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load i16, i16* [[ALLOCAWGRESULT]], align 2
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call i16 @_Z30__finalize_work_group_identitys(i16 [[LOADWGFINALRESULT]])
; CHECK-NEXT:    store i16 -1, i16* [[ALLOCAWGRESULT]], align 2
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM2:%.*]] = call i32 @_Z28work_group_reduce_logical_oriPi(i32 noundef 8, i32* [[ALLOCAWGRESULT1]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load i32, i32* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call i32 @_Z30__finalize_work_group_identityi(i32 [[LOADWGFINALRESULT]])
; CHECK-NEXT:    store i32 0, i32* [[ALLOCAWGRESULT1]], align 4
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM4:%.*]] = call signext <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_sPS_(<16 x i16> noundef signext <i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4>, <16 x i16>* [[ALLOCAWGRESULT3]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT:%.*]] = load <16 x i16>, <16 x i16>* [[ALLOCAWGRESULT3]], align 32
; CHECK-NEXT:    [[CALLFINALIZEWG:%.*]] = call <16 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv16_s(<16 x i16> [[LOADWGFINALRESULT]])
; CHECK-NEXT:    store <16 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <16 x i16>* [[ALLOCAWGRESULT3]], align 32
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    [[CALLWGFORITEM6:%.*]] = call <16 x i32> @_Z28work_group_reduce_logical_orDv16_iPS_(<16 x i32> noundef <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>, <16 x i32>* [[ALLOCAWGRESULT5]])
; CHECK-NEXT:    call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT:    [[LOADWGFINALRESULT7:%.*]] = load <16 x i32>, <16 x i32>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    [[CALLFINALIZEWG8:%.*]] = call <16 x i32> @_Z39__finalize_work_group_reduce_logical_orDv16_i(<16 x i32> [[LOADWGFINALRESULT7]])
; CHECK-NEXT:    store <16 x i32> zeroinitializer, <16 x i32>* [[ALLOCAWGRESULT5]], align 64
; CHECK-NEXT:    call void @dummy_barrier.()
; CHECK-NEXT:    ret void
;
  %call1 = call i16 @_Z29work_group_reduce_bitwise_ands(i16 noundef signext 4)
  %call2 = call i32 @_Z28work_group_reduce_logical_ori(i32 noundef 8)

  %call3 = call signext <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_s(<16 x i16> noundef signext <i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4>)
  %call4 = call <16 x i32> @_Z28work_group_reduce_logical_orDv16_i(<16 x i32> noundef <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>)

  ret void
}

;; Instructions inserted by GroupBuiltin should not have debug info
; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-25: WARNING
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
