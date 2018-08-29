; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="basic-aa,type-based-aa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we have GEP savings of 42 composed of the following-
; Savings of 4 (1 * 4) due to address simplification in parameter load (%arg)[i1][%tmp211].
; Savings of 4 (1 * 4) due to address simplification in parameter load (%arg)[i1 + 1][%tmp211].
; Savings of 4 (1 * 4) due to address simplification in parameter load (%arg)[i1 + -2][%tmp211].
; Savings of 4 (1 * 4) due to address simplification in parameter load (%arg)[i1 + -1][%tmp211].
; Savings of 8 (2 * 4) due to address simplification in alloca load (%tmp4)[0][i1][%tmp211].
; Savings of 6 (2 * 3) due to reuse from (%arg)[i1 + 1][%tmp211] to (%arg)[i1][%tmp211].
; Savings of 6 (2 * 3) due to reuse from (%arg)[i1][%tmp211] to (%arg)[i1 + -1][%tmp211].
; Savings of 6 (2 * 3) due to reuse from (%arg)[i1 + -1][%tmp211] to (%arg)[i1 + -2][%tmp211].

; + DO i1 = 0, 3, 1   <DO_LOOP>
; |   %tmp215 = (%arg)[i1][%tmp211];
; |   %tmp228 = 0;
; |   if (%tmp215 != 0)
; |   {
; |      %tmp220 = (@Logtable)[0][%tmp215];
; |      %tmp228 = (@Alogtable)[0][zext.i8.i32(%tmp220) + -255 * ((25 + zext.i8.i32(%tmp220)) /u 255) + 25];
; |   }
; |   %tmp232 = (%arg)[i1 + 1][%tmp211];
; |   %tmp245 = 0;
; |   if (%tmp232 != 0)
; |   {
; |      %tmp237 = (@Logtable)[0][%tmp232];
; |      %tmp245 = (@Alogtable)[0][zext.i8.i32(%tmp237) + -255 * ((1 + zext.i8.i32(%tmp237)) /u 255) + 1];
; |   }
; |   %tmp246 = %tmp245  ^  %tmp228;
; |   %tmp251 = %tmp246  ^  (%arg)[i1 + -2][%tmp211];
; |   %tmp256 = %tmp251  ^  (%arg)[i1 + -1][%tmp211];
; |   (%tmp4)[0][i1][%tmp211] = %tmp256;
; + END LOOP

; CHECK: GEPSavings: 42

@Logtable = external dso_local hidden unnamed_addr constant [256 x i8], align 16
@Alogtable = external dso_local hidden unnamed_addr constant [256 x i8], align 16

define void @foo([8 x i8]* nocapture %arg, i64 %tmp211) {
entry:
  %tmp4 = alloca [4 x [8 x i8]], align 16
  br label %bb212

bb212:                                            ; preds = %bb244, %entry
  %tmp213 = phi i64 [ 0, %entry ], [ %tmp229, %bb244 ]
  %tmp214 = getelementptr inbounds [8 x i8], [8 x i8]* %arg, i64 %tmp213, i64 %tmp211
  %tmp215 = load i8, i8* %tmp214, align 1, !tbaa !2
  %tmp216 = icmp eq i8 %tmp215, 0
  br i1 %tmp216, label %bb227, label %bb217

bb217:                                            ; preds = %bb212
  %tmp218 = zext i8 %tmp215 to i64
  %tmp219 = getelementptr inbounds [256 x i8], [256 x i8]* @Logtable, i64 0, i64 %tmp218
  %tmp220 = load i8, i8* %tmp219, align 1, !tbaa !6
  %tmp221 = zext i8 %tmp220 to i32
  %tmp222 = add nuw nsw i32 %tmp221, 25
  %tmp223 = urem i32 %tmp222, 255
  %tmp224 = zext i32 %tmp223 to i64
  %tmp225 = getelementptr inbounds [256 x i8], [256 x i8]* @Alogtable, i64 0, i64 %tmp224
  %tmp226 = load i8, i8* %tmp225, align 1, !tbaa !6
  br label %bb227

bb227:                                            ; preds = %bb217, %bb212
  %tmp228 = phi i8 [ %tmp226, %bb217 ], [ 0, %bb212 ]
  %tmp229 = add nuw nsw i64 %tmp213, 1
  %tmp230 = and i64 %tmp229, 3
  %tmp231 = getelementptr inbounds [8 x i8], [8 x i8]* %arg, i64 %tmp230, i64 %tmp211
  %tmp232 = load i8, i8* %tmp231, align 1, !tbaa !2
  %tmp233 = icmp eq i8 %tmp232, 0
  br i1 %tmp233, label %bb244, label %bb234

bb234:                                            ; preds = %bb227
  %tmp235 = zext i8 %tmp232 to i64
  %tmp236 = getelementptr inbounds [256 x i8], [256 x i8]* @Logtable, i64 0, i64 %tmp235
  %tmp237 = load i8, i8* %tmp236, align 1, !tbaa !6
  %tmp238 = zext i8 %tmp237 to i32
  %tmp239 = add nuw nsw i32 %tmp238, 1
  %tmp240 = urem i32 %tmp239, 255
  %tmp241 = zext i32 %tmp240 to i64
  %tmp242 = getelementptr inbounds [256 x i8], [256 x i8]* @Alogtable, i64 0, i64 %tmp241
  %tmp243 = load i8, i8* %tmp242, align 1, !tbaa !6
  br label %bb244

bb244:                                            ; preds = %bb234, %bb227
  %tmp245 = phi i8 [ %tmp243, %bb234 ], [ 0, %bb227 ]
  %tmp246 = xor i8 %tmp245, %tmp228
  %tmp247 = add nuw nsw i64 %tmp213, 2
  %tmp248 = and i64 %tmp247, 3
  %tmp249 = getelementptr inbounds [8 x i8], [8 x i8]* %arg, i64 %tmp248, i64 %tmp211
  %tmp250 = load i8, i8* %tmp249, align 1, !tbaa !2
  %tmp251 = xor i8 %tmp246, %tmp250
  %tmp252 = add nuw nsw i64 %tmp213, 3
  %tmp253 = and i64 %tmp252, 3
  %tmp254 = getelementptr inbounds [8 x i8], [8 x i8]* %arg, i64 %tmp253, i64 %tmp211
  %tmp255 = load i8, i8* %tmp254, align 1, !tbaa !2
  %tmp256 = xor i8 %tmp251, %tmp255
  %tmp257 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %tmp4, i64 0, i64 %tmp213, i64 %tmp211
  store i8 %tmp256, i8* %tmp257, align 1, !tbaa !11
  %tmp258 = icmp eq i64 %tmp229, 4
  br i1 %tmp258, label %exit, label %bb212

exit:
  ret void
}

!0 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 2ba068ccb1aca0c7f72a8ed4f88f8c343c5b5d8c)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA8_h", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!4, !4, i64 0}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA3_A4_A2_h", !9, i64 0}
!9 = !{!"array@_ZTSA4_A2_h", !10, i64 0}
!10 = !{!"array@_ZTSA2_h", !4, i64 0}
!11 = !{!12, !4, i64 0}
!12 = !{!"array@_ZTSA4_A8_h", !3, i64 0}

