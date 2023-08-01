; SIMD loops should not be multiversioned even if there is if-condition between region and loop

; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

;void foo(int *a, long *b, int N, int *ptr) {
;  int i;
;  if (ptr[42] == 42) { // moved inside #pragma omp simd
;    #pragma omp simd
;    for (i=0;i<N;i++) {
;      a[i] = b[i];
;    }
;  }
;}

; <0>          BEGIN REGION { }
; <2>                %simd = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; <4>                %arrayv42 = (%ptr)[42];
; <6>                if (%arrayv42 == 42)
; <6>                {
; <30>                  + DO i1 = 0, zext.i32.i64((-1 + %N)), 1   <DO_LOOP> <simd>
; <14>                  |   %0 = (%b)[i1];
; <17>                  |   (%a)[i1] = %0;
; <30>                  + END LOOP
; <6>                }
; <28>               @llvm.directive.region.exit(%simd); [ DIR.OMP.END.SIMD() ]
; <29>               ret ;
; <0>          END REGION

; CHECK: Function
; CHECK: DO i1 =
; CHECK-SAME: simd
; CHECK-NOT: DO i1 =

; ModuleID = 'ptr-types.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture %a, ptr nocapture readonly %b, i32 %N, ptr %ptr) #0 {
entry:
  %simd = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %arrayidx42 = getelementptr inbounds i32, ptr %ptr, i64 42
  %arrayv42 = load i32, ptr %arrayidx42, align 4
  %cmp42 = icmp ne i32 %arrayv42, 42
  br i1 %cmp42, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, ptr %b, i64 %indvars.iv
  %0 = load i64, ptr %arrayidx, align 8
  %conv = trunc i64 %0 to i32
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %conv, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %simd) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

