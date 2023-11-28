; REQUIRES: asserts
; RUN: opt < %s -passes='deaddopevectorelimination' --debug-only=deaddopevectorelimination -whole-program-assume -disable-output 2>&1 | FileCheck %s
; 
; The test was inherited from "tc -x efi2_linux64_xmain_fort -t parsuiteiF/arraystruct -l opt_O3_ipo_openmp_zmm_xH"
; This test checks that there is no infinite recursion in dead dope vector elimination analysis
; which leats to stack overflow.
;
; CHECK: Start DeadDopeVectorElimination

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"DIFFICULTSTRUCT$.btLAYERING_TYPE" = type <{ i32, [200 x i32], [300 x float] }>

@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata"
@difficultstruct_mp_layering_ = internal global %"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@strlit = internal unnamed_addr constant [22 x i8] c"passed arraystruct.f90"
@strlit.1 = internal unnamed_addr constant [37 x i8] c"2-failed arraystruct.f90 :: someReals"
@strlit.2 = internal unnamed_addr constant [36 x i8] c"2-failed arraystruct.f90 :: someInts"
@strlit.3 = internal unnamed_addr constant [30 x i8] c"2-failed arraystruct.f90 :: id"
@strlit.4 = internal unnamed_addr constant [37 x i8] c"1-failed arraystruct.f90 :: someReals"
@strlit.5 = internal unnamed_addr constant [36 x i8] c"1-failed arraystruct.f90 :: someInts"
@strlit.6 = internal unnamed_addr constant [30 x i8] c"1-failed arraystruct.f90 :: id"
@anon.3aa04fbdb729f5e459cf2463edda4fdd.0 = internal unnamed_addr constant i32 65536, align 4
@anon.3aa04fbdb729f5e459cf2463edda4fdd.1 = internal unnamed_addr constant i32 2, align 4
@strlit.12 = internal unnamed_addr constant [0 x i8] zeroinitializer

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #1 {
  %1 = alloca [8 x i64], align 16, !llfort.type_idx !4
  %2 = alloca [4 x i8], align 1, !llfort.type_idx !5
  %3 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %4 = alloca [4 x i8], align 1, !llfort.type_idx !7
  %5 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %6 = alloca [4 x i8], align 1, !llfort.type_idx !9
  %7 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %8 = alloca [4 x i8], align 1, !llfort.type_idx !10
  %9 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %10 = alloca [4 x i8], align 1, !llfort.type_idx !11
  %11 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %12 = alloca [4 x i8], align 1, !llfort.type_idx !12
  %13 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %14 = alloca [4 x i8], align 1, !llfort.type_idx !13
  %15 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %16 = alloca [4 x i8], align 1, !llfort.type_idx !14
  %17 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %18 = alloca [4 x i8], align 1, !llfort.type_idx !15
  %19 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %20 = alloca [4 x i8], align 1, !llfort.type_idx !16
  %21 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %22 = alloca [4 x i8], align 1, !llfort.type_idx !17
  %23 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %24 = alloca [4 x i8], align 1, !llfort.type_idx !18
  %25 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %26 = alloca [4 x i8], align 1, !llfort.type_idx !19
  %27 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %28 = alloca [4 x i8], align 1, !llfort.type_idx !20
  %29 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %30 = alloca [4 x i8], align 1, !llfort.type_idx !21
  %31 = alloca <{ i64, ptr }>, align 8, !llfort.type_idx !6
  %32 = tail call i32 @for_set_fpe_(ptr nonnull @anon.3aa04fbdb729f5e459cf2463edda4fdd.0) #4, !llfort.type_idx !22
  %33 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.3aa04fbdb729f5e459cf2463edda4fdd.1) #4, !llfort.type_idx !22
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 5), align 8, !tbaa !23
  %34 = load i64, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 3), align 8, !tbaa !29, !llfort.type_idx !30
  %35 = and i64 %34, -68451041281
  %36 = or i64 %35, 1073741824
  store i64 %36, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 3), align 8, !tbaa !29
  %37 = trunc i64 %34 to i32
  %38 = shl i32 %37, 1
  %39 = and i32 %38, 2
  %40 = lshr i64 %34, 15
  %41 = trunc i64 %40 to i32
  %42 = and i32 %41, 65011712
  %43 = or i32 %42, %39
  %44 = or i32 %43, 262145
  %45 = tail call i32 @for_alloc_allocatable_handle(i64 200400, ptr nonnull @difficultstruct_mp_layering_, i32 %44, ptr null) #4, !llfort.type_idx !22
  %46 = icmp eq i32 %45, 0
  br i1 %46, label %49, label %47

