; Test to check that memory allocated for VPPrivate alias instructions is not
; leaked in scenarios where alias is not lowered into explicit VPInstruction in
; VPlan CFG (consider early bailout after entities importing). We were hitting
; an assert in VPValue's destructor that an extraneous user was not dropped
; before deleting the VPValue.

; RUN: opt -vplan-vec -vplan-force-vf=4 -S < %s | FileCheck %s

; CHECK: @foo

define internal void @foo(i32* %temp42, i32* %temp44, i32 %ub.new) {
entry:
  %"target_parallel_do_simd_$I0.linear" = alloca i32
  %temp27 = alloca float
  %temp = alloca float
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %bb8
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"target_parallel_do_simd_$I0.linear", i32 1), "QUAL.OMP.PRIVATE"(float* %temp27), "QUAL.OMP.PRIVATE"(float* %temp) ]
  br label %DIR.OMP.SIMD.11

DIR.OMP.SIMD.11:                                  ; preds = %DIR.OMP.SIMD.1
  %"(i8*)temp$" = bitcast float* %temp27 to i8*
  %"(i8*)temp$24" = bitcast float* %temp to i8*
  br label %bb12

bb12:                                             ; preds = %DIR.OMP.SIMD.11, %bb12
  %temp46.local.0 = phi i32 [ %add33, %bb12 ], [ 0, %DIR.OMP.SIMD.11 ]
  %temp_fetch6 = load i32, i32* %temp42, align 1
  %temp_fetch8 = load i32, i32* %temp44, align 1
  call void @myfunc(i8* nonnull %"(i8*)temp$", i8* nonnull %"(i8*)temp$24")
  %add33 = add nuw i32 %temp46.local.0, 1
  %exitcond = icmp eq i32 %temp46.local.0, %ub.new
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %bb12

DIR.OMP.END.SIMD.2:                               ; preds = %bb12
  %temp46.local.0.lcssa = phi i32 [ %temp46.local.0, %bb12 ]
  %temp_fetch6.lcssa = phi i32 [ %temp_fetch6, %bb12 ]
  %temp_fetch8.lcssa = phi i32 [ %temp_fetch8, %bb12 ]
  %mul12.le = mul nsw i32 %temp_fetch8.lcssa, %temp46.local.0.lcssa
  %add14.le = add nsw i32 %mul12.le, %temp_fetch6.lcssa
  store i32 %add14.le, i32* %"target_parallel_do_simd_$I0.linear", align 8
  br label %DIR.OMP.END.SIMD.22

DIR.OMP.END.SIMD.22:                              ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @myfunc(i8* nonnull, i8* nonnull)
