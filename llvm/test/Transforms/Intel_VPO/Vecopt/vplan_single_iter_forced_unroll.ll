; RUN: opt -passes='vplan-vec,print' -vplan-force-vf=4 -vplan-force-uf=2 -disable-output < %s 2>&1 | FileCheck %s
;
; When the combination of VF/UF results in a one-iteration vector loop, we try to
; optimize the loop out at VPlan level in the LLVM IR path as later transforms
; are not always able to clean up the same. Doing this too early was causing
; couple of issues.
; - trying to set branch latch condition during unroll in VPlan was hitting
; an assert once the branch condition is changed to a constant(true/false).
; - the code which tries to set the unroll factor in VectorTripCountCalculation
; does not kick in once the latch condition is set to a constant.
;
; To fix the issue, we defer the change that optimizes out the loop to run after
; unroller has run.
;
define void @foo() #0 {
; CHECK:       vector.body:
; CHECK:         call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK:         br label %[[VPLANNEDBB3:.*]]
; CHECK:       [[VPLANNEDBB3]]:
; CHECK-NEXT:    call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK-NEXT:    call void @baz()
; CHECK:         br i1 false, label %vector.body, label [[VPLANNEDBB4:%.*]]
;
entry:
  %entry.region1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop.preheader

loop.preheader:
  br label %loop

loop:
  %index = phi i32 [ 0, %loop.preheader ], [ %indvar, %loop ]
  call void @baz()
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %loop, label %end.region

end.region:
  call void @llvm.directive.region.exit(token %entry.region1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @baz()