47:                                               ; preds = %0
  %48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 6, i64 0, i32 2), i32 0), !llfort.type_idx !31
  br label %56

49:                                               ; preds = %0
  %50 = load i64, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 3), align 8, !tbaa !29, !llfort.type_idx !30
  %51 = and i64 %50, 1030792151296
  %52 = or i64 %51, 133
  store i64 %52, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 3), align 8, !tbaa !29
  store i64 2004, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 1), align 8, !tbaa !32
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 4), align 16, !tbaa !33
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 2), align 16, !tbaa !34
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %53, align 1, !tbaa !35
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 6, i64 0), i32 0), !llfort.type_idx !36
  store i64 100, ptr %54, align 1, !tbaa !37
  %55 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$", ptr @difficultstruct_mp_layering_, i64 0, i32 6, i64 0, i32 1), i32 0), !llfort.type_idx !38
  store i64 2004, ptr %55, align 1, !tbaa !39
  br label %56

56:                                               ; preds = %49, %47
  %57 = phi ptr [ %48, %47 ], [ %53, %49 ]
  %58 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !40, !llfort.type_idx !46
  %59 = load i64, ptr %57, align 1, !tbaa !47, !llfort.type_idx !31
  br label %60

60:                                               ; preds = %77, %56
  %61 = phi i64 [ %78, %77 ], [ 1, %56 ]
  %62 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %59, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %58, i64 %61), !llfort.type_idx !46
  %63 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %62, i64 0, i32 0, !llfort.type_idx !48
  store i32 0, ptr %63, align 1, !tbaa !49
  %64 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %62, i64 0, i32 1, i64 0, !llfort.type_idx !52
  br label %65

65:                                               ; preds = %65, %60
  %66 = phi i64 [ %68, %65 ], [ 1, %60 ]
  %67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %64, i64 %66), !llfort.type_idx !22
  store i32 0, ptr %67, align 1, !tbaa !53
  %68 = add nuw nsw i64 %66, 1
  %69 = icmp eq i64 %68, 201
  br i1 %69, label %70, label %65

70:                                               ; preds = %65
  %71 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %62, i64 0, i32 2, i64 0, !llfort.type_idx !55
  br label %72

72:                                               ; preds = %72, %70
  %73 = phi i64 [ 1, %70 ], [ %75, %72 ]
  %74 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %71, i64 %73), !llfort.type_idx !56
  store float 0.000000e+00, ptr %74, align 1, !tbaa !57
  %75 = add nuw nsw i64 %73, 1
  %76 = icmp eq i64 %75, 301
  br i1 %76, label %77, label %72

77:                                               ; preds = %72
  %78 = add nuw nsw i64 %61, 1
  %79 = icmp eq i64 %78, 101
  br i1 %79, label %80, label %60

80:                                               ; preds = %77
  %81 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 0
  %82 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 1
  %83 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 2
  %84 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 3
  %85 = getelementptr inbounds <{ i64, ptr }>, ptr %3, i64 0, i32 0
  %86 = getelementptr inbounds <{ i64, ptr }>, ptr %3, i64 0, i32 1
  %87 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 0
  %88 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 1
  %89 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 2
  %90 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 3
  %91 = getelementptr inbounds <{ i64 }>, ptr %5, i64 0, i32 0
  %92 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 0
  %93 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 1
  %94 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 2
  %95 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 3
  %96 = getelementptr inbounds <{ i64 }>, ptr %7, i64 0, i32 0
  %97 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 0
  %98 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 1
  %99 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 2
  %100 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 3
  %101 = getelementptr inbounds <{ i64, ptr }>, ptr %9, i64 0, i32 0
  %102 = getelementptr inbounds <{ i64, ptr }>, ptr %9, i64 0, i32 1
  %103 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 0
  %104 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 1
  %105 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 2
  %106 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 3
  %107 = getelementptr inbounds <{ i64 }>, ptr %11, i64 0, i32 0
  %108 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 0
  %109 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 1
  %110 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 2
  %111 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 3
  %112 = getelementptr inbounds <{ i64, ptr }>, ptr %13, i64 0, i32 0
  %113 = getelementptr inbounds <{ i64, ptr }>, ptr %13, i64 0, i32 1
  %114 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 0
  %115 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 1
  %116 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 2
  %117 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 3
  %118 = getelementptr inbounds <{ i64 }>, ptr %15, i64 0, i32 0
  br label %119

