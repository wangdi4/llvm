; REQUIRES: proto_bor
; RUN: llc < %s -O3 -intel-opt-report=high -opt-report-embed -enable-protobuf-opt-report --filetype=obj -o %t.o
; RUN: intel-bin-opt-report %t.o | FileCheck %s

; Check reader tool's correctness for a test case which has combinations of
; vectorizer and complete unroll remarks.
; __attribute__((noinline)) void vectorize(float *a, float *b, float *c) {
; #pragma omp simd
;   for (int i = 0; i < 100; ++i)
;     a[i] = b[i] + c[i];
; }
;
; __attribute__((noinline)) void complete_unroll(float *a, float *b, float *c) {
; #pragma unroll(8)
;   for (int i = 0; i < 8; ++i)
;     a[i] = b[i] + c[i];
; }
;
; __attribute__((noinline)) void both(float *a, float *b, float *c) {
; #pragma omp simd
;   for (int i = 0; i < 100; ++i)
;     a[i] = b[i] + c[i];
;
; #pragma unroll(8)
;   for (int i = 0; i < 8; ++i)
;     a[i] = b[i] + c[i];
; }
;
; int main() {
;   float a[100], b[100], c[100];
;   vectorize(a, b, c);
;   float a1[8], b1[8], c1[8];
;   complete_unroll(a1, b1, c1);
;   both(a, b, c);
;   return 0;
; }

; CHECK:      --- Start Intel Binary Optimization Report ---
; CHECK-NEXT: Version: 1.5
; CHECK-NEXT: Property Message Map:
; CHECK-DAG:    C_LOOP_COMPLETE_UNROLL_FACTOR --> Loop completely unrolled by %d
; CHECK-DAG:    C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-DAG:    C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; CHECK-NEXT: Number of reports: 6

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 14dfc559358af039e2d93b8df06cec5b
; CHECK-DAG:  Number of remarks: 2
; CHECK-DAG:    Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; CHECK-DAG:    Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 8
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 1fbb69cbd79087c3b110f2aa82d9c4c9
; CHECK-DAG:  Number of remarks: 0
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 777fc77cf64d4415d3258c0a0bd210d7
; CHECK-DAG:  Number of remarks: 2
; CHECK-DAG:    Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; CHECK-DAG:    Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 8
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: c1c30188f6c29dac507dbf9d16871a9f
; CHECK-DAG:  Number of remarks: 1
; CHECK-DAG:    Property: C_LOOP_COMPLETE_UNROLL_FACTOR, Remark ID: 25436, Remark Args: 4
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 224b9b91a17b07c76c76233dd02efbba
; CHECK-DAG:  Number of remarks: 0
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: a818ab6943787180e6e19d0fb1433f56
; CHECK-DAG:  Number of remarks: 1
; CHECK-DAG:    Property: C_LOOP_COMPLETE_UNROLL_FACTOR, Remark ID: 25436, Remark Args: 4
; CHECK-DAG:  ==== Loop End ====

; CHECK:      --- End Intel Binary Optimization Report ---

