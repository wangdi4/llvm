; RUN: opt -hir-cost-model-throttling=0 -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Check that (i1 + 1 == 1) split i1 loop and IO statements are "peeled".

; CHECK-NOT: DO
; CHECK-NOT: if (i1 + 1 == 1)
; CHECK: for_write_seq_lis_xmit
; CHECK: DO i1 = 0, %32 + -2, 1
; CHECK-NOT: if (i1 + 1 == 1)

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"POWER_MOD$.btPOWERTYP" = type { i32, i32, [80 x i8], i32, double, double, i32, i32, ptr, i32, ptr, i32 }

@globalvar_mod_mp_coords_ = external hidden unnamed_addr global [3 x i32], align 8
@globalvar_mod_mp_dt_ = external hidden unnamed_addr global double, align 8
@globalvar_mod_mp_stride_ = external hidden global i32, align 8
@globalvar_mod_mp_nts_ = external hidden global i32, align 8
@globalvar_mod_mp_out_ = external hidden global i32, align 8
@power_mod_mp_first_power_ = external hidden unnamed_addr global ptr, align 8
@plane_source_mod_mp_plane_source_block_index_ = external hidden unnamed_addr global i32, align 8
@anon.d9c51c694055688cf115173cc61c23d0.14 = external hidden unnamed_addr constant [53 x i8]
@anon.d9c51c694055688cf115173cc61c23d0.13 = external hidden unnamed_addr constant [27 x i8]
@anon.d9c51c694055688cf115173cc61c23d0.9 = external hidden unnamed_addr constant [17 x i8]
@anon.d9c51c694055688cf115173cc61c23d0.3 = external hidden unnamed_addr constant [29 x i8]
@anon.d9c51c694055688cf115173cc61c23d0.2 = external hidden unnamed_addr constant [4 x i8]
@anon.d9c51c694055688cf115173cc61c23d0.1 = external hidden unnamed_addr constant [33 x i8]
@"leapfrog_$TMPPOWER" = external hidden unnamed_addr global ptr, align 8
@anon.d9c51c694055688cf115173cc61c23d0.0 = external hidden unnamed_addr constant [16 x i8]
@"pscyee_mpi_$PERF" = external hidden global [2 x double], align 32
@"pscyee_mpi_$FILENUMBER" = external hidden global i32, align 8

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #0

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis_xmit(ptr, ptr, ptr) local_unnamed_addr #0

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable

; Function Attrs: nounwind uwtable
declare hidden fastcc void @power_mod_mp_power_dft_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", double, i32) unnamed_addr #2

; Function Attrs: nounwind uwtable
declare hidden fastcc void @power_mod_mp_power_print_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture "ptrnoalias", i32) unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @power_mod_mp_power_updateh_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias") unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @update_mod_mp_updateh_(ptr noalias nocapture readonly "ptrnoalias", i32, i32, ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias") unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @material_mod_mp_mat_updatee_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias") unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @plane_source_mod_mp_plane_source_apply_(ptr noalias nocapture "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", i32, i32) unnamed_addr #3

