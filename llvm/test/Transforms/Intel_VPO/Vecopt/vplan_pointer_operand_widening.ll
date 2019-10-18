; This test verifies that we generate correct base-pointers for cases where we
; have to generate scatter-gathers for store/load instructions

; Check LLVM-IR codegen path.
; RUN: opt < %s -S -VPlanDriver -vplan-force-vf=2  | FileCheck %s

; Check VPValue-codegen path.
; RUN: opt < %s -S -VPlanDriver -vplan-force-vf=2 -vplan-use-entity-instr -enable-vp-value-codegen | FileCheck %s

; CHECK-LABEL:@foo
;CHECK: [[VEC_BASE_PTR1:%.*]] = shufflevector <2 x i32*> {{.*}}, <2 x i32*> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS1:%.*]] = getelementptr i32, <6 x i32*> [[VEC_BASE_PTR1]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: [[G1:%.*]] = call <6 x i32> @llvm.masked.gather.v6i32.v6p0i32(<6 x i32*> [[ELEM_BASE_PTRS1]], i32 16, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <6 x i32> undef)

;CHECK: [[VEC_BASE_PTR2:%.*]] = shufflevector <2 x i32*> {{.*}}, <2 x i32*> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS2:%.*]] = getelementptr i32, <6 x i32*> [[VEC_BASE_PTR2]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: call void @llvm.masked.scatter.v6i32.v6p0i32(<6 x i32> [[G1]], <6 x i32*> [[ELEM_BASE_PTRS2]], i32 16, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)

;CHECK: [[VEC_BASE_PTR3:%.*]] = shufflevector <2 x i32*> {{.*}}, <2 x i32*> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS3:%.*]] = getelementptr i32, <6 x i32*> [[VEC_BASE_PTR3]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: call void @llvm.masked.scatter.v6i32.v6p0i32(<6 x i32> [[G1]], <6 x i32*> [[ELEM_BASE_PTRS3]], i32 16, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%Struct = type { <3 x i32>, i32 }

define void @foo(%Struct *%a) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %block ]
  %base = getelementptr %Struct, %Struct *%a, i64 %indvars.iv
  %ptr = getelementptr inbounds %Struct, %Struct * %base, i32 0, i32 0
  %ld = load <3 x i32>, <3 x i32>* %ptr
  br label %block

block:
  %phi = phi <3 x i32>* [ %ptr, %for.body ]
  store <3 x i32> %ld, <3 x i32>* %phi
  store <3 x i32> %ld, <3 x i32>* %ptr

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  ret void
}

; CHECK-LABEL: @foo_c
;CHECK: [[VEC_BASE_PTR1:%.*]] = shufflevector <2 x i32*> {{.*}}, <2 x i32*> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS1:%.*]] = getelementptr i32, <6 x i32*> [[VEC_BASE_PTR1]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK: [[G1:%.*]] = call <6 x i32> @llvm.masked.gather.v6i32.v6p0i32(<6 x i32*> [[ELEM_BASE_PTRS1]], i32 16, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <6 x i32> undef)
;CHECK: [[GEP2:%.*]] = getelementptr inbounds %Struct, <2 x %Struct*> [[GEP1:%.*]], <2 x i32> <i32 1, i32 1>, <2 x i32> zeroinitializer
;CHECK-NEXT: [[GEP3:%.*]] = getelementptr inbounds %Struct, <2 x %Struct*> [[GEP1:%.*]], <2 x i32> zeroinitializer, <2 x i32> zeroinitializer
;CHECK-NEXT: [[PRED_PHI:%.*]] = select <2 x i1> {{.*}}, <2 x <3 x i32>*> [[GEP3]], <2 x <3 x i32>*> [[GEP2]]
;CHECK-NEXT: [[BASE_ADDR:%.*]] = bitcast <2 x <3 x i32>*> [[PRED_PHI]] to <2 x i32*>
;CHECK-NEXT: [[VEC_BASE_PTR2:%.*]] = shufflevector <2 x i32*> [[BASE_ADDR]], <2 x i32*> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS2:%.*]] = getelementptr i32, <6 x i32*> [[VEC_BASE_PTR2]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: call void @llvm.masked.scatter.v6i32.v6p0i32(<6 x i32> [[G1]], <6 x i32*> [[ELEM_BASE_PTRS2]], i32 16, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)

define void @foo_c(%Struct *%a, i1 %flag) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %IfElseEnd ]
  %base = getelementptr %Struct, %Struct *%a, i64 %indvars.iv
  %ptr = getelementptr inbounds %Struct, %Struct * %base, i32 0, i32 0
  %ld = load <3 x i32>, <3 x i32>* %ptr
  %ld.0 = extractelement <3 x i32> %ld, i32 0
  %cmp = icmp eq i32 %ld.0, 42
  br i1 %cmp, label %block1, label %block2

block1:
  %ptr1 = getelementptr inbounds %Struct, %Struct * %base, i32 0, i32 0
  br label %IfElseEnd

block2:
  %ptr2 = getelementptr inbounds %Struct, %Struct * %base, i32 1, i32 0
  br label %IfElseEnd

IfElseEnd:
  %phi = phi <3 x i32>* [ %ptr1, %block1], [ %ptr2, %block2]
  store <3 x i32> %ld, <3 x i32>* %phi
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  ret void
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