119:                                              ; preds = %172, %80
  %120 = phi i64 [ 1, %80 ], [ %173, %172 ]
  %121 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %122 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %123 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %122, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %121, i64 %120), !llfort.type_idx !46
  %124 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %123, i64 0, i32 0, !llfort.type_idx !48
  %125 = load i32, ptr %124, align 1, !tbaa !60, !llfort.type_idx !48
  %126 = icmp eq i32 %125, 0
  br i1 %126, label %138, label %127

127:                                              ; preds = %119
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %81, align 1, !tbaa !63
  store i8 4, ptr %82, align 1, !tbaa !63
  store i8 2, ptr %83, align 1, !tbaa !63
  store i8 0, ptr %84, align 1, !tbaa !63
  store i64 30, ptr %85, align 8, !tbaa !64
  store ptr @strlit.6, ptr %86, align 8, !tbaa !66
  %128 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %2, ptr nonnull %3) #4, !llfort.type_idx !22
  %129 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %130 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %131 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %130, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %129, i64 %120), !llfort.type_idx !46
  %132 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %131, i64 0, i32 0, !llfort.type_idx !48
  %133 = load i32, ptr %132, align 1, !tbaa !60, !llfort.type_idx !48
  store i8 9, ptr %87, align 1, !tbaa !63
  store i8 1, ptr %88, align 1, !tbaa !63
  store i8 2, ptr %89, align 1, !tbaa !63
  store i8 0, ptr %90, align 1, !tbaa !63
  store i32 %133, ptr %91, align 8, !tbaa !68
  %134 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %4, ptr nonnull %5) #4, !llfort.type_idx !22
  store i8 9, ptr %92, align 1, !tbaa !63
  store i8 1, ptr %93, align 1, !tbaa !63
  store i8 1, ptr %94, align 1, !tbaa !63
  store i8 0, ptr %95, align 1, !tbaa !63
  %135 = trunc i64 %120 to i32
  store i32 %135, ptr %96, align 8, !tbaa !70
  %136 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %6, ptr nonnull %7) #4, !llfort.type_idx !22
  %137 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %138

138:                                              ; preds = %127, %119
  %139 = trunc i64 %120 to i32
  br label %140

140:                                              ; preds = %153, %138
  %141 = phi i64 [ %154, %153 ], [ 1, %138 ]
  %142 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %143 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %144 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %143, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %142, i64 %120), !llfort.type_idx !46
  %145 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %144, i64 0, i32 1, i64 0, !llfort.type_idx !52
  %146 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %145, i64 %141), !llfort.type_idx !22
  %147 = load i32, ptr %146, align 1, !tbaa !72, !llfort.type_idx !74
  %148 = icmp eq i32 %147, 0
  br i1 %148, label %153, label %149

149:                                              ; preds = %140
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %97, align 1, !tbaa !63
  store i8 4, ptr %98, align 1, !tbaa !63
  store i8 2, ptr %99, align 1, !tbaa !63
  store i8 0, ptr %100, align 1, !tbaa !63
  store i64 36, ptr %101, align 8, !tbaa !75
  store ptr @strlit.5, ptr %102, align 8, !tbaa !77
  %150 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %8, ptr nonnull %9) #4, !llfort.type_idx !22
  store i8 9, ptr %103, align 1, !tbaa !63
  store i8 1, ptr %104, align 1, !tbaa !63
  store i8 1, ptr %105, align 1, !tbaa !63
  store i8 0, ptr %106, align 1, !tbaa !63
  store i32 %139, ptr %107, align 8, !tbaa !79
  %151 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %10, ptr nonnull %11) #4, !llfort.type_idx !22
  %152 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %153

153:                                              ; preds = %149, %140
  %154 = add nuw nsw i64 %141, 1
  %155 = icmp eq i64 %154, 201
  br i1 %155, label %156, label %140

