; This test verifies that "store float 1.000000e+00, float*
; %a__dyncom_ptr_fetch.2.B22" is not eliminated by InstCombine due
; to incorrect Andersens's points-to info. Memory is allocated to
; @a__dyncom_ptr in @f90_dyncom. Address of @a__dyncom_ptr is passed to
; @f90_dyncom indirectly through @"_UNNAMED_MAIN$$_rtcommontab".
; Earlier AndersensAA was incorrectly computing that no memory was allocated
; a__dyncom_ptr.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: store float 1.000000e+00, {{.*}}B22

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%rtcommontabtype_1 = type { i64, i8*, i64, [1 x %rtcommontabentry_type] }
%rtcommontabentry_type = type { i8*, i8*, i64 }

@"_UNNAMED_MAIN$$_rtcommontab" = internal global %rtcommontabtype_1 { i64 1, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @strlit, i32 0, i32 0), i64 0, [1 x %rtcommontabentry_type] [%rtcommontabentry_type { i8* getelementptr inbounds ([2 x i8], [2 x i8]* @strlit.1, i32 0, i32 0), i8* bitcast (i8** @a__dyncom_ptr to i8*), i64 8 }] }
@strlit = internal unnamed_addr constant [16 x i8] c"_UNNAMED_MAIN$$\00"
@a_ = common unnamed_addr global [8 x i8] zeroinitializer, align 32
@a__dyncom_ptr = internal global i8* null, align 8
@strlit.1 = internal unnamed_addr constant [2 x i8] c"A\00"
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64 }>, align 8
  %"(&)val$9" = alloca [4 x i8], align 1
  %argblock10 = alloca <{ i64 }>, align 8
  %func_result = tail call i32 @for_set_fpe_(i32* nonnull @0) #2
  %func_result2 = tail call i32 @for_set_reentrancy(i32* nonnull @1) #2
  %a__dyncom_ptr_fetch.1 = load i8*, i8** @a__dyncom_ptr, align 8
  %rel.1.not = icmp eq i8* %a__dyncom_ptr_fetch.1, null
  %0 = bitcast i8* %a__dyncom_ptr_fetch.1 to float*
  br i1 %rel.1.not, label %dyncom_needs_alloc2, label %dyncom_all_allocated1.split

dyncom_needs_alloc2:                              ; preds = %alloca_0
  tail call void @f90_dyncom(i8* bitcast (%rtcommontabtype_1* @"_UNNAMED_MAIN$$_rtcommontab" to i8*)) #2
  %a__dyncom_ptr_fetch.2.B22.pre = load float*, float** bitcast (i8** @a__dyncom_ptr to float**), align 8
  %1 = bitcast float* %a__dyncom_ptr_fetch.2.B22.pre to i8*
  br label %dyncom_all_allocated1.split

dyncom_all_allocated1.split:                      ; preds = %alloca_0, %dyncom_needs_alloc2
  %a__dyncom_ptr_fetch.5 = phi i8* [ %1, %dyncom_needs_alloc2 ], [ %a__dyncom_ptr_fetch.1, %alloca_0 ]
  %a__dyncom_ptr_fetch.2.B22 = phi float* [ %a__dyncom_ptr_fetch.2.B22.pre, %dyncom_needs_alloc2 ], [ %0, %alloca_0 ]
  store float 1.000000e+00, float* %a__dyncom_ptr_fetch.2.B22, align 8
  %PtrToInt = ptrtoint i8* %a__dyncom_ptr_fetch.5 to i64
  %add.1 = add nsw i64 %PtrToInt, 4
  %"(float*)a__dyncom_ptr_fetch.7.C$" = inttoptr i64 %add.1 to float*
  store float 1.000000e+00, float* %"(float*)a__dyncom_ptr_fetch.7.C$", align 8
  %a__dyncom_ptr_fetch.8.B23 = load float*, float** bitcast (i8** @a__dyncom_ptr to float**), align 8
  %"(float*)a__dyncom_ptr_fetch.8.B$_fetch.9" = load float, float* %a__dyncom_ptr_fetch.8.B23, align 8
  %.fca.0.gep18 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 0
  store i8 26, i8* %.fca.0.gep18, align 1
  %.fca.1.gep19 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 1
  store i8 1, i8* %.fca.1.gep19, align 1
  %.fca.2.gep20 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 2
  store i8 2, i8* %.fca.2.gep20, align 1
  %.fca.3.gep21 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep21, align 1
  %bitcast = bitcast <{ i64 }>* %argblock to float*
  store float %"(float*)a__dyncom_ptr_fetch.8.B$_fetch.9", float* %bitcast, align 8
  %"(i8*)$io_ctx$" = bitcast [8 x i64]* %"$io_ctx" to i8*
  %"(i8*)argblock$" = bitcast <{ i64 }>* %argblock to i8*
  %func_result6 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %"(i8*)$io_ctx$", i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep18, i8* nonnull %"(i8*)argblock$") #2
  %a__dyncom_ptr_fetch.10 = load i8*, i8** @a__dyncom_ptr, align 8
  %PtrToInt8 = ptrtoint i8* %a__dyncom_ptr_fetch.10 to i64
  %add.4 = add nsw i64 %PtrToInt8, 4
  %"(float*)a__dyncom_ptr_fetch.10.C$" = inttoptr i64 %add.4 to float*
  %"(float*)a__dyncom_ptr_fetch.10.C$_fetch.11" = load float, float* %"(float*)a__dyncom_ptr_fetch.10.C$", align 8
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 0
  store i8 26, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$9", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %bitcast12 = bitcast <{ i64 }>* %argblock10 to float*
  store float %"(float*)a__dyncom_ptr_fetch.10.C$_fetch.11", float* %bitcast12, align 8
  %"(i8*)argblock10$" = bitcast <{ i64 }>* %argblock10 to i8*
  %func_result17 = call i32 @for_write_seq_lis_xmit(i8* nonnull %"(i8*)$io_ctx$", i8* nonnull %.fca.0.gep, i8* nonnull %"(i8*)argblock10$") #2
  ret void
}

declare void @f90_dyncom(i8*) local_unnamed_addr

declare i32 @for_set_fpe_(i32* nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare i32 @for_write_seq_lis_xmit(i8* nocapture readonly, i8* nocapture readonly, i8*) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind }
