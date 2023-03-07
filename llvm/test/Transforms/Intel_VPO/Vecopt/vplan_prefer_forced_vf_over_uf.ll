;
; RUN: opt -passes='vplan-vec,print' -vplan-force-vf=4 -vplan-force-uf=2 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vplan-vec,print' -vplan-force-vf=8 -vplan-force-uf=2 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vplan-vec,print' -vplan-force-vf=4 -vplan-force-uf=4 -disable-output < %s 2>&1 | FileCheck %s
;
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -vplan-force-uf=2 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=8 -vplan-force-uf=2 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -vplan-force-uf=4 -disable-output < %s 2>&1 | FileCheck %s
;
; When ForcedVF*ForcedUF exceeds known trip count of the loop, we were
; bailing out of vectorization altogether. We now give precedence to
; ForcedVF and unroll as much as possible. LIT test to check for the
; same.
;
define void @foo() #0 {
; CHECK:         @baz()
; CHECK-NEXT:    @baz()
; CHECK-NEXT:    @baz()
; CHECK-NEXT:    @baz()
; 
; CHECK:         @baz()
; CHECK-NEXT:    @baz()
; CHECK-NEXT:    @baz()
; CHECK-NEXT:    @baz()
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
declare void @baz() nounwind
