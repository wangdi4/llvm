; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
; float A[1000];
; float B[1000];
; float C[1000];
;
;void *  sub() {
;int i;
;int N = 1000;
;  #pragma  prefetch *:3:20
;  #pragma  prefetch A:2:10
;  for (i=0; i< N; i++) {
;    A[i] = B[2 * i] + C[i] *  2.0;
;  }
;}
;
;*** IR Dump Before HIR Prefetching ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<27>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<9>                |   %conv = fpext.float.double((@B)[0][2 * i1]);
;<12>               |   %conv3 = fpext.float.double((@C)[0][i1]);
;<13>               |   %mul4 = %conv3  *  2.000000e+00;
;<14>               |   %add = %mul4  +  %conv;
;<15>               |   %conv5 = fptrunc.double.float(%add);
;<17>               |   (@A)[0][i1] = %conv5;
;<27>               + END LOOP
;<27>
;<26>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;Function: sub
;
; CHECK:           BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK-NEXT:           |   %conv = fpext.float.double((@B)[0][2 * i1]);
; CHECK-NEXT:           |   %conv3 = fpext.float.double((@C)[0][i1]);
; CHECK-NEXT:           |   %mul4 = %conv3  *  2.000000e+00;
; CHECK-NEXT:           |   %add = %mul4  +  %conv;
; CHECK-NEXT:           |   %conv5 = fptrunc.double.float(%add);
; CHECK-NEXT:           |   (@A)[0][i1] = %conv5;
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 10]),  0,  1,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@B)[0][2 * i1 + 40]),  0,  0,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@C)[0][i1 + 20]),  0,  0,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret &((undef)[0]);
; CHECK:          END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = dso_local global [1000 x float] zeroinitializer, align 16
@A = dso_local global [1000 x float] zeroinitializer, align 16
@B = dso_local global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @sub() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr null), "QUAL.PRAGMA.HINT"(i32 3), "QUAL.PRAGMA.DISTANCE"(i32 20), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 2), "QUAL.PRAGMA.DISTANCE"(i32 10)]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %1, !intel-tbaa !2
  %2 = load float, ptr %arrayidx, align 8, !tbaa !2
  %conv = fpext float %2 to double
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @C, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load float, ptr %arrayidx2, align 4, !tbaa !2
  %conv3 = fpext float %3 to double
  %mul4 = fmul fast double %conv3, 2.000000e+00
  %add = fadd fast double %mul4, %conv
  %conv5 = fptrunc double %add to float
  %arrayidx7 = getelementptr inbounds [1000 x float], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %conv5, ptr %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret ptr undef
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+64bit,+adx,+aes,+avx,+avx2,+bmi,+bmi2,+cmov,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt,-amx-bf16,-amx-int8,-amx-tile,-avx512bf16,-avx512bitalg,-avx512bw,-avx512cd,-avx512dq,-avx512er,-avx512f,-avx512ifma,-avx512pf,-avx512vbmi,-avx512vbmi2,-avx512vl,-avx512vnni,-avx512vp2intersect,-avx512vpopcntdq,-avxvnni,-cldemote,-clflushopt,-clwb,-clzero,-enqcmd,-fma4,-gfni,-hreset,-kl,-lwp,-movdir64b,-movdiri,-mwaitx,-pconfig,-pku,-prefetchwt1,-ptwrite,-rdpid,-serialize,-sgx,-sha,-shstk,-sse4a,-tbm,-tsxldtrk,-uintr,-vaes,-vpclmulqdq,-waitpkg,-wbnoinvd,-widekl,-xop,-xsavec,-xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
