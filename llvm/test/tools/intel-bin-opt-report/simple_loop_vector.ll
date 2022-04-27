; REQUIRES: proto_bor
; RUN: llc < %s -O3 -intel-opt-report=high -opt-report-embed -enable-protobuf-opt-report --filetype=obj -o %t.o
; RUN: intel-bin-opt-report %t.o | FileCheck %s

; Check reader tool's correctness for simple loop that is vectorized.
; __attribute__((noinline)) void foo(float *a, float *b, float *c) {
; #pragma omp simd
;   for (int i = 0; i < 100; ++i)
;     a[i] = b[i] + c[i];
; }
;
; int main() {
;   float a[100], b[100], c[100];
;   foo(a, b, c);
;   return 0;
; }

; CHECK:      --- Start Intel Binary Optimization Report ---
; CHECK-NEXT: Version: 1.5
; CHECK-NEXT: Property Message Map:
; CHECK-DAG:    C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-DAG:    C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; CHECK-NEXT: Number of reports: 2

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: e9a3f5d79e7ea2f824577e5b6b1e4d85
; CHECK-DAG:  Number of remarks: 2
; CHECK-DAG:    Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; CHECK-DAG:    Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 8
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: acaeb5319062b23bfbf47f58b6360c4b
; CHECK-DAG:  Number of remarks: 0
; CHECK-DAG:  ==== Loop End ====

; CHECK:      --- End Intel Binary Optimization Report ---

