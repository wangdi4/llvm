; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
;
; RUN: opt -enable-new-pm=0 -hir-framework -vpo-wrncollection -vpo-wrninfo -hir-vplan-vec -debug-only=vpo-wrninfo %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-vplan-vec,print<hir-framework>,print<vpo-wrncollection>,require<vpo-wrninfo>" -debug-only=vpo-wrninfo %s 2>&1 | FileCheck %s
; WARNING!!!
; WARNING!!!      ** CONTAINS INTEL IP **
; WARNING!!!      DO NOT SHARE EXTERNALLY
; WARNING!!!

; Test src:
; void foo(int *a, int step) {
;   int x = 10;
;   int n = 2;
; #pragma omp simd linear(n : step) lastprivate(x)
;   for (int i = 0; i < 100; i++) {
;     x = i;
;     a[i] = x;
;     n += step;
;   }
; }
;
;
; ModuleID = 'simd_test.ll'
source_filename = "simd_test.ll"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %a, i32 %step) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %x = alloca i32, align 4
  %n = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 10, i32* %x, align 4, !tbaa !2
  %1 = bitcast i32* %n to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 2, i32* %n, align 4, !tbaa !2
  br label %omp.inner.for.body.lr.ph.split

omp.inner.for.body.lr.ph.split:                   ; preds = %omp.inner.for.body.lr.ph
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32* %n, i32 %step), "QUAL.OMP.LASTPRIVATE"(i32* %x), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
; Check for wregion-collection's prints to ensure clauses from HIR were parsed.
; CHECK: LASTPRIVATE clause (size=1): (&((LINEAR i32* %x)[i64 0])
; CHECK: LINEAR clause (size=1): (&((LINEAR i32* %n)[i64 0])
; CHECK-SAME: LINEAR i32 %step

  %n.promoted = load i32, i32* %n, align 4, !tbaa !2
  %3 = mul i32 %step, 100
  %indvars.iv.in1 = bitcast i64 0 to i64, !in.de.ssa !6
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph.split
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph.split ], !in.de.ssa !6
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %4 = trunc i64 %indvars.iv to i32
  store i32 %4, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  %indvars.iv.in = bitcast i64 %indvars.iv.next to i64, !in.de.ssa !6
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %5 = add i32 %n.promoted, %3
  store i32 99, i32* %x, align 4, !tbaa !2
  store i32 %5, i32* %n, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"indvars.iv.de.ssa"}
; end INTEL_FEATURE_SW_ADVANCED