; Function Attrs: nounwind uwtable
define hidden fastcc void @leapfrog_(ptr noalias nocapture readonly "ptrnoalias" %0, ptr noalias nocapture readonly "ptrnoalias" %1, ptr noalias nocapture readonly "ptrnoalias" %2, ptr noalias nocapture "ptrnoalias" %3, ptr noalias nocapture "ptrnoalias" %4, ptr noalias nocapture "ptrnoalias" %5, ptr noalias nocapture "ptrnoalias" %6, ptr noalias nocapture "ptrnoalias" %7, ptr noalias nocapture "ptrnoalias" %8) unnamed_addr #4 {
  %10 = alloca [8 x i64], align 32
  %11 = alloca i32, align 8
  %12 = alloca [4 x i8], align 1
  %13 = alloca { i64, ptr }, align 8
  %14 = alloca [4 x i8], align 1
  %15 = alloca { i64, ptr }, align 8
  %16 = alloca [4 x i8], align 1
  %17 = alloca { i64, ptr }, align 8
  %18 = alloca [4 x i8], align 1
  %19 = alloca { i64, ptr }, align 8
  %20 = alloca [4 x i8], align 1
  %21 = alloca { i32 }, align 8
  %22 = alloca [4 x i8], align 1
  %23 = alloca { i64, ptr }, align 8
  %24 = alloca [4 x i8], align 1
  %25 = alloca { i32 }, align 8
  %26 = alloca [4 x i8], align 1
  %27 = alloca { i64, ptr }, align 8
  %28 = alloca [4 x i8], align 1
  %29 = alloca { i32 }, align 8
  %30 = alloca [4 x i8], align 1
  %31 = alloca { i64, ptr }, align 8
  %32 = load i32, ptr @globalvar_mod_mp_nts_, align 8
  %33 = icmp slt i32 %32, 1
  br i1 %33, label %34, label %36

34:                                               ; preds = %9
  %35 = bitcast ptr %10 to ptr
  br label %182

36:                                               ; preds = %9
  %37 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 0
  %38 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 1
  %39 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 2
  %40 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 3
  %41 = getelementptr inbounds { i64, ptr }, ptr %13, i64 0, i32 0
  %42 = getelementptr inbounds { i64, ptr }, ptr %13, i64 0, i32 1
  %43 = bitcast ptr %10 to ptr
  %44 = bitcast ptr %13 to ptr
  %45 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 0
  %46 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 1
  %47 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 2
  %48 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 3
  %49 = getelementptr inbounds { i64, ptr }, ptr %15, i64 0, i32 0
  %50 = getelementptr inbounds { i64, ptr }, ptr %15, i64 0, i32 1
  %51 = bitcast ptr %15 to ptr
  %52 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 0
  %53 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 1
  %54 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 2
  %55 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 3
  %56 = getelementptr inbounds { i64, ptr }, ptr %17, i64 0, i32 0
  %57 = getelementptr inbounds { i64, ptr }, ptr %17, i64 0, i32 1
  %58 = bitcast ptr %17 to ptr
  %59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @globalvar_mod_mp_coords_, i64 2)
  %60 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 0
  %61 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 1
  %62 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 2
  %63 = getelementptr inbounds [4 x i8], ptr %26, i64 0, i64 3
  %64 = getelementptr inbounds { i64, ptr }, ptr %27, i64 0, i32 0
  %65 = getelementptr inbounds { i64, ptr }, ptr %27, i64 0, i32 1
  %66 = bitcast ptr %27 to ptr
  %67 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 0
  %68 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 1
  %69 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 2
  %70 = getelementptr inbounds [4 x i8], ptr %28, i64 0, i64 3
  %71 = getelementptr inbounds { i32 }, ptr %29, i64 0, i32 0
  %72 = bitcast ptr %29 to ptr
  %73 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 0
  %74 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 1
  %75 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 2
  %76 = getelementptr inbounds [4 x i8], ptr %18, i64 0, i64 3
  %77 = getelementptr inbounds { i64, ptr }, ptr %19, i64 0, i32 0
  %78 = getelementptr inbounds { i64, ptr }, ptr %19, i64 0, i32 1
  %79 = bitcast ptr %19 to ptr
  %80 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 0
  %81 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 1
  %82 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 2
  %83 = getelementptr inbounds [4 x i8], ptr %20, i64 0, i64 3
  %84 = getelementptr inbounds { i32 }, ptr %21, i64 0, i32 0
  %85 = bitcast ptr %21 to ptr
  %86 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 0
  %87 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 1
  %88 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 2
  %89 = getelementptr inbounds [4 x i8], ptr %22, i64 0, i64 3
  %90 = getelementptr inbounds { i64, ptr }, ptr %23, i64 0, i32 0
  %91 = getelementptr inbounds { i64, ptr }, ptr %23, i64 0, i32 1
  %92 = bitcast ptr %23 to ptr
  %93 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 0
  %94 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 1
  %95 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 2
  %96 = getelementptr inbounds [4 x i8], ptr %24, i64 0, i64 3
  %97 = getelementptr inbounds { i32 }, ptr %25, i64 0, i32 0
  %98 = bitcast ptr %25 to ptr
  %99 = add i32 %32, 1
  br label %100

