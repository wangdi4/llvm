; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -S 2>&1 | FileCheck %s

; Check that the conditionals are removed from the blocks if.else.i and
; lzma_next_end.exit in @lzma_end.

; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Analysing lzma_end
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Second IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Success
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Second IF BB is proven
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Success

; CHECK-LABEL: define dso_local void @lzma_end(
; CHECK: if.else.i:
; CHECK-NEXT: br label
; CHECK: lzma_next_end.exit:
; CHECK-NEXT: phi
; CHECK-NEXT: phi
; CHECK-NEXT: call
; CHECK-NEXT: br label

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_stream = type { i8*, i64, i64, i8*, i64, i64, %struct.lzma_allocator*, %struct.lzma_internal_s*, i8*, i8*, i8*, i8*, i64, i64, i64, i64, i32, i32 }
%struct.lzma_allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
%struct.lzma_internal_s = type { %struct.lzma_next_coder_s, i32, i64, [4 x i8], i8 }
%struct.lzma_next_coder_s = type { i8*, i64, i64, i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, i64*, i64, i32)*, void (i8*, %struct.lzma_allocator*)*, i32 (i8*)*, i32 (i8*, i64*, i64*, i64)*, i32 (i8*, %struct.lzma_allocator*, %struct.lzma_filter*, %struct.lzma_filter*)* }
%struct.lzma_filter = type { i64, i8* }

; Function Attrs: nounwind uwtable
define dso_local void @lzma_end(%struct.lzma_stream* %strm) local_unnamed_addr #0 {
entry:
  %.compoundliteral.sroa.3.i = alloca { i64, i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, i64*, i64, i32)*, void (i8*, %struct.lzma_allocator*)*, i32 (i8*)*, i32 (i8*, i64*, i64*, i64)*, i32 (i8*, %struct.lzma_allocator*, %struct.lzma_filter*, %struct.lzma_filter*)* }, align 8
  %cmp.not = icmp eq %struct.lzma_stream* %strm, null
  br i1 %cmp.not, label %if.end, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %internal = getelementptr inbounds %struct.lzma_stream, %struct.lzma_stream* %strm, i64 0, i32 7, !intel-tbaa !6
  %0 = load %struct.lzma_internal_s*, %struct.lzma_internal_s** %internal, align 8, !tbaa !6
  %cmp1.not = icmp eq %struct.lzma_internal_s* %0, null
  br i1 %cmp1.not, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %1 = bitcast %struct.lzma_internal_s* %0 to i8*
  %next = getelementptr inbounds %struct.lzma_internal_s, %struct.lzma_internal_s* %0, i64 0, i32 0, !intel-tbaa !15
  %allocator = getelementptr inbounds %struct.lzma_stream, %struct.lzma_stream* %strm, i64 0, i32 6, !intel-tbaa !22
  %2 = load %struct.lzma_allocator*, %struct.lzma_allocator** %allocator, align 8, !tbaa !22
  %.compoundliteral.sroa.3.i.0.i.0..sroa_cast19 = bitcast { i64, i32 (i8*, %struct.lzma_allocator*, i8*, i64*, i64, i8*, i64*, i64, i32)*, void (i8*, %struct.lzma_allocator*)*, i32 (i8*)*, i32 (i8*, i64*, i64*, i64)*, i32 (i8*, %struct.lzma_allocator*, %struct.lzma_filter*, %struct.lzma_filter*)* }* %.compoundliteral.sroa.3.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 48, i8* nonnull %.compoundliteral.sroa.3.i.0.i.0..sroa_cast19)
  %init.i = getelementptr inbounds %struct.lzma_next_coder_s, %struct.lzma_next_coder_s* %next, i64 0, i32 2, !intel-tbaa !23
  %3 = load i64, i64* %init.i, align 8, !tbaa !24
  %cmp.not.i = icmp eq i64 %3, 0
  br i1 %cmp.not.i, label %lzma_next_end.exit, label %if.then.i

