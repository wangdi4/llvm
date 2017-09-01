; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we create a liveout copy for %s.41037 to resolve live range issue.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %s.41037.out = %s.41037;
; CHECK: |   %s.41037 = -0.000000e+00  -  %s.41037;
; CHECK: |   %add941042 = %add941042  -  %s.41037.out;
; CHECK: |   %u.01036 = %u.01036  +  %in1;
; CHECK: |   %sub96 = %s.41037  -  %u.01036;
; CHECK: |   %x.21035 = %x.21035  +  %sub96;
; CHECK: |   %mul98 = %u.01036  *  %s.41037;
; CHECK: |   %v.11033 = %v.11033  -  %mul98;
; CHECK: |   %div100 = %s.41037  /  %u.01036;
; CHECK: |   %w.01034 = %w.01034  +  %div100;
; CHECK: + END LOOP


define void @foo(double %sa.promoted, double %s.3.lcssa, double %in, double %in1, i64 %n) {
entry:
  br label %for.body92

for.body92:                                       ; preds = %for.body92, %entry
  %add941042 = phi double [ %sa.promoted, %entry ], [ %add94, %for.body92 ]
  %i.31038 = phi i64 [ 1, %entry ], [ %inc103, %for.body92 ]
  %s.41037 = phi double [ %s.3.lcssa, %entry ], [ %sub93, %for.body92 ]
  %u.01036 = phi double [ %in, %entry ], [ %add95, %for.body92 ]
  %x.21035 = phi double [ 0.000000e+00, %entry ], [ %add97, %for.body92 ]
  %w.01034 = phi double [ 0.000000e+00, %entry ], [ %add101, %for.body92 ]
  %v.11033 = phi double [ 0.000000e+00, %entry ], [ %sub99, %for.body92 ]
  %sub93 = fsub double -0.000000e+00, %s.41037
  %add94 = fsub double %add941042, %s.41037
  %add95 = fadd double %u.01036, %in1
  %sub96 = fsub double %sub93, %add95
  %add97 = fadd double %x.21035, %sub96
  %mul98 = fmul double %add95, %sub93
  %sub99 = fsub double %v.11033, %mul98
  %div100 = fdiv double %sub93, %add95
  %add101 = fadd double %w.01034, %div100
  %inc103 = add nuw nsw i64 %i.31038, 1
  %exitcond1062 = icmp eq i64 %i.31038, %n
  br i1 %exitcond1062, label %for.cond89.for.end104_crit_edge, label %for.body92

for.cond89.for.end104_crit_edge:                  ; preds = %for.body92
  %add94.lcssa = phi double [ %add94, %for.body92 ]
  %add97.lcssa = phi double [ %add97, %for.body92 ]
  %sub99.lcssa = phi double [ %sub99, %for.body92 ]
  %add101.lcssa = phi double [ %add101, %for.body92 ]
  ret void
}
