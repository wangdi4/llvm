; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Check to see that variable stride is loaded from %c argument.

; Return: i64
; Arguments: i64 (vector), i32 (uniform/stride), ptr (linear)
; ISA class: XMM
; "vector-variants"="_ZGVxN4vuls1_foo"
; Origin: opaque_linear_ptr_variable_stride.ll

define dso_local noundef i64 @foo(i64 noundef %v, i32 noundef %c, ptr noundef %x) local_unnamed_addr #0 {
; CHECK-LABEL: define dso_local x86_regcallcc noundef { <2 x i64>, <2 x i64> } @_ZGVxN4vuls1_foo
; CHECK-SAME: (<2 x i64> noundef [[V_0:%.*]], <2 x i64> noundef [[V_1:%.*]], i32 noundef [[C:%.*]], ptr noundef [[X:%.*]]) local_unnamed_addr #[[ATTR1:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_X:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    store ptr [[X]], ptr [[ALLOCA_X]], align 8
; CHECK-NEXT:    [[ALLOCA_C:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 [[C]], ptr [[ALLOCA_C]], align 4
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <4 x i64>, align 32
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <4 x i64>, align 32
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <2 x i64> [[V_0]], ptr [[VEC_V_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <2 x i64> [[V_1]], ptr [[VEC_V_GEP_1]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED.PTR_TO_PTR"(ptr [[ALLOCA_X]], i8 0, i32 1, ptr [[ALLOCA_C]]), "QUAL.OMP.UNIFORM:TYPED"(ptr [[ALLOCA_C]], i32 0, i32 1) ]
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    [[LOAD_X:%.*]] = load ptr, ptr [[ALLOCA_X]], align 8
; CHECK-NEXT:    [[LOAD_C:%.*]] = load i32, ptr [[ALLOCA_C]], align 4
; CHECK-NEXT:    br label [[SIMD_LOOP_HEADER:%.*]]
; CHECK:       simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER]] ], [ [[INDVAR:%.*]], [[SIMD_LOOP_LATCH:%.*]] ]
; CHECK-NEXT:    [[TMP0:%.*]] = sext i32 [[LOAD_C]] to i64
; CHECK-NEXT:    [[TMP1:%.*]] = mul i64 [[TMP0]], 8
; CHECK-NEXT:    [[TMP2:%.*]] = trunc i64 [[TMP1]] to i32
; CHECK-NEXT:    [[STRIDE_BYTES:%.*]] = mul i32 [[TMP2]], [[INDEX]]
; CHECK-NEXT:    [[LOAD_X_GEP:%.*]] = getelementptr i8, ptr [[LOAD_X]], i32 [[STRIDE_BYTES]]
;    ...skip...
;
entry:
  tail call void @llvm.intel.directive.elementsize(ptr %x, i64 8)
  %0 = load i64, ptr %x, align 8
  %add = add nsw i64 %0, 1
  %res = add nsw i64 %v, %add
  ret i64 %res
}

declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)

attributes #0 = { "vector-variants"="_ZGVxN4vuls1_foo" }