; ModuleID = 'test1.c'
source_filename = "test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !6 !intel.optreport.rootnode !8 {
DIR.OMP.SIMD.118:
  %0 = bitcast float* %b to <8 x float>*, !dbg !26
  %gepload = load <8 x float>, <8 x float>* %0, align 4, !dbg !26, !tbaa !27
  %1 = bitcast float* %c to <8 x float>*, !dbg !31
  %gepload25 = load <8 x float>, <8 x float>* %1, align 4, !dbg !31, !tbaa !27
  %2 = fadd fast <8 x float> %gepload25, %gepload, !dbg !32
  %3 = bitcast float* %a to <8 x float>*, !dbg !33
  store <8 x float> %2, <8 x float>* %3, align 4, !dbg !34, !tbaa !27
  %arrayIdx.1 = getelementptr inbounds float, float* %b, i64 8, !dbg !26
  %4 = bitcast float* %arrayIdx.1 to <8 x float>*, !dbg !26
  %gepload.1 = load <8 x float>, <8 x float>* %4, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.1 = getelementptr inbounds float, float* %c, i64 8, !dbg !31
  %5 = bitcast float* %arrayIdx24.1 to <8 x float>*, !dbg !31
  %gepload25.1 = load <8 x float>, <8 x float>* %5, align 4, !dbg !31, !tbaa !27
  %6 = fadd fast <8 x float> %gepload25.1, %gepload.1, !dbg !32
  %arrayIdx26.1 = getelementptr inbounds float, float* %a, i64 8, !dbg !33
  %7 = bitcast float* %arrayIdx26.1 to <8 x float>*, !dbg !33
  store <8 x float> %6, <8 x float>* %7, align 4, !dbg !34, !tbaa !27
  %arrayIdx.2 = getelementptr inbounds float, float* %b, i64 16, !dbg !26
  %8 = bitcast float* %arrayIdx.2 to <8 x float>*, !dbg !26
  %gepload.2 = load <8 x float>, <8 x float>* %8, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.2 = getelementptr inbounds float, float* %c, i64 16, !dbg !31
  %9 = bitcast float* %arrayIdx24.2 to <8 x float>*, !dbg !31
  %gepload25.2 = load <8 x float>, <8 x float>* %9, align 4, !dbg !31, !tbaa !27
  %10 = fadd fast <8 x float> %gepload25.2, %gepload.2, !dbg !32
  %arrayIdx26.2 = getelementptr inbounds float, float* %a, i64 16, !dbg !33
  %11 = bitcast float* %arrayIdx26.2 to <8 x float>*, !dbg !33
  store <8 x float> %10, <8 x float>* %11, align 4, !dbg !34, !tbaa !27
  %arrayIdx.3 = getelementptr inbounds float, float* %b, i64 24, !dbg !26
  %12 = bitcast float* %arrayIdx.3 to <8 x float>*, !dbg !26
  %gepload.3 = load <8 x float>, <8 x float>* %12, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.3 = getelementptr inbounds float, float* %c, i64 24, !dbg !31
  %13 = bitcast float* %arrayIdx24.3 to <8 x float>*, !dbg !31
  %gepload25.3 = load <8 x float>, <8 x float>* %13, align 4, !dbg !31, !tbaa !27
  %14 = fadd fast <8 x float> %gepload25.3, %gepload.3, !dbg !32
  %arrayIdx26.3 = getelementptr inbounds float, float* %a, i64 24, !dbg !33
  %15 = bitcast float* %arrayIdx26.3 to <8 x float>*, !dbg !33
  store <8 x float> %14, <8 x float>* %15, align 4, !dbg !34, !tbaa !27
  %arrayIdx.4 = getelementptr inbounds float, float* %b, i64 32, !dbg !26
  %16 = bitcast float* %arrayIdx.4 to <8 x float>*, !dbg !26
  %gepload.4 = load <8 x float>, <8 x float>* %16, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.4 = getelementptr inbounds float, float* %c, i64 32, !dbg !31
  %17 = bitcast float* %arrayIdx24.4 to <8 x float>*, !dbg !31
  %gepload25.4 = load <8 x float>, <8 x float>* %17, align 4, !dbg !31, !tbaa !27
  %18 = fadd fast <8 x float> %gepload25.4, %gepload.4, !dbg !32
  %arrayIdx26.4 = getelementptr inbounds float, float* %a, i64 32, !dbg !33
  %19 = bitcast float* %arrayIdx26.4 to <8 x float>*, !dbg !33
  store <8 x float> %18, <8 x float>* %19, align 4, !dbg !34, !tbaa !27
  %arrayIdx.5 = getelementptr inbounds float, float* %b, i64 40, !dbg !26
  %20 = bitcast float* %arrayIdx.5 to <8 x float>*, !dbg !26
  %gepload.5 = load <8 x float>, <8 x float>* %20, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.5 = getelementptr inbounds float, float* %c, i64 40, !dbg !31
  %21 = bitcast float* %arrayIdx24.5 to <8 x float>*, !dbg !31
  %gepload25.5 = load <8 x float>, <8 x float>* %21, align 4, !dbg !31, !tbaa !27
  %22 = fadd fast <8 x float> %gepload25.5, %gepload.5, !dbg !32
  %arrayIdx26.5 = getelementptr inbounds float, float* %a, i64 40, !dbg !33
  %23 = bitcast float* %arrayIdx26.5 to <8 x float>*, !dbg !33
  store <8 x float> %22, <8 x float>* %23, align 4, !dbg !34, !tbaa !27
  %arrayIdx.6 = getelementptr inbounds float, float* %b, i64 48, !dbg !26
  %24 = bitcast float* %arrayIdx.6 to <8 x float>*, !dbg !26
  %gepload.6 = load <8 x float>, <8 x float>* %24, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.6 = getelementptr inbounds float, float* %c, i64 48, !dbg !31
  %25 = bitcast float* %arrayIdx24.6 to <8 x float>*, !dbg !31
  %gepload25.6 = load <8 x float>, <8 x float>* %25, align 4, !dbg !31, !tbaa !27
  %26 = fadd fast <8 x float> %gepload25.6, %gepload.6, !dbg !32
  %arrayIdx26.6 = getelementptr inbounds float, float* %a, i64 48, !dbg !33
  %27 = bitcast float* %arrayIdx26.6 to <8 x float>*, !dbg !33
  store <8 x float> %26, <8 x float>* %27, align 4, !dbg !34, !tbaa !27
  %arrayIdx.7 = getelementptr inbounds float, float* %b, i64 56, !dbg !26
  %28 = bitcast float* %arrayIdx.7 to <8 x float>*, !dbg !26
  %gepload.7 = load <8 x float>, <8 x float>* %28, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.7 = getelementptr inbounds float, float* %c, i64 56, !dbg !31
  %29 = bitcast float* %arrayIdx24.7 to <8 x float>*, !dbg !31
  %gepload25.7 = load <8 x float>, <8 x float>* %29, align 4, !dbg !31, !tbaa !27
  %30 = fadd fast <8 x float> %gepload25.7, %gepload.7, !dbg !32
  %arrayIdx26.7 = getelementptr inbounds float, float* %a, i64 56, !dbg !33
  %31 = bitcast float* %arrayIdx26.7 to <8 x float>*, !dbg !33
  store <8 x float> %30, <8 x float>* %31, align 4, !dbg !34, !tbaa !27
  %arrayIdx.8 = getelementptr inbounds float, float* %b, i64 64, !dbg !26
  %32 = bitcast float* %arrayIdx.8 to <8 x float>*, !dbg !26
  %gepload.8 = load <8 x float>, <8 x float>* %32, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.8 = getelementptr inbounds float, float* %c, i64 64, !dbg !31
  %33 = bitcast float* %arrayIdx24.8 to <8 x float>*, !dbg !31
  %gepload25.8 = load <8 x float>, <8 x float>* %33, align 4, !dbg !31, !tbaa !27
  %34 = fadd fast <8 x float> %gepload25.8, %gepload.8, !dbg !32
  %arrayIdx26.8 = getelementptr inbounds float, float* %a, i64 64, !dbg !33
  %35 = bitcast float* %arrayIdx26.8 to <8 x float>*, !dbg !33
  store <8 x float> %34, <8 x float>* %35, align 4, !dbg !34, !tbaa !27
  %arrayIdx.9 = getelementptr inbounds float, float* %b, i64 72, !dbg !26
  %36 = bitcast float* %arrayIdx.9 to <8 x float>*, !dbg !26
  %gepload.9 = load <8 x float>, <8 x float>* %36, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.9 = getelementptr inbounds float, float* %c, i64 72, !dbg !31
  %37 = bitcast float* %arrayIdx24.9 to <8 x float>*, !dbg !31
  %gepload25.9 = load <8 x float>, <8 x float>* %37, align 4, !dbg !31, !tbaa !27
  %38 = fadd fast <8 x float> %gepload25.9, %gepload.9, !dbg !32
  %arrayIdx26.9 = getelementptr inbounds float, float* %a, i64 72, !dbg !33
  %39 = bitcast float* %arrayIdx26.9 to <8 x float>*, !dbg !33
  store <8 x float> %38, <8 x float>* %39, align 4, !dbg !34, !tbaa !27
  %arrayIdx.10 = getelementptr inbounds float, float* %b, i64 80, !dbg !26
  %40 = bitcast float* %arrayIdx.10 to <8 x float>*, !dbg !26
  %gepload.10 = load <8 x float>, <8 x float>* %40, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.10 = getelementptr inbounds float, float* %c, i64 80, !dbg !31
  %41 = bitcast float* %arrayIdx24.10 to <8 x float>*, !dbg !31
  %gepload25.10 = load <8 x float>, <8 x float>* %41, align 4, !dbg !31, !tbaa !27
  %42 = fadd fast <8 x float> %gepload25.10, %gepload.10, !dbg !32
  %arrayIdx26.10 = getelementptr inbounds float, float* %a, i64 80, !dbg !33
  %43 = bitcast float* %arrayIdx26.10 to <8 x float>*, !dbg !33
  store <8 x float> %42, <8 x float>* %43, align 4, !dbg !34, !tbaa !27
  %arrayIdx.11 = getelementptr inbounds float, float* %b, i64 88, !dbg !26
  %44 = bitcast float* %arrayIdx.11 to <8 x float>*, !dbg !26
  %gepload.11 = load <8 x float>, <8 x float>* %44, align 4, !dbg !26, !tbaa !27
  %arrayIdx24.11 = getelementptr inbounds float, float* %c, i64 88, !dbg !31
  %45 = bitcast float* %arrayIdx24.11 to <8 x float>*, !dbg !31
  %gepload25.11 = load <8 x float>, <8 x float>* %45, align 4, !dbg !31, !tbaa !27
  %46 = fadd fast <8 x float> %gepload25.11, %gepload.11, !dbg !32
  %arrayIdx26.11 = getelementptr inbounds float, float* %a, i64 88, !dbg !33
  %47 = bitcast float* %arrayIdx26.11 to <8 x float>*, !dbg !33
  store <8 x float> %46, <8 x float>* %47, align 4, !dbg !34, !tbaa !27
  %arrayIdx31 = getelementptr inbounds float, float* %b, i64 96, !dbg !26
  %gepload32 = load float, float* %arrayIdx31, align 4, !dbg !26, !tbaa !27, !llvm.access.group !35
  %arrayIdx33 = getelementptr inbounds float, float* %c, i64 96, !dbg !31
  %gepload34 = load float, float* %arrayIdx33, align 4, !dbg !31, !tbaa !27, !llvm.access.group !35
  %48 = fadd fast float %gepload34, %gepload32, !dbg !32
  %arrayIdx35 = getelementptr inbounds float, float* %a, i64 96, !dbg !33
  store float %48, float* %arrayIdx35, align 4, !dbg !34, !tbaa !27, !llvm.access.group !35
  %arrayIdx31.1 = getelementptr inbounds float, float* %b, i64 97, !dbg !26
  %gepload32.1 = load float, float* %arrayIdx31.1, align 4, !dbg !26, !tbaa !27, !llvm.access.group !35
  %arrayIdx33.1 = getelementptr inbounds float, float* %c, i64 97, !dbg !31
  %gepload34.1 = load float, float* %arrayIdx33.1, align 4, !dbg !31, !tbaa !27, !llvm.access.group !35
  %49 = fadd fast float %gepload34.1, %gepload32.1, !dbg !32
  %arrayIdx35.1 = getelementptr inbounds float, float* %a, i64 97, !dbg !33
  store float %49, float* %arrayIdx35.1, align 4, !dbg !34, !tbaa !27, !llvm.access.group !35
  %arrayIdx31.2 = getelementptr inbounds float, float* %b, i64 98, !dbg !26
  %gepload32.2 = load float, float* %arrayIdx31.2, align 4, !dbg !26, !tbaa !27, !llvm.access.group !35
  %arrayIdx33.2 = getelementptr inbounds float, float* %c, i64 98, !dbg !31
  %gepload34.2 = load float, float* %arrayIdx33.2, align 4, !dbg !31, !tbaa !27, !llvm.access.group !35
  %50 = fadd fast float %gepload34.2, %gepload32.2, !dbg !32
  %arrayIdx35.2 = getelementptr inbounds float, float* %a, i64 98, !dbg !33
  store float %50, float* %arrayIdx35.2, align 4, !dbg !34, !tbaa !27, !llvm.access.group !35
  %arrayIdx31.3 = getelementptr inbounds float, float* %b, i64 99, !dbg !26
  %gepload32.3 = load float, float* %arrayIdx31.3, align 4, !dbg !26, !tbaa !27, !llvm.access.group !35
  %arrayIdx33.3 = getelementptr inbounds float, float* %c, i64 99, !dbg !31
  %gepload34.3 = load float, float* %arrayIdx33.3, align 4, !dbg !31, !tbaa !27, !llvm.access.group !35
  %51 = fadd fast float %gepload34.3, %gepload32.3, !dbg !32
  %arrayIdx35.3 = getelementptr inbounds float, float* %a, i64 99, !dbg !33
  store float %51, float* %arrayIdx35.3, align 4, !dbg !34, !tbaa !27, !llvm.access.group !35
  ret void, !dbg !36
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !dbg !37 {
entry:
  %a = alloca [100 x float], align 16
  %b = alloca [100 x float], align 16
  %c = alloca [100 x float], align 16
  %0 = bitcast [100 x float]* %a to i8*, !dbg !38
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #3, !dbg !38
  %1 = bitcast [100 x float]* %b to i8*, !dbg !38
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %1) #3, !dbg !38
  %2 = bitcast [100 x float]* %c to i8*, !dbg !38
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %2) #3, !dbg !38
  %arraydecay = getelementptr inbounds [100 x float], [100 x float]* %a, i64 0, i64 0, !dbg !39
  %arraydecay1 = getelementptr inbounds [100 x float], [100 x float]* %b, i64 0, i64 0, !dbg !40
  %arraydecay2 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 0, !dbg !41
  call void @foo(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2), !dbg !42
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %2) #3, !dbg !43
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %1) #3, !dbg !43
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #3, !dbg !43
  ret i32 0, !dbg !44
}

