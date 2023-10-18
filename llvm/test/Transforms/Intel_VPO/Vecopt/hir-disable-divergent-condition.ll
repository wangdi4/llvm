; Test the -vplan-vectorize-divergent-branch-loops flag.  This flag is
; intended for use by the driver when -fprofile-sample-generate is selected.
; SPGO sampling needs to avoid collapsing the loop's internal control flow
; into straight-line masked operations.

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-enable-divergent-branches=0 -intel-opt-report=medium -disable-output %s 2>&1 | FileCheck %s --check-prefix=DISABLE

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-enable-divergent-branches=1 -intel-opt-report=medium -disable-output %s 2>&1 | FileCheck %s --check-prefix=ENABLE

; DISABLE: remark #15436: loop was not vectorized: The loop contains a divergent conditional branch, and the user has suppressed vectorization of such loops.

; ENABLE: remark #15300: LOOP WAS VECTORIZED

define void @foo(ptr noalias %x, ptr noalias %y, ptr noalias %z, i64 %n) {
entry:
  %cmp33.not = icmp eq i64 %n, 0
  br i1 %cmp33.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:
  br label %for.body

for.body:
  %i.034 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, ptr %x, i64 %i.034
  %0 = load double, ptr %arrayidx, align 8
  %cmp1 = fcmp fast oeq double %0, 0.000000e+00
  %1 = getelementptr inbounds double, ptr %z, i64 %i.034
  %2 = load double, ptr %1, align 8
  br i1 %cmp1, label %if.then, label %if.else

if.then:
  %mul.if = fmul fast double %2, 4.000000e+00
  br label %for.inc

if.else:
  %mul.else = fmul fast double %2, -3.300000e+00
  br label %for.inc

for.inc:
  %sub15.sink = phi double [ %mul.if, %if.then ], [ %mul.else, %if.else ]
  %3 = getelementptr inbounds double, ptr %y, i64 %i.034
  store double %sub15.sink, ptr %3, align 8
  %inc = add nuw i64 %i.034, 1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void
}
