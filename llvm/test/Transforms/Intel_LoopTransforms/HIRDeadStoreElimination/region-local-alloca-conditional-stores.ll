; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s
;
; Verify that test case compiles successfully and almost all stores to %lc are
; eliminated as it is recognized as region local. 
;
;
;<0>          BEGIN REGION { modified }
;<51>               if ((%lc)[0][16] == 81)
;<51>               {
;<59>                  (%lc)[0][0] = 25;
;<60>                  %3 = (%lc)[0][0];
;<61>                  %4 = (%lc)[0][1];
;<62>                  (%lc)[0][1] = %3 + %4;
;<63>                  (%lc)[0][1] = 25;
;<64>                  %3 = (%lc)[0][0];
;<65>                  %4 = (%lc)[0][2];
;<66>                  (%lc)[0][2] = %3 + %4;
;<67>                  (%lc)[0][2] = 25;
;<68>                  %3 = (%lc)[0][0];
;<69>                  %4 = (%lc)[0][3];
;<70>                  (%lc)[0][3] = %3 + %4;
;<53>                  (%lc)[0][3] = 25;
;<51>               }
;<71>               (%lc)[0][9] = 77;
;<72>               (%lc)[0][8] = 77;
;<73>               (%lc)[0][7] = 77;
;<74>               (%lc)[0][6] = 77;
;<75>               (%lc)[0][5] = 77;
;<76>               (%lc)[0][4] = 77;
;<77>               (%lc)[0][3] = 77;
;<58>               (%lc)[0][2] = 77;
;<78>               if ((%lc)[0][15] == 81)
;<78>               {
;<86>                  (%lc)[0][0] = 25;
;<87>                  %3 = (%lc)[0][0];
;<88>                  %4 = (%lc)[0][1];
;<89>                  (%lc)[0][1] = %3 + %4;
;<90>                  (%lc)[0][1] = 25;
;<91>                  %3 = (%lc)[0][0];
;<92>                  %4 = (%lc)[0][2];
;<93>                  (%lc)[0][2] = %3 + %4;
;<94>                  (%lc)[0][2] = 25;
;<95>                  %3 = (%lc)[0][0];
;<96>                  %4 = (%lc)[0][3];
;<97>                  (%lc)[0][3] = %3 + %4;
;<80>                  (%lc)[0][3] = 25;
;<78>               }
;<98>               (%lc)[0][9] = 77;
;<99>               (%lc)[0][8] = 77;
;<100>              (%lc)[0][7] = 77;
;<101>              (%lc)[0][6] = 77;
;<102>              (%lc)[0][5] = 77;
;<103>              (%lc)[0][4] = 77;
;<104>              (%lc)[0][3] = 77;
;<85>               (%lc)[0][2] = 77;
;
;                   ...
;
;<402>              if ((%lc)[0][3] == 81)
;<402>              {
;<410>                 (%lc)[0][0] = 25;
;<411>                 %3 = (%lc)[0][0];
;<412>                 %4 = (%lc)[0][1];
;<413>                 (%lc)[0][1] = %3 + %4;
;<414>                 (%lc)[0][1] = 25;
;<415>                 %3 = (%lc)[0][0];
;<416>                 %4 = (%lc)[0][2];
;<417>                 (%lc)[0][2] = %3 + %4;
;<418>                 (%lc)[0][2] = 25;
;<419>                 %3 = (%lc)[0][0];
;<420>                 %4 = (%lc)[0][3];
;<421>                 (%lc)[0][3] = %3 + %4;
;<404>                 (%lc)[0][3] = 25;
;<405>                 %3 = (%lc)[0][0];
;<406>                 %4 = (%lc)[0][4];
;<407>                 (%lc)[0][4] = %3 + %4;
;<402>              }
;<422>              (%lc)[0][9] = 77;
;<423>              (%lc)[0][8] = 77;
;<424>              (%lc)[0][7] = 77;
;<425>              (%lc)[0][6] = 77;
;<426>              (%lc)[0][5] = 77;
;<427>              (%lc)[0][4] = 77;
;<428>              (%lc)[0][3] = 77;
;<409>              (%lc)[0][2] = 77;
;<5>                if ((%lc)[0][2] == 81)
;<5>                {
;<429>                 (%lc)[0][0] = 25;
;<430>                 %3 = (%lc)[0][0];
;<431>                 %4 = (%lc)[0][1];
;<432>                 (%lc)[0][1] = %3 + %4;
;<433>                 (%lc)[0][1] = 25;
;<434>                 %3 = (%lc)[0][0];
;<435>                 %4 = (%lc)[0][2];
;<436>                 (%lc)[0][2] = %3 + %4;
;<437>                 (%lc)[0][2] = 25;
;<438>                 %3 = (%lc)[0][0];
;<439>                 %4 = (%lc)[0][3];
;<440>                 (%lc)[0][3] = %3 + %4;
;<14>                  (%lc)[0][3] = 25;
;<15>                  %3 = (%lc)[0][0];
;<17>                  %4 = (%lc)[0][4];
;<19>                  (%lc)[0][4] = %3 + %4;
;<5>                }
;<441>              (%lc)[0][9] = 77;
;<442>              (%lc)[0][8] = 77;
;<443>              (%lc)[0][7] = 77;
;<444>              (%lc)[0][6] = 77;
;<445>              (%lc)[0][5] = 77;
;<446>              (%lc)[0][4] = 77;
;<447>              (%lc)[0][3] = 77;
;<33>               (%lc)[0][2] = 77;
;<0>          END REGION
;
;*** IR Dump After HIR Dead Store Elimination (hir-dead-store-elimination) ***
;Function: main
;
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT: if ((%lc)[0][16] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %4 = (%lc)[0][2];
; CHECK:         %4 = (%lc)[0][3];
; CHECK:         %3 = 25;
; CHECK:         %4 = (%lc)[0][4];
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][15] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][14] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][13] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][12] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][11] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NEXT: if ((%lc)[0][10] == 81)
; CHECK:      {
; CHECK:         %4 = (%lc)[0][1];
; CHECK:         (%lc)[0][1] = 25;
; CHECK:         %3 = 25;
; CHECK:         %4 = 77;
; CHECK:      }
; CHECK-NOT: %lc
; CHECK:   END REGION
;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %lc = alloca [100 x i32], align 16
  %0 = bitcast ptr %lc to ptr
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %0) #3
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(400) %0, i8 0, i64 400, i1 false)
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %lc, i64 0, i64 0
  store i32 100, ptr %arrayidx7, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc17
  %indvars.iv36 = phi i64 [ 16, %entry ], [ %indvars.iv.next37, %for.inc17 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr %lc, i64 0, i64 %indvars.iv36, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %cmp1 = icmp eq i32 %1, 81
  br i1 %cmp1, label %for.body4.preheader, label %if.end

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 1, %for.body4.preheader ]
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx6 = getelementptr inbounds [100 x i32], ptr %lc, i64 0, i64 %2, !intel-tbaa !3
  store i32 25, ptr %arrayidx6, align 4, !tbaa !3
  %3 = load i32, ptr %arrayidx7, align 16, !tbaa !3
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %lc, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %4 = load i32, ptr %arrayidx9, align 4, !tbaa !3
  %add = add i32 %4, %3
  store i32 %add, ptr %arrayidx9, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond.not, label %if.end.loopexit, label %for.body4, !llvm.loop !8

if.end.loopexit:                                  ; preds = %for.body4
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %for.body
  br label %for.body12

for.body12:                                       ; preds = %if.end, %for.body12
  %indvars.iv34 = phi i64 [ 9, %if.end ], [ %indvars.iv.next35, %for.body12 ]
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr %lc, i64 0, i64 %indvars.iv34, !intel-tbaa !3
  store i32 77, ptr %arrayidx14, align 4, !tbaa !3
  %indvars.iv.next35 = add nsw i64 %indvars.iv34, -1
  %cmp11 = icmp ugt i64 %indvars.iv.next35, 1
  br i1 %cmp11, label %for.body12, label %for.inc17, !llvm.loop !10

for.inc17:                                        ; preds = %for.body12
  %indvars.iv.next37 = add nsw i64 %indvars.iv36, -1
  %cmp = icmp ugt i64 %indvars.iv.next37, 1
  br i1 %cmp, label %for.body, label %for.end19, !llvm.loop !11

for.end19:                                        ; preds = %for.inc17
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %0) #3
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly mustprogress
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nofree nosync nounwind readnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { argmemonly nofree nounwind willreturn writeonly mustprogress }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_j", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = distinct !{!11, !9}

