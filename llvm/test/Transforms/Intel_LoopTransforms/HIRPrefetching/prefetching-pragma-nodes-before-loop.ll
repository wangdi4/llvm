; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Check the pragma in the nodes before the loop and prefetch the same variable within different streams
;
; Source code
;void *  sub(float *A, float *B, float *C) {
;int i;
;int N = 1000;
;  #pragma  noprefetch C
;  #pragma  prefetch A:1:40
;  #pragma  prefetch B
;  for (i=0; i< N; i++) {
;    A[i] = B[i] + B[i + 10000] + C[i] *  2.0;
;  }
;}
;
;*** IR Dump Before HIR Prefetching ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<3>                %1 = (%B.addr)[0];
;<4>                %2 = (%C.addr)[0];
;<5>                %3 = (%A.addr)[0];
;<33>
;<33>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<14>               |   %add3 = (%1)[i1 + 10000]  +  (%1)[i1];
;<15>               |   %conv = fpext.float.double(%add3);
;<18>               |   %conv6 = fpext.float.double((%2)[i1]);
;<19>               |   %mul = %conv6  *  2.000000e+00;
;<20>               |   %add7 = %mul  +  %conv;
;<21>               |   %conv8 = fptrunc.double.float(%add7);
;<23>               |   (%3)[i1] = %conv8;
;<33>               + END LOOP
;<33>
;<32>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;Function: sub
;
; CHECK:     BEGIN REGION { modified }
; CHECK:            %1 = (%B.addr)[0];
; CHECK:            %2 = (%C.addr)[0];
; CHECK:            %3 = (%A.addr)[0];
;
; CHECK:       + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK-NEXT:       |   %add3 = (%1)[i1 + 10000]  +  (%1)[i1];
; CHECK-NEXT:       |   %conv = fpext.float.double(%add3);
; CHECK-NEXT:       |   %conv6 = fpext.float.double((%2)[i1]);
; CHECK-NEXT:       |   %mul = %conv6  *  2.000000e+00;
; CHECK-NEXT:       |   %add7 = %mul  +  %conv;
; CHECK-NEXT:       |   %conv8 = fptrunc.double.float(%add7);
; CHECK-NEXT:       |   (%3)[i1] = %conv8;
; CHECK-NEXT:       |   @llvm.prefetch.p0i8(&((i8*)(%1)[i1 + 27]),  0,  3,  1);
; CHECK-NEXT:       |   @llvm.prefetch.p0i8(&((i8*)(%1)[i1 + 10027]),  0,  3,  1);
; CHECK-NEXT:       |   @llvm.prefetch.p0i8(&((i8*)(%3)[i1 + 40]),  0,  2,  1);
; CHECK-NEXT:       + END LOOP
;
; CHECK:            ret &((undef)[0]);
; CHECK:      END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local noalias i8* @sub(float* %A, float* %B, float* %C) local_unnamed_addr #0 {
entry:
  %A.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %C.addr = alloca float*, align 8
  store float* %A, float** %A.addr, align 8, !tbaa !2
  store float* %B, float** %B.addr, align 8, !tbaa !2
  store float* %C, float** %C.addr, align 8, !tbaa !2
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(float** %C.addr), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(float** %A.addr), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 40), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(float** %B.addr),"QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1)]
  %1 = load float*, float** %B.addr, align 8, !tbaa !2
  %2 = load float*, float** %C.addr, align 8, !tbaa !2
  %3 = load float*, float** %A.addr, align 8, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float, float* %1, i64 %indvars.iv
  %4 = load float, float* %ptridx, align 4, !tbaa !6
  %5 = add nuw nsw i64 %indvars.iv, 10000
  %ptridx2 = getelementptr inbounds float, float* %1, i64 %5
  %6 = load float, float* %ptridx2, align 4, !tbaa !6
  %add3 = fadd fast float %6, %4
  %conv = fpext float %add3 to double
  %ptridx5 = getelementptr inbounds float, float* %2, i64 %indvars.iv
  %7 = load float, float* %ptridx5, align 4, !tbaa !6
  %conv6 = fpext float %7 to double
  %mul = fmul fast double %conv6, 2.000000e+00
  %add7 = fadd fast double %mul, %conv
  %conv8 = fptrunc double %add7 to float
  %ptridx10 = getelementptr inbounds float, float* %3, i64 %indvars.iv
  store float %conv8, float* %ptridx10, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret i8* undef
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
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