if.then.i:                                        ; preds = %if.then
  %end.i = getelementptr inbounds %struct.lzma_next_coder_s, %struct.lzma_next_coder_s* %next, i64 0, i32 4, !intel-tbaa !25
  %4 = load void (i8*, %struct.lzma_allocator*)*, void (i8*, %struct.lzma_allocator*)** %end.i, align 8, !tbaa !26
  %cmp1.not.i = icmp eq void (i8*, %struct.lzma_allocator*)* %4, null
  %5 = getelementptr inbounds %struct.lzma_next_coder_s, %struct.lzma_next_coder_s* %next, i64 0, i32 0
  %6 = load i8*, i8** %5, align 8, !tbaa !27
  br i1 %cmp1.not.i, label %if.else.i, label %if.then2.i

if.then2.i:                                       ; preds = %if.then.i
  br label %if.end.i

if.else.i:                                        ; preds = %if.then.i
  %cmp.not.i.i = icmp eq %struct.lzma_allocator* %2, null
  br i1 %cmp.not.i.i, label %if.else.i.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %if.else.i
  %free.i.i = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %2, i64 0, i32 1, !intel-tbaa !28
  %7 = load void (i8*, i8*)*, void (i8*, i8*)** %free.i.i, align 8, !tbaa !28
  %cmp1.not.i.i = icmp eq void (i8*, i8*)* %7, null
  br i1 %cmp1.not.i.i, label %if.else.i.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %land.lhs.true.i.i
  %opaque.i.i = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %2, i64 0, i32 2, !intel-tbaa !32
  %8 = load i8*, i8** %opaque.i.i, align 8, !tbaa !32
  br label %if.end.i

if.else.i.i:                                      ; preds = %land.lhs.true.i.i, %if.else.i
  tail call void @free(i8* %6) #5
  br label %if.end.i

if.end.i:                                         ; preds = %if.else.i.i, %if.then.i.i, %if.then2.i
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(48) %.compoundliteral.sroa.3.i.0.i.0..sroa_cast19, i8 0, i64 48, i1 false)
  store i8* null, i8** %5, align 8, !tbaa.struct !33
  %.compoundliteral.sroa.2.0..sroa_idx9.i = getelementptr inbounds %struct.lzma_next_coder_s, %struct.lzma_next_coder_s* %next, i64 0, i32 1
  store i64 -1, i64* %.compoundliteral.sroa.2.0..sroa_idx9.i, align 8, !tbaa.struct !38
  %.compoundliteral.sroa.3.0..sroa_cast10.i = bitcast i64* %init.i to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(48) %.compoundliteral.sroa.3.0..sroa_cast10.i, i8* noundef nonnull align 8 dereferenceable(48) %.compoundliteral.sroa.3.i.0.i.0..sroa_cast19, i64 48, i1 false) #5, !tbaa.struct !39
  %.phi.trans.insert = bitcast %struct.lzma_internal_s** %internal to i8**
  %.pre = load i8*, i8** %.phi.trans.insert, align 8, !tbaa !6
  %.pre23 = load %struct.lzma_allocator*, %struct.lzma_allocator** %allocator, align 8, !tbaa !22
  br label %lzma_next_end.exit

lzma_next_end.exit:                               ; preds = %if.then, %if.end.i
  %9 = phi %struct.lzma_allocator* [ %.pre23, %if.end.i ], [ %2, %if.then ]
  %10 = phi i8* [ %.pre, %if.end.i ], [ %1, %if.then ]
  call void @llvm.lifetime.end.p0i8(i64 48, i8* nonnull %.compoundliteral.sroa.3.i.0.i.0..sroa_cast19)
  %cmp.not.i12 = icmp eq %struct.lzma_allocator* %9, null
  br i1 %cmp.not.i12, label %if.else.i15, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %lzma_next_end.exit
  %free.i = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %9, i64 0, i32 1, !intel-tbaa !28
  %11 = load void (i8*, i8*)*, void (i8*, i8*)** %free.i, align 8, !tbaa !28
  %cmp1.not.i13 = icmp eq void (i8*, i8*)* %11, null
  br i1 %cmp1.not.i13, label %if.else.i15, label %if.then.i14