100:                                              ; preds = %171, %36
  %101 = phi i32 [ 1, %36 ], [ %172, %171 ]
  %102 = icmp eq i32 %101, 1
  br i1 %102, label %103, label %108

103:                                              ; preds = %100
  store i8 56, ptr %37, align 1
  store i8 4, ptr %38, align 1
  store i8 1, ptr %39, align 1
  store i8 0, ptr %40, align 1
  store i64 53, ptr %41, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.14, ptr %42, align 8
  %104 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %43, i32 -1, i64 1239157112576, ptr nonnull %37, ptr nonnull %44) #5
  store i8 56, ptr %45, align 1
  store i8 4, ptr %46, align 1
  store i8 1, ptr %47, align 1
  store i8 0, ptr %48, align 1
  store i64 27, ptr %49, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.13, ptr %50, align 8
  %105 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %43, i32 -1, i64 1239157112576, ptr nonnull %45, ptr nonnull %51) #5
  br label %108

106:                                              ; preds = %108
  store i8 56, ptr %52, align 1
  store i8 4, ptr %53, align 1
  store i8 1, ptr %54, align 1
  store i8 0, ptr %55, align 1
  store i64 17, ptr %56, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.9, ptr %57, align 8
  %107 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %43, i32 -1, i64 1239157112576, ptr nonnull %52, ptr nonnull %58) #5
  br label %113

108:                                              ; preds = %103, %100
  %109 = load i32, ptr %1, align 1
  %110 = load i32, ptr %2, align 1
  call fastcc void @update_mod_mp_updateh_(ptr %0, i32 %109, i32 %110, ptr %6, ptr %7, ptr %8, ptr %3, ptr %4, ptr %5) #5
  call fastcc void @power_mod_mp_power_updateh_(ptr %3, ptr %4, ptr %5, ptr %6, ptr %8, ptr %0, ptr nonnull %1, ptr nonnull %2) #5
  call fastcc void @upml_mod_mp_upml_updateh_(ptr %0, ptr nonnull %1, ptr nonnull %2, ptr %6, ptr %7, ptr %8, ptr %3, ptr %4, ptr %5) #5
  %111 = icmp eq i32 %101, 1
  br i1 %111, label %106, label %113

112:                                              ; preds = %113
  call fastcc void @plane_source_mod_mp_plane_source_apply_(ptr %3, ptr %0, i32 %110, i32 %101) #5
  br label %131

113:                                              ; preds = %108, %106
  call fastcc void @material_mod_mp_mat_updatee_(ptr %0, ptr nonnull %1, ptr nonnull %2, ptr %6, ptr %7, ptr %8, ptr %3, ptr %4, ptr %5) #5
  %114 = load i32, ptr %59, align 1
  %115 = load i32, ptr @plane_source_mod_mp_plane_source_block_index_, align 8
  %116 = icmp eq i32 %114, %115
  br i1 %116, label %112, label %131