; ModuleID = 'test3.c'
source_filename = "test3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @vectorize(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !6 !intel.optreport.rootnode !8 {
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

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local void @complete_unroll(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #2 !dbg !37 !intel.optreport.rootnode !38 {
entry:
  %gepload = load float, float* %c, align 4, !dbg !47, !tbaa !27
  %gepload26 = load float, float* %b, align 4, !dbg !48, !tbaa !27
  %0 = fadd fast float %gepload, %gepload26, !dbg !49
  store float %0, float* %a, align 4, !dbg !50, !tbaa !27
  %arrayIdx = getelementptr inbounds float, float* %c, i64 1, !dbg !47
  %gepload27 = load float, float* %arrayIdx, align 4, !dbg !47, !tbaa !27
  %arrayIdx28 = getelementptr inbounds float, float* %b, i64 1, !dbg !48
  %gepload29 = load float, float* %arrayIdx28, align 4, !dbg !48, !tbaa !27
  %1 = fadd fast float %gepload27, %gepload29, !dbg !49
  %arrayIdx30 = getelementptr inbounds float, float* %a, i64 1, !dbg !51
  store float %1, float* %arrayIdx30, align 4, !dbg !50, !tbaa !27
  %arrayIdx32 = getelementptr inbounds float, float* %c, i64 2, !dbg !47
  %gepload33 = load float, float* %arrayIdx32, align 4, !dbg !47, !tbaa !27
  %arrayIdx34 = getelementptr inbounds float, float* %b, i64 2, !dbg !48
  %gepload35 = load float, float* %arrayIdx34, align 4, !dbg !48, !tbaa !27
  %2 = fadd fast float %gepload33, %gepload35, !dbg !49
  %arrayIdx36 = getelementptr inbounds float, float* %a, i64 2, !dbg !51
  store float %2, float* %arrayIdx36, align 4, !dbg !50, !tbaa !27
  %arrayIdx38 = getelementptr inbounds float, float* %c, i64 3, !dbg !47
  %gepload39 = load float, float* %arrayIdx38, align 4, !dbg !47, !tbaa !27
  %arrayIdx40 = getelementptr inbounds float, float* %b, i64 3, !dbg !48
  %gepload41 = load float, float* %arrayIdx40, align 4, !dbg !48, !tbaa !27
  %3 = fadd fast float %gepload39, %gepload41, !dbg !49
  %arrayIdx42 = getelementptr inbounds float, float* %a, i64 3, !dbg !51
  store float %3, float* %arrayIdx42, align 4, !dbg !50, !tbaa !27
  %arrayIdx44 = getelementptr inbounds float, float* %c, i64 4, !dbg !47
  %gepload45 = load float, float* %arrayIdx44, align 4, !dbg !47, !tbaa !27
  %arrayIdx46 = getelementptr inbounds float, float* %b, i64 4, !dbg !48
  %gepload47 = load float, float* %arrayIdx46, align 4, !dbg !48, !tbaa !27
  %4 = fadd fast float %gepload45, %gepload47, !dbg !49
  %arrayIdx48 = getelementptr inbounds float, float* %a, i64 4, !dbg !51
  store float %4, float* %arrayIdx48, align 4, !dbg !50, !tbaa !27
  %arrayIdx50 = getelementptr inbounds float, float* %c, i64 5, !dbg !47
  %gepload51 = load float, float* %arrayIdx50, align 4, !dbg !47, !tbaa !27
  %arrayIdx52 = getelementptr inbounds float, float* %b, i64 5, !dbg !48
  %gepload53 = load float, float* %arrayIdx52, align 4, !dbg !48, !tbaa !27
  %5 = fadd fast float %gepload51, %gepload53, !dbg !49
  %arrayIdx54 = getelementptr inbounds float, float* %a, i64 5, !dbg !51
  store float %5, float* %arrayIdx54, align 4, !dbg !50, !tbaa !27
  %arrayIdx56 = getelementptr inbounds float, float* %c, i64 6, !dbg !47
  %gepload57 = load float, float* %arrayIdx56, align 4, !dbg !47, !tbaa !27
  %arrayIdx58 = getelementptr inbounds float, float* %b, i64 6, !dbg !48
  %gepload59 = load float, float* %arrayIdx58, align 4, !dbg !48, !tbaa !27
  %6 = fadd fast float %gepload57, %gepload59, !dbg !49
  %arrayIdx60 = getelementptr inbounds float, float* %a, i64 6, !dbg !51
  store float %6, float* %arrayIdx60, align 4, !dbg !50, !tbaa !27
  %arrayIdx62 = getelementptr inbounds float, float* %c, i64 7, !dbg !47
  %gepload63 = load float, float* %arrayIdx62, align 4, !dbg !47, !tbaa !27
  %arrayIdx64 = getelementptr inbounds float, float* %b, i64 7, !dbg !48
  %gepload65 = load float, float* %arrayIdx64, align 4, !dbg !48, !tbaa !27
  %7 = fadd fast float %gepload63, %gepload65, !dbg !49
  %arrayIdx66 = getelementptr inbounds float, float* %a, i64 7, !dbg !51
  store float %7, float* %arrayIdx66, align 4, !dbg !50, !tbaa !27
  ret void, !dbg !52
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @both(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !53 !intel.optreport.rootnode !54 {
DIR.OMP.SIMD.139:
  %0 = bitcast float* %b to <8 x float>*, !dbg !69
  %gepload = load <8 x float>, <8 x float>* %0, align 4, !dbg !69, !tbaa !27
  %1 = bitcast float* %c to <8 x float>*, !dbg !70
  %gepload61 = load <8 x float>, <8 x float>* %1, align 4, !dbg !70, !tbaa !27
  %2 = fadd fast <8 x float> %gepload61, %gepload, !dbg !71
  %3 = bitcast float* %a to <8 x float>*, !dbg !72
  store <8 x float> %2, <8 x float>* %3, align 4, !dbg !73, !tbaa !27
  %arrayIdx.1 = getelementptr inbounds float, float* %b, i64 8, !dbg !69
  %4 = bitcast float* %arrayIdx.1 to <8 x float>*, !dbg !69
  %gepload.1 = load <8 x float>, <8 x float>* %4, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.1 = getelementptr inbounds float, float* %c, i64 8, !dbg !70
  %5 = bitcast float* %arrayIdx60.1 to <8 x float>*, !dbg !70
  %gepload61.1 = load <8 x float>, <8 x float>* %5, align 4, !dbg !70, !tbaa !27
  %6 = fadd fast <8 x float> %gepload61.1, %gepload.1, !dbg !71
  %arrayIdx62.1 = getelementptr inbounds float, float* %a, i64 8, !dbg !72
  %7 = bitcast float* %arrayIdx62.1 to <8 x float>*, !dbg !72
  store <8 x float> %6, <8 x float>* %7, align 4, !dbg !73, !tbaa !27
  %arrayIdx.2 = getelementptr inbounds float, float* %b, i64 16, !dbg !69
  %8 = bitcast float* %arrayIdx.2 to <8 x float>*, !dbg !69
  %gepload.2 = load <8 x float>, <8 x float>* %8, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.2 = getelementptr inbounds float, float* %c, i64 16, !dbg !70
  %9 = bitcast float* %arrayIdx60.2 to <8 x float>*, !dbg !70
  %gepload61.2 = load <8 x float>, <8 x float>* %9, align 4, !dbg !70, !tbaa !27
  %10 = fadd fast <8 x float> %gepload61.2, %gepload.2, !dbg !71
  %arrayIdx62.2 = getelementptr inbounds float, float* %a, i64 16, !dbg !72
  %11 = bitcast float* %arrayIdx62.2 to <8 x float>*, !dbg !72
  store <8 x float> %10, <8 x float>* %11, align 4, !dbg !73, !tbaa !27
  %arrayIdx.3 = getelementptr inbounds float, float* %b, i64 24, !dbg !69
  %12 = bitcast float* %arrayIdx.3 to <8 x float>*, !dbg !69
  %gepload.3 = load <8 x float>, <8 x float>* %12, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.3 = getelementptr inbounds float, float* %c, i64 24, !dbg !70
  %13 = bitcast float* %arrayIdx60.3 to <8 x float>*, !dbg !70
  %gepload61.3 = load <8 x float>, <8 x float>* %13, align 4, !dbg !70, !tbaa !27
  %14 = fadd fast <8 x float> %gepload61.3, %gepload.3, !dbg !71
  %arrayIdx62.3 = getelementptr inbounds float, float* %a, i64 24, !dbg !72
  %15 = bitcast float* %arrayIdx62.3 to <8 x float>*, !dbg !72
  store <8 x float> %14, <8 x float>* %15, align 4, !dbg !73, !tbaa !27
  %arrayIdx.4 = getelementptr inbounds float, float* %b, i64 32, !dbg !69
  %16 = bitcast float* %arrayIdx.4 to <8 x float>*, !dbg !69
  %gepload.4 = load <8 x float>, <8 x float>* %16, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.4 = getelementptr inbounds float, float* %c, i64 32, !dbg !70
  %17 = bitcast float* %arrayIdx60.4 to <8 x float>*, !dbg !70
  %gepload61.4 = load <8 x float>, <8 x float>* %17, align 4, !dbg !70, !tbaa !27
  %18 = fadd fast <8 x float> %gepload61.4, %gepload.4, !dbg !71
  %arrayIdx62.4 = getelementptr inbounds float, float* %a, i64 32, !dbg !72
  %19 = bitcast float* %arrayIdx62.4 to <8 x float>*, !dbg !72
  store <8 x float> %18, <8 x float>* %19, align 4, !dbg !73, !tbaa !27
  %arrayIdx.5 = getelementptr inbounds float, float* %b, i64 40, !dbg !69
  %20 = bitcast float* %arrayIdx.5 to <8 x float>*, !dbg !69
  %gepload.5 = load <8 x float>, <8 x float>* %20, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.5 = getelementptr inbounds float, float* %c, i64 40, !dbg !70
  %21 = bitcast float* %arrayIdx60.5 to <8 x float>*, !dbg !70
  %gepload61.5 = load <8 x float>, <8 x float>* %21, align 4, !dbg !70, !tbaa !27
  %22 = fadd fast <8 x float> %gepload61.5, %gepload.5, !dbg !71
  %arrayIdx62.5 = getelementptr inbounds float, float* %a, i64 40, !dbg !72
  %23 = bitcast float* %arrayIdx62.5 to <8 x float>*, !dbg !72
  store <8 x float> %22, <8 x float>* %23, align 4, !dbg !73, !tbaa !27
  %arrayIdx.6 = getelementptr inbounds float, float* %b, i64 48, !dbg !69
  %24 = bitcast float* %arrayIdx.6 to <8 x float>*, !dbg !69
  %gepload.6 = load <8 x float>, <8 x float>* %24, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.6 = getelementptr inbounds float, float* %c, i64 48, !dbg !70
  %25 = bitcast float* %arrayIdx60.6 to <8 x float>*, !dbg !70
  %gepload61.6 = load <8 x float>, <8 x float>* %25, align 4, !dbg !70, !tbaa !27
  %26 = fadd fast <8 x float> %gepload61.6, %gepload.6, !dbg !71
  %arrayIdx62.6 = getelementptr inbounds float, float* %a, i64 48, !dbg !72
  %27 = bitcast float* %arrayIdx62.6 to <8 x float>*, !dbg !72
  store <8 x float> %26, <8 x float>* %27, align 4, !dbg !73, !tbaa !27
  %arrayIdx.7 = getelementptr inbounds float, float* %b, i64 56, !dbg !69
  %28 = bitcast float* %arrayIdx.7 to <8 x float>*, !dbg !69
  %gepload.7 = load <8 x float>, <8 x float>* %28, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.7 = getelementptr inbounds float, float* %c, i64 56, !dbg !70
  %29 = bitcast float* %arrayIdx60.7 to <8 x float>*, !dbg !70
  %gepload61.7 = load <8 x float>, <8 x float>* %29, align 4, !dbg !70, !tbaa !27
  %30 = fadd fast <8 x float> %gepload61.7, %gepload.7, !dbg !71
  %arrayIdx62.7 = getelementptr inbounds float, float* %a, i64 56, !dbg !72
  %31 = bitcast float* %arrayIdx62.7 to <8 x float>*, !dbg !72
  store <8 x float> %30, <8 x float>* %31, align 4, !dbg !73, !tbaa !27
  %arrayIdx.8 = getelementptr inbounds float, float* %b, i64 64, !dbg !69
  %32 = bitcast float* %arrayIdx.8 to <8 x float>*, !dbg !69
  %gepload.8 = load <8 x float>, <8 x float>* %32, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.8 = getelementptr inbounds float, float* %c, i64 64, !dbg !70
  %33 = bitcast float* %arrayIdx60.8 to <8 x float>*, !dbg !70
  %gepload61.8 = load <8 x float>, <8 x float>* %33, align 4, !dbg !70, !tbaa !27
  %34 = fadd fast <8 x float> %gepload61.8, %gepload.8, !dbg !71
  %arrayIdx62.8 = getelementptr inbounds float, float* %a, i64 64, !dbg !72
  %35 = bitcast float* %arrayIdx62.8 to <8 x float>*, !dbg !72
  store <8 x float> %34, <8 x float>* %35, align 4, !dbg !73, !tbaa !27
  %arrayIdx.9 = getelementptr inbounds float, float* %b, i64 72, !dbg !69
  %36 = bitcast float* %arrayIdx.9 to <8 x float>*, !dbg !69
  %gepload.9 = load <8 x float>, <8 x float>* %36, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.9 = getelementptr inbounds float, float* %c, i64 72, !dbg !70
  %37 = bitcast float* %arrayIdx60.9 to <8 x float>*, !dbg !70
  %gepload61.9 = load <8 x float>, <8 x float>* %37, align 4, !dbg !70, !tbaa !27
  %38 = fadd fast <8 x float> %gepload61.9, %gepload.9, !dbg !71
  %arrayIdx62.9 = getelementptr inbounds float, float* %a, i64 72, !dbg !72
  %39 = bitcast float* %arrayIdx62.9 to <8 x float>*, !dbg !72
  store <8 x float> %38, <8 x float>* %39, align 4, !dbg !73, !tbaa !27
  %arrayIdx.10 = getelementptr inbounds float, float* %b, i64 80, !dbg !69
  %40 = bitcast float* %arrayIdx.10 to <8 x float>*, !dbg !69
  %gepload.10 = load <8 x float>, <8 x float>* %40, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.10 = getelementptr inbounds float, float* %c, i64 80, !dbg !70
  %41 = bitcast float* %arrayIdx60.10 to <8 x float>*, !dbg !70
  %gepload61.10 = load <8 x float>, <8 x float>* %41, align 4, !dbg !70, !tbaa !27
  %42 = fadd fast <8 x float> %gepload61.10, %gepload.10, !dbg !71
  %arrayIdx62.10 = getelementptr inbounds float, float* %a, i64 80, !dbg !72
  %43 = bitcast float* %arrayIdx62.10 to <8 x float>*, !dbg !72
  store <8 x float> %42, <8 x float>* %43, align 4, !dbg !73, !tbaa !27
  %arrayIdx.11 = getelementptr inbounds float, float* %b, i64 88, !dbg !69
  %44 = bitcast float* %arrayIdx.11 to <8 x float>*, !dbg !69
  %gepload.11 = load <8 x float>, <8 x float>* %44, align 4, !dbg !69, !tbaa !27
  %arrayIdx60.11 = getelementptr inbounds float, float* %c, i64 88, !dbg !70
  %45 = bitcast float* %arrayIdx60.11 to <8 x float>*, !dbg !70
  %gepload61.11 = load <8 x float>, <8 x float>* %45, align 4, !dbg !70, !tbaa !27
  %46 = fadd fast <8 x float> %gepload61.11, %gepload.11, !dbg !71
  %arrayIdx62.11 = getelementptr inbounds float, float* %a, i64 88, !dbg !72
  %47 = bitcast float* %arrayIdx62.11 to <8 x float>*, !dbg !72
  store <8 x float> %46, <8 x float>* %47, align 4, !dbg !73, !tbaa !27
  %arrayIdx67 = getelementptr inbounds float, float* %b, i64 96, !dbg !69
  %gepload68 = load float, float* %arrayIdx67, align 4, !dbg !69, !tbaa !27, !llvm.access.group !74
  %arrayIdx69 = getelementptr inbounds float, float* %c, i64 96, !dbg !70
  %gepload70 = load float, float* %arrayIdx69, align 4, !dbg !70, !tbaa !27, !llvm.access.group !74
  %48 = fadd fast float %gepload70, %gepload68, !dbg !71
  %arrayIdx71 = getelementptr inbounds float, float* %a, i64 96, !dbg !72
  store float %48, float* %arrayIdx71, align 4, !dbg !73, !tbaa !27, !llvm.access.group !74
  %arrayIdx67.1 = getelementptr inbounds float, float* %b, i64 97, !dbg !69
  %gepload68.1 = load float, float* %arrayIdx67.1, align 4, !dbg !69, !tbaa !27, !llvm.access.group !74
  %arrayIdx69.1 = getelementptr inbounds float, float* %c, i64 97, !dbg !70
  %gepload70.1 = load float, float* %arrayIdx69.1, align 4, !dbg !70, !tbaa !27, !llvm.access.group !74
  %49 = fadd fast float %gepload70.1, %gepload68.1, !dbg !71
  %arrayIdx71.1 = getelementptr inbounds float, float* %a, i64 97, !dbg !72
  store float %49, float* %arrayIdx71.1, align 4, !dbg !73, !tbaa !27, !llvm.access.group !74
  %arrayIdx67.2 = getelementptr inbounds float, float* %b, i64 98, !dbg !69
  %gepload68.2 = load float, float* %arrayIdx67.2, align 4, !dbg !69, !tbaa !27, !llvm.access.group !74
  %arrayIdx69.2 = getelementptr inbounds float, float* %c, i64 98, !dbg !70
  %gepload70.2 = load float, float* %arrayIdx69.2, align 4, !dbg !70, !tbaa !27, !llvm.access.group !74
  %50 = fadd fast float %gepload70.2, %gepload68.2, !dbg !71
  %arrayIdx71.2 = getelementptr inbounds float, float* %a, i64 98, !dbg !72
  store float %50, float* %arrayIdx71.2, align 4, !dbg !73, !tbaa !27, !llvm.access.group !74
  %arrayIdx67.3 = getelementptr inbounds float, float* %b, i64 99, !dbg !69
  %gepload68.3 = load float, float* %arrayIdx67.3, align 4, !dbg !69, !tbaa !27, !llvm.access.group !74
  %arrayIdx69.3 = getelementptr inbounds float, float* %c, i64 99, !dbg !70
  %gepload70.3 = load float, float* %arrayIdx69.3, align 4, !dbg !70, !tbaa !27, !llvm.access.group !74
  %51 = fadd fast float %gepload70.3, %gepload68.3, !dbg !71
  %arrayIdx71.3 = getelementptr inbounds float, float* %a, i64 99, !dbg !72
  store float %51, float* %arrayIdx71.3, align 4, !dbg !73, !tbaa !27, !llvm.access.group !74
  %gepload73 = load float, float* %c, align 4, !dbg !75, !tbaa !27
  %gepload74 = load float, float* %b, align 4, !dbg !76, !tbaa !27
  %52 = fadd fast float %gepload73, %gepload74, !dbg !77
  store float %52, float* %a, align 4, !dbg !78, !tbaa !27
  %arrayIdx75 = getelementptr inbounds float, float* %c, i64 1, !dbg !75
  %gepload76 = load float, float* %arrayIdx75, align 4, !dbg !75, !tbaa !27
  %arrayIdx77 = getelementptr inbounds float, float* %b, i64 1, !dbg !76
  %gepload78 = load float, float* %arrayIdx77, align 4, !dbg !76, !tbaa !27
  %53 = fadd fast float %gepload76, %gepload78, !dbg !77
  %arrayIdx79 = getelementptr inbounds float, float* %a, i64 1, !dbg !79
  store float %53, float* %arrayIdx79, align 4, !dbg !78, !tbaa !27
  %arrayIdx81 = getelementptr inbounds float, float* %c, i64 2, !dbg !75
  %gepload82 = load float, float* %arrayIdx81, align 4, !dbg !75, !tbaa !27
  %arrayIdx83 = getelementptr inbounds float, float* %b, i64 2, !dbg !76
  %gepload84 = load float, float* %arrayIdx83, align 4, !dbg !76, !tbaa !27
  %54 = fadd fast float %gepload82, %gepload84, !dbg !77
  %arrayIdx85 = getelementptr inbounds float, float* %a, i64 2, !dbg !79
  store float %54, float* %arrayIdx85, align 4, !dbg !78, !tbaa !27
  %arrayIdx87 = getelementptr inbounds float, float* %c, i64 3, !dbg !75
  %gepload88 = load float, float* %arrayIdx87, align 4, !dbg !75, !tbaa !27
  %arrayIdx89 = getelementptr inbounds float, float* %b, i64 3, !dbg !76
  %gepload90 = load float, float* %arrayIdx89, align 4, !dbg !76, !tbaa !27
  %55 = fadd fast float %gepload88, %gepload90, !dbg !77
  %arrayIdx91 = getelementptr inbounds float, float* %a, i64 3, !dbg !79
  store float %55, float* %arrayIdx91, align 4, !dbg !78, !tbaa !27
  %arrayIdx93 = getelementptr inbounds float, float* %c, i64 4, !dbg !75
  %gepload94 = load float, float* %arrayIdx93, align 4, !dbg !75, !tbaa !27
  %arrayIdx95 = getelementptr inbounds float, float* %b, i64 4, !dbg !76
  %gepload96 = load float, float* %arrayIdx95, align 4, !dbg !76, !tbaa !27
  %56 = fadd fast float %gepload94, %gepload96, !dbg !77
  %arrayIdx97 = getelementptr inbounds float, float* %a, i64 4, !dbg !79
  store float %56, float* %arrayIdx97, align 4, !dbg !78, !tbaa !27
  %arrayIdx99 = getelementptr inbounds float, float* %c, i64 5, !dbg !75
  %gepload100 = load float, float* %arrayIdx99, align 4, !dbg !75, !tbaa !27
  %arrayIdx101 = getelementptr inbounds float, float* %b, i64 5, !dbg !76
  %gepload102 = load float, float* %arrayIdx101, align 4, !dbg !76, !tbaa !27
  %57 = fadd fast float %gepload100, %gepload102, !dbg !77
  %arrayIdx103 = getelementptr inbounds float, float* %a, i64 5, !dbg !79
  store float %57, float* %arrayIdx103, align 4, !dbg !78, !tbaa !27
  %arrayIdx105 = getelementptr inbounds float, float* %c, i64 6, !dbg !75
  %gepload106 = load float, float* %arrayIdx105, align 4, !dbg !75, !tbaa !27
  %arrayIdx107 = getelementptr inbounds float, float* %b, i64 6, !dbg !76
  %gepload108 = load float, float* %arrayIdx107, align 4, !dbg !76, !tbaa !27
  %58 = fadd fast float %gepload106, %gepload108, !dbg !77
  %arrayIdx109 = getelementptr inbounds float, float* %a, i64 6, !dbg !79
  store float %58, float* %arrayIdx109, align 4, !dbg !78, !tbaa !27
  %arrayIdx111 = getelementptr inbounds float, float* %c, i64 7, !dbg !75
  %gepload112 = load float, float* %arrayIdx111, align 4, !dbg !75, !tbaa !27
  %arrayIdx113 = getelementptr inbounds float, float* %b, i64 7, !dbg !76
  %gepload114 = load float, float* %arrayIdx113, align 4, !dbg !76, !tbaa !27
  %59 = fadd fast float %gepload112, %gepload114, !dbg !77
  %arrayIdx115 = getelementptr inbounds float, float* %a, i64 7, !dbg !79
  store float %59, float* %arrayIdx115, align 4, !dbg !78, !tbaa !27
  ret void, !dbg !80
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #3 !dbg !81 {
entry:
  %a = alloca [100 x float], align 16
  %b = alloca [100 x float], align 16
  %c = alloca [100 x float], align 16
  %a1 = alloca [8 x float], align 16
  %b1 = alloca [8 x float], align 16
  %c1 = alloca [8 x float], align 16
  %0 = bitcast [100 x float]* %a to i8*, !dbg !82
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #4, !dbg !82
  %1 = bitcast [100 x float]* %b to i8*, !dbg !82
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %1) #4, !dbg !82
  %2 = bitcast [100 x float]* %c to i8*, !dbg !82
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %2) #4, !dbg !82
  %arraydecay = getelementptr inbounds [100 x float], [100 x float]* %a, i64 0, i64 0, !dbg !83
  %arraydecay1 = getelementptr inbounds [100 x float], [100 x float]* %b, i64 0, i64 0, !dbg !84
  %arraydecay2 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 0, !dbg !85
  call void @vectorize(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2), !dbg !86
  %3 = bitcast [8 x float]* %a1 to i8*, !dbg !87
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %3) #4, !dbg !87
  %4 = bitcast [8 x float]* %b1 to i8*, !dbg !87
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %4) #4, !dbg !87
  %5 = bitcast [8 x float]* %c1 to i8*, !dbg !87
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %5) #4, !dbg !87
  %arraydecay3 = getelementptr inbounds [8 x float], [8 x float]* %a1, i64 0, i64 0, !dbg !88
  %arraydecay4 = getelementptr inbounds [8 x float], [8 x float]* %b1, i64 0, i64 0, !dbg !89
  %arraydecay5 = getelementptr inbounds [8 x float], [8 x float]* %c1, i64 0, i64 0, !dbg !90
  call void @complete_unroll(float* nonnull %arraydecay3, float* nonnull %arraydecay4, float* nonnull %arraydecay5), !dbg !91
  call void @both(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2), !dbg !92
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %5) #4, !dbg !93
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %4) #4, !dbg !93
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %3) #4, !dbg !93
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %2) #4, !dbg !93
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %1) #4, !dbg !93
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #4, !dbg !93
  ret i32 0, !dbg !94
}

