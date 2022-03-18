;  Test case  for Opt Report for distribute point pragma
;
;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-distribute-memrec  -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-loop-distribute-memrec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;
;  for (i =0 ; i<n ; i++) {
;    a1[i] += b2  - 11.0;
;#pragma distribute_point
;    a2[i] += b2  + 12.0;
;    a3[i] += b2   + i; }
;
;OPTREPORT: LOOP BEGIN
;OPTREPORT: <Distributed chunk1>
;OPTREPORT:    remark #25483: Distribute point pragma processed
;OPTREPORT:    remark #25426: Loop distributed (2 way)
;OPTREPORT: LOOP END
;OPTREPORT: LOOP BEGIN
;OPTREPORT: <Distributed chunk2>
;OPTREPORT: LOOP END

;Module Before HIR; ModuleID = 'distpoint4.c'
source_filename = "distpoint4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a3 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %0 = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @a3, i64 0, i64 5), align 4, !tbaa !2
  %cmp24 = icmp sgt i32 %n, 0
  br i1 %cmp24, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %conv = fpext float %0 to double
  %sub = fadd double %conv, -1.100000e+01
  %add4 = fadd double %conv, 1.200000e+01
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @a1, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %conv1 = fpext float %1 to double
  %add = fadd double %sub, %conv1
  %conv2 = fptrunc double %add to float
  store float %conv2, float* %arrayidx, align 4, !tbaa !2
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %arrayidx6 = getelementptr inbounds [1000 x float], [1000 x float]* @a2, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx6, align 4, !tbaa !2
  %conv7 = fpext float %3 to double
  %add8 = fadd double %add4, %conv7
  %conv9 = fptrunc double %add8 to float
  store float %conv9, float* %arrayidx6, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %4 = trunc i64 %indvars.iv to i32
  %conv10 = sitofp i32 %4 to float
  %add11 = fadd float %0, %conv10
  %arrayidx13 = getelementptr inbounds [1000 x float], [1000 x float]* @a3, i64 0, i64 %indvars.iv
  %5 = load float, float* %arrayidx13, align 4, !tbaa !2
  %add14 = fadd float %add11, %5
  store float %add14, float* %arrayidx13, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 65e8f9d46b54671e271ba934ab45010c98c98cce) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d5a4f1cbcbff7885d7b00fe044003cf37ba39d4d)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
