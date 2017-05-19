;RUN: opt -vector-library=SVML -vec-clone -VPODriver -disable-vplan-subregions -disable-vplan-predicator -disable-vplan-codegen -S %s | FileCheck %s

;CHECK: call void @__svml_sincosf4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define x86_regcallcc void @foo(float* noalias nocapture readonly %a, float* noalias %vsin, float* noalias %vcos, i32 %i) local_unnamed_addr #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  call void @llvm.intel.directive(metadata !5)
  call void @llvm.intel.directive.qual.opnd.i32(metadata !6, i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !7, float* %a, float* %vsin, float* %vcos)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !9, i32 %i, i32 1)
  call void @llvm.intel.directive(metadata !10)
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %idxprom = sext i32 %i to i64
  %stride.mul = mul i32 1, %index
  %stride.cast = sext i32 %stride.mul to i64
  %stride.add = add i64 %idxprom, %stride.cast
  %arrayidx = getelementptr inbounds float, float* %a, i64 %stride.add
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds float, float* %vsin, i64 %stride.add
  %arrayidx4 = getelementptr inbounds float, float* %vcos, i64 %stride.add
  tail call void @sincosf(float %0, float* %arrayidx2, float* %arrayidx4) #3
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.intel.directive(metadata !13)
  call void @llvm.intel.directive(metadata !10)
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare void @sincosf(float, float*, float*) local_unnamed_addr #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20624)"}
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
