; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -aa-pipeline="basic-aa" -print-after=hir-loop-distribute-memrec -disable-output -hir-loop-distribute-scex-cost=3 < %s 2>&1 | FileCheck %s

; %fetch.90 is being used inside the if predicate and also defined in the if.
; Currently this test checks that we do not do distribution for this loop.

; CHECK: BEGIN REGION { }
; CHECK-NOT: modified
;              + DO i1 = 0, undef + -2, 1   <DO_LOOP>
;              |   %fetch.90.pre792795 = undef;
;              |   if (trunc.i32.i1(%fetch.90) != 0)
;              |   {
;              |      (undef)[0] = 0;
;              |      @for_write_seq_fmt_xmit();
;              |      (undef)[0] = 1;
;              |      %fetch.90.pre = (@parac_mp_paral_)[0].3;
;              |      %fetch.90 = %fetch.90.pre;
;              |      %fetch.90.pre792795 = %fetch.90.pre;
;              |   }
;              + END LOOP
;        END REGION


; TODO: we can handle this case if we add load/stores for %fetch.90 before the if predicates. Also investigate why we have such piblock in distribution.

;              + DO i1 = 0, (%indvars.iv781 + -2)/u64, 1
;              |   %min = (-64 * i1 + %indvars.iv781 + -2 <= 63) ? -64 * i1 + %indvars.iv781 + -2 : 63;
;              |
;              |   + DO i2 = 0, %min, 1
;              |   |   if (trunc.i32.i1(%fetch.90) != 0)
;              |   |   {
;...
;              |   |      %fetch.90.pre = (@parac_mp_paral_)[0].3;
;              |   |      %fetch.90 = %fetch.90.pre;
;              |   |      (%.TempArray)[0][i2] = %fetch.90.pre;
;              |   |   }
;              |   + END LOOP
;              |
;              |
;              |   + DO i2 = 0, %min, 1
;              |   |   if (trunc.i32.i1(%fetch.90) != 0)
;              |   |   {
;              |   |      %fetch.90.pre = (@parac_mp_paral_)[0].3;
;              |   |      %fetch.90 = (%.TempArray)[0][i2];
;              |   |      %fetch.90.pre792796 = %fetch.90.pre;
;              |   |   }
;              |   + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"PARAC" = type <{ i32, i32, i32, i32, i32 }>

@parac_mp_paral_ = external local_unnamed_addr global %"PARAC", align 8

define void @cl_init_utils_mp_cl_init_() local_unnamed_addr #0 {
alloca_1:
  br i1 undef, label %bb96_endif, label %bb_new167_then

bb82:                                             ; preds = %bb_new167_then, %bb87
  br label %bb86

bb86:                                             ; preds = %bb92_endif, %bb82
  %fetch.90 = phi i32 [ undef, %bb82 ], [ %fetch.90793, %bb92_endif ]
  %indvars.iv775 = phi i64 [ 1, %bb82 ], [ %indvars.iv.next776, %bb92_endif ]
  %and.30 = and i32 %fetch.90, 1
  %rel.61.not = icmp eq i32 %and.30, 0
  br i1 %rel.61.not, label %bb92_endif, label %bb_new172_then

bb_new172_then:                                   ; preds = %bb86
  store i8 0, ptr undef, align 1
  call void @for_write_seq_fmt_xmit() #2
  store i8 1, ptr undef, align 1
  %fetch.90.pre = load i32, ptr getelementptr inbounds (%"PARAC", ptr @parac_mp_paral_, i64 0, i32 3), align 4
  br label %bb92_endif

bb92_endif:                                       ; preds = %bb_new172_then, %bb86
  %fetch.90.pre792795 = phi i32 [ undef, %bb86 ], [ %fetch.90.pre, %bb_new172_then ]
  %fetch.90793 = phi i32 [ %fetch.90, %bb86 ], [ %fetch.90.pre, %bb_new172_then ]
  %indvars.iv.next776 = add nuw nsw i64 %indvars.iv775, 1
  %exitcond780 = icmp eq i64 %indvars.iv.next776, undef
  br i1 %exitcond780, label %bb87, label %bb86

bb87:                                             ; preds = %bb92_endif
  %fetch.90.pre792795.lcssa = phi i32 [ %fetch.90.pre792795, %bb92_endif ]
  br label %bb82

bb_new167_then:                                   ; preds = %alloca_1
  br label %bb82

bb96_endif:                                       ; preds = %alloca_1
  ret void
}

declare void @for_write_seq_fmt_xmit() local_unnamed_addr #1

attributes #0 = { "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nounwind }

!nvvm.annotations = !{}