156:                                              ; preds = %169, %153
  %157 = phi i64 [ %170, %169 ], [ 1, %153 ]
  %158 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %159 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %160 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %159, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %158, i64 %120), !llfort.type_idx !46
  %161 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %160, i64 0, i32 2, i64 0, !llfort.type_idx !55
  %162 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %161, i64 %157), !llfort.type_idx !56
  %163 = load float, ptr %162, align 1, !tbaa !81, !llfort.type_idx !83
  %164 = fcmp reassoc ninf nsz arcp contract afn une float %163, 0.000000e+00
  br i1 %164, label %165, label %169

165:                                              ; preds = %156
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %108, align 1, !tbaa !63
  store i8 4, ptr %109, align 1, !tbaa !63
  store i8 2, ptr %110, align 1, !tbaa !63
  store i8 0, ptr %111, align 1, !tbaa !63
  store i64 37, ptr %112, align 8, !tbaa !84
  store ptr @strlit.4, ptr %113, align 8, !tbaa !86
  %166 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %12, ptr nonnull %13) #4, !llfort.type_idx !22
  store i8 9, ptr %114, align 1, !tbaa !63
  store i8 1, ptr %115, align 1, !tbaa !63
  store i8 1, ptr %116, align 1, !tbaa !63
  store i8 0, ptr %117, align 1, !tbaa !63
  store i32 %139, ptr %118, align 8, !tbaa !88
  %167 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %14, ptr nonnull %15) #4, !llfort.type_idx !22
  %168 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %169

169:                                              ; preds = %165, %156
  %170 = add nuw nsw i64 %157, 1
  %171 = icmp eq i64 %170, 301
  br i1 %171, label %172, label %156

172:                                              ; preds = %169
  %173 = add nuw nsw i64 %120, 1
  %174 = icmp eq i64 %173, 101
  br i1 %174, label %175, label %119

175:                                              ; preds = %172
  %176 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !90, !llfort.type_idx !46
  %177 = load i64, ptr %57, align 1, !tbaa !96, !llfort.type_idx !31
  br label %178

178:                                              ; preds = %201, %175
  %179 = phi i64 [ 1, %175 ], [ %202, %201 ]
  %180 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %177, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %176, i64 %179), !llfort.type_idx !46
  %181 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %180, i64 0, i32 0, !llfort.type_idx !48
  %182 = trunc i64 %179 to i32
  store i32 %182, ptr %181, align 1, !tbaa !97
  %183 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %180, i64 0, i32 1, i64 0, !llfort.type_idx !52
  br label %184

184:                                              ; preds = %184, %178
  %185 = phi i64 [ %189, %184 ], [ 1, %178 ]
  %186 = add nuw nsw i64 %185, %179
  %187 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %183, i64 %185), !llfort.type_idx !22
  %188 = trunc i64 %186 to i32
  store i32 %188, ptr %187, align 1, !tbaa !100
  %189 = add nuw nsw i64 %185, 1
  %190 = icmp eq i64 %189, 201
  br i1 %190, label %191, label %184

191:                                              ; preds = %184
  %192 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %180, i64 0, i32 2, i64 0, !llfort.type_idx !55
  br label %193

193:                                              ; preds = %193, %191
  %194 = phi i64 [ 1, %191 ], [ %199, %193 ]
  %195 = sub nsw i64 %179, %194
  %196 = trunc i64 %195 to i32
  %197 = sitofp i32 %196 to float, !llfort.type_idx !56
  %198 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %192, i64 %194), !llfort.type_idx !56
  store float %197, ptr %198, align 1, !tbaa !102
  %199 = add nuw nsw i64 %194, 1
  %200 = icmp eq i64 %199, 301
  br i1 %200, label %201, label %193

201:                                              ; preds = %193
  %202 = add nuw nsw i64 %179, 1
  %203 = icmp eq i64 %202, 101
  br i1 %203, label %204, label %178

