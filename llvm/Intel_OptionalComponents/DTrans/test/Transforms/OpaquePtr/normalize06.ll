; RUN: opt -opaque-pointers -S -whole-program-assume -dtrans-normalizeop < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -S -whole-program-assume -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS6x264_t.x264_t = type { i32, %struct._ZTS16pixel_function_t.pixel_function_t, %struct._ZTS16pixel_function_t.pixel_function_t, i32 }
%struct._ZTS16pixel_function_t.pixel_function_t = type { [7 x ptr], [7 x ptr] }

@glob = dso_local global i32 0, align 4

; Check that a GEP operator with name %dtnorm is inserted so that the PHI node
; inputs are normalized to the same ptr type.

; CHECK: %field1 = getelementptr inbounds %struct._ZTS6x264_t.x264_t, ptr %i1, i32 0, i32 1
; CHECK: %dtnorm = getelementptr [7 x ptr], ptr %field1, i64 0, i32 0
; CHECK: %myptr = phi ptr [ %dtnorm, %if.then ], [ %arraydecay7, %if.else ]

define dso_local void @_Z3fooP6x264_t(ptr noundef "intel_dtrans_func_index"="1" %h) !intel.dtrans.func.type !13 {
entry:
  %h.addr = alloca ptr, align 8, !intel_dtrans_type !14
  store ptr %h, ptr %h.addr, align 8
  %i = load i32, ptr @glob, align 4
  %cmp = icmp sgt i32 %i, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %i1 = load ptr, ptr %h.addr, align 8
  %field1 = getelementptr inbounds %struct._ZTS6x264_t.x264_t, ptr %i1, i32 0, i32 1
  br label %if.end

if.else:                                          ; preds = %entry
  %i4 = load ptr, ptr %h.addr, align 8
  %field25 = getelementptr inbounds %struct._ZTS6x264_t.x264_t, ptr %i4, i32 0, i32 2
  %myarray06 = getelementptr inbounds %struct._ZTS16pixel_function_t.pixel_function_t, ptr %field25, i32 0, i32 0
  %arraydecay7 = getelementptr inbounds [7 x ptr], ptr %myarray06, i64 0, i64 0
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %myptr = phi ptr [ %field1, %if.then ], [ %arraydecay7, %if.else ]
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %myptr, ptr align 8 %myptr, i64 56, i1 false)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !8}
!llvm.ident = !{!12}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS6x264_t.x264_t zeroinitializer, i32 4, !6, !7, !7, !6}
!6 = !{i32 0, i32 0}
!7 = !{%struct._ZTS16pixel_function_t.pixel_function_t zeroinitializer, i32 0}
!8 = !{!"S", %struct._ZTS16pixel_function_t.pixel_function_t zeroinitializer, i32 2, !9, !9}
!9 = !{!"A", i32 7, !10}
!10 = !{!11, i32 1}
!11 = !{!"F", i1 false, i32 0, !6}
!12 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!13 = distinct !{!14}
!14 = !{%struct._ZTS6x264_t.x264_t zeroinitializer, i32 1}
!15 = !{!16, !16, i64 0}
!16 = !{!"pointer@_ZTSP6x264_t", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C++ TBAA"}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !17, i64 0}
!21 = !{!22, !23, i64 8}
!22 = !{!"struct@_ZTS6x264_t", !20, i64 0, !23, i64 8, !23, i64 120, !20, i64 232}
!23 = !{!"struct@_ZTS16pixel_function_t", !24, i64 0, !24, i64 56}
!24 = !{!"array@_ZTSA7_PFivE", !25, i64 0}
!25 = !{!"pointer@_ZTSPFivE", !17, i64 0}
!26 = !{!23, !24, i64 0}
!27 = !{!22, !23, i64 120}
!28 = !{!23, !24, i64 56}