117:                                              ; preds = %128
  store i8 56, ptr %73, align 1
  store i8 4, ptr %74, align 1
  store i8 2, ptr %75, align 1
  store i8 0, ptr %76, align 1
  store i64 29, ptr %77, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.3, ptr %78, align 8
  %118 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %43, i32 -1, i64 1239157112576, ptr nonnull %73, ptr nonnull %79) #5
  %119 = load i32, ptr @"pscyee_mpi_$FILENUMBER", align 8
  store i8 9, ptr %80, align 1
  store i8 1, ptr %81, align 1
  store i8 2, ptr %82, align 1
  store i8 0, ptr %83, align 1
  store i32 %119, ptr %84, align 8
  %120 = call i32 @for_write_seq_lis_xmit(ptr nonnull %43, ptr nonnull %80, ptr nonnull %85) #5
  store i8 56, ptr %86, align 1
  store i8 4, ptr %87, align 1
  store i8 2, ptr %88, align 1
  store i8 0, ptr %89, align 1
  store i64 4, ptr %90, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.2, ptr %91, align 8
  %121 = call i32 @for_write_seq_lis_xmit(ptr nonnull %43, ptr nonnull %86, ptr nonnull %92) #5
  store i8 9, ptr %93, align 1
  store i8 1, ptr %94, align 1
  store i8 1, ptr %95, align 1
  store i8 0, ptr %96, align 1
  store i32 %101, ptr %97, align 8
  %122 = call i32 @for_write_seq_lis_xmit(ptr nonnull %43, ptr nonnull %93, ptr nonnull %98) #5
  call fastcc void @writeout_(ptr %0, ptr nonnull %1, ptr nonnull %2, ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8) #5
  %123 = load i32, ptr @"pscyee_mpi_$FILENUMBER", align 8
  %124 = add nsw i32 %123, 1
  store i32 %124, ptr @"pscyee_mpi_$FILENUMBER", align 8
  br label %166

125:                                              ; preds = %128
  store i8 56, ptr %60, align 1
  store i8 4, ptr %61, align 1
  store i8 2, ptr %62, align 1
  store i8 0, ptr %63, align 1
  store i64 33, ptr %64, align 8
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.1, ptr %65, align 8
  %126 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %43, i32 -1, i64 1239157112576, ptr nonnull %60, ptr nonnull %66) #5
  store i8 9, ptr %67, align 1
  store i8 1, ptr %68, align 1
  store i8 1, ptr %69, align 1
  store i8 0, ptr %70, align 1
  store i32 %101, ptr %71, align 8
  %127 = call i32 @for_write_seq_lis_xmit(ptr nonnull %43, ptr nonnull %67, ptr nonnull %72) #5
  br label %166

128:                                              ; preds = %131
  %129 = load i32, ptr @globalvar_mod_mp_out_, align 8
  %130 = icmp sgt i32 %129, 0
  br i1 %130, label %117, label %125

131:                                              ; preds = %113, %112
  call fastcc void @upml_mod_mp_upml_updatee_simple_(ptr %0, ptr nonnull %1, ptr nonnull %2, ptr %6, ptr %7, ptr %8, ptr %3, ptr %4, ptr %5) #5
  %132 = add nsw i32 %101, -1
  %133 = sitofp i32 %132 to double
  %134 = load double, ptr @globalvar_mod_mp_dt_, align 8
  %135 = fmul fast double %134, %133
  call fastcc void @power_mod_mp_power_dft_(ptr %3, ptr %4, ptr %5, ptr %6, ptr %7, ptr %8, double %135, i32 %101) #5
  %136 = load i32, ptr @globalvar_mod_mp_stride_, align 8
  %137 = srem i32 %101, %136
  %138 = icmp eq i32 %137, 0
  %139 = load i32, ptr @globalvar_mod_mp_nts_, align 8
  %140 = icmp ne i32 %139, %101
  %141 = and i1 %138, %140
  br i1 %141, label %128, label %166

142:                                              ; preds = %144
  call fastcc void @power_mod_mp_power_print_(ptr nonnull %147, ptr nonnull %11, i32 %145) #5
  %143 = load ptr, ptr @"leapfrog_$TMPPOWER", align 8
  br label %159