204:                                              ; preds = %201
  %205 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 0
  %206 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 1
  %207 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 2
  %208 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 3
  %209 = getelementptr inbounds <{ i64, ptr }>, ptr %17, i64 0, i32 0
  %210 = getelementptr inbounds <{ i64, ptr }>, ptr %17, i64 0, i32 1
  %211 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 0
  %212 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 1
  %213 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 2
  %214 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 3
  %215 = getelementptr inbounds <{ i64 }>, ptr %19, i64 0, i32 0
  %216 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 0
  %217 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 1
  %218 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 2
  %219 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 3
  %220 = getelementptr inbounds <{ i64 }>, ptr %21, i64 0, i32 0
  %221 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 0
  %222 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 1
  %223 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 2
  %224 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 3
  %225 = getelementptr inbounds <{ i64, ptr }>, ptr %23, i64 0, i32 0
  %226 = getelementptr inbounds <{ i64, ptr }>, ptr %23, i64 0, i32 1
  %227 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 0
  %228 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 1
  %229 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 2
  %230 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 3
  %231 = getelementptr inbounds <{ i64 }>, ptr %25, i64 0, i32 0
  %232 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 0
  %233 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 1
  %234 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 2
  %235 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 3
  %236 = getelementptr inbounds <{ i64, ptr }>, ptr %27, i64 0, i32 0
  %237 = getelementptr inbounds <{ i64, ptr }>, ptr %27, i64 0, i32 1
  %238 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 0
  %239 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 1
  %240 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 2
  %241 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 3
  %242 = getelementptr inbounds <{ i64 }>, ptr %29, i64 0, i32 0
  br label %243

243:                                              ; preds = %302, %204
  %244 = phi i64 [ 1, %204 ], [ %303, %302 ]
  %245 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %246 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %247 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %246, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %245, i64 %244), !llfort.type_idx !46
  %248 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %247, i64 0, i32 0, !llfort.type_idx !48
  %249 = load i32, ptr %248, align 1, !tbaa !60, !llfort.type_idx !48
  %250 = zext i32 %249 to i64
  %251 = icmp eq i64 %244, %250
  br i1 %251, label %263, label %252

252:                                              ; preds = %243
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %205, align 1, !tbaa !63
  store i8 4, ptr %206, align 1, !tbaa !63
  store i8 2, ptr %207, align 1, !tbaa !63
  store i8 0, ptr %208, align 1, !tbaa !63
  store i64 30, ptr %209, align 8, !tbaa !104
  store ptr @strlit.3, ptr %210, align 8, !tbaa !106
  %253 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %16, ptr nonnull %17) #4, !llfort.type_idx !22
  %254 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %255 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %256 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %255, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %254, i64 %244), !llfort.type_idx !46
  %257 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %256, i64 0, i32 0, !llfort.type_idx !48
  %258 = load i32, ptr %257, align 1, !tbaa !60, !llfort.type_idx !48
  store i8 9, ptr %211, align 1, !tbaa !63
  store i8 1, ptr %212, align 1, !tbaa !63
  store i8 2, ptr %213, align 1, !tbaa !63
  store i8 0, ptr %214, align 1, !tbaa !63
  store i32 %258, ptr %215, align 8, !tbaa !108
  %259 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %18, ptr nonnull %19) #4, !llfort.type_idx !22
  store i8 9, ptr %216, align 1, !tbaa !63
  store i8 1, ptr %217, align 1, !tbaa !63
  store i8 1, ptr %218, align 1, !tbaa !63
  store i8 0, ptr %219, align 1, !tbaa !63
  %260 = trunc i64 %244 to i32
  store i32 %260, ptr %220, align 8, !tbaa !110
  %261 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %20, ptr nonnull %21) #4, !llfort.type_idx !22
  %262 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %263

263:                                              ; preds = %252, %243
  %264 = trunc i64 %244 to i32
  br label %265

265:                                              ; preds = %280, %263
  %266 = phi i64 [ %281, %280 ], [ 1, %263 ]
  %267 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %268 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %269 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %268, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %267, i64 %244), !llfort.type_idx !46
  %270 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %269, i64 0, i32 1, i64 0, !llfort.type_idx !52
  %271 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %270, i64 %266), !llfort.type_idx !22
  %272 = load i32, ptr %271, align 1, !tbaa !72, !llfort.type_idx !112
  %273 = add nuw nsw i64 %266, %244
  %274 = zext i32 %272 to i64
  %275 = icmp eq i64 %273, %274
  br i1 %275, label %280, label %276

