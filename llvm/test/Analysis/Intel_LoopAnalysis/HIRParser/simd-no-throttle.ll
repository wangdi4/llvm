; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that simd loop is not throttled due to presence of goto.

; CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]

; CHECK: + DO i1 = 0, 2047, 1   <DO_LOOP>
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   if (%0 !=u 0.000000e+00)
; CHECK: |   {
; CHECK: |      %1 = (%b)[i1];
; CHECK: |      if (%1 !=u 0.000000e+00)
; CHECK: |      {
; CHECK: |         %2 = (%c)[i1];
; CHECK: |         %mul = %1  *  %2;
; CHECK: |         %add = %0  +  %mul;
; CHECK: |         (%d)[i1] = %add;
; CHECK: |         goto for.inc;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %4 = (i32*)(%c)[i1];
; CHECK: |   (i32*)(%d)[i1] = %4;
; CHECK: |   for.inc:
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]



; ModuleID = 'short-cir.ll'
source_filename = "short-cir.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture readonly %a, ptr nocapture readonly %b, ptr nocapture readonly %c, ptr nocapture %d) local_unnamed_addr #0 {
entry:
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %tobool = fcmp une float %0, 0.000000e+00
  br i1 %tobool, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %arrayidx2 = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4
  %tobool3 = fcmp une float %1, 0.000000e+00
  br i1 %tobool3, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %arrayidx9 = getelementptr inbounds float, ptr %c, i64 %indvars.iv
  %2 = load float, ptr %arrayidx9, align 4
  %mul = fmul float %1, %2
  %add = fadd float %0, %mul
  %arrayidx11 = getelementptr inbounds float, ptr %d, i64 %indvars.iv
  store float %add, ptr %arrayidx11, align 4
  br label %for.inc

if.else:                                          ; preds = %land.lhs.true, %for.body
  %arrayidx13 = getelementptr inbounds float, ptr %c, i64 %indvars.iv
  %3 = bitcast ptr %arrayidx13 to ptr
  %4 = load i32, ptr %3, align 4
  %arrayidx15 = getelementptr inbounds float, ptr %d, i64 %indvars.iv
  %5 = bitcast ptr %arrayidx15 to ptr
  store i32 %4, ptr %5, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2048
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

