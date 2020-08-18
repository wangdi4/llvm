;  XFAIL: *
; RUN: opt -hir-details -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s
; RUN: opt -hir-details -passes="hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Note: test uses an LLVM IR after SSA deconstruction.

; Check that after distribution and scalar expansion of %t.1
; TempArray stores and loads will have consistent BlobDDRefs.

; The issue here that "%t.1" picks up linear form of "%conv + 1"
; with BlobDDRef for "%conv", creating a USE before DEF.

; BEGIN REGION { }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   %0 = (%b)[i1];
;       |   %t.1 = 0;
;       |   if (%n != 0) <distribute_point>
;       |   {
;       |      %conv = fptosi.float.i32(%0);
;       |      %conv3 = sitofp.i32.float(i1);
;       |      %add = %conv3  +  1.000000e+00;
;       |      (%b)[i1] = %add;
;       |      (%a)[i1 + -1] = 100;
;       |      %t.1 = %conv + 1;
;       |   }
;       + END LOOP
; END REGION

; The loop after the distribution

; BEGIN REGION { modified }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   %0 = (%b)[i1];
;       |   (%.TempArray)[0][i1] = %0;
;       |   %t.1 = 0;
;       |   (%.TempArray1)[0][i1] = 0;
;       + END LOOP
;
;
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   %0 = (%.TempArray)[0][i1];
;       |   %t.1 = (%.TempArray1)[0][i1];  << Here %t.1 should not have %conv BlobDDRef
;       |   if (%n != 0)
;       |   {
;       |      %conv = fptosi.float.i32(%0);
;       |      %conv3 = sitofp.i32.float(i1);
;       |      %add = %conv3  +  1.000000e+00;
;       |      (%b)[i1] = %add;
;       |      (%a)[i1 + -1] = 100;
;       |      %t.1 = %conv + 1;
;       |   }
;       + END LOOP
; END REGION

; CHECK: %t.1 = (%.TempArray1)[0][i1];
; CHECK: <LVAL-REG> NON-LINEAR i32 %t.1
; CHECK-NOT: <BLOB>
; CHECK: <RVAL-REG> (LINEAR [64 x i32]* %.TempArray1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(float* nocapture %b, i32* nocapture %a, i32 %n) {
entry:
  %tobool = icmp eq i32 %n, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  %t.1.lcssa = phi i32 [ %t.1, %if.end ]
  %conv9 = sitofp i32 %t.1.lcssa to float
  ret float %conv9

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ], !in.de.ssa !1
  %z = phi i32 [ 0, %entry ], [ 0, %if.end ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %t.1.in1 = call i32 @llvm.ssa.copy.i32(i32 %z), !in.de.ssa !2
  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %conv = fptosi float %0 to i32
  %2 = trunc i64 %indvars.iv to i32
  %conv3 = sitofp i32 %2 to float
  %add = fadd float %conv3, 1.000000e+00
  store float %add, float* %arrayidx, align 4
  %3 = add nsw i64 %indvars.iv, -1
  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 %3
  store i32 100, i32* %arrayidx7, align 4
  %add8 = add nsw i32 %conv, 1
  %t.1.in = call i32 @llvm.ssa.copy.i32(i32 %add8), !in.de.ssa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %t.1 = phi i32 [ %add8, %if.then ], [ %z, %for.body ], !in.de.ssa !2
  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 32
  %indvars.iv.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next), !in.de.ssa !1
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
declare i64 @llvm.ssa.copy.i64(i64 returned) #2
declare i32 @llvm.ssa.copy.i32(i32 returned) #2

attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }

!1 = !{!"indvars.iv.de.ssa"}
!2 = !{!"t.1.de.ssa"}