276:                                              ; preds = %265
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %221, align 1, !tbaa !63
  store i8 4, ptr %222, align 1, !tbaa !63
  store i8 2, ptr %223, align 1, !tbaa !63
  store i8 0, ptr %224, align 1, !tbaa !63
  store i64 36, ptr %225, align 8, !tbaa !113
  store ptr @strlit.2, ptr %226, align 8, !tbaa !115
  %277 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %22, ptr nonnull %23) #4, !llfort.type_idx !22
  store i8 9, ptr %227, align 1, !tbaa !63
  store i8 1, ptr %228, align 1, !tbaa !63
  store i8 1, ptr %229, align 1, !tbaa !63
  store i8 0, ptr %230, align 1, !tbaa !63
  store i32 %264, ptr %231, align 8, !tbaa !117
  %278 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %24, ptr nonnull %25) #4, !llfort.type_idx !22
  %279 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %280

280:                                              ; preds = %276, %265
  %281 = add nuw nsw i64 %266, 1
  %282 = icmp eq i64 %281, 201
  br i1 %282, label %283, label %265

283:                                              ; preds = %299, %280
  %284 = phi i64 [ %300, %299 ], [ 1, %280 ]
  %285 = load ptr, ptr @difficultstruct_mp_layering_, align 16, !tbaa !59, !llfort.type_idx !46
  %286 = load i64, ptr %57, align 1, !tbaa !35, !llfort.type_idx !31
  %287 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %286, i64 2004, ptr elementtype(%"DIFFICULTSTRUCT$.btLAYERING_TYPE") %285, i64 %244), !llfort.type_idx !46
  %288 = getelementptr inbounds %"DIFFICULTSTRUCT$.btLAYERING_TYPE", ptr %287, i64 0, i32 2, i64 0, !llfort.type_idx !55
  %289 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %288, i64 %284), !llfort.type_idx !56
  %290 = load float, ptr %289, align 1, !tbaa !81, !llfort.type_idx !119
  %291 = sub nsw i64 %244, %284
  %292 = trunc i64 %291 to i32
  %293 = sitofp i32 %292 to float, !llfort.type_idx !56
  %294 = fcmp reassoc ninf nsz arcp contract afn une float %290, %293
  br i1 %294, label %295, label %299

295:                                              ; preds = %283
  store i64 0, ptr %1, align 16, !tbaa !63
  store i8 56, ptr %232, align 1, !tbaa !63
  store i8 4, ptr %233, align 1, !tbaa !63
  store i8 2, ptr %234, align 1, !tbaa !63
  store i8 0, ptr %235, align 1, !tbaa !63
  store i64 37, ptr %236, align 8, !tbaa !120
  store ptr @strlit.1, ptr %237, align 8, !tbaa !122
  %296 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %26, ptr nonnull %27) #4, !llfort.type_idx !22
  store i8 9, ptr %238, align 1, !tbaa !63
  store i8 1, ptr %239, align 1, !tbaa !63
  store i8 1, ptr %240, align 1, !tbaa !63
  store i8 0, ptr %241, align 1, !tbaa !63
  store i32 %264, ptr %242, align 8, !tbaa !124
  %297 = call i32 @for_write_seq_lis_xmit(ptr nonnull %1, ptr nonnull %28, ptr nonnull %29) #4, !llfort.type_idx !22
  %298 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr nonnull @strlit.12, i32 0, i32 0, i64 2253038970797824, i32 0, i32 0) #4, !llfort.type_idx !22
  br label %299

299:                                              ; preds = %295, %283
  %300 = add nuw nsw i64 %284, 1
  %301 = icmp eq i64 %300, 301
  br i1 %301, label %302, label %283

302:                                              ; preds = %299
  %303 = add nuw nsw i64 %244, 1
  %304 = icmp eq i64 %303, 101
  br i1 %304, label %305, label %243

