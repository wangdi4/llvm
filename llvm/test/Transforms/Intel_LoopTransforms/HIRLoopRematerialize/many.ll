; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-rematerialize,print<hir>" -aa-pipeline="basic-aa" -hir-loop-rematerialize-tc-lb=1 -hir-allow-loop-materialization-regions=true < %s 2>&1 -disable-output | FileCheck %s

; TODO: copy_v4 - extract to another lit test.
;       Previously before opaquifying the llvm IR, copy_v4 was loop rematerialized as before.
;       No londer so.

; Before transform
; Function: copy_v4

;         BEGIN REGION { }
;               (i64*)(%a)[0] = (i64*)(%b)[0];
;               (i64*)(%a)[1] = (i64*)(%b)[1];
;               (i64*)(%a)[2] = (i64*)(%b)[2];
;               (i64*)(%a)[3] = (i64*)(%b)[3];
;               ret ;
;         END REGION

; After transform
; Function: copy_v4

;          BEGIN REGION { }
;                + DO i1 = 0, 3, 1   <DO_LOOP>
;                |   (i64*)(%a)[i1] = (i64*)(%b)[i1];
;                + END LOOP

;                ret ;
;          END REGION

; CHECK-LABEL: Function: copy_v4

; CHECK:         BEGIN REGION { }
; CHECK:               (%a)[0] = (%b)[0];
; CHECK:               (i64*)(%a)[1] = (i64*)(%b)[1];
; CHECK:               (i64*)(%a)[2] = (i64*)(%b)[2];
; CHECK:               (i64*)(%a)[3] = (i64*)(%b)[3];
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK-LABEL: Function: copy_v4

; CHECK:          BEGIN REGION { }
; CHECK:               (%a)[0] = (%b)[0];
; CHECK:               (i64*)(%a)[1] = (i64*)(%b)[1];
; CHECK:               (i64*)(%a)[2] = (i64*)(%b)[2];
; CHECK:               (i64*)(%a)[3] = (i64*)(%b)[3];
; CHECK:               ret ;
; CHECK:          END REGION

; CHECK-LABEL: Function: add_v3

; CHECK:         BEGIN REGION { }
; CHECK:               %add = (%b)[0]  +  (%a)[0];
; CHECK:               (%a)[0] = %add;
; CHECK:               %add4 = (%b)[1]  +  (%a)[1];
; CHECK:               (%a)[1] = %add4;
; CHECK:               %add7 = (%b)[2]  +  (%a)[2];
; CHECK:               (%a)[2] = %add7;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK-LABEL: Function: add_v3

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   %add = (%b)[i1]  +  (%a)[i1];
; CHECK:               |   (%a)[i1] = %add;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:         END REGION

; CHECK-LABEL: Function: mul_v3_fl

