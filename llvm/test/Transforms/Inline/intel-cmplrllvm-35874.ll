; RUN: opt -aa-pipeline="scoped-noalias-aa" -passes='cgscc(inline),early-cse<memssa>' -S < %s 2>&1 | FileCheck %s

; CMPLRLLVM-35874: Check that when multiple levels of inlining are applied
; through arguments that are marked ptrnoalias and alias, that bitcasts are
; taken into account when computing pointer-no-alias-loads.

; CHECK-LABEL: define void @mod1_mp_proc1_
; CHECK: store float 3.000000e+00,
; CHECK-NOT: store float 3.000000e+00,{{.*}}alias
; CHECK-LABEL: define void @mod2_mp_proc2_
; CHECK: store float 3.000000e+00, {{.*}} !alias.scope ![[AS0:[0-9]+]], !noalias ![[NAS0:[0-9]+]]
; CHECK-LABEL: define void @MAIN__
; CHECK: store float 3.000000e+00, {{.*}} !alias.scope ![[AS1:[0-9]+]], !noalias ![[NAS1:[0-9]+]]

; CHECK-DAG: ![[D0:[0-9]+]] = distinct !{![[D0]], !"mod1_mp_proc1_"}
; CHECK-DAG: ![[D1:[0-9]+]] = distinct !{![[D1]], !"mod2_mp_proc2_"}
; CHECK-DAG: ![[D2:[0-9]+]] = distinct !{![[D2]], !"mod1_mp_proc1_"}

; CHECK-DAG: ![[S0:[0-9]+]] = distinct !{![[S0]], ![[D0]], !"mod1_mp_proc1_: %A"}
; CHECK-DAG: ![[S1:[0-9]+]] = distinct !{![[S1]], ![[D0]], !"mod1_mp_proc1_: ptrnoalias %A"}
; CHECK-DAG: ![[S2:[0-9]+]] = distinct !{![[S2]], ![[D1]], !"mod2_mp_proc2_: %mod1_$a_"}
; CHECK-DAG: ![[S3:[0-9]+]] = distinct !{![[S3]], ![[D1]], !"mod2_mp_proc2_: %mod1_$b_"}
; CHECK-DAG: ![[S4:[0-9]+]] = distinct !{![[S4]], ![[D1]], !"mod2_mp_proc2_: ptrnoalias %mod1_$a_"}
; CHECK-DAG: ![[S5:[0-9]+]] = distinct !{![[S5]], ![[D2]], !"mod1_mp_proc1_: %A"}
; CHECK-DAG: ![[S6:[0-9]+]] = distinct !{![[S6]], ![[D2]], !"mod1_mp_proc1_: ptrnoalias %A"}
; CHECK-DAG: ![[NAS0]] = !{![[S0]]}
; CHECK-DAG: ![[AS0]] = !{![[S1]]}
; CHECK-DAG: ![[AS1]] = !{![[S6]], ![[S4]]}
; CHECK-DAG: ![[NAS1]] = !{![[S5]], ![[S2]], ![[S3]]}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.0" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.3" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@strlit = internal unnamed_addr constant [6 x i8] c"failed"
@strlit.1 = internal unnamed_addr constant [6 x i8] c"passed"
@"test_$A" = internal global [2 x float] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define void @mod1._() local_unnamed_addr #0 {
alloca_0:
  ret void
}

; Function Attrs: nounwind uwtable
define void @mod1_mp_proc1_(%"QNCA_a0$float*$rank1$"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %A) local_unnamed_addr #0 {
alloca_1:
  %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %A, i64 0, i32 0
  %"A.addr_a0$_fetch.1" = load float*, float** %"A.addr_a0$", align 1, !tbaa !0
  %"A.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %A, i64 0, i32 6, i64 0, i32 1
  %"A.dim_info$.spacing$[]_fetch.2" = load i64, i64* %"A.dim_info$.spacing$", align 1, !tbaa !5
  store float 3.000000e+00, float* %"A.addr_a0$_fetch.1", align 1, !tbaa !6
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nounwind uwtable
define void @mod2._() local_unnamed_addr #0 {
alloca_2:
  ret void
}

