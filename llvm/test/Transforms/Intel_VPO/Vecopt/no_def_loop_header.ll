; RUN: opt -S -VPlanDriver -vpo-vplan-build-stress-test -debug-only=vplan-divergence-analysis < %s --disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

define dso_local void @test1(i32 *%a) {
entry:
  br label %simd.begin

simd.begin:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %preheader

preheader:
  %0 = load i32, i32* %a, align 4
  %idxprom25 = sext i32 %0 to i64
  br label %header

;; %ld is divergent and its divergence makes inner loop backedge divergent too.
;; That means that different lanes [for outer loops] might have different trip
;; counts for the inner loops making any inner IV divergent too.

header:
  %iv = phi i32 [ %iv.next, %latch ], [ 0, %preheader ]
; CHECK: Divergent: [Shape: Random] i32 [[IV:%vp[0-9]+]] = phi  [ i32 [[IV_NEXT:%vp[0-9]+]], BB5 ],  [ i32 0, BB2 ]
  br label %exiting1

exiting1:
  %gep = getelementptr inbounds i32, i32* %a, i32 %iv
; CHECK: Divergent: [Shape: Random] i32* [[GEP:%vp[0-9]+]] = getelementptr inbounds i32* %a i32 [[IV]]
  %ld = load i32, i32* %gep, align 8
; CHECK: Divergent: [Shape: Random] i32 [[LD:%vp[0-9]+]] = load i32* [[GEP]]
  %cmp1 = icmp ne i32 %ld, 3
; CHECK: Divergent: [Shape: Random] i1 [[CMP1:%vp[0-9]+]] = icmp i32 [[LD]] i32 3
  br i1 %cmp1, label %exiting2, label %loop.exit

exiting2:
  %cmp2 = icmp slt i32 %iv, 42
; CHECK: Divergent: [Shape: Random] i1 [[CMP2:%vp[0-9]+]] = icmp i32 [[IV]] i32 42
  br i1 %cmp2, label %latch, label %loop.exit

latch:
  %iv.next = add i32 %iv, 1
; CHECK: Divergent: [Shape: Random] i32 [[IV_NEXT]] = add i32 [[IV]] i32 1
  br label %header

loop.exit:
  br label %simd.end

simd.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
