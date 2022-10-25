; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-elim-ro-field-access-op -S -debug-only=elim-ro-field-access 2>&1 | FileCheck %s

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
; CHECK-NEXT: br label

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS14lzma_allocator.lzma_allocator = type { ptr, ptr, ptr }
%struct._ZTS11lzma_stream.lzma_stream = type { ptr, i64, i64, ptr, i64, i64, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i32, i32 }
%struct._ZTS15lzma_internal_s.lzma_internal_s = type { %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, i32, i64, [4 x i8], i8 }
%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s = type { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }
%struct._ZTS11lzma_filter.lzma_filter = type { i64, ptr }

; Function Attrs: nounwind uwtable
define dso_local void @lzma_end(ptr "intel_dtrans_func_index"="1" %strm) local_unnamed_addr #0 !intel.dtrans.func.type !36 {
entry:
  %cmp.not = icmp eq ptr %strm, null
  br i1 %cmp.not, label %if.end, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %internal = getelementptr inbounds %struct._ZTS11lzma_stream.lzma_stream, ptr %strm, i64 0, i32 7, !intel-tbaa !38
  %i = load ptr, ptr %internal, align 8, !tbaa !38
  %cmp1.not = icmp eq ptr %i, null
  br i1 %cmp1.not, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %i1 = bitcast ptr %i to ptr
  %next = getelementptr inbounds %struct._ZTS15lzma_internal_s.lzma_internal_s, ptr %i, i64 0, i32 0, !intel-tbaa !47
  %allocator = getelementptr inbounds %struct._ZTS11lzma_stream.lzma_stream, ptr %strm, i64 0, i32 6, !intel-tbaa !54
  %i2 = load ptr, ptr %allocator, align 8, !tbaa !54
  %init.i = getelementptr inbounds %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, ptr %next, i64 0, i32 2, !intel-tbaa !55
  %i3 = load i64, ptr %init.i, align 8, !tbaa !56
  %cmp.not.i = icmp eq i64 %i3, 0
  br i1 %cmp.not.i, label %lzma_next_end.exit, label %if.then.i

if.then.i:                                        ; preds = %if.then
  %end.i = getelementptr inbounds %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, ptr %next, i64 0, i32 4, !intel-tbaa !57
  %i4 = load ptr, ptr %end.i, align 8, !tbaa !58
  %cmp1.not.i = icmp eq ptr %i4, null
  %i5 = getelementptr inbounds %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, ptr %next, i64 0, i32 0
  %i6 = load ptr, ptr %i5, align 8, !tbaa !59
  br i1 %cmp1.not.i, label %if.else.i, label %if.then2.i

if.then2.i:                                       ; preds = %if.then.i
  br label %if.end.i

if.else.i:                                        ; preds = %if.then.i
  %cmp.not.i.i = icmp eq ptr %i2, null
  br i1 %cmp.not.i.i, label %if.else.i.i, label %land.lhs.true.i.i

land.lhs.true.i.i:                                ; preds = %if.else.i
  %free.i.i = getelementptr inbounds %struct._ZTS14lzma_allocator.lzma_allocator, ptr %i2, i64 0, i32 1, !intel-tbaa !60
  %i7 = load ptr, ptr %free.i.i, align 8, !tbaa !60
  %cmp1.not.i.i = icmp eq ptr %i7, null
  br i1 %cmp1.not.i.i, label %if.else.i.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %land.lhs.true.i.i
  %opaque.i.i = getelementptr inbounds %struct._ZTS14lzma_allocator.lzma_allocator, ptr %i2, i64 0, i32 2, !intel-tbaa !64
  %i8 = load ptr, ptr %opaque.i.i, align 8, !tbaa !64
  br label %if.end.i

if.else.i.i:                                      ; preds = %land.lhs.true.i.i, %if.else.i
  tail call void @free(ptr %i6) #5
  br label %if.end.i

if.end.i:                                         ; preds = %if.else.i.i, %if.then.i.i, %if.then2.i
  store ptr null, ptr %i5, align 8, !tbaa.struct !65
  %.phi.trans.insert = bitcast ptr %internal to ptr
  %.pre = load ptr, ptr %.phi.trans.insert, align 8, !tbaa !38
  %.pre23 = load ptr, ptr %allocator, align 8, !tbaa !54
  br label %lzma_next_end.exit

lzma_next_end.exit:                               ; preds = %if.end.i, %if.then
  %i9 = phi ptr [ %.pre23, %if.end.i ], [ %i2, %if.then ]
  %i10 = phi ptr [ %.pre, %if.end.i ], [ %i1, %if.then ]
  %cmp.not.i12 = icmp eq ptr %i9, null
  br i1 %cmp.not.i12, label %if.else.i15, label %land.lhs.true.i

land.lhs.true.i:                                  ; preds = %lzma_next_end.exit
  %free.i = getelementptr inbounds %struct._ZTS14lzma_allocator.lzma_allocator, ptr %i9, i64 0, i32 1, !intel-tbaa !60
  %i11 = load ptr, ptr %free.i, align 8, !tbaa !60
  %cmp1.not.i13 = icmp eq ptr %i11, null
  br i1 %cmp1.not.i13, label %if.else.i15, label %if.then.i14

if.then.i14:                                      ; preds = %land.lhs.true.i
  %opaque.i = getelementptr inbounds %struct._ZTS14lzma_allocator.lzma_allocator, ptr %i9, i64 0, i32 2, !intel-tbaa !64
  %i12 = load ptr, ptr %opaque.i, align 8, !tbaa !64
  br label %lzma_free.exit

if.else.i15:                                      ; preds = %land.lhs.true.i, %lzma_next_end.exit
  tail call void @free(ptr %i10) #5
  br label %lzma_free.exit

lzma_free.exit:                                   ; preds = %if.else.i15, %if.then.i14
  store ptr null, ptr %internal, align 8, !tbaa !38
  br label %if.end