attributes #0 = { noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nofree noinline norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test3.c", directory: "/iusers/karthik1/tools/binoptrpt_reader/tests")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!6 = distinct !DISubprogram(name: "vectorize", scope: !1, file: !1, line: 3, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
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
!37 = distinct !DISubprogram(name: "complete_unroll", scope: !1, file: !1, line: 9, type: !7, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!38 = distinct !{!"intel.optreport.rootnode", !39}
!39 = distinct !{!"intel.optreport", !40}
!40 = !{!"intel.optreport.first_child", !41}
!41 = distinct !{!"intel.optreport.rootnode", !42}
!42 = distinct !{!"intel.optreport", !43, !45}
!43 = !{!"intel.optreport.debug_location", !44}
!44 = !DILocation(line: 11, column: 3, scope: !37)
!45 = !{!"intel.optreport.remarks", !46}
!46 = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 4}
!47 = !DILocation(line: 12, column: 19, scope: !37)
!48 = !DILocation(line: 12, column: 12, scope: !37)
!49 = !DILocation(line: 12, column: 17, scope: !37)
!50 = !DILocation(line: 12, column: 10, scope: !37)
!51 = !DILocation(line: 12, column: 5, scope: !37)
!52 = !DILocation(line: 13, column: 1, scope: !37)
!53 = distinct !DISubprogram(name: "both", scope: !1, file: !1, line: 15, type: !7, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!54 = distinct !{!"intel.optreport.rootnode", !55}
!55 = distinct !{!"intel.optreport", !56}
!56 = !{!"intel.optreport.first_child", !57}
!57 = distinct !{!"intel.optreport.rootnode", !58}
!58 = distinct !{!"intel.optreport", !59, !45, !61}
!59 = !{!"intel.optreport.debug_location", !60}
!60 = !DILocation(line: 21, column: 3, scope: !53)
!61 = !{!"intel.optreport.next_sibling", !62}
!62 = distinct !{!"intel.optreport.rootnode", !63}
!63 = distinct !{!"intel.optreport", !64, !15, !66}
!64 = !{!"intel.optreport.debug_location", !65}
!65 = !DILocation(line: 16, column: 1, scope: !53)
!66 = !{!"intel.optreport.next_sibling", !67}
!67 = distinct !{!"intel.optreport.rootnode", !68}
!68 = distinct !{!"intel.optreport", !64, !22, !24}
!69 = !DILocation(line: 18, column: 12, scope: !53)
!70 = !DILocation(line: 18, column: 19, scope: !53)
!71 = !DILocation(line: 18, column: 17, scope: !53)
!72 = !DILocation(line: 18, column: 5, scope: !53)
!73 = !DILocation(line: 18, column: 10, scope: !53)
!74 = distinct !{}
!75 = !DILocation(line: 22, column: 19, scope: !53)
!76 = !DILocation(line: 22, column: 12, scope: !53)
!77 = !DILocation(line: 22, column: 17, scope: !53)
!78 = !DILocation(line: 22, column: 10, scope: !53)
!79 = !DILocation(line: 22, column: 5, scope: !53)
!80 = !DILocation(line: 23, column: 1, scope: !53)
!81 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 25, type: !7, scopeLine: 25, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!82 = !DILocation(line: 26, column: 3, scope: !81)
!83 = !DILocation(line: 27, column: 13, scope: !81)
!84 = !DILocation(line: 27, column: 16, scope: !81)
!85 = !DILocation(line: 27, column: 19, scope: !81)
!86 = !DILocation(line: 27, column: 3, scope: !81)
!87 = !DILocation(line: 28, column: 3, scope: !81)
!88 = !DILocation(line: 29, column: 19, scope: !81)
!89 = !DILocation(line: 29, column: 23, scope: !81)
!90 = !DILocation(line: 29, column: 27, scope: !81)
!91 = !DILocation(line: 29, column: 3, scope: !81)
!92 = !DILocation(line: 30, column: 3, scope: !81)
!93 = !DILocation(line: 32, column: 1, scope: !81)
!94 = !DILocation(line: 31, column: 3, scope: !81)
