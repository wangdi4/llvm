; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -ip-gen-cloning-force-off-callback-cloning -S -debug-only=ipcloning 2>&1 | FileCheck %s

; Check that callback cloning does not occur from the clones of @foo to the
; callback functions referenced in the call to @__kmpc_fork_call, because
; -ip-gen-cloning-force-off-callback-cloning is specified.

; CHECK: Not attempting callback cloning for foo
; CHECK: Cloned call:{{.*}}foo.1(i32 100)
; CHECK: Cloned call:{{.*}}foo.2(i32 200)
; CHECK-NOT: Cloned callback in foo.2:{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5
; CHECK-NOT: Cloned callback in foo.1:{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5

; CHECK: define dso_local i32 @main()
; CHECK: tail call fastcc i32 @foo.1(i32 100)
; CHECK: tail call fastcc i32 @foo.2(i32 200)
; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split5(
; CHECK: define internal fastcc i32 @foo.1
; CHECK: call{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5 to
; CHECK: define internal fastcc i32 @foo.2
; CHECK: call{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5 to

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@a = internal global [200 x i32] zeroinitializer, align 16
@.kmpc_loc.0.0.4 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.3, i32 0, i32 0) }
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.3, i32 0, i32 0) }
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.3, i32 0, i32 0) }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
  %1 = tail call fastcc i32 @foo(i32 100)
  %2 = tail call fastcc i32 @foo(i32 200)
  %3 = add nsw i32 %2, %1
  ret i32 %3
}

; Function Attrs: nofree noinline norecurse nounwind uwtable
define internal fastcc i32 @foo(i32 %0) unnamed_addr {
  %2 = alloca i32*, align 8
  %3 = alloca i32, align 4
  store i32* getelementptr inbounds ([200 x i32], [200 x i32]* @a, i64 0, i64 0), i32** %2, align 8, !tbaa !5
  %4 = add nsw i32 %0, -1
  %5 = bitcast i32* %3 to i8*
  store i32 %4, i32* %3, align 4, !tbaa !9
  %t0 = zext i32 %0 to i64
  call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t* nonnull @.kmpc_loc.0.0.4, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*, i64)* @foo.DIR.OMP.PARALLEL.LOOP.2.split5 to void (i32*, i32*, ...)*), i32** nonnull %2, i64 0, i32* nonnull %3, i64 %t0)
  %6 = load i32, i32* getelementptr inbounds ([200 x i32], [200 x i32]* @a, i64 0, i64 0), align 16, !tbaa !9
  ret i32 %6
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split5(i32* nocapture readonly %0, i32* nocapture readnone %1, i32** nocapture readonly %2, i64 %3, i32* nocapture readonly %4, i64 %t0) {
  ret void
}

; Function Attrs: nounwind
declare !callback !28 void @__kmpc_fork_call(%struct.ident_t* %0, i32 %1, void (i32*, i32*, ...)* %2, ...) local_unnamed_addr

!llvm.ident = !{!0}
!nvvm.annotations = !{}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{!6, !6, i64 0}
!6 = !{!"pointer@_ZTSPi", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
!11 = distinct !{!11, !12, !"OMPAliasScope"}
!12 = distinct !{!12, !"OMPDomain"}
!13 = !{!14, !15}
!14 = distinct !{!14, !12, !"OMPAliasScope"}
!15 = distinct !{!15, !12, !"OMPAliasScope"}
!16 = !{i32 0, i32 2147483647}
!17 = distinct !{!17, !12, !"OMPAliasScope"}
!18 = distinct !{}
!19 = !{!11, !20, !14, !17}
!20 = distinct !{!20, !12, !"OMPAliasScope"}
!21 = distinct !{!21, !22, !23}
!22 = !{!"llvm.loop.parallel_accesses", !18}
!23 = !{!"llvm.loop.isvectorized", i32 1}
!24 = distinct !{!24, !25}
!25 = !{!"llvm.loop.unroll.disable"}
!26 = distinct !{!26, !22, !27, !23}
!27 = !{!"llvm.loop.unroll.runtime.disable"}
!28 = !{!29}
!29 = !{i64 2, i64 -1, i64 -1, i1 true}
; end INTEL_FEATURE_SW_ADVANCED