attributes #0 = { noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test1.c", directory: "/iusers/karthik1/tools/binoptrpt_reader/tests")
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
!12 = distinct !{!"intel.optreport", !13, !15, !19}
!13 = !{!"intel.optreport.debug_location", !14}
!14 = !DILocation(line: 4, column: 1, scope: !6)
!15 = !{!"intel.optreport.remarks", !16, !17, !18}
!16 = !{!"intel.optreport.remark", i32 15300, !"LOOP WAS VECTORIZED"}
!17 = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", !"8"}
!18 = !{!"intel.optreport.remark", i32 0, !"LLorg: Loop has been completely unrolled"}
!19 = !{!"intel.optreport.next_sibling", !20}
!20 = distinct !{!"intel.optreport.rootnode", !21}
!21 = distinct !{!"intel.optreport", !13, !22, !24}
!22 = !{!"intel.optreport.origin", !23}
!23 = !{!"intel.optreport.remark", i32 0, !"Remainder loop for vectorization"}
!24 = !{!"intel.optreport.remarks", !25, !18}
!25 = !{!"intel.optreport.remark", i32 15441, !"remainder loop was not vectorized: %s ", !""}
!26 = !DILocation(line: 6, column: 12, scope: !6)
!27 = !{!28, !28, i64 0}
!28 = !{!"float", !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 6, column: 19, scope: !6)
!32 = !DILocation(line: 6, column: 17, scope: !6)
!33 = !DILocation(line: 6, column: 5, scope: !6)
!34 = !DILocation(line: 6, column: 10, scope: !6)
!35 = distinct !{}
!36 = !DILocation(line: 7, column: 1, scope: !6)
!37 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !7, scopeLine: 9, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!38 = !DILocation(line: 10, column: 3, scope: !37)
!39 = !DILocation(line: 11, column: 7, scope: !37)
!40 = !DILocation(line: 11, column: 10, scope: !37)
!41 = !DILocation(line: 11, column: 13, scope: !37)
!42 = !DILocation(line: 11, column: 3, scope: !37)
!43 = !DILocation(line: 13, column: 1, scope: !37)
!44 = !DILocation(line: 12, column: 3, scope: !37)
