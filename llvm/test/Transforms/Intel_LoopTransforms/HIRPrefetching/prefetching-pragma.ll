; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
;void *  sub(float *A, float *B, float *C,  int N) {
;int i;
;  #pragma  noprefetch C
;  #pragma  prefetch A:2:40
;  #pragma  prefetch B
;  for (i=0; i< N; i++) {
;    A[i] = B[i] + C[i] *  2.0;
;  }
;}
;
;<0>          BEGIN REGION { }
;<8>                   %1 = (%B.addr)[0];
;<9>                   %2 = (%C.addr)[0];
;<10>                  %3 = (%A.addr)[0];
;<37>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<16>               |   %4 = (%1)[i1];
;<17>               |   %conv = fpext.float.double(%4);
;<19>               |   %5 = (%2)[i1];
;<20>               |   %conv3 = fpext.float.double(%5);
;<21>               |   %mul = %conv3  *  2.000000e+00;
;<22>               |   %add = %mul  +  %conv;
;<23>               |   %conv4 = fptrunc.double.float(%add);
;<25>               |   (%3)[i1] = %conv4;
;<37>               + END LOOP
;<37>
;<36>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;Function: sub
;
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:         %1 = (%B.addr)[0];
; CHECK-NEXT:         %2 = (%C.addr)[0];
; CHECK-NEXT:         %3 = (%A.addr)[0];
; CHECK-NEXT:      + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:      |   %conv = fpext.float.double((%1)[i1]);
; CHECK-NEXT:      |   %conv3 = fpext.float.double((%2)[i1]);
; CHECK-NEXT:      |   %mul = %conv3  *  2.000000e+00;
; CHECK-NEXT:      |   %add = %mul  +  %conv;
; CHECK-NEXT:      |   %conv4 = fptrunc.double.float(%add);
; CHECK-NEXT:      |   (%3)[i1] = %conv4;
; CHECK-NEXT:      |   @llvm.prefetch.p0(&((i8*)(%1)[i1 + 36]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0(&((i8*)(%3)[i1 + 40]),  0,  1,  1);
; CHECK-NEXT:      + END LOOP
;
; CHECK:           ret &((undef)[0]);
; CHECK:       END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @sub(ptr %A, ptr %B, ptr %C, i32 %N) local_unnamed_addr #0 {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %C.addr = alloca ptr, align 8
  store ptr %A, ptr %A.addr, align 8, !tbaa !2
  store ptr %B, ptr %B.addr, align 8, !tbaa !2
  store ptr %C, ptr %C.addr, align 8, !tbaa !2
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(ptr %C.addr), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr %A.addr), "QUAL.PRAGMA.HINT"(i32 2), "QUAL.PRAGMA.DISTANCE"(i32 40), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr %B.addr), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1)]
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %1 = load ptr, ptr %B.addr, align 8, !tbaa !2
  %2 = load ptr, ptr %C.addr, align 8, !tbaa !2
  %3 = load ptr, ptr %A.addr, align 8, !tbaa !2
  %wide.trip.count14 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds float, ptr %1, i64 %indvars.iv
  %4 = load float, ptr %ptridx, align 4, !tbaa !6
  %conv = fpext float %4 to double
  %ptridx2 = getelementptr inbounds float, ptr %2, i64 %indvars.iv
  %5 = load float, ptr %ptridx2, align 4, !tbaa !6
  %conv3 = fpext float %5 to double
  %mul = fmul fast double %conv3, 2.000000e+00
  %add = fadd fast double %mul, %conv
  %conv4 = fptrunc double %add to float
  %ptridx6 = getelementptr inbounds float, ptr %3, i64 %indvars.iv
  store float %conv4, ptr %ptridx6, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count14
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"()]
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
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