; Function Attrs: nounwind uwtable
define void @mod2_mp_proc2_(%"QNCA_a0$float*$rank1$.0"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"mod1_$a_", float* noalias dereferenceable(4) %"mod1_$b_") local_unnamed_addr #0 {
alloca_3:
  %"mod1_$a_.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"mod1_$a_", i64 0, i32 0
  %"mod1_$a_.addr_a0$_fetch.4" = load float*, float** %"mod1_$a_.addr_a0$", align 1, !tbaa !8
  %"mod1_$a_.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"mod1_$a_", i64 0, i32 6, i64 0, i32 1
  %"mod1_$a_.dim_info$.spacing$[]_fetch.5" = load i64, i64* %"mod1_$a_.dim_info$.spacing$", align 1, !tbaa !13
  store float 0.000000e+00, float* %"mod1_$a_.addr_a0$_fetch.4", align 1, !tbaa !14
  %"(%QNCA_a0$float*$rank1$*)mod1_$a_$" = bitcast %"QNCA_a0$float*$rank1$.0"* %"mod1_$a_" to %"QNCA_a0$float*$rank1$"*
  call void @mod1_mp_proc1_(%"QNCA_a0$float*$rank1$"* nonnull %"(%QNCA_a0$float*$rank1$*)mod1_$a_$")
  %"mod1_$a_.addr_a0$_fetch.7" = load float*, float** %"mod1_$a_.addr_a0$", align 1, !tbaa !8
  %"mod1_$a_.dim_info$.spacing$[]_fetch.8" = load i64, i64* %"mod1_$a_.dim_info$.spacing$", align 1, !tbaa !13
  %"mod1_$a_.addr_a0$_fetch.7[]_fetch.10" = load float, float* %"mod1_$a_.addr_a0$_fetch.7", align 1, !tbaa !14
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"mod1_$a_.addr_a0$_fetch.7[]_fetch.10", 1.000000e+00
  store float %add.1, float* %"mod1_$b_", align 1, !tbaa !16
  ret void
}

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_4:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"test_$B" = alloca float, align 8
  %"var$6" = alloca %"QNCA_a0$float*$rank1$.3", align 8
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64, i8* }>, align 8
  %"(&)val$9" = alloca [4 x i8], align 1
  %argblock10 = alloca <{ i64, i8* }>, align 8
  %"(&)val$17" = alloca [4 x i8], align 1
  %argblock18 = alloca <{ i64 }>, align 8
  %func_result = call i32 @for_set_fpe_(i32* nonnull @0) #3
  %func_result2 = call i32 @for_set_reentrancy(i32* nonnull @1) #3
  br label %loop_test18

loop_test18:                                      ; preds = %loop_body19, %alloca_4
  %"$loop_ctr.0" = phi i64 [ 1, %alloca_4 ], [ %add.2, %loop_body19 ]
  %rel.1 = icmp ult i64 %"$loop_ctr.0", 3
  br i1 %rel.1, label %loop_body19, label %loop_exit20

loop_body19:                                      ; preds = %loop_test18
  %0 = sub nsw i64 %"$loop_ctr.0", 1
  %1 = getelementptr inbounds float, float* getelementptr inbounds ([2 x float], [2 x float]* @"test_$A", i64 0, i64 0), i64 %0
  store float 0.000000e+00, float* %1, align 1, !tbaa !18
  %add.2 = add nuw nsw i64 %"$loop_ctr.0", 1
  br label %loop_test18

