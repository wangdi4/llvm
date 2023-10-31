; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Test to check that VecClone calculates stride correctly for variable stride
; integer case. Stride should come from LOAD_C0 (uniform parameter %c)

; Return: i64
; Arguments: i64 (vector), i64 (stride), i64 (linear)
; ISA class: YMM1
; "vector-variants"="_ZGVyN8vuls1_foo"
; Origin: linear_variable_stride.ll


define dso_local noundef i64 @foo(i64 noundef signext %v, i64 noundef signext %c, i64 noundef %x) local_unnamed_addr #0 {
; CHECK-LABEL: define dso_local x86_regcallcc noundef { <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @_ZGVyN8vuls1_foo
; CHECK-SAME: (<2 x i64> noundef signext [[V_0:%.*]], <2 x i64> noundef signext [[V_1:%.*]], <2 x i64> noundef signext [[V_2:%.*]], <2 x i64> noundef signext [[V_3:%.*]], i64 noundef signext [[C:%.*]], i64 noundef [[X:%.*]]) local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_X:%.*]] = alloca i64, align 8
; CHECK-NEXT:    store i64 [[X]], ptr [[ALLOCA_X]], align 8
; CHECK-NEXT:    [[ALLOCA_C:%.*]] = alloca i64, align 8
; CHECK-NEXT:    store i64 [[C]], ptr [[ALLOCA_C]], align 8
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <2 x i64> [[V_0]], ptr [[VEC_V_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <2 x i64> [[V_1]], ptr [[VEC_V_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_2:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 2
; CHECK-NEXT:    store <2 x i64> [[V_2]], ptr [[VEC_V_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_3:%.*]] = getelementptr inbounds <2 x i64>, ptr [[VEC_V]], i32 3
; CHECK-NEXT:    store <2 x i64> [[V_3]], ptr [[VEC_V_GEP_3]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:TYPED.PTR_TO_PTR"(ptr [[ALLOCA_X]], i8 0, i32 1, ptr [[ALLOCA_C]]), "QUAL.OMP.UNIFORM:TYPED"(ptr [[ALLOCA_C]], i64 0, i32 1) ]
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    [[LOAD_X:%.*]] = load i64, ptr [[ALLOCA_X]], align 8
; CHECK-NEXT:    [[LOAD_C:%.*]] = load i64, ptr [[ALLOCA_C]], align 8
; CHECK-NEXT:    br label [[SIMD_LOOP_HEADER:%.*]]
; CHECK:       simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER]] ], [ [[INDVAR:%.*]], [[SIMD_LOOP_LATCH:%.*]] ]
; CHECK-NEXT:    [[PHI_CAST:%.*]] = zext i32 [[INDEX]] to i64
; CHECK-NEXT:    [[STRIDE_MUL:%.*]] = mul i64 [[LOAD_C]], [[PHI_CAST]]
; CHECK-NEXT:    [[STRIDE_ADD:%.*]] = add i64 [[LOAD_X]], [[STRIDE_MUL]]
; CHECK-NEXT:    [[ADD:%.*]] = add nsw i64 [[STRIDE_ADD]], 1
; CHECK-NEXT:    [[VEC_V_GEP:%.*]] = getelementptr i64, ptr [[VEC_V]], i32 [[INDEX]]
; CHECK-NEXT:    [[VEC_V_ELEM:%.*]] = load i64, ptr [[VEC_V_GEP]], align 8
; CHECK-NEXT:    [[RES:%.*]] = add nsw i64 [[VEC_V_ELEM]], [[ADD]]
; CHECK-NEXT:    [[VEC_RETVAL_GEP:%.*]] = getelementptr i64, ptr [[VEC_RETVAL]], i32 [[INDEX]]
; CHECK-NEXT:    store i64 [[RES]], ptr [[VEC_RETVAL_GEP]], align 8
;    ...skip...
;
entry:
  %add = add nsw i64 %x, 1
  %res = add nsw i64 %v, %add
  ret i64 %res
}

attributes #0 = { "vector-variants"="_ZGVyN8vuls1_foo" }