if.then.i14:                                      ; preds = %land.lhs.true.i
  %opaque.i = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %9, i64 0, i32 2, !intel-tbaa !32
  %12 = load i8*, i8** %opaque.i, align 8, !tbaa !32
  br label %lzma_free.exit

if.else.i15:                                      ; preds = %land.lhs.true.i, %lzma_next_end.exit
  tail call void @free(i8* %10) #5
  br label %lzma_free.exit

lzma_free.exit:                                   ; preds = %if.then.i14, %if.else.i15
  store %struct.lzma_internal_s* null, %struct.lzma_internal_s** %internal, align 8, !tbaa !6
  br label %if.end

if.end:                                           ; preds = %lzma_free.exit, %land.lhs.true, %entry
  ret void
}

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare dso_local void @free(i8* nocapture noundef) local_unnamed_addr #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #4

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #4

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #2 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #3 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { argmemonly nofree nosync nounwind willreturn }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!6 = !{!7, !13, i64 56}
!7 = !{!"struct@", !8, i64 0, !11, i64 8, !11, i64 16, !8, i64 24, !11, i64 32, !11, i64 40, !12, i64 48, !13, i64 56, !14, i64 64, !14, i64 72, !14, i64 80, !14, i64 88, !11, i64 96, !11, i64 104, !11, i64 112, !11, i64 120, !9, i64 128, !9, i64 132}
!8 = !{!"pointer@_ZTSPh", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!"long", !9, i64 0}
!12 = !{!"pointer@_ZTSP14lzma_allocator", !9, i64 0}
!13 = !{!"pointer@_ZTSP15lzma_internal_s", !9, i64 0}
!14 = !{!"pointer@_ZTSPv", !9, i64 0}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@lzma_internal_s", !17, i64 0, !9, i64 64, !11, i64 72, !20, i64 80, !21, i64 84}
!17 = !{!"struct@lzma_next_coder_s", !14, i64 0, !11, i64 8, !11, i64 16, !18, i64 24, !19, i64 32, !18, i64 40, !18, i64 48, !18, i64 56}
!18 = !{!"unspecified pointer", !9, i64 0}
!19 = !{!"pointer@_ZTSPFvPvP14lzma_allocatorE", !9, i64 0}
!20 = !{!"array@_ZTSA4_b", !21, i64 0}
!21 = !{!"_Bool", !9, i64 0}
!22 = !{!7, !12, i64 48}
!23 = !{!17, !11, i64 16}
!24 = !{!16, !11, i64 16}
!25 = !{!17, !19, i64 32}
!26 = !{!16, !19, i64 32}
!27 = !{!17, !14, i64 0}
!28 = !{!29, !31, i64 8}
!29 = !{!"struct@", !30, i64 0, !31, i64 8, !14, i64 16}
!30 = !{!"pointer@_ZTSPFPvS_mmE", !9, i64 0}
!31 = !{!"pointer@_ZTSPFvPvS_E", !9, i64 0}
!32 = !{!29, !14, i64 16}
!33 = !{i64 0, i64 8, !34, i64 8, i64 8, !35, i64 16, i64 8, !35, i64 24, i64 8, !36, i64 32, i64 8, !37, i64 40, i64 8, !36, i64 48, i64 8, !36, i64 56, i64 8, !36}
!34 = !{!14, !14, i64 0}
!35 = !{!11, !11, i64 0}
!36 = !{!18, !18, i64 0}
!37 = !{!19, !19, i64 0}
!38 = !{i64 0, i64 8, !35, i64 8, i64 8, !35, i64 16, i64 8, !36, i64 24, i64 8, !37, i64 32, i64 8, !36, i64 40, i64 8, !36, i64 48, i64 8, !36}
!39 = !{i64 0, i64 8, !35, i64 8, i64 8, !36, i64 16, i64 8, !37, i64 24, i64 8, !36, i64 32, i64 8, !36, i64 40, i64 8, !36}