loop_exit20:                                      ; preds = %loop_test18
  %"var$6.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 3
  store i64 0, i64* %"var$6.flags$", align 8, !tbaa !22
  %"var$6.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 1
  store i64 4, i64* %"var$6.addr_length$", align 8, !tbaa !25
  %"var$6.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 4
  store i64 1, i64* %"var$6.dim$", align 8, !tbaa !26
  %"var$6.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 2
  store i64 0, i64* %"var$6.codim$", align 8, !tbaa !27
  %"var$6.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 6, i64 0, i32 1
  store i64 4, i64* %"var$6.dim_info$.spacing$", align 1, !tbaa !28
  %"var$6.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 6, i64 0, i32 2
  store i64 1, i64* %"var$6.dim_info$.lower_bound$", align 1, !tbaa !29
  %"var$6.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 6, i64 0, i32 0
  store i64 2, i64* %"var$6.dim_info$.extent$", align 1, !tbaa !30
  %"var$6.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.3", %"QNCA_a0$float*$rank1$.3"* %"var$6", i64 0, i32 0
  store float* getelementptr inbounds ([2 x float], [2 x float]* @"test_$A", i64 0, i64 0), float** %"var$6.addr_a0$", align 8, !tbaa !31
  %"var$6.flags$_fetch.14" = load i64, i64* %"var$6.flags$", align 8, !tbaa !22
  %or.1 = or i64 %"var$6.flags$_fetch.14", 1
  store i64 %or.1, i64* %"var$6.flags$", align 8, !tbaa !22
  %"(%QNCA_a0$float*$rank1$.0*)var$6$" = bitcast %"QNCA_a0$float*$rank1$.3"* %"var$6" to %"QNCA_a0$float*$rank1$.0"*
  call void @mod2_mp_proc2_(%"QNCA_a0$float*$rank1$.0"* nonnull %"(%QNCA_a0$float*$rank1$.0*)var$6$", float* nonnull %"test_$B")
  %"test_$B_fetch.15" = load float, float* %"test_$B", align 8, !tbaa !32
  %rel.2 = fcmp reassoc ninf nsz arcp contract afn oeq float %"test_$B_fetch.15", 4.000000e+00
  br i1 %rel.2, label %bb_new28_then, label %bb_new31_else

bb_new28_then:                                    ; preds = %loop_exit20
  %.fca.0.gep28 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 0
  store i8 56, i8* %.fca.0.gep28, align 1, !tbaa !34
  %.fca.1.gep29 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 1
  store i8 4, i8* %.fca.1.gep29, align 1, !tbaa !34
  %.fca.2.gep30 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 2
  store i8 1, i8* %.fca.2.gep30, align 1, !tbaa !34
  %.fca.3.gep31 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep31, align 1, !tbaa !34
  %BLKFIELD_i64_ = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i64 0, i32 0
  store i64 6, i64* %BLKFIELD_i64_, align 8, !tbaa !35
  %"BLKFIELD_i8*_" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i64 0, i32 1
  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @strlit.1, i64 0, i64 0), i8** %"BLKFIELD_i8*_", align 8, !tbaa !37
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$$" = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 0
  %"(i8*)argblock$" = bitcast <{ i64, i8* }>* %argblock to i8*
  %func_result8 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* nonnull %"(i8*)(&)val$$", i8* nonnull %"(i8*)argblock$") #3
  br label %bb11_endif

