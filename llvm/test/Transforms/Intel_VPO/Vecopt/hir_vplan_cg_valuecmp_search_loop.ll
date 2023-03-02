; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -hir-details-no-verbose-indent -vplan-force-vf=4 -debug-only=VPOCGHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=SEARCHLOOP
; BEGIN REGION { modified }
;      %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;      + DO i1 = 0, 1023, 1   <DO_MULTI_EXIT_LOOP>
;      |   if ((ptr)(%sp)[i1] == &((%lp)[0]))
;      |   {
;      |      %l1.07.out = i1;
;      |      goto for.end;
;      |   }
;      + END LOOP
;
;      @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec' -debug-only=vplan-idioms -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=NOTSEARCHLOOP
; BEGIN REGION { modified }
;      %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;      + DO i1 = 0, 1023, 1   <DO_MULTI_EXIT_LOOP>
;      |   %0 = (ptr)(%sp)[i1];
;      |   if (&((%0)[0]) == &((%lp)[0]))
;      |   {
;      |      %l1.07.out = i1;
;      |      goto for.end;
;      |   }
;      + END LOOP
;
;      @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; REQUIRES: asserts

; SEARCHLOOP:       VPCodegen: First Pred WideInst: %wide.cmp. = (<4 x ptr>*)(%sp)[i1 + %peel.factor1] == &((<4 x ptr>)(%lp)[0]);
;
; SEARCHLOOP:       + DO i1 = 0, 4 {{.*}}   <DO_MULTI_EXIT_LOOP> <auto-vectorized> <nounroll> <novectorize>
; SEARCHLOOP-NEXT:  |   %wide.cmp. = (<4 x ptr>*)(%sp)[i1 + %peel.factor1] == &((<4 x ptr>)(%lp)[0]);
;
; NOTSEARCHLOOP:   HLInst <4> %0 = (ptr)(%sp)[i1]; 
; NOTSEARCHLOOP:   Search loop idiom was not recognized.
;
%struct.S1 = type { %struct.S2 }
%struct.S2 = type { ptr }

define i64 @foo(ptr %lp, ptr %sp) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.07 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %a = getelementptr inbounds %struct.S1, ptr %sp, i64 %l1.07
  %0 = load ptr, ptr %a, align 8
  %cmp1 = icmp eq ptr %0, %lp
  br i1 %cmp1, label %for.end, label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nuw nsw i64 %l1.07, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %for.inc
  %l1.0.lcssa = phi i64 [ %l1.07, %for.body ], [ 1024, %for.inc ]
  ret i64 %l1.0.lcssa
}