; CHECK:          BEGIN REGION { }
; CHECK:                %mul = (%a)[0]  *  %f;
; CHECK:                (%a)[0] = %mul;
; CHECK:                %mul2 = (%a)[1]  *  %f;
; CHECK:                (%a)[1] = %mul2;
; CHECK:               %mul4 = (%a)[2]  *  %f;
; CHECK:               (%a)[2] = %mul4;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: mul_v3_fl
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %mul = (%a)[i1]  *  %f;
; CHECK:                |   (%a)[i1] = %mul;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: mul_v3_v3fl
;
; CHECK:          BEGIN REGION { }
; CHECK:                %mul = (%b)[0]  *  %f;
; CHECK:                (%a)[0] = %mul;
; CHECK:                %mul3 = (%b)[1]  *  %f;
; CHECK:                (%a)[1] = %mul3;
; CHECK:               %mul6 = (%b)[2]  *  %f;
; CHECK:               (%a)[2] = %mul6;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: mul_v3_v3fl
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %mul = (%b)[i1]  *  %f;
; CHECK:                |   (%a)[i1] = %mul;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: madd_v3_v3fl
;
; CHECK:          BEGIN REGION { }
; CHECK:                %mul = (%b)[0]  *  %f;
; CHECK:                %add = (%a)[0]  +  %mul;
; CHECK:                (%a)[0] = %add;
; CHECK:                %mul3 = (%b)[1]  *  %f;
; CHECK:               %add5 = (%a)[1]  +  %mul3;
; CHECK:               (%a)[1] = %add5;
; CHECK:               %mul7 = (%b)[2]  *  %f;
; CHECK:               %add9 = (%a)[2]  +  %mul7;
; CHECK:               (%a)[2] = %add9;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: madd_v3_v3fl
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %mul = (%b)[i1]  *  %f;
; CHECK:                |   %add = (%a)[i1]  +  %mul;
; CHECK:                |   (%a)[i1] = %add;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: negate_v3_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:                %sub = -0.000000e+00  -  (%a)[0];
; CHECK:                (%r)[0] = %sub;
; CHECK:                %sub3 = -0.000000e+00  -  (%a)[1];
; CHECK:                (%r)[1] = %sub3;
; CHECK:               %sub6 = -0.000000e+00  -  (%a)[2];
; CHECK:               (%r)[2] = %sub6;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: negate_v3_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %sub = -0.000000e+00  -  (%a)[i1];
; CHECK:                |   (%r)[i1] = %sub;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: normal_short_to_float_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:                %conv1 = sitofp.i16.float((%in)[0]);
; CHECK:                %mul = %conv1  *  0x3F00002000000000;
; CHECK:                (%out)[0] = %mul;
; CHECK:                %conv5 = sitofp.i16.float((%in)[1]);
; CHECK:                %mul6 = %conv5  *  0x3F00002000000000;
; CHECK:               (%out)[1] = %mul6;
; CHECK:               %conv10 = sitofp.i16.float((%in)[2]);
; CHECK:               %mul11 = %conv10  *  0x3F00002000000000;
; CHECK:               (%out)[2] = %mul11;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: normal_short_to_float_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %conv1 = sitofp.i16.float((%in)[i1]);
; CHECK:                |   %mul = %conv1  *  0x3F00002000000000;
; CHECK:                |   (%out)[i1] = %mul;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: normal_float_to_short_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:                %mul = (%in)[0]  *  3.276700e+04;
; CHECK:                %conv = fptosi.float.i16(%mul);
; CHECK:                (%out)[0] = %conv;
; CHECK:                %mul3 = (%in)[1]  *  3.276700e+04;
; CHECK:                %conv4 = fptosi.float.i16(%mul3);
; CHECK:               (%out)[1] = %conv4;
; CHECK:               %mul7 = (%in)[2]  *  3.276700e+04;
; CHECK:               %conv8 = fptosi.float.i16(%mul7);
; CHECK:               (%out)[2] = %conv8;
; CHECK:               ret ;
; CHECK:          END REGION
;
; CHECK-LABEL: Function: normal_float_to_short_v3
;
; CHECK:          BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   %mul = (%in)[i1]  *  3.276700e+04;
; CHECK:                |   %conv = fptosi.float.i16(%mul);
; CHECK:                |   (%out)[i1] = %conv;
; CHECK:               + END LOOP
;
; CHECK:               ret ;
; CHECK:          END REGION