bb_new31_else:                                    ; preds = %loop_exit20
  %.fca.0.gep24 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 0
  store i8 56, i8* %.fca.0.gep24, align 1, !tbaa !34
  %.fca.1.gep25 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 1
  store i8 4, i8* %.fca.1.gep25, align 1, !tbaa !34
  %.fca.2.gep26 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 2
  store i8 2, i8* %.fca.2.gep26, align 1, !tbaa !34
  %.fca.3.gep27 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep27, align 1, !tbaa !34
  %BLKFIELD_i64_11 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock10, i64 0, i32 0
  store i64 6, i64* %BLKFIELD_i64_11, align 8, !tbaa !39
  %"BLKFIELD_i8*_12" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock10, i64 0, i32 1
  store i8* getelementptr inbounds ([6 x i8], [6 x i8]* @strlit, i64 0, i64 0), i8** %"BLKFIELD_i8*_12", align 8, !tbaa !41
  %"(i8*)$io_ctx$14" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)(&)val$9$" = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 0
  %"(i8*)argblock10$" = bitcast <{ i64, i8* }>* %argblock10 to i8*
  %func_result16 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %"(i8*)$io_ctx$14", i32 -1, i64 1239157112576, i8* nonnull %"(i8*)(&)val$9$", i8* nonnull %"(i8*)argblock10$") #3
  %"test_$B_fetch.18" = load float, float* %"test_$B", align 8, !tbaa !32
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$17", i64 0, i64 0
  store i8 26, i8* %.fca.0.gep, align 1, !tbaa !34
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$17", i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1, !tbaa !34
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$17", i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1, !tbaa !34
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$17", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1, !tbaa !34
  %bitcast = bitcast <{ i64 }>* %argblock18 to float*
  store float %"test_$B_fetch.18", float* %bitcast, align 8, !tbaa !43
  %"(i8*)(&)val$17$" = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$17", i64 0, i64 0
  %"(i8*)argblock18$" = bitcast <{ i64 }>* %argblock18 to i8*
  %func_result22 = call i32 @for_write_seq_lis_xmit(i8* nonnull %"(i8*)$io_ctx$14", i8* nonnull %"(i8*)(&)val$17$", i8* nonnull %"(i8*)argblock18$") #3
  br label %bb11_endif

bb11_endif:                                       ; preds = %bb_new31_else, %bb_new28_then
  ret void
}

declare i32 @for_set_fpe_(i32* nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis_xmit(i8* nocapture readonly, i8* nocapture readonly, i8*) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}

!0 = !{!1, !2, i64 0}
!1 = !{!"ifx$descr$1", !2, i64 0, !2, i64 8, !2, i64 16, !2, i64 24, !2, i64 32, !2, i64 40, !2, i64 48, !2, i64 56, !2, i64 64}
!2 = !{!"ifx$descr$field", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$mod1_mp_proc1_"}
!5 = !{!1, !2, i64 56}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$1", !3, i64 0}
!8 = !{!9, !10, i64 0}
!9 = !{!"ifx$descr$2", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64}
!10 = !{!"ifx$descr$field", !11, i64 0}
!11 = !{!"Generic Fortran Symbol", !12, i64 0}
!12 = !{!"ifx$root$2$mod2_mp_proc2_"}
!13 = !{!9, !10, i64 56}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$2", !11, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$3", !11, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$4", !20, i64 0}
!20 = !{!"Generic Fortran Symbol", !21, i64 0}
!21 = !{!"ifx$root$3$MAIN__"}
!22 = !{!23, !24, i64 24}
!23 = !{!"ifx$descr$3", !24, i64 0, !24, i64 8, !24, i64 16, !24, i64 24, !24, i64 32, !24, i64 40, !24, i64 48, !24, i64 56, !24, i64 64}
!24 = !{!"ifx$descr$field", !20, i64 0}
!25 = !{!23, !24, i64 8}
!26 = !{!23, !24, i64 32}
!27 = !{!23, !24, i64 16}
!28 = !{!23, !24, i64 56}
!29 = !{!23, !24, i64 64}
!30 = !{!23, !24, i64 48}
!31 = !{!23, !24, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"ifx$unique_sym$5", !20, i64 0}
!34 = !{!20, !20, i64 0}
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$7", !20, i64 0}
!37 = !{!38, !38, i64 0}
!38 = !{!"ifx$unique_sym$8", !20, i64 0}
!39 = !{!40, !40, i64 0}
!40 = !{!"ifx$unique_sym$10", !20, i64 0}
!41 = !{!42, !42, i64 0}
!42 = !{!"ifx$unique_sym$11", !20, i64 0}
!43 = !{!44, !44, i64 0}
!44 = !{!"ifx$unique_sym$12", !20, i64 0}
