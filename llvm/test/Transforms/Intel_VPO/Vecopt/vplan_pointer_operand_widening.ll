; This test verifies that we generate correct base-pointers for cases where we
; have to generate scatter-gathers for store/load instructions

; RUN: opt < %s -S -passes=vplan-vec -vplan-force-vf=2  | FileCheck %s

; CHECK-LABEL:@foo
;CHECK: [[VEC_BASE_PTR1:%.*]] = shufflevector <2 x ptr> {{.*}}, <2 x ptr> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS1:%.*]] = getelementptr i32, <6 x ptr> [[VEC_BASE_PTR1]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: [[G1:%.*]] = call <6 x i32> @llvm.masked.gather.v6i32.v6p0(<6 x ptr> [[ELEM_BASE_PTRS1]], i32 4, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <6 x i32> poison)

;CHECK: [[VEC_BASE_PTR2:%.*]] = shufflevector <2 x ptr> {{.*}}, <2 x ptr> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS2:%.*]] = getelementptr i32, <6 x ptr> [[VEC_BASE_PTR2]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: call void @llvm.masked.scatter.v6i32.v6p0(<6 x i32> [[G1]], <6 x ptr> [[ELEM_BASE_PTRS2]], i32 4, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)

;CHECK: [[VEC_BASE_PTR3:%.*]] = shufflevector <2 x ptr> {{.*}}, <2 x ptr> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK-NEXT: [[ELEM_BASE_PTRS3:%.*]] = getelementptr i32, <6 x ptr> [[VEC_BASE_PTR3]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
;CHECK-NEXT: call void @llvm.masked.scatter.v6i32.v6p0(<6 x i32> [[G1]], <6 x ptr> [[ELEM_BASE_PTRS3]], i32 4, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%Struct = type { <3 x i32>, i32 }

define void @foo(ptr %a) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %block ]
  %base = getelementptr %Struct, ptr %a, i64 %indvars.iv
  %ld = load <3 x i32>, ptr %base
  br label %block

block:
  %phi = phi ptr [ %base, %for.body ]
  store <3 x i32> %ld, ptr %phi
  store <3 x i32> %ld, ptr %base

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
; CHECK:         [[VECBASEPTR_0:%.*]] = shufflevector <2 x ptr> [[TMP0:%.*]], <2 x ptr> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[ELEMBASEPTR_0:%.*]] = getelementptr i32, <6 x ptr> [[VECBASEPTR_0]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <6 x i32> @llvm.masked.gather.v6i32.v6p0(<6 x ptr> [[ELEMBASEPTR_0]], i32 4, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <6 x i32> poison)
; CHECK:         [[MM_VECTORGEP60:%.*]] = getelementptr inbounds [[STRUCT0:%.*]], <2 x ptr> [[MM_VECTORGEP0:%.*]], <2 x i32> <i32 1, i32 1>, <2 x i32> zeroinitializer
; CHECK-NEXT:    br label [[VPLANNEDBB70:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB5:
; CHECK-NEXT:    br label [[VPLANNEDBB90:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB6:
; CHECK-NEXT:    [[PREDBLEND0:%.*]] = select <2 x i1> [[TMP1:%.*]], <2 x ptr> [[MM_VECTORGEP0]], <2 x ptr> [[MM_VECTORGEP60]]
; CHECK-NEXT:    [[VECBASEPTR_100:%.*]] = shufflevector <2 x ptr> [[PREDBLEND0]], <2 x ptr> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[ELEMBASEPTR_110:%.*]] = getelementptr i32, <6 x ptr> [[VECBASEPTR_100]], <6 x i64> <i64 0, i64 1, i64 2, i64 0, i64 1, i64 2>
; CHECK-NEXT:    call void @llvm.masked.scatter.v6i32.v6p0(<6 x i32> [[WIDE_MASKED_GATHER0]], <6 x ptr> [[ELEMBASEPTR_110]], i32 4, <6 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)

define void @foo_c(ptr %a) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %IfElseEnd ]
  %base = getelementptr %Struct, ptr %a, i64 %indvars.iv
  %ld = load <3 x i32>, ptr %base
  %ld.0 = extractelement <3 x i32> %ld, i32 0
  %cmp = icmp eq i32 %ld.0, 42
  br i1 %cmp, label %block1, label %block2

block1:
  br label %IfElseEnd

block2:
  %ptr2 = getelementptr inbounds %Struct, ptr %base, i32 1, i32 0
  br label %IfElseEnd

IfElseEnd:
  %phi = phi ptr [ %base, %block1], [ %ptr2, %block2]
  store <3 x i32> %ld, ptr %phi
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
