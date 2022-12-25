; RUN: opt -S -passes="tbaa-prop" %s | FileCheck %s

; CHECK-NOT: load{{.*}}paint{{.*}}tbaa
; CHECK-NOT: store{{.*}}paint{{.*}}tbaa

; This code is based on 526.blender:
;
; direct_link_paint(FileData *fd, Paint **paint) {
;   *paint = ...
;   ... = (*paint)->xxx
; }
;
; direct_link_scene(...) {
;   direct_link_paint(fd, (Paint**)&sce->toolsettings->sculpt);
;   direct_link_paint(fd, (Paint**)&sce->toolsettings->vpaint);
; }
;
; The pointer cast done by the caller, creates a potential aliasing error.
; The "sculpt" and "vpaint" fields are pointers to structures that are
; similar to "Paint", but they are otherwise unrelated.
; After inlining, writes to sce->toolsettings->sculpt and reads/writes to
; *paint may be recognized by TBAA as non-aliasing, if BasicAA cannot trace
; the pointer sources.
; We "fix" this code by removing the TBAA MD, in this situation:
; callee has 2 opaque ptr type parameters
; 2nd parameter is "pointer@XXX" type
;
; caller passes GEP to callee
; GEP has intel-tbaa MD which is !{!struct !access_type...} and
; the !access_type is "pointer@YYY" which doesn't match the callee type.

define internal fastcc void @direct_link_paint(ptr nocapture %fd.1256.val, ptr nocapture noundef %paint) unnamed_addr {
  %t0 = load ptr, ptr %paint, align 8, !tbaa !7
  store ptr undef, ptr %paint, align 8, !tbaa !7
  ret void
}

%struct.s1 = type { ptr, ptr }
%struct.s2 = type { ptr, ptr }
%struct.match = type { ptr, ptr }

define void @direct_link_scene(ptr %first, ptr %ps) {
  %sculpt = getelementptr inbounds %struct.s1, ptr %ps, i64 0, i32 1, !intel-tbaa !5
  call fastcc void @direct_link_paint(ptr %first, ptr noundef nonnull %sculpt)
  %vpaint = getelementptr inbounds %struct.s2, ptr %ps, i64 0, i32 1, !intel-tbaa !9
  call fastcc void @direct_link_paint(ptr %first, ptr noundef nonnull %vpaint)
  ret void
}

; This is a "negative" case where the caller and callee match. We should not
; remove the TBAA in this case.
define internal fastcc void @neg_callee(ptr nocapture %fd.1256.val, ptr nocapture noundef %dont_remove) unnamed_addr {
  %t0 = load ptr, ptr %dont_remove, align 8, !tbaa !7
  store ptr undef, ptr %dont_remove, align 8, !tbaa !7
  ret void
}

define void @neg_caller(ptr %first, ptr %ps) {
  %sculpt = getelementptr inbounds %struct.match, ptr %ps, i64 0, i32 1, !intel-tbaa !11
  call fastcc void @neg_callee(ptr %first, ptr noundef nonnull %sculpt)
  %vpaint = getelementptr inbounds %struct.match, ptr %ps, i64 0, i32 1, !intel-tbaa !11
  call fastcc void @neg_callee(ptr %first, ptr noundef nonnull %vpaint)
  ret void
}

!0 = !{!"Simple C/C++ TBAA"}
!1 = !{!"omnipotent char", !0, i64 0}
!2 = !{!"pointer@type1", !1, i64 0}
!3 = !{!"pointer@type2", !1, i64 0}
!4 = !{!"int", !1, i64 0}
!5 = !{!6, !2, i64 8}
!6 = !{!"struct@s1", !2, i64 0, !8, i64 8}
!7 = !{!3, !3, i64 0}
!8 = !{!"pointer@type3", !1, i64 0}
!9 = !{!10, !8, i64 8}
!10 = !{!"struct@s2", !2, i64 0, !8, i64 8}
!11 = !{!12, !3, i64 8}
!12 = !{!"struct@match", !3, i64 0, !3, i64 8}