305:                                              ; preds = %302
  store i64 0, ptr %1, align 16, !tbaa !63
  %306 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 0
  store i8 56, ptr %306, align 1, !tbaa !63
  %307 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 1
  store i8 4, ptr %307, align 1, !tbaa !63
  %308 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 2
  store i8 1, ptr %308, align 1, !tbaa !63
  %309 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 3
  store i8 0, ptr %309, align 1, !tbaa !63
  %310 = getelementptr inbounds <{ i64, ptr }>, ptr %31, i64 0, i32 0, !llfort.type_idx !126
  store i64 22, ptr %310, align 8, !tbaa !127
  %311 = getelementptr inbounds <{ i64, ptr }>, ptr %31, i64 0, i32 1, !llfort.type_idx !129
  store ptr @strlit, ptr %311, align 8, !tbaa !130
  %312 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %30, ptr nonnull %31) #4, !llfort.type_idx !22
  ret void
}

declare !llfort.intrin_id !132 dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !133 dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !134 dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: nofree
declare !llfort.intrin_id !135 dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !136 dso_local i32 @for_write_seq_lis_xmit(ptr nocapture readonly, ptr nocapture readonly, ptr) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !137 dso_local i32 @for_stop_core_quiet(ptr nocapture readonly, i32, i32, i64, i32, i32, ...) local_unnamed_addr #2

