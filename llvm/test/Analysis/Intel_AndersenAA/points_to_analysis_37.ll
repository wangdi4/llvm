; This test verifies that "store float 0x4084D4CCC0000000, float* %thisislive"
; is NOT removed by instcombine using Andersens's points-to analysis. This test
; verifies that constant expressions like ptrtoint in store instructions
; are handled correctly.
; Ex:  store i64 ptrtoint ([3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA" to i64), i64* @cray_pointer10_m1_mp_preca_

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: store float 0x4084D4CCC0000000,


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"CRAY_POINTER10_M1$.btSTR" = type <{ i32, float }>

@cray_pointer10_m1_mp_preca_ = internal unnamed_addr global i64 0, align 8
@"cray_pointer10_$RA" = internal global [3 x %"CRAY_POINTER10_M1$.btSTR"] zeroinitializer

; Function Attrs: nofree
define dso_local void @MAIN__() local_unnamed_addr {
bb:
  %i2 = alloca <{ i64 }>, align 8
  %i4 = alloca [8 x i64], align 16
  %i5 = alloca [4 x i8], align 1
  %i9 = alloca [8 x i64], align 16
  %i20 = alloca [4 x i8], align 1
  %i21 = alloca <{ i64 }>, align 8
  store i32 0, i32* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 0, i32 0), align 16
  store float 1.100000e+01, float* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 0, i32 1), align 4
  store i32 1, i32* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 1, i32 0), align 8
  store float 2.200000e+01, float* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 1, i32 1), align 4
  store i32 2, i32* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 2, i32 0), align 16
  store float 3.300000e+01, float* getelementptr inbounds ([3 x %"CRAY_POINTER10_M1$.btSTR"], [3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA", i64 0, i64 2, i32 1), align 4
  %i41 = bitcast [8 x i64]* %i9 to i8*
  %i52 = bitcast [8 x i64]* %i4 to i8*
  %i53 = getelementptr inbounds [4 x i8], [4 x i8]* %i5, i64 0, i64 0
  %i60 = getelementptr inbounds [4 x i8], [4 x i8]* %i5, i64 0, i64 1
  %i61 = getelementptr inbounds [4 x i8], [4 x i8]* %i5, i64 0, i64 2
  %i62 = getelementptr inbounds [4 x i8], [4 x i8]* %i5, i64 0, i64 3
  store i64 ptrtoint ([3 x %"CRAY_POINTER10_M1$.btSTR"]* @"cray_pointer10_$RA" to i64), i64* @cray_pointer10_m1_mp_preca_, align 8
  %i98 = getelementptr inbounds [4 x i8], [4 x i8]* %i20, i64 0, i64 0
  store i8 26, i8* %i98, align 1
  %i99 = getelementptr inbounds [4 x i8], [4 x i8]* %i20, i64 0, i64 1
  store i8 1, i8* %i99, align 1
  %i100 = getelementptr inbounds [4 x i8], [4 x i8]* %i20, i64 0, i64 2
  store i8 1, i8* %i100, align 1
  %i101 = getelementptr inbounds [4 x i8], [4 x i8]* %i20, i64 0, i64 3
  store i8 0, i8* %i101, align 1
  %i102 = bitcast <{ i64 }>* %i21 to float*
  store float 2.200000e+01, float* %i102, align 8
  %i103 = bitcast <{ i64 }>* %i21 to i8*
  %i104 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %i41, i32 -2, i64 1239157112576, i8* nonnull %i98, i8* nonnull %i103)
  %i105 = bitcast <{ i64 }>* %i2 to i8*
  %i107 = load %"CRAY_POINTER10_M1$.btSTR"*, %"CRAY_POINTER10_M1$.btSTR"** bitcast (i64* @cray_pointer10_m1_mp_preca_ to %"CRAY_POINTER10_M1$.btSTR"**), align 8
  %i109 = getelementptr inbounds %"CRAY_POINTER10_M1$.btSTR", %"CRAY_POINTER10_M1$.btSTR"* %i107, i64 1, i32 0
  store i32 55, i32* %i109, align 1
  %thisislive = getelementptr inbounds %"CRAY_POINTER10_M1$.btSTR", %"CRAY_POINTER10_M1$.btSTR"* %i107, i64 1, i32 1
  store float 0x4084D4CCC0000000, float* %thisislive, align 1
  store i8 9, i8* %i53, align 1
  store i8 1, i8* %i60, align 1
  store i8 1, i8* %i61, align 1
  store i8 0, i8* %i62, align 1
  %i111 = bitcast <{ i64 }>* %i2 to i32*
  store i32 55, i32* %i111, align 8
  %i112 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %i52, i32 -2, i64 1239157112576, i8* nonnull %i53, i8* nonnull %i105)
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr
