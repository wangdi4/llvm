; REQUIRES: proto_bor
; RUN: llc < %s -O3 -intel-opt-report=high -opt-report-embed -enable-protobuf-opt-report --filetype=obj -o %t.o
; RUN: intel-bin-opt-report %t.o | FileCheck %s

; Check reader tool's correctness for a simple loop which is completely unrolled.
; __attribute__((noinline)) void foo(float *a, float *b, float *c) {
; #pragma unroll(8)
;   for (int i = 0; i < 8; ++i)
;     a[i] = b[i] + c[i];
; }
;
; int main() {
;   float a[8], b[8], c[8];
;   foo(a, b, c);
;   return 0;
; }

; CHECK:      --- Start Intel Binary Optimization Report ---
; CHECK-NEXT: Version: 1.5
; CHECK-NEXT: Property Message Map:
; CHECK-DAG:    C_LOOP_COMPLETE_UNROLL_FACTOR --> Loop completely unrolled by %d
; CHECK-NEXT: Number of reports: 1
; CHECK-EMPTY:
; CHECK-NEXT: === Loop Begin ===
; CHECK-NEXT: Anchor ID: e9a3f5d79e7ea2f824577e5b6b1e4d85
; CHECK-NEXT: Number of remarks: 1
; CHECK-NEXT:   Property: C_LOOP_COMPLETE_UNROLL_FACTOR, Remark ID: 25436, Remark Args: 4
; CHECK-NEXT: ==== Loop End ====
; CHECK-EMPTY:
; CHECK-NEXT: --- End Intel Binary Optimization Report ---

; ModuleID = 'test2.c'
source_filename = "test2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local void @foo(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !6 !intel.optreport.rootnode !8 {
entry:
  %gepload = load float, float* %c, align 4, !dbg !17, !tbaa !18
  %gepload26 = load float, float* %b, align 4, !dbg !22, !tbaa !18
  %0 = fadd fast float %gepload, %gepload26, !dbg !23
  store float %0, float* %a, align 4, !dbg !24, !tbaa !18
  %arrayIdx = getelementptr inbounds float, float* %c, i64 1, !dbg !17
  %gepload27 = load float, float* %arrayIdx, align 4, !dbg !17, !tbaa !18
  %arrayIdx28 = getelementptr inbounds float, float* %b, i64 1, !dbg !22
  %gepload29 = load float, float* %arrayIdx28, align 4, !dbg !22, !tbaa !18
  %1 = fadd fast float %gepload27, %gepload29, !dbg !23
  %arrayIdx30 = getelementptr inbounds float, float* %a, i64 1, !dbg !25
  store float %1, float* %arrayIdx30, align 4, !dbg !24, !tbaa !18
  %arrayIdx32 = getelementptr inbounds float, float* %c, i64 2, !dbg !17
  %gepload33 = load float, float* %arrayIdx32, align 4, !dbg !17, !tbaa !18
  %arrayIdx34 = getelementptr inbounds float, float* %b, i64 2, !dbg !22
  %gepload35 = load float, float* %arrayIdx34, align 4, !dbg !22, !tbaa !18
  %2 = fadd fast float %gepload33, %gepload35, !dbg !23
  %arrayIdx36 = getelementptr inbounds float, float* %a, i64 2, !dbg !25
  store float %2, float* %arrayIdx36, align 4, !dbg !24, !tbaa !18
  %arrayIdx38 = getelementptr inbounds float, float* %c, i64 3, !dbg !17
  %gepload39 = load float, float* %arrayIdx38, align 4, !dbg !17, !tbaa !18
  %arrayIdx40 = getelementptr inbounds float, float* %b, i64 3, !dbg !22
  %gepload41 = load float, float* %arrayIdx40, align 4, !dbg !22, !tbaa !18
  %3 = fadd fast float %gepload39, %gepload41, !dbg !23
  %arrayIdx42 = getelementptr inbounds float, float* %a, i64 3, !dbg !25
  store float %3, float* %arrayIdx42, align 4, !dbg !24, !tbaa !18
  %arrayIdx44 = getelementptr inbounds float, float* %c, i64 4, !dbg !17
  %gepload45 = load float, float* %arrayIdx44, align 4, !dbg !17, !tbaa !18
  %arrayIdx46 = getelementptr inbounds float, float* %b, i64 4, !dbg !22
  %gepload47 = load float, float* %arrayIdx46, align 4, !dbg !22, !tbaa !18
  %4 = fadd fast float %gepload45, %gepload47, !dbg !23
  %arrayIdx48 = getelementptr inbounds float, float* %a, i64 4, !dbg !25
  store float %4, float* %arrayIdx48, align 4, !dbg !24, !tbaa !18
  %arrayIdx50 = getelementptr inbounds float, float* %c, i64 5, !dbg !17
  %gepload51 = load float, float* %arrayIdx50, align 4, !dbg !17, !tbaa !18
  %arrayIdx52 = getelementptr inbounds float, float* %b, i64 5, !dbg !22
  %gepload53 = load float, float* %arrayIdx52, align 4, !dbg !22, !tbaa !18
  %5 = fadd fast float %gepload51, %gepload53, !dbg !23
  %arrayIdx54 = getelementptr inbounds float, float* %a, i64 5, !dbg !25
  store float %5, float* %arrayIdx54, align 4, !dbg !24, !tbaa !18
  %arrayIdx56 = getelementptr inbounds float, float* %c, i64 6, !dbg !17
  %gepload57 = load float, float* %arrayIdx56, align 4, !dbg !17, !tbaa !18
  %arrayIdx58 = getelementptr inbounds float, float* %b, i64 6, !dbg !22
  %gepload59 = load float, float* %arrayIdx58, align 4, !dbg !22, !tbaa !18
  %6 = fadd fast float %gepload57, %gepload59, !dbg !23
  %arrayIdx60 = getelementptr inbounds float, float* %a, i64 6, !dbg !25
  store float %6, float* %arrayIdx60, align 4, !dbg !24, !tbaa !18
  %arrayIdx62 = getelementptr inbounds float, float* %c, i64 7, !dbg !17
  %gepload63 = load float, float* %arrayIdx62, align 4, !dbg !17, !tbaa !18
  %arrayIdx64 = getelementptr inbounds float, float* %b, i64 7, !dbg !22
  %gepload65 = load float, float* %arrayIdx64, align 4, !dbg !22, !tbaa !18
  %7 = fadd fast float %gepload63, %gepload65, !dbg !23
  %arrayIdx66 = getelementptr inbounds float, float* %a, i64 7, !dbg !25
  store float %7, float* %arrayIdx66, align 4, !dbg !24, !tbaa !18
  ret void, !dbg !26
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !dbg !27 {
entry:
  %a = alloca [8 x float], align 16
  %b = alloca [8 x float], align 16
  %c = alloca [8 x float], align 16
  %0 = bitcast [8 x float]* %a to i8*, !dbg !28
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %0) #3, !dbg !28
  %1 = bitcast [8 x float]* %b to i8*, !dbg !28
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %1) #3, !dbg !28
  %2 = bitcast [8 x float]* %c to i8*, !dbg !28
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %2) #3, !dbg !28
  %arraydecay = getelementptr inbounds [8 x float], [8 x float]* %a, i64 0, i64 0, !dbg !29
  %arraydecay1 = getelementptr inbounds [8 x float], [8 x float]* %b, i64 0, i64 0, !dbg !30
  %arraydecay2 = getelementptr inbounds [8 x float], [8 x float]* %c, i64 0, i64 0, !dbg !31
  call void @foo(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2), !dbg !32
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %2) #3, !dbg !33
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %1) #3, !dbg !33
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %0) #3, !dbg !33
  ret i32 0, !dbg !34
}