144:                                              ; preds = %169, %159
  %145 = phi i32 [ %164, %159 ], [ 1, %169 ]
  %146 = phi i64 [ %163, %159 ], [ %167, %169 ]
  %147 = inttoptr i64 %146 to ptr
  %148 = getelementptr inbounds %"POWER_MOD$.btPOWERTYP", ptr %147, i64 0, i32 1
  %149 = load i32, ptr %148, align 1
  %150 = icmp sgt i32 %101, %149
  %151 = getelementptr inbounds %"POWER_MOD$.btPOWERTYP", ptr %147, i64 0, i32 0
  %152 = load i32, ptr %151, align 1
  %153 = srem i32 %101, %152
  %154 = icmp eq i32 %153, 0
  %155 = and i1 %150, %154
  %156 = load i32, ptr @globalvar_mod_mp_nts_, align 8
  %157 = icmp eq i32 %101, %156
  %158 = or i1 %155, %157
  br i1 %158, label %142, label %159

159:                                              ; preds = %144, %142
  %160 = phi ptr [ %143, %142 ], [ %147, %144 ]
  %161 = getelementptr inbounds %"POWER_MOD$.btPOWERTYP", ptr %160, i64 0, i32 10
  %162 = bitcast ptr %161 to ptr
  %163 = load i64, ptr %162, align 1
  store i64 %163, ptr @"leapfrog_$TMPPOWER", align 8
  %164 = add nsw i32 %145, 1
  %165 = icmp eq i64 %163, 0
  br i1 %165, label %170, label %144

166:                                              ; preds = %131, %125, %117
  %167 = load i64, ptr @power_mod_mp_first_power_, align 8
  store i64 %167, ptr @"leapfrog_$TMPPOWER", align 8
  store i32 1, ptr %11, align 8
  %168 = icmp eq i64 %167, 0
  br i1 %168, label %171, label %169

169:                                              ; preds = %166
  br label %144

170:                                              ; preds = %159
  br label %171

171:                                              ; preds = %170, %166
  %172 = add nuw i32 %101, 1
  %173 = icmp ne i32 %172, %99
  br i1 %173, label %100, label %181

174:                                              ; preds = %177
  store double 0.000000e+00, ptr %178, align 1
  %175 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull @"pscyee_mpi_$PERF", i64 2)
  %176 = bitcast ptr %175 to ptr
  store i64 0, ptr %176, align 1
  br label %194

177:                                              ; preds = %182
  %178 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"pscyee_mpi_$PERF", i64 1)
  %179 = load double, ptr %178, align 1
  %180 = fcmp fast ogt double %179, 0.000000e+00
  br i1 %180, label %174, label %194

181:                                              ; preds = %171
  br label %182

182:                                              ; preds = %181, %34
  %183 = phi ptr [ %35, %34 ], [ %43, %181 ]
  %184 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 0
  store i8 56, ptr %184, align 1
  %185 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 1
  store i8 4, ptr %185, align 1
  %186 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 2
  store i8 1, ptr %186, align 1
  %187 = getelementptr inbounds [4 x i8], ptr %30, i64 0, i64 3
  store i8 0, ptr %187, align 1
  %188 = getelementptr inbounds { i64, ptr }, ptr %31, i64 0, i32 0
  store i64 16, ptr %188, align 8
  %189 = getelementptr inbounds { i64, ptr }, ptr %31, i64 0, i32 1
  store ptr @anon.d9c51c694055688cf115173cc61c23d0.0, ptr %189, align 8
  %190 = bitcast ptr %31 to ptr
  %191 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %183, i32 -1, i64 1239157112576, ptr nonnull %184, ptr nonnull %190) #5
  %192 = load i32, ptr @globalvar_mod_mp_nts_, align 8
  %193 = icmp sgt i32 %192, 1
  br i1 %193, label %177, label %194

194:                                              ; preds = %182, %177, %174
  ret void
}

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @upml_mod_mp_upml_updateh_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias") unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @upml_mod_mp_upml_updatee_simple_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias", ptr noalias nocapture "ptrnoalias") unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
declare hidden fastcc void @writeout_(ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias", ptr noalias nocapture readonly "ptrnoalias") unnamed_addr #3

attributes #0 = { nofree "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind uwtable "intel-lang"="fortran" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
