; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -ip-gen-cloning-force-on-callback-cloning -S 2>&1 | FileCheck %s

; Check that callback cloning occurs from the clones of @foo to the callback
; functions referenced in the call to @__kmpc_fork_call.
; This is the same test as ip_cloning_callback01-opaque-ptr.ll, but checks for
; IR without requiring asserts.

; CHECK: define dso_local i32 @main()
; CHECK: tail call fastcc i32 @foo.1(i32 100)
; CHECK: tail call fastcc i32 @foo.2(i32 200)
; CHECK: define internal fastcc i32 @foo.1
; CHECK: call{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5.[[R0:[0-9]+]]
; CHECK: define internal fastcc i32 @foo.2
; CHECK: call{{.*}}@__kmpc_fork_call{{.*}}@foo.DIR.OMP.PARALLEL.LOOP.2.split5.[[R1:[0-9]+]]
; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split5.[[R0]]
; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split5.[[R1]]

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@a = internal global [200 x i32] zeroinitializer, align 16
@.kmpc_loc.0.0.4 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.3, i32 0, i32 0) }
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.3, i32 0, i32 0) }
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.3, i32 0, i32 0) }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

define dso_local i32 @main() local_unnamed_addr {
bb:
  %i = tail call fastcc i32 @foo(i32 100)
  %i1 = tail call fastcc i32 @foo(i32 200)
  %i2 = add nsw i32 %i1, %i
  ret i32 %i2
}

define internal fastcc i32 @foo(i32 %arg) unnamed_addr {
bb:
  %i = alloca ptr, align 8
  %i1 = alloca i32, align 4
  store ptr getelementptr inbounds ([200 x i32], ptr @a, i64 0, i64 0), ptr %i, align 8, !tbaa !5
  %i2 = add nsw i32 %arg, -1
  store i32 %i2, ptr %i1, align 4, !tbaa !9
  %t0 = zext i32 %arg to i64
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @.kmpc_loc.0.0.4, i32 3, ptr @foo.DIR.OMP.PARALLEL.LOOP.2.split5, ptr nonnull %i, i64 0, ptr nonnull %i1, i64 %t0)
  %i4 = load i32, ptr getelementptr inbounds ([200 x i32], ptr @a, i64 0, i64 0), align 16, !tbaa !9
  ret i32 %i4
}

define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split5(ptr nocapture readonly %arg, ptr nocapture readnone %arg1, ptr nocapture readonly %arg2, i64 %arg3, ptr nocapture readonly %arg4, i64 %t0) {
bb:
  ret void
}

declare !callback !11 void @__kmpc_fork_call(ptr, i32, ptr, ...) local_unnamed_addr

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
!11 = !{!12}
!12 = !{i64 2, i64 -1, i64 -1, i1 true}
; end INTEL_FEATURE_SW_ADVANCED