;Module Before HIR
; ModuleID = 'many.c'
source_filename = "many.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @copy_v4(ptr noalias nocapture %a, ptr noalias nocapture readonly %b) local_unnamed_addr #1 {
entry:
  %0 = load i64, ptr %b, align 8, !tbaa !2
  store i64 %0, ptr %a, align 8, !tbaa !2
  %arrayidx2 = getelementptr inbounds double, ptr %b, i64 1
  %1 = load i64, ptr %arrayidx2, align 8, !tbaa !2
  %arrayidx3 = getelementptr inbounds double, ptr %a, i64 1
  store i64 %1, ptr %arrayidx3, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds double, ptr %b, i64 2
  %2 = load i64, ptr %arrayidx4, align 8, !tbaa !2
  %arrayidx5 = getelementptr inbounds double, ptr %a, i64 2
  store i64 %2, ptr %arrayidx5, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds double, ptr %b, i64 3
  %3 = load i64, ptr %arrayidx6, align 8, !tbaa !2
  %arrayidx7 = getelementptr inbounds double, ptr %a, i64 3
  store i64 %3, ptr %arrayidx7, align 8, !tbaa !2
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @add_v3(ptr noalias nocapture %a, ptr noalias nocapture readonly %b) local_unnamed_addr #1 {
entry:
  %0 = load double, ptr %b, align 8, !tbaa !2
  %1 = load double, ptr %a, align 8, !tbaa !2
  %add = fadd double %0, %1
  store double %add, ptr %a, align 8, !tbaa !2
  %arrayidx2 = getelementptr inbounds double, ptr %b, i64 1
  %2 = load double, ptr %arrayidx2, align 8, !tbaa !2
  %arrayidx3 = getelementptr inbounds double, ptr %a, i64 1
  %3 = load double, ptr %arrayidx3, align 8, !tbaa !2
  %add4 = fadd double %2, %3
  store double %add4, ptr %arrayidx3, align 8, !tbaa !2
  %arrayidx5 = getelementptr inbounds double, ptr %b, i64 2
  %4 = load double, ptr %arrayidx5, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds double, ptr %a, i64 2
  %5 = load double, ptr %arrayidx6, align 8, !tbaa !2
  %add7 = fadd double %4, %5
  store double %add7, ptr %arrayidx6, align 8, !tbaa !2
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @mul_v3_fl(ptr noalias nocapture %a, float %f) local_unnamed_addr #1 {
entry:
  %0 = load float, ptr %a, align 4, !tbaa !6
  %mul = fmul float %0, %f
  store float %mul, ptr %a, align 4, !tbaa !6
  %arrayidx1 = getelementptr inbounds float, ptr %a, i64 1
  %1 = load float, ptr %arrayidx1, align 4, !tbaa !6
  %mul2 = fmul float %1, %f
  store float %mul2, ptr %arrayidx1, align 4, !tbaa !6
  %arrayidx3 = getelementptr inbounds float, ptr %a, i64 2
  %2 = load float, ptr %arrayidx3, align 4, !tbaa !6
  %mul4 = fmul float %2, %f
  store float %mul4, ptr %arrayidx3, align 4, !tbaa !6
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @mul_v3_v3fl(ptr noalias nocapture %a, ptr nocapture readonly %b, float %f) local_unnamed_addr #1 {
entry:
  %0 = load float, ptr %b, align 4, !tbaa !6
  %mul = fmul float %0, %f
  store float %mul, ptr %a, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds float, ptr %b, i64 1
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !6
  %mul3 = fmul float %1, %f
  %arrayidx4 = getelementptr inbounds float, ptr %a, i64 1
  store float %mul3, ptr %arrayidx4, align 4, !tbaa !6
  %arrayidx5 = getelementptr inbounds float, ptr %b, i64 2
  %2 = load float, ptr %arrayidx5, align 4, !tbaa !6
  %mul6 = fmul float %2, %f
  %arrayidx7 = getelementptr inbounds float, ptr %a, i64 2
  store float %mul6, ptr %arrayidx7, align 4, !tbaa !6
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @madd_v3_v3fl(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, float %f) local_unnamed_addr #1 {
entry:
  %0 = load float, ptr %b, align 4, !tbaa !6
  %mul = fmul float %0, %f
  %1 = load float, ptr %a, align 4, !tbaa !6
  %add = fadd float %1, %mul
  store float %add, ptr %a, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds float, ptr %b, i64 1
  %2 = load float, ptr %arrayidx2, align 4, !tbaa !6
  %mul3 = fmul float %2, %f
  %arrayidx4 = getelementptr inbounds float, ptr %a, i64 1
  %3 = load float, ptr %arrayidx4, align 4, !tbaa !6
  %add5 = fadd float %3, %mul3
  store float %add5, ptr %arrayidx4, align 4, !tbaa !6
  %arrayidx6 = getelementptr inbounds float, ptr %b, i64 2
  %4 = load float, ptr %arrayidx6, align 4, !tbaa !6
  %mul7 = fmul float %4, %f
  %arrayidx8 = getelementptr inbounds float, ptr %a, i64 2
  %5 = load float, ptr %arrayidx8, align 4, !tbaa !6
  %add9 = fadd float %5, %mul7
  store float %add9, ptr %arrayidx8, align 4, !tbaa !6
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @negate_v3_v3(ptr noalias nocapture %r, ptr noalias nocapture readonly %a) local_unnamed_addr #1 {
entry:
  %0 = load float, ptr %a, align 4, !tbaa !6
  %sub = fsub float -0.000000e+00, %0
  store float %sub, ptr %r, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds float, ptr %a, i64 1
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !6
  %sub3 = fsub float -0.000000e+00, %1
  %arrayidx4 = getelementptr inbounds float, ptr %r, i64 1
  store float %sub3, ptr %arrayidx4, align 4, !tbaa !6
  %arrayidx5 = getelementptr inbounds float, ptr %a, i64 2
  %2 = load float, ptr %arrayidx5, align 4, !tbaa !6
  %sub6 = fsub float -0.000000e+00, %2
  %arrayidx7 = getelementptr inbounds float, ptr %r, i64 2
  store float %sub6, ptr %arrayidx7, align 4, !tbaa !6
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @normal_short_to_float_v3(ptr noalias nocapture %out, ptr nocapture readonly %in) local_unnamed_addr #1 {
entry:
  %0 = load i16, ptr %in, align 2, !tbaa !8
  %conv1 = sitofp i16 %0 to float
  %mul = fmul float %conv1, 0x3F00002000000000
  store float %mul, ptr %out, align 4, !tbaa !6
  %arrayidx3 = getelementptr inbounds i16, ptr %in, i64 1
  %1 = load i16, ptr %arrayidx3, align 2, !tbaa !8
  %conv5 = sitofp i16 %1 to float
  %mul6 = fmul float %conv5, 0x3F00002000000000
  %arrayidx7 = getelementptr inbounds float, ptr %out, i64 1
  store float %mul6, ptr %arrayidx7, align 4, !tbaa !6
  %arrayidx8 = getelementptr inbounds i16, ptr %in, i64 2
  %2 = load i16, ptr %arrayidx8, align 2, !tbaa !8
  %conv10 = sitofp i16 %2 to float
  %mul11 = fmul float %conv10, 0x3F00002000000000
  %arrayidx12 = getelementptr inbounds float, ptr %out, i64 2
  store float %mul11, ptr %arrayidx12, align 4, !tbaa !6
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local void @normal_float_to_short_v3(ptr nocapture %out, ptr noalias nocapture readonly %in) local_unnamed_addr #1 {
entry:
  %0 = load float, ptr %in, align 4, !tbaa !6
  %mul = fmul float %0, 3.276700e+04
  %conv = fptosi float %mul to i16
  store i16 %conv, ptr %out, align 2, !tbaa !8
  %arrayidx2 = getelementptr inbounds float, ptr %in, i64 1
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !6
  %mul3 = fmul float %1, 3.276700e+04
  %conv4 = fptosi float %mul3 to i16
  %arrayidx5 = getelementptr inbounds i16, ptr %out, i64 1
  store i16 %conv4, ptr %arrayidx5, align 2, !tbaa !8
  %arrayidx6 = getelementptr inbounds float, ptr %in, i64 2
  %2 = load float, ptr %arrayidx6, align 4, !tbaa !6
  %mul7 = fmul float %2, 3.276700e+04
  %conv8 = fptosi float %mul7 to i16
  %arrayidx9 = getelementptr inbounds i16, ptr %out, i64 2
  store i16 %conv8, ptr %arrayidx9, align 2, !tbaa !8
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1) #2

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 5bcc07a3062bc172ccee604ad6c78f4be2ff02df) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a8d6867359b5a7e34274495337d251a2bf4516e3)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"short", !4, i64 0}
