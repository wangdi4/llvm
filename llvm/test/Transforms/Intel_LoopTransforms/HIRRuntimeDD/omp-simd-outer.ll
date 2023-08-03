; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Loops inside SIMD regions should not be multiversioned.

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(64),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%block_id.linear.iv)[0])1) ]
;       %conv = sitofp.i32.double(%midPoint);
;
;       + DO i1 = 0, %block_end + -1, 1   <DO_LOOP> <simd>
;       |   @llvm.lifetime.start.p0(8,  &((i8*)(%block_id.linear.iv)[0]));
;       |   (%block_id.linear.iv)[0] = i1;
;       |   if (%trip_count >= 1)
;       |   {
;       |      + DO i2 = 0, sext.i32.i64((1 + %trip_count)) + -2, 1   <DO_LOOP>
;       |      |   %3 = (%sfLema)[i2 + 1];
;       |      |   %4 = (%lema)[i2 + 1];
;       |      |   %sub8 = %conv  -  %4;
;       |      |   %mul9 = %3  *  %sub8;
;       |      |   %add12 = %4  +  %mul9;
;       |      |   (%lema)[i2 + 1] = %add12;
;       |      |   %5 = (%sfSema)[i2 + 1];
;       |      |   %6 = (%sema)[i2 + 1];
;       |      |   %sub18 = %conv  -  %6;
;       |      |   %mul19 = %5  *  %sub18;
;       |      |   %add22 = %6  +  %mul19;
;       |      |   (%sema)[i2 + 1] = %add22;
;       |      + END LOOP
;       |   }
;       |   @llvm.lifetime.end.p0(8,  &((i8*)(%block_id.linear.iv)[0]));
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; END REGION

; CHECK: BEGIN REGION
; CHECK: DO i2
; CHECK-NOT: DO i2
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %lema, ptr nocapture %sema, i32 %trip_count, i32 %midPoint, i64 %block_end, ptr nocapture readonly %sfLema, ptr nocapture readonly %sfSema) local_unnamed_addr #0 {
entry:
  %block_id.linear.iv = alloca i64, align 8
  %cmp = icmp eq i64 %block_end, 0
  br i1 %cmp, label %omp.precond.end, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 64), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %block_id.linear.iv, i32 1) ]
  %cmp540 = icmp slt i32 %trip_count, 1
  %conv = sitofp i32 %midPoint to double
  %1 = add nuw nsw i32 %trip_count, 1
  %wide.trip.count = sext i32 %1 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %for.cond.cleanup
  %.omp.iv.local.034 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add23, %for.cond.cleanup ]
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %block_id.linear.iv) #2
  store i64 %.omp.iv.local.034, ptr %block_id.linear.iv, align 8, !tbaa !2
  br i1 %cmp540, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %omp.inner.for.body
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %block_id.linear.iv) #2
  %add23 = add nuw i64 %.omp.iv.local.034, 1
  %exitcond42 = icmp eq i64 %add23, %block_end
  br i1 %exitcond42, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 1, %for.body.preheader ]
  %ptridx = getelementptr inbounds double, ptr %sfLema, i64 %indvars.iv
  %2 = load double, ptr %ptridx, align 8, !tbaa !6
  %ptridx7 = getelementptr inbounds double, ptr %lema, i64 %indvars.iv
  %3 = load double, ptr %ptridx7, align 8, !tbaa !6
  %sub8 = fsub double %conv, %3
  %mul9 = fmul double %2, %sub8
  %add12 = fadd double %3, %mul9
  store double %add12, ptr %ptridx7, align 8, !tbaa !6
  %ptridx14 = getelementptr inbounds double, ptr %sfSema, i64 %indvars.iv
  %4 = load double, ptr %ptridx14, align 8, !tbaa !6
  %ptridx17 = getelementptr inbounds double, ptr %sema, i64 %indvars.iv
  %5 = load double, ptr %ptridx17, align 8, !tbaa !6
  %sub18 = fsub double %conv, %5
  %mul19 = fmul double %4, %sub18
  %add22 = fadd double %5, %mul19
  store double %add22, ptr %ptridx17, align 8, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

DIR.OMP.END.SIMD.3:                               ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"double", !4, i64 0}
