; Test for indirect call serialization.
; RUN: opt -vplan-enable-soa=false %s -passes=vplan-vec -vplan-force-vf=2 -S | FileCheck %s
; RUN: opt -vplan-enable-soa=false %s -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -intel-opt-report=high -disable-output 2>&1 | FileCheck %s -check-prefixes=OPTREPORT

; OPTREPORT:      remark #15485: serialized function calls: 1
; OPTREPORT-NEXT: remark #15560: Indirect call cannot be vectorized.
;
; CHECK-LABEL:       vector.body:
; CHECK:    [[F1:%.*]] = extractelement <2 x ptr> [[WIDE_LOAD:%.*]], i32 1
; CHECK-NEXT:    [[F0:%.*]] = extractelement <2 x ptr> [[WIDE_LOAD]], i32 0
; CHECK:    [[ARG1:%.*]] = extractelement <2 x i32> [[WIDE_LOAD1:%.*]], i32 1
; CHECK-NEXT:    [[ARG0:%.*]] = extractelement <2 x i32> [[WIDE_LOAD1]], i32 0
; CHECK-NEXT:    [[CALL0:%.*]] = call i32 [[F0]](i32 [[ARG0]])
; CHECK-NEXT:    [[INS0:%.*]] = insertelement <2 x i32> undef, i32 [[CALL0]], i32 0
; CHECK-NEXT:    [[CALL1:%.*]] = call i32 [[F1]](i32 [[ARG1]])
; CHECK-NEXT:    [[INS1:%.*]] = insertelement <2 x i32> [[INS0]], i32 [[CALL1]], i32 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %a, ptr nocapture readonly %c, ptr nocapture readonly %func, i32 %n) local_unnamed_addr #1 {
entry:
  %b.priv = alloca i32, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
%0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %i.lpriv, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %b.priv, i32 0, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.lpriv, align 4, !tbaa !2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %b.priv) #3
  store i32 0, ptr %b.priv, align 4, !tbaa !2
  %arrayidx = getelementptr inbounds ptr, ptr %func, i64 %indvars.iv
  %2 = load ptr, ptr %arrayidx, align 8, !tbaa !6
  %arrayidx7 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx7, align 4, !tbaa !2
  %call = call i32 %2(i32 %3) #4
  %4 = load i32, ptr %b.priv, align 4, !tbaa !2
  %add8 = add nsw i32 %4, %call
  %5 = load i32, ptr %i.lpriv, align 4, !tbaa !2
  %idxprom9 = sext i32 %5 to i64
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %idxprom9
  %6 = load i32, ptr %arrayidx10, align 4, !tbaa !2
  %add11 = add nsw i32 %6, %add8
  store i32 %add11, ptr %arrayidx10, align 4, !tbaa !2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %b.priv) #3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }
attributes #4 = { noinline nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPFiiE", !4, i64 0}
!8 = !{!9, !3, i64 0}
!9 = !{!"array@_ZTSA256_i", !3, i64 0}
!10 = !{!11, !7, i64 0}
!11 = !{!"array@_ZTSA256_PFiiE", !7, i64 0}
