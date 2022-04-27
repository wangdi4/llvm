; REQUIRES: proto_bor
; RUN: llc < %s -O3 -intel-opt-report=high -opt-report-embed -enable-protobuf-opt-report --filetype=obj -o %t1.o
; RUN: llc < %p/Inputs/non_lto_linker_test_sub.ll -O3 -intel-opt-report=high -opt-report-embed -enable-protobuf-opt-report --filetype=obj -o %t2.o
; RUN: ld.lld -e main %t1.o %t2.o -o %t.o
; RUN: intel-bin-opt-report %t.o | FileCheck %s

; Check reader tool's correctness for a scenario when linker is used in non-LTO
; mode to produce object file stitched together with 2 separate object files.

; === main.c ===
; __attribute__((noinline)) void both(float *a, float *b, float *c) {
;   ...
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

; === sub.c ===
; __attribute__((noinline)) void vectorize(float *a, float *b, float *c) {
;   ...
; }
;
; __attribute__((noinline)) void complete_unroll(float *a, float *b, float *c) {
;   ...
; }

; CHECK:      --- Start Intel Binary Optimization Report ---
; CHECK-NEXT: Version: 1.5
; CHECK-NEXT: Property Message Map:
; CHECK-DAG:    C_LOOP_COMPLETE_UNROLL_FACTOR --> Loop completely unrolled by %d
; CHECK-DAG:    C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; CHECK-DAG:    C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-NEXT: Number of reports: 3

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 744b9d238efbcaee987c8f04cc652df3
; CHECK-DAG:  Number of remarks: 0
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 0c5aafd7631f98d9ec45b3f7255d3ff5
; CHECK-DAG:  Number of remarks: 1
; CHECK-DAG:    Property: C_LOOP_COMPLETE_UNROLL_FACTOR, Remark ID: 25436, Remark Args: 4
; CHECK-DAG:  ==== Loop End ====

; CHECK-DAG:  === Loop Begin ===
; CHECK-DAG:  Anchor ID: 220a1a20df64c72b99cf5a01c42bbc3c
; CHECK-DAG:  Number of remarks: 2
; CHECK-DAG:    Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; CHECK-DAG:    Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 8
; CHECK-DAG:  ==== Loop End ====

; CHECK:      --- End Intel Binary Optimization Report ---


; CHECK:      --- Start Intel Binary Optimization Report ---
; CHECK-NEXT: Version: 1.5
; CHECK-NEXT: Property Message Map:
; CHECK-DAG:    C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; CHECK-DAG:    C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-DAG:    C_LOOP_COMPLETE_UNROLL_FACTOR --> Loop completely unrolled by %d
; CHECK-NEXT: Number of reports: 3

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

; CHECK:      --- End Intel Binary Optimization Report ---

; ModuleID = 'linker_test_main.c'
source_filename = "linker_test_main.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @both(float* nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c) local_unnamed_addr #0 !dbg !7 !intel.optreport.rootnode !9 {
DIR.OMP.SIMD.139:
  %0 = bitcast float* %b to <8 x float>*, !dbg !34
  %gepload = load <8 x float>, <8 x float>* %0, align 4, !dbg !34, !tbaa !35
  %1 = bitcast float* %c to <8 x float>*, !dbg !39
  %gepload45 = load <8 x float>, <8 x float>* %1, align 4, !dbg !39, !tbaa !35
  %2 = fadd fast <8 x float> %gepload45, %gepload, !dbg !40
  %3 = bitcast float* %a to <8 x float>*, !dbg !41
  store <8 x float> %2, <8 x float>* %3, align 4, !dbg !42, !tbaa !35
  %4 = getelementptr inbounds float, float* %b, i64 8, !dbg !34
  %5 = bitcast float* %4 to <8 x float>*, !dbg !34
  %gepload.1 = load <8 x float>, <8 x float>* %5, align 4, !dbg !34, !tbaa !35
  %6 = getelementptr inbounds float, float* %c, i64 8, !dbg !39
  %7 = bitcast float* %6 to <8 x float>*, !dbg !39
  %gepload45.1 = load <8 x float>, <8 x float>* %7, align 4, !dbg !39, !tbaa !35
  %8 = fadd fast <8 x float> %gepload45.1, %gepload.1, !dbg !40
  %9 = getelementptr inbounds float, float* %a, i64 8, !dbg !41
  %10 = bitcast float* %9 to <8 x float>*, !dbg !41
  store <8 x float> %8, <8 x float>* %10, align 4, !dbg !42, !tbaa !35
  %11 = getelementptr inbounds float, float* %b, i64 16, !dbg !34
  %12 = bitcast float* %11 to <8 x float>*, !dbg !34
  %gepload.2 = load <8 x float>, <8 x float>* %12, align 4, !dbg !34, !tbaa !35
  %13 = getelementptr inbounds float, float* %c, i64 16, !dbg !39
  %14 = bitcast float* %13 to <8 x float>*, !dbg !39
  %gepload45.2 = load <8 x float>, <8 x float>* %14, align 4, !dbg !39, !tbaa !35
  %15 = fadd fast <8 x float> %gepload45.2, %gepload.2, !dbg !40
  %16 = getelementptr inbounds float, float* %a, i64 16, !dbg !41
  %17 = bitcast float* %16 to <8 x float>*, !dbg !41
  store <8 x float> %15, <8 x float>* %17, align 4, !dbg !42, !tbaa !35
  %18 = getelementptr inbounds float, float* %b, i64 24, !dbg !34
  %19 = bitcast float* %18 to <8 x float>*, !dbg !34
  %gepload.3 = load <8 x float>, <8 x float>* %19, align 4, !dbg !34, !tbaa !35
  %20 = getelementptr inbounds float, float* %c, i64 24, !dbg !39
  %21 = bitcast float* %20 to <8 x float>*, !dbg !39
  %gepload45.3 = load <8 x float>, <8 x float>* %21, align 4, !dbg !39, !tbaa !35
  %22 = fadd fast <8 x float> %gepload45.3, %gepload.3, !dbg !40
  %23 = getelementptr inbounds float, float* %a, i64 24, !dbg !41
  %24 = bitcast float* %23 to <8 x float>*, !dbg !41
  store <8 x float> %22, <8 x float>* %24, align 4, !dbg !42, !tbaa !35
  %25 = getelementptr inbounds float, float* %b, i64 32, !dbg !34
  %26 = bitcast float* %25 to <8 x float>*, !dbg !34
  %gepload.4 = load <8 x float>, <8 x float>* %26, align 4, !dbg !34, !tbaa !35
  %27 = getelementptr inbounds float, float* %c, i64 32, !dbg !39
  %28 = bitcast float* %27 to <8 x float>*, !dbg !39
  %gepload45.4 = load <8 x float>, <8 x float>* %28, align 4, !dbg !39, !tbaa !35
  %29 = fadd fast <8 x float> %gepload45.4, %gepload.4, !dbg !40
  %30 = getelementptr inbounds float, float* %a, i64 32, !dbg !41
  %31 = bitcast float* %30 to <8 x float>*, !dbg !41
  store <8 x float> %29, <8 x float>* %31, align 4, !dbg !42, !tbaa !35
  %32 = getelementptr inbounds float, float* %b, i64 40, !dbg !34
  %33 = bitcast float* %32 to <8 x float>*, !dbg !34
  %gepload.5 = load <8 x float>, <8 x float>* %33, align 4, !dbg !34, !tbaa !35
  %34 = getelementptr inbounds float, float* %c, i64 40, !dbg !39
  %35 = bitcast float* %34 to <8 x float>*, !dbg !39
  %gepload45.5 = load <8 x float>, <8 x float>* %35, align 4, !dbg !39, !tbaa !35
  %36 = fadd fast <8 x float> %gepload45.5, %gepload.5, !dbg !40
  %37 = getelementptr inbounds float, float* %a, i64 40, !dbg !41
  %38 = bitcast float* %37 to <8 x float>*, !dbg !41
  store <8 x float> %36, <8 x float>* %38, align 4, !dbg !42, !tbaa !35
  %39 = getelementptr inbounds float, float* %b, i64 48, !dbg !34
  %40 = bitcast float* %39 to <8 x float>*, !dbg !34
  %gepload.6 = load <8 x float>, <8 x float>* %40, align 4, !dbg !34, !tbaa !35
  %41 = getelementptr inbounds float, float* %c, i64 48, !dbg !39
  %42 = bitcast float* %41 to <8 x float>*, !dbg !39
  %gepload45.6 = load <8 x float>, <8 x float>* %42, align 4, !dbg !39, !tbaa !35
  %43 = fadd fast <8 x float> %gepload45.6, %gepload.6, !dbg !40
  %44 = getelementptr inbounds float, float* %a, i64 48, !dbg !41
  %45 = bitcast float* %44 to <8 x float>*, !dbg !41
  store <8 x float> %43, <8 x float>* %45, align 4, !dbg !42, !tbaa !35
  %46 = getelementptr inbounds float, float* %b, i64 56, !dbg !34
  %47 = bitcast float* %46 to <8 x float>*, !dbg !34
  %gepload.7 = load <8 x float>, <8 x float>* %47, align 4, !dbg !34, !tbaa !35
  %48 = getelementptr inbounds float, float* %c, i64 56, !dbg !39
  %49 = bitcast float* %48 to <8 x float>*, !dbg !39
  %gepload45.7 = load <8 x float>, <8 x float>* %49, align 4, !dbg !39, !tbaa !35
  %50 = fadd fast <8 x float> %gepload45.7, %gepload.7, !dbg !40
  %51 = getelementptr inbounds float, float* %a, i64 56, !dbg !41
  %52 = bitcast float* %51 to <8 x float>*, !dbg !41
  store <8 x float> %50, <8 x float>* %52, align 4, !dbg !42, !tbaa !35
  %53 = getelementptr inbounds float, float* %b, i64 64, !dbg !34
  %54 = bitcast float* %53 to <8 x float>*, !dbg !34
  %gepload.8 = load <8 x float>, <8 x float>* %54, align 4, !dbg !34, !tbaa !35
  %55 = getelementptr inbounds float, float* %c, i64 64, !dbg !39
  %56 = bitcast float* %55 to <8 x float>*, !dbg !39
  %gepload45.8 = load <8 x float>, <8 x float>* %56, align 4, !dbg !39, !tbaa !35
  %57 = fadd fast <8 x float> %gepload45.8, %gepload.8, !dbg !40
  %58 = getelementptr inbounds float, float* %a, i64 64, !dbg !41
  %59 = bitcast float* %58 to <8 x float>*, !dbg !41
  store <8 x float> %57, <8 x float>* %59, align 4, !dbg !42, !tbaa !35
  %60 = getelementptr inbounds float, float* %b, i64 72, !dbg !34
  %61 = bitcast float* %60 to <8 x float>*, !dbg !34
  %gepload.9 = load <8 x float>, <8 x float>* %61, align 4, !dbg !34, !tbaa !35
  %62 = getelementptr inbounds float, float* %c, i64 72, !dbg !39
  %63 = bitcast float* %62 to <8 x float>*, !dbg !39
  %gepload45.9 = load <8 x float>, <8 x float>* %63, align 4, !dbg !39, !tbaa !35
  %64 = fadd fast <8 x float> %gepload45.9, %gepload.9, !dbg !40
  %65 = getelementptr inbounds float, float* %a, i64 72, !dbg !41
  %66 = bitcast float* %65 to <8 x float>*, !dbg !41
  store <8 x float> %64, <8 x float>* %66, align 4, !dbg !42, !tbaa !35
  %67 = getelementptr inbounds float, float* %b, i64 80, !dbg !34
  %68 = bitcast float* %67 to <8 x float>*, !dbg !34
  %gepload.10 = load <8 x float>, <8 x float>* %68, align 4, !dbg !34, !tbaa !35
  %69 = getelementptr inbounds float, float* %c, i64 80, !dbg !39
  %70 = bitcast float* %69 to <8 x float>*, !dbg !39
  %gepload45.10 = load <8 x float>, <8 x float>* %70, align 4, !dbg !39, !tbaa !35
  %71 = fadd fast <8 x float> %gepload45.10, %gepload.10, !dbg !40
  %72 = getelementptr inbounds float, float* %a, i64 80, !dbg !41
  %73 = bitcast float* %72 to <8 x float>*, !dbg !41
  store <8 x float> %71, <8 x float>* %73, align 4, !dbg !42, !tbaa !35
  %74 = getelementptr inbounds float, float* %b, i64 88, !dbg !34
  %75 = bitcast float* %74 to <8 x float>*, !dbg !34
  %gepload.11 = load <8 x float>, <8 x float>* %75, align 4, !dbg !34, !tbaa !35
  %76 = getelementptr inbounds float, float* %c, i64 88, !dbg !39
  %77 = bitcast float* %76 to <8 x float>*, !dbg !39
  %gepload45.11 = load <8 x float>, <8 x float>* %77, align 4, !dbg !39, !tbaa !35
  %78 = fadd fast <8 x float> %gepload45.11, %gepload.11, !dbg !40
  %79 = getelementptr inbounds float, float* %a, i64 88, !dbg !41
  %80 = bitcast float* %79 to <8 x float>*, !dbg !41
  store <8 x float> %78, <8 x float>* %80, align 4, !dbg !42, !tbaa !35
  %81 = getelementptr inbounds float, float* %b, i64 96, !dbg !34
  %gepload50 = load float, float* %81, align 4, !dbg !34, !tbaa !35, !llvm.access.group !43
  %82 = getelementptr inbounds float, float* %c, i64 96, !dbg !39
  %gepload51 = load float, float* %82, align 4, !dbg !39, !tbaa !35, !llvm.access.group !43
  %83 = fadd fast float %gepload51, %gepload50, !dbg !40
  %84 = getelementptr inbounds float, float* %a, i64 96, !dbg !41
  store float %83, float* %84, align 4, !dbg !42, !tbaa !35, !llvm.access.group !43
  %85 = getelementptr inbounds float, float* %b, i64 97, !dbg !34
  %gepload50.1 = load float, float* %85, align 4, !dbg !34, !tbaa !35, !llvm.access.group !43
  %86 = getelementptr inbounds float, float* %c, i64 97, !dbg !39
  %gepload51.1 = load float, float* %86, align 4, !dbg !39, !tbaa !35, !llvm.access.group !43
  %87 = fadd fast float %gepload51.1, %gepload50.1, !dbg !40
  %88 = getelementptr inbounds float, float* %a, i64 97, !dbg !41
  store float %87, float* %88, align 4, !dbg !42, !tbaa !35, !llvm.access.group !43
  %89 = getelementptr inbounds float, float* %b, i64 98, !dbg !34
  %gepload50.2 = load float, float* %89, align 4, !dbg !34, !tbaa !35, !llvm.access.group !43
  %90 = getelementptr inbounds float, float* %c, i64 98, !dbg !39
  %gepload51.2 = load float, float* %90, align 4, !dbg !39, !tbaa !35, !llvm.access.group !43
  %91 = fadd fast float %gepload51.2, %gepload50.2, !dbg !40
  %92 = getelementptr inbounds float, float* %a, i64 98, !dbg !41
  store float %91, float* %92, align 4, !dbg !42, !tbaa !35, !llvm.access.group !43
  %93 = getelementptr inbounds float, float* %b, i64 99, !dbg !34
  %gepload50.3 = load float, float* %93, align 4, !dbg !34, !tbaa !35, !llvm.access.group !43
  %94 = getelementptr inbounds float, float* %c, i64 99, !dbg !39
  %gepload51.3 = load float, float* %94, align 4, !dbg !39, !tbaa !35, !llvm.access.group !43
  %95 = fadd fast float %gepload51.3, %gepload50.3, !dbg !40
  %96 = getelementptr inbounds float, float* %a, i64 99, !dbg !41
  store float %95, float* %96, align 4, !dbg !42, !tbaa !35, !llvm.access.group !43
  %gepload53 = load float, float* %c, align 4, !dbg !44, !tbaa !35
  %gepload54 = load float, float* %b, align 4, !dbg !45, !tbaa !35
  %97 = fadd fast float %gepload53, %gepload54, !dbg !46
  store float %97, float* %a, align 4, !dbg !47, !tbaa !35
  %98 = getelementptr inbounds float, float* %c, i64 1, !dbg !44
  %gepload55 = load float, float* %98, align 4, !dbg !44, !tbaa !35
  %99 = getelementptr inbounds float, float* %b, i64 1, !dbg !45
  %gepload56 = load float, float* %99, align 4, !dbg !45, !tbaa !35
  %100 = fadd fast float %gepload55, %gepload56, !dbg !46
  %101 = getelementptr inbounds float, float* %a, i64 1, !dbg !48
  store float %100, float* %101, align 4, !dbg !47, !tbaa !35
  %102 = getelementptr inbounds float, float* %c, i64 2, !dbg !44
  %gepload58 = load float, float* %102, align 4, !dbg !44, !tbaa !35
  %103 = getelementptr inbounds float, float* %b, i64 2, !dbg !45
  %gepload59 = load float, float* %103, align 4, !dbg !45, !tbaa !35
  %104 = fadd fast float %gepload58, %gepload59, !dbg !46
  %105 = getelementptr inbounds float, float* %a, i64 2, !dbg !48
  store float %104, float* %105, align 4, !dbg !47, !tbaa !35
  %106 = getelementptr inbounds float, float* %c, i64 3, !dbg !44
  %gepload61 = load float, float* %106, align 4, !dbg !44, !tbaa !35
  %107 = getelementptr inbounds float, float* %b, i64 3, !dbg !45
  %gepload62 = load float, float* %107, align 4, !dbg !45, !tbaa !35
  %108 = fadd fast float %gepload61, %gepload62, !dbg !46
  %109 = getelementptr inbounds float, float* %a, i64 3, !dbg !48
  store float %108, float* %109, align 4, !dbg !47, !tbaa !35
  %110 = getelementptr inbounds float, float* %c, i64 4, !dbg !44
  %gepload64 = load float, float* %110, align 4, !dbg !44, !tbaa !35
  %111 = getelementptr inbounds float, float* %b, i64 4, !dbg !45
  %gepload65 = load float, float* %111, align 4, !dbg !45, !tbaa !35
  %112 = fadd fast float %gepload64, %gepload65, !dbg !46
  %113 = getelementptr inbounds float, float* %a, i64 4, !dbg !48
  store float %112, float* %113, align 4, !dbg !47, !tbaa !35
  %114 = getelementptr inbounds float, float* %c, i64 5, !dbg !44
  %gepload67 = load float, float* %114, align 4, !dbg !44, !tbaa !35
  %115 = getelementptr inbounds float, float* %b, i64 5, !dbg !45
  %gepload68 = load float, float* %115, align 4, !dbg !45, !tbaa !35
  %116 = fadd fast float %gepload67, %gepload68, !dbg !46
  %117 = getelementptr inbounds float, float* %a, i64 5, !dbg !48
  store float %116, float* %117, align 4, !dbg !47, !tbaa !35
  %118 = getelementptr inbounds float, float* %c, i64 6, !dbg !44
  %gepload70 = load float, float* %118, align 4, !dbg !44, !tbaa !35
  %119 = getelementptr inbounds float, float* %b, i64 6, !dbg !45
  %gepload71 = load float, float* %119, align 4, !dbg !45, !tbaa !35
  %120 = fadd fast float %gepload70, %gepload71, !dbg !46
  %121 = getelementptr inbounds float, float* %a, i64 6, !dbg !48
  store float %120, float* %121, align 4, !dbg !47, !tbaa !35
  %122 = getelementptr inbounds float, float* %c, i64 7, !dbg !44
  %gepload73 = load float, float* %122, align 4, !dbg !44, !tbaa !35
  %123 = getelementptr inbounds float, float* %b, i64 7, !dbg !45
  %gepload74 = load float, float* %123, align 4, !dbg !45, !tbaa !35
  %124 = fadd fast float %gepload73, %gepload74, !dbg !46
  %125 = getelementptr inbounds float, float* %a, i64 7, !dbg !48
  store float %124, float* %125, align 4, !dbg !47, !tbaa !35
  ret void, !dbg !49
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 !dbg !50 {
entry:
  %a = alloca [100 x float], align 16
  %b = alloca [100 x float], align 16
  %c = alloca [100 x float], align 16
  %a1 = alloca [8 x float], align 16
  %b1 = alloca [8 x float], align 16
  %c1 = alloca [8 x float], align 16
  %0 = bitcast [100 x float]* %a to i8*, !dbg !51
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #4, !dbg !51
  %1 = bitcast [100 x float]* %b to i8*, !dbg !51
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %1) #4, !dbg !51
  %2 = bitcast [100 x float]* %c to i8*, !dbg !51
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %2) #4, !dbg !51
  %arraydecay = getelementptr inbounds [100 x float], [100 x float]* %a, i64 0, i64 0, !dbg !52
  %arraydecay1 = getelementptr inbounds [100 x float], [100 x float]* %b, i64 0, i64 0, !dbg !53
  %arraydecay2 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 0, !dbg !54
  %call = call i32 (float*, float*, float*, ...) bitcast (i32 (...)* @vectorize to i32 (float*, float*, float*, ...)*)(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2) #4, !dbg !55
  %3 = bitcast [8 x float]* %a1 to i8*, !dbg !56
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %3) #4, !dbg !56
  %4 = bitcast [8 x float]* %b1 to i8*, !dbg !56
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %4) #4, !dbg !56
  %5 = bitcast [8 x float]* %c1 to i8*, !dbg !56
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %5) #4, !dbg !56
  %arraydecay3 = getelementptr inbounds [8 x float], [8 x float]* %a1, i64 0, i64 0, !dbg !57
  %arraydecay4 = getelementptr inbounds [8 x float], [8 x float]* %b1, i64 0, i64 0, !dbg !58
  %arraydecay5 = getelementptr inbounds [8 x float], [8 x float]* %c1, i64 0, i64 0, !dbg !59
  %call6 = call i32 (float*, float*, float*, ...) bitcast (i32 (...)* @complete_unroll to i32 (float*, float*, float*, ...)*)(float* nonnull %arraydecay3, float* nonnull %arraydecay4, float* nonnull %arraydecay5) #4, !dbg !60
  call void @both(float* nonnull %arraydecay, float* nonnull %arraydecay1, float* nonnull %arraydecay2), !dbg !61
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %5) #4, !dbg !62
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %4) #4, !dbg !62
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %3) #4, !dbg !62
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %2) #4, !dbg !62
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %1) #4, !dbg !62
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #4, !dbg !62
  ret i32 0, !dbg !63
}

