; Check if valid VF is used to vectorize vconflict idiom when target
; does not support avx512vl instructions.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -mattr=+avx512cd,+evex512 -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=8 < %s 2>&1 | FileCheck %s --check-prefix=VEC
; RUN: opt -mattr=+avx512cd,+evex512 -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=NO-VEC

; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <15>               + END LOOP


; VEC-LABEL:  BEGIN REGION { modified }
; VEC-NEXT:         + DO i1 = 0, 1023, 8   <DO_LOOP> <auto-vectorized> <novectorize>
; VEC-NEXT:         |   %.vec = (<8 x i64>*)(%B)[i1];
; VEC-NEXT:         |   %.vec1 = (<8 x float>*)(%A)[%.vec];
; VEC-NEXT:         |   %conflicts = @llvm.x86.avx512.conflict.q.512(%.vec);
; VEC-NEXT:         |   %llvm.ctpop.v8i64 = @llvm.ctpop.v8i64(%conflicts);
; VEC-NEXT:         |   %.vec2 = sitofp.<8 x i64>.<8 x float>(%llvm.ctpop.v8i64);
; VEC-NEXT:         |   %.vec3 = %.vec2  +  1.000000e+00;
; VEC-NEXT:         |   %.vec4 = %.vec3  *  2.000000e+00;
; VEC-NEXT:         |   %.vec5 = %.vec1  +  %.vec4;
; VEC-NEXT:         |   (<8 x float>*)(%A)[%.vec] = %.vec5;
; VEC-NEXT:         + END LOOP
; VEC-NEXT:   END REGION

; NO-VEC: No vectorization factor was found that can satisfy all VConflict idioms in the loop.
; NO-VEC:    + DO i1 = 0, 1023, 1   <DO_LOOP>

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo1PfPi(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i64, ptr %B, i64 %indvars.iv
  %0 = load i64, ptr %ptridx, align 4
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %0
  %1 = load float, ptr %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, ptr %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