if.end:                                           ; preds = %lzma_free.exit, %land.lhs.true, %entry
  ret void
}

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !72 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #4

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nofree nounwind willreturn writeonly }
attributes #3 = { argmemonly nofree nounwind willreturn }
attributes #4 = { argmemonly nofree nosync nounwind willreturn }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !11, !17, !21, !34}
!llvm.ident = !{!35}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS11lzma_stream.lzma_stream zeroinitializer, i32 18, !6, !7, !7, !6, !7, !7, !8, !9, !6, !6, !6, !6, !7, !7, !7, !7, !10, !10}
!6 = !{i8 0, i32 1}
!7 = !{i64 0, i32 0}
!8 = !{%struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 1}
!9 = !{%struct._ZTS15lzma_internal_s.lzma_internal_s zeroinitializer, i32 1}
!10 = !{i32 0, i32 0}
!11 = !{!"S", %struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 3, !12, !14, !6}
!12 = !{!13, i32 1}
!13 = !{!"F", i1 false, i32 3, !6, !6, !7, !7}
!14 = !{!15, i32 1}
!15 = !{!"F", i1 false, i32 2, !16, !6, !6}
!16 = !{!"void", i32 0}
!17 = !{!"S", %struct._ZTS15lzma_internal_s.lzma_internal_s zeroinitializer, i32 5, !18, !10, !7, !19, !20}
!18 = !{%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 0}
!19 = !{!"A", i32 4, !20}
!20 = !{i8 0, i32 0}
!21 = !{!"S", %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 8, !6, !7, !7, !22, !25, !27, !29, !31}
!22 = !{!23, i32 1}
!23 = !{!"F", i1 false, i32 9, !10, !6, !8, !6, !24, !7, !6, !24, !7, !10}
!24 = !{i64 0, i32 1}
!25 = !{!26, i32 1}
!26 = !{!"F", i1 false, i32 2, !16, !6, !8}
!27 = !{!28, i32 1}
!28 = !{!"F", i1 false, i32 1, !10, !6}
!29 = !{!30, i32 1}
!30 = !{!"F", i1 false, i32 4, !10, !6, !24, !24, !7}
!31 = !{!32, i32 1}
!32 = !{!"F", i1 false, i32 4, !10, !6, !8, !33, !33}
!33 = !{%struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 1}
!34 = !{!"S", %struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 2, !7, !6}
!35 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!36 = distinct !{!37}
!37 = !{%struct._ZTS11lzma_stream.lzma_stream zeroinitializer, i32 1}
!38 = !{!39, !45, i64 56}
!39 = !{!"struct@", !40, i64 0, !43, i64 8, !43, i64 16, !40, i64 24, !43, i64 32, !43, i64 40, !44, i64 48, !45, i64 56, !46, i64 64, !46, i64 72, !46, i64 80, !46, i64 88, !43, i64 96, !43, i64 104, !43, i64 112, !43, i64 120, !41, i64 128, !41, i64 132}
!40 = !{!"pointer@_ZTSPh", !41, i64 0}
!41 = !{!"omnipotent char", !42, i64 0}
!42 = !{!"Simple C/C++ TBAA"}
!43 = !{!"long", !41, i64 0}
!44 = !{!"pointer@_ZTSP14lzma_allocator", !41, i64 0}
!45 = !{!"pointer@_ZTSP15lzma_internal_s", !41, i64 0}
!46 = !{!"pointer@_ZTSPv", !41, i64 0}
!47 = !{!48, !49, i64 0}
!48 = !{!"struct@lzma_internal_s", !49, i64 0, !41, i64 64, !43, i64 72, !52, i64 80, !53, i64 84}
!49 = !{!"struct@lzma_next_coder_s", !46, i64 0, !43, i64 8, !43, i64 16, !50, i64 24, !51, i64 32, !50, i64 40, !50, i64 48, !50, i64 56}
!50 = !{!"unspecified pointer", !41, i64 0}
!51 = !{!"pointer@_ZTSPFvPvP14lzma_allocatorE", !41, i64 0}
!52 = !{!"array@_ZTSA4_b", !53, i64 0}
!53 = !{!"_Bool", !41, i64 0}
!54 = !{!39, !44, i64 48}
!55 = !{!49, !43, i64 16}
!56 = !{!48, !43, i64 16}
!57 = !{!49, !51, i64 32}
!58 = !{!48, !51, i64 32}
!59 = !{!49, !46, i64 0}
!60 = !{!61, !63, i64 8}
!61 = !{!"struct@", !62, i64 0, !63, i64 8, !46, i64 16}
!62 = !{!"pointer@_ZTSPFPvS_mmE", !41, i64 0}
!63 = !{!"pointer@_ZTSPFvPvS_E", !41, i64 0}
!64 = !{!61, !46, i64 16}
!65 = !{i64 0, i64 8, !66, i64 8, i64 8, !67, i64 16, i64 8, !67, i64 24, i64 8, !68, i64 32, i64 8, !69, i64 40, i64 8, !68, i64 48, i64 8, !68, i64 56, i64 8, !68}
!66 = !{!46, !46, i64 0}
!67 = !{!43, !43, i64 0}
!68 = !{!50, !50, i64 0}
!69 = !{!51, !51, i64 0}
!70 = !{i64 0, i64 8, !67, i64 8, i64 8, !67, i64 16, i64 8, !68, i64 24, i64 8, !69, i64 32, i64 8, !68, i64 40, i64 8, !68, i64 48, i64 8, !68}
!71 = !{i64 0, i64 8, !67, i64 8, i64 8, !68, i64 16, i64 8, !69, i64 24, i64 8, !68, i64 32, i64 8, !68, i64 40, i64 8, !68}
!72 = distinct !{!6}
