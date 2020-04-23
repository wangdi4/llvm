; RUN: opt -vector-library=SVML -VPlanDriver -S -vplan-force-vf=4  %s | FileCheck -DVL=4 %s
; RUN: opt -vector-library=SVML -VPlanDriver -S -vplan-force-vf=8  %s | FileCheck -DVL=8 %s
; RUN: opt -vector-library=SVML -VPlanDriver -S -vplan-force-vf=16 %s | FileCheck -DVL=16 %s
; RUN: opt -vector-library=SVML -VPlanDriver -S -vplan-force-vf=32 %s | FileCheck -DVL=32 %s

; TODO: Fast-math flags are not represented in VPValue yet. Update check when feature is implemented.
; CHECK-LABEL: test_sinf
; CHECK:  [[RESULT:%.*]] = call svml_cc <[[VL]] x float> @__svml_sinf[[VL]](<[[VL]] x float> {{.*}})
; CHECK:  [[PTR:%.*]] = bitcast float* {{.*}} to <[[VL]] x float>*
; CHECK:  store <[[VL]] x float> [[RESULT]], <[[VL]] x float>* [[PTR]], align 4

; CHECK-LABEL: test_sin
; CHECK:  [[RESULT:%.*]] = call svml_cc <[[VL]] x double> @__svml_sin[[VL]](<[[VL]] x double> {{.*}})
; CHECK:  [[PTR:%.*]] = bitcast double* {{.*}} to <[[VL]] x double>*
; CHECK:  store <[[VL]] x double> [[RESULT]], <[[VL]] x double>* [[PTR]], align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define x86_regcallcc void @test_sinf(float* nocapture %a, float* nocapture readonly %b, i32 %i) local_unnamed_addr #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM"(float* %a, float* %b) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds float, float* %b, i64 %stride.add
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %call = tail call fast float @sinf(float %0) #3
  %arrayidx2 = getelementptr inbounds float, float* %a, i64 %stride.add
  store float %call, float* %arrayidx2, align 4, !tbaa !1
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind uwtable
define x86_regcallcc void @test_sin(double* nocapture %a, double* nocapture readonly %b, i32 %i) local_unnamed_addr #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM"(double* %a, double* %b) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds double, double* %b, i64 %stride.add
  %0 = load double, double* %arrayidx, align 8
  %call = tail call fast double @sin(double %0) #3
  %arrayidx2 = getelementptr inbounds double, double* %a, i64 %stride.add
  store double %call, double* %arrayidx2, align 8
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind readnone
declare float @sinf(float) local_unnamed_addr #1

declare double @sin(double) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind readnone }


!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!"DIR.OMP.SIMD"}
!6 = !{!"QUAL.OMP.SIMDLEN"}
!7 = !{!"QUAL.OMP.UNIFORM"}
!8 = !{!"QUAL.OMP.PRIVATE"}
!9 = !{!"QUAL.OMP.LINEAR"}
!10 = !{!"DIR.QUAL.LIST.END"}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.unroll.disable"}
!13 = !{!"DIR.OMP.END.SIMD"}
!14 = distinct !{!14, !12}
!15 = distinct !{!15, !12}
!16 = distinct !{!16, !12}
!17 = distinct !{!17, !12}
!18 = distinct !{!18, !12}
!19 = distinct !{!19, !12}
!20 = distinct !{!20, !12}