attributes #0 = { nofree nounwind }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "prefer-vector-width"="512" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2, !3}
!ifx.types.dv = !{!138}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 1, !"LTOPostLink", i32 1}
!4 = !{i64 95}
!5 = !{i64 113}
!6 = !{i64 115}
!7 = !{i64 118}
!8 = !{i64 120}
!9 = !{i64 123}
!10 = !{i64 131}
!11 = !{i64 134}
!12 = !{i64 141}
!13 = !{i64 144}
!14 = !{i64 151}
!15 = !{i64 154}
!16 = !{i64 157}
!17 = !{i64 164}
!18 = !{i64 167}
!19 = !{i64 174}
!20 = !{i64 177}
!21 = !{i64 183}
!22 = !{i64 2}
!23 = !{!24, !25, i64 40}
!24 = !{!"ifx$descr$3", !25, i64 0, !25, i64 8, !25, i64 16, !25, i64 24, !25, i64 32, !25, i64 40, !25, i64 48, !25, i64 56, !25, i64 64}
!25 = !{!"ifx$descr$field", !26, i64 0}
!26 = !{!"Fortran Dope Vector Symbol", !27, i64 0}
!27 = !{!"Generic Fortran Symbol", !28, i64 0}
!28 = !{!"ifx$root$3$MAIN__"}
!29 = !{!24, !25, i64 24}
!30 = !{i64 38}
!31 = !{i64 29}
!32 = !{!24, !25, i64 8}
!33 = !{!24, !25, i64 32}
!34 = !{!24, !25, i64 16}
!35 = !{!24, !25, i64 64}
!36 = !{i64 27}
!37 = !{!24, !25, i64 48}
!38 = !{i64 28}
!39 = !{!24, !25, i64 56}
!40 = !{!41, !42, i64 0}
!41 = !{!"ifx$descr$1$0", !42, i64 0, !42, i64 8, !42, i64 16, !42, i64 24, !42, i64 32, !42, i64 40, !42, i64 48, !42, i64 56, !42, i64 64}
!42 = !{!"ifx$descr$field$0", !43, i64 0}
!43 = !{!"Fortran Dope Vector Symbol", !44, i64 0}
!44 = !{!"Generic Fortran Symbol", !45, i64 0}
!45 = !{!"ifx$root$1$fill1_$0"}
!46 = !{i64 15}
!47 = !{!41, !42, i64 64}
!48 = !{i64 17}
!49 = !{!50, !50, i64 0}
!50 = !{!"ifx$unique_sym$2$0", !51, i64 0}
!51 = !{!"Fortran Data Symbol", !44, i64 0}
!52 = !{i64 19}
!53 = !{!54, !54, i64 0}
!54 = !{!"ifx$unique_sym$4$0", !51, i64 0}
!55 = !{i64 21}
!56 = !{i64 5}
!57 = !{!58, !58, i64 0}
!58 = !{!"ifx$unique_sym$5$0", !51, i64 0}
!59 = !{!24, !25, i64 0}
!60 = !{!61, !61, i64 0}
!61 = !{!"ifx$unique_sym$13", !62, i64 0}
!62 = !{!"Fortran Data Symbol", !27, i64 0}
!63 = !{!62, !62, i64 0}
!64 = !{!65, !65, i64 0}
!65 = !{!"ifx$unique_sym$15", !62, i64 0}
!66 = !{!67, !67, i64 0}
!67 = !{!"ifx$unique_sym$16", !62, i64 0}
!68 = !{!69, !69, i64 0}
!69 = !{!"ifx$unique_sym$17", !62, i64 0}
!70 = !{!71, !71, i64 0}
!71 = !{!"ifx$unique_sym$18", !62, i64 0}
!72 = !{!73, !73, i64 0}
!73 = !{!"ifx$unique_sym$21", !62, i64 0}
!74 = !{i64 130}
!75 = !{!76, !76, i64 0}
!76 = !{!"ifx$unique_sym$23", !62, i64 0}
!77 = !{!78, !78, i64 0}
!78 = !{!"ifx$unique_sym$24", !62, i64 0}
!79 = !{!80, !80, i64 0}
!80 = !{!"ifx$unique_sym$25", !62, i64 0}
!81 = !{!82, !82, i64 0}
!82 = !{!"ifx$unique_sym$27", !62, i64 0}
!83 = !{i64 140}
!84 = !{!85, !85, i64 0}
!85 = !{!"ifx$unique_sym$29", !62, i64 0}
!86 = !{!87, !87, i64 0}
!87 = !{!"ifx$unique_sym$30", !62, i64 0}
!88 = !{!89, !89, i64 0}
!89 = !{!"ifx$unique_sym$31", !62, i64 0}
!90 = !{!91, !92, i64 0}
!91 = !{!"ifx$descr$2$1", !92, i64 0, !92, i64 8, !92, i64 16, !92, i64 24, !92, i64 32, !92, i64 40, !92, i64 48, !92, i64 56, !92, i64 64}
!92 = !{!"ifx$descr$field$1", !93, i64 0}
!93 = !{!"Fortran Dope Vector Symbol", !94, i64 0}
!94 = !{!"Generic Fortran Symbol", !95, i64 0}
!95 = !{!"ifx$root$2$fill2_$1"}
!96 = !{!91, !92, i64 64}
!97 = !{!98, !98, i64 0}
!98 = !{!"ifx$unique_sym$7$1", !99, i64 0}
!99 = !{!"Fortran Data Symbol", !94, i64 0}
!100 = !{!101, !101, i64 0}
!101 = !{!"ifx$unique_sym$9$1", !99, i64 0}
!102 = !{!103, !103, i64 0}
!103 = !{!"ifx$unique_sym$10$1", !99, i64 0}
!104 = !{!105, !105, i64 0}
!105 = !{!"ifx$unique_sym$34", !62, i64 0}
!106 = !{!107, !107, i64 0}
!107 = !{!"ifx$unique_sym$35", !62, i64 0}
!108 = !{!109, !109, i64 0}
!109 = !{!"ifx$unique_sym$36", !62, i64 0}
!110 = !{!111, !111, i64 0}
!111 = !{!"ifx$unique_sym$37", !62, i64 0}
!112 = !{i64 163}
!113 = !{!114, !114, i64 0}
!114 = !{!"ifx$unique_sym$40", !62, i64 0}
!115 = !{!116, !116, i64 0}
!116 = !{!"ifx$unique_sym$41", !62, i64 0}
!117 = !{!118, !118, i64 0}
!118 = !{!"ifx$unique_sym$42", !62, i64 0}
!119 = !{i64 173}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$45", !62, i64 0}
!122 = !{!123, !123, i64 0}
!123 = !{!"ifx$unique_sym$46", !62, i64 0}
!124 = !{!125, !125, i64 0}
!125 = !{!"ifx$unique_sym$47", !62, i64 0}
!126 = !{i64 184}
!127 = !{!128, !128, i64 0}
!128 = !{!"ifx$unique_sym$50", !62, i64 0}
!129 = !{i64 185}
!130 = !{!131, !131, i64 0}
!131 = !{!"ifx$unique_sym$51", !62, i64 0}
!132 = !{i32 97}
!133 = !{i32 98}
!134 = !{i32 94}
!135 = !{i32 336}
!136 = !{i32 338}
!137 = !{i32 81}
!138 = !{%"QNCA_a0$DIFFICULTSTRUCT$.btLAYERING_TYPE*$rank1$" zeroinitializer, %"DIFFICULTSTRUCT$.btLAYERING_TYPE" zeroinitializer}
