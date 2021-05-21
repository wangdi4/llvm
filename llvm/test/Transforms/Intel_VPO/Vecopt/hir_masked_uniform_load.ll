; LIT test to check vector code generation of a uniform load under mask.
; The test checks that we do a scalar load under appropriate if condition.
;
; Scalar HIR:
;    DO i1 = 0, 99, 1   <DO_LOOP>
;      %0 = (%arr)[i1];
;      if (i1 > %0)
;      {
;         %conv = sitofp.i64.double(i1);
;         %1 = (%lp)[0];
;         (%darr)[i1 + %1] = %conv;
;      }
;    END LOOP
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir -disable-output -tbaa < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir,print<hir>" -vplan-force-vf=4 -enable-vp-value-codegen-hir -disable-output < %s 2>&1 | FileCheck %s


define void @foo(i64* nocapture readonly %lp, double* nocapture %darr, i64* nocapture readonly %arr) {
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:       %.vec = (<4 x i64>*)(%arr)[i1];
; CHECK-NEXT:       %.vec1 = i1 + <i64 0, i64 1, i64 2, i64 3> > %.vec;
; CHECK-NEXT:       %.vec2 = sitofp.<4 x i64>.<4 x double>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; CHECK-NEXT:       %0 = bitcast.<4 x i1>.i4(%.vec1);
; CHECK-NEXT:       %cmp = %0 != 0;
; CHECK-NEXT:       %.unifload = undef;
; CHECK-NEXT:       if (%cmp == 1)
; CHECK-NEXT:       {
; CHECK-NEXT:          %.unifload = (%lp)[0];
; CHECK-NEXT:       }
; CHECK-NEXT:       (<4 x double>*)(%darr)[i1 + %.unifload] = %.vec2; Mask = @{%.vec1}
; CHECK-NEXT:  END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i64, i64* %arr, i64 %l1.09
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %cmp1 = icmp sgt i64 %l1.09, %0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %conv = sitofp i64 %l1.09 to double
  %1 = load i64, i64* %lp, align 8, !tbaa !2
  %add = add nsw i64 %1, %l1.09
  %arrayidx2 = getelementptr inbounds double, double* %darr, i64 %add
  store double %conv, double* %arrayidx2, align 8, !tbaa !6
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}


; Scalar HIR:
;    DO i1 = 0, 99, 1   <DO_LOOP>
;      if (%n > 10)
;      {
;         %conv = sitofp.i64.double(i1);
;         %uniform.ldval = (%lp)[0];
;         (%darr)[i1 + %uniform.ldval] = %conv;
;      }
;    END LOOP
;
define void @foo_uniform_if(i64 %n, i64* nocapture readonly %lp, double* nocapture %darr, i64* nocapture readonly %arr) {
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:       %.vec = %n > 10;
; CHECK-NEXT:       %unifcond = extractelement %.vec,  0;
; CHECK-NEXT:       if (%unifcond == 1)
; CHECK-NEXT:       {
; CHECK-NEXT:       }
; CHECK-NEXT:       else
; CHECK-NEXT:       {
; CHECK-NEXT:          goto BB12.35;
; CHECK-NEXT:       }
; CHECK-NEXT:       %.vec1 = sitofp.<4 x i64>.<4 x double>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; CHECK-NEXT:       %.unifload = (%lp)[0];
; CHECK-NEXT:       (<4 x double>*)(%darr)[i1 + %.unifload] = %.vec1;
; CHECK-NEXT:       BB12.35:
; CHECK-NEXT:  END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %uniform.cond = icmp sgt i64 %n, 10
  br i1 %uniform.cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %conv = sitofp i64 %l1.09 to double
  %uniform.ldval = load i64, i64* %lp, align 8, !tbaa !2
  %add = add nsw i64 %uniform.ldval, %l1.09
  %arrayidx2 = getelementptr inbounds double, double* %darr, i64 %add
  store double %conv, double* %arrayidx2, align 8, !tbaa !6
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