declare dso_local i32 @vectorize(...) local_unnamed_addr #3

declare dso_local i32 @complete_unroll(...) local_unnamed_addr #3

attributes #0 = { noinline nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #3 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "linker_test_main.c", directory: "/iusers/karthik1/tools/binoptrpt_reader/tests")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!7 = distinct !DISubprogram(name: "both", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = distinct !{!"intel.optreport.rootnode", !10}
!10 = distinct !{!"intel.optreport", !11}
!11 = !{!"intel.optreport.first_child", !12}
!12 = distinct !{!"intel.optreport.rootnode", !13}
!13 = distinct !{!"intel.optreport", !14, !16, !18}
!14 = !{!"intel.optreport.debug_location", !15}
!15 = !DILocation(line: 7, column: 3, scope: !7)
!16 = !{!"intel.optreport.remarks", !17}
!17 = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 4}
!18 = !{!"intel.optreport.next_sibling", !19}
!19 = distinct !{!"intel.optreport.rootnode", !20}
!20 = distinct !{!"intel.optreport", !21, !23, !27}
!21 = !{!"intel.optreport.debug_location", !22}
!22 = !DILocation(line: 2, column: 1, scope: !7)
!23 = !{!"intel.optreport.remarks", !24, !25, !26}
!24 = !{!"intel.optreport.remark", i32 15300, !"LOOP WAS VECTORIZED"}
!25 = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", !"8"}
!26 = !{!"intel.optreport.remark", i32 0, !"LLorg: Loop has been completely unrolled"}
!27 = !{!"intel.optreport.next_sibling", !28}
!28 = distinct !{!"intel.optreport.rootnode", !29}
!29 = distinct !{!"intel.optreport", !21, !30, !32}
!30 = !{!"intel.optreport.origin", !31}
!31 = !{!"intel.optreport.remark", i32 0, !"Remainder loop for vectorization"}
!32 = !{!"intel.optreport.remarks", !33, !26}
!33 = !{!"intel.optreport.remark", i32 15441, !"remainder loop was not vectorized: %s ", !""}
!34 = !DILocation(line: 4, column: 12, scope: !7)
!35 = !{!36, !36, i64 0}
!36 = !{!"float", !37, i64 0}
!37 = !{!"omnipotent char", !38, i64 0}
!38 = !{!"Simple C/C++ TBAA"}
!39 = !DILocation(line: 4, column: 19, scope: !7)
!40 = !DILocation(line: 4, column: 17, scope: !7)
!41 = !DILocation(line: 4, column: 5, scope: !7)
!42 = !DILocation(line: 4, column: 10, scope: !7)
!43 = distinct !{}
!44 = !DILocation(line: 8, column: 19, scope: !7)
!45 = !DILocation(line: 8, column: 12, scope: !7)
!46 = !DILocation(line: 8, column: 17, scope: !7)
!47 = !DILocation(line: 8, column: 10, scope: !7)
!48 = !DILocation(line: 8, column: 5, scope: !7)
!49 = !DILocation(line: 9, column: 1, scope: !7)
!50 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !8, scopeLine: 11, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!51 = !DILocation(line: 12, column: 3, scope: !50)
!52 = !DILocation(line: 13, column: 13, scope: !50)
!53 = !DILocation(line: 13, column: 16, scope: !50)
!54 = !DILocation(line: 13, column: 19, scope: !50)
!55 = !DILocation(line: 13, column: 3, scope: !50)
!56 = !DILocation(line: 14, column: 3, scope: !50)
!57 = !DILocation(line: 15, column: 19, scope: !50)
!58 = !DILocation(line: 15, column: 23, scope: !50)
!59 = !DILocation(line: 15, column: 27, scope: !50)
!60 = !DILocation(line: 15, column: 3, scope: !50)
!61 = !DILocation(line: 16, column: 3, scope: !50)
!62 = !DILocation(line: 18, column: 1, scope: !50)
!63 = !DILocation(line: 17, column: 3, scope: !50)