attributes #0 = { nofree noinline norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test2.c", directory: "/iusers/karthik1/tools/binoptrpt_reader/tests")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 3, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !2)
!8 = distinct !{!"intel.optreport.rootnode", !9}
!9 = distinct !{!"intel.optreport", !10}
!10 = !{!"intel.optreport.first_child", !11}
!11 = distinct !{!"intel.optreport.rootnode", !12}
!12 = distinct !{!"intel.optreport", !13, !15}
!13 = !{!"intel.optreport.debug_location", !14}
!14 = !DILocation(line: 5, column: 3, scope: !6)
!15 = !{!"intel.optreport.remarks", !16}
!16 = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 4}
!17 = !DILocation(line: 6, column: 19, scope: !6)
!18 = !{!19, !19, i64 0}
!19 = !{!"float", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = !DILocation(line: 6, column: 12, scope: !6)
!23 = !DILocation(line: 6, column: 17, scope: !6)
!24 = !DILocation(line: 6, column: 10, scope: !6)
!25 = !DILocation(line: 6, column: 5, scope: !6)
!26 = !DILocation(line: 7, column: 1, scope: !6)
!27 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !7, scopeLine: 9, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!28 = !DILocation(line: 10, column: 3, scope: !27)
!29 = !DILocation(line: 11, column: 7, scope: !27)
!30 = !DILocation(line: 11, column: 10, scope: !27)
!31 = !DILocation(line: 11, column: 13, scope: !27)
!32 = !DILocation(line: 11, column: 3, scope: !27)
!33 = !DILocation(line: 13, column: 1, scope: !27)
!34 = !DILocation(line: 12, column: 3, scope: !27)
