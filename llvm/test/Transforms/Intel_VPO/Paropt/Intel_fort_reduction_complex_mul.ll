; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check reduction operation with complex type.
;      PROGRAM OMP_TEST
;
;        IMPLICIT NONE
;
;        COMPLEX R( 1 )
;        COMPLEX TMP
;        INTEGER ::NUM_THREADS=4
;
;        CALL OMP_SET_DYNAMIC( .FALSE. )
;
;        CALL OMP_SET_NUM_THREADS( NUM_THREADS )
;        R( 1 ) = CMPLX(0.123, 0.123)
;C$OMP   PARALLEL REDUCTION( * : R )
;          R( 1 ) = R( 1 ) * NUM_THREADS
;C$OMP   END PARALLEL
;
;C       calculate correct result
;        TMP = CMPLX(0.123, 0.123)
;        TMP = TMP * NUM_THREADS
;
;        IF( ABS( R( 1 ) - TMP ) .GT.
;     +      ( 1.0E-05 * ( ABS( R( 1 ) ) +
;     +      ABS( TMP ) ) ) ) THEN
;          PRINT *, 'R( 1 ): ', R( 1 ), 'expected value: ',
;     +          TMP
;          PRINT *, '*** TEST FAILED ***'
;          STOP
;        ENDIF
;
;        PRINT *, '*** TEST PASSED ***'
;
;      END


source_filename = "reduction_complex_mul.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

@0 = internal unnamed_addr constant [19 x i8] c"*** TEST PASSED ***"
@1 = internal unnamed_addr constant [19 x i8] c"*** TEST FAILED ***"
@2 = internal unnamed_addr constant [16 x i8] c"expected value: "
@3 = internal unnamed_addr constant [8 x i8] c"R( 1 ): "
@"omp_test_$NUM_THREADS" = internal global i32 4, align 8
@4 = internal unnamed_addr constant i32 2
@5 = internal unnamed_addr constant i32 0
@6 = internal unnamed_addr constant [0 x i8] zeroinitializer

define void @MAIN__() #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 16
  %"omp_test_$TMP" = alloca %complex_64bit, align 8
  %"omp_test_$R" = alloca [1 x %complex_64bit], align 16
  %"var$2" = alloca i32, align 4
  %addressof = alloca [4 x i8]
  %ARGBLOCK_0 = alloca { i64, i8* }
  %addressof94 = alloca [4 x i8]
  %ARGBLOCK_1 = alloca { %complex_64bit }
  %addressof105 = alloca [4 x i8]
  %ARGBLOCK_2 = alloca { i64, i8* }
  %addressof120 = alloca [4 x i8]
  %ARGBLOCK_3 = alloca { %complex_64bit }
  %"var$3" = alloca i32, align 4
  %addressof131 = alloca [4 x i8]
  %ARGBLOCK_4 = alloca { i64, i8* }
  %"var$4" = alloca i32, align 4
  %addressof150 = alloca [4 x i8]
  %ARGBLOCK_5 = alloca { i64, i8* }
  %strlit = load [19 x i8], [19 x i8]* @0
  %strlit1 = load [19 x i8], [19 x i8]* @1
  %strlit2 = load [16 x i8], [16 x i8]* @2
  %strlit3 = load [8 x i8], [8 x i8]* @3
  br label %bb2

bb2:                                              ; preds = %alloca
  br label %bb3

bb3:                                              ; preds = %bb2
  %func_result = call i32 @for_set_reentrancy(i32* @4)
  br label %bb4

bb7:                                              ; preds = %bb8
  br label %bb9

bb5:                                              ; preds = %bb4
  br label %bb8

bb8:                                              ; preds = %bb5
  br label %bb7

bb9:                                              ; preds = %bb7
  call void @omp_set_dynamic_(i32* @5)
  br label %bb6

bb6:                                              ; preds = %bb9
  br label %bb10

bb4:                                              ; preds = %bb3
  br label %bb5

bb13:                                             ; preds = %bb14
  br label %bb15

bb11:                                             ; preds = %bb10
  br label %bb14

bb14:                                             ; preds = %bb11
  br label %bb13

bb15:                                             ; preds = %bb13
  call void @omp_set_num_threads_(i32* @"omp_test_$NUM_THREADS")
  br label %bb12

bb12:                                             ; preds = %bb15
  br label %bb16

bb10:                                             ; preds = %bb6
  br label %bb11

bb17:                                             ; preds = %bb16
  %func_result2 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast, i64 1)
  store %complex_64bit { float 0x3FBF7CEDA0000000, float 0x3FBF7CEDA0000000 }, %complex_64bit* %func_result2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.MUL:CMPLX"([1 x %complex_64bit]* %"omp_test_$R"), "QUAL.OMP.SHARED"(i32* @"omp_test_$NUM_THREADS") ]
  br label %bb18

; CHECK:  store %complex_64bit { float 1.000000e+00, float 0.000000e+00 }, %complex_64bit* %red.cpy.dest.ptr

bb16:                                             ; preds = %bb12
  %ptr_cast = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb17

bb19:                                             ; preds = %bb18
  %func_result4 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast21, i64 1)
  %func_result4_fetch = load %complex_64bit, %complex_64bit* %func_result4
  %"omp_test_$NUM_THREADS_fetch" = load i32, i32* @"omp_test_$NUM_THREADS"
  %int_cast = sitofp i32 %"omp_test_$NUM_THREADS_fetch" to float
  %insertval = insertvalue %complex_64bit zeroinitializer, float %int_cast, 0
  %insertval6 = insertvalue %complex_64bit %insertval, float 0.000000e+00, 1
  %func_result4_fetch_comp_0 = extractvalue %complex_64bit %func_result4_fetch, 0
  %insertval6_comp_0 = extractvalue %complex_64bit %insertval6, 0
  %func_result4_fetch_comp_1 = extractvalue %complex_64bit %func_result4_fetch, 1
  %insertval6_comp_1 = extractvalue %complex_64bit %insertval6, 1
  %mul = fmul float %func_result4_fetch_comp_1, %insertval6_comp_1
  %mul8 = fmul float %func_result4_fetch_comp_0, %insertval6_comp_0
  %sub = fsub float %mul8, %mul
  %mul10 = fmul float %func_result4_fetch_comp_1, %insertval6_comp_0
  %mul12 = fmul float %func_result4_fetch_comp_0, %insertval6_comp_1
  %add = fadd float %mul12, %mul10
  %insertval14 = insertvalue %complex_64bit zeroinitializer, float %sub, 0
  %insertval16 = insertvalue %complex_64bit %insertval14, float %add, 1
  br label %bb20

bb20:                                             ; preds = %bb19
  %ptr_cast18 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb21

; CHECK:       %{{.*}} = load %complex_64bit, %complex_64bit* %red.cpy.src.ptr
; CHECK-NEXT:  %{{.*}} = load %complex_64bit, %complex_64bit* %red.cpy.dest.ptr{{.*}}
; CHECK-NEXT:  %complex_0 = extractvalue %complex_64bit %{{.*}}, 0
; CHECK-NEXT:  %complex_0{{.*}} = extractvalue %complex_64bit %{{.*}}, 0
; CHECK-NEXT:  %complex_1 = extractvalue %complex_64bit %{{.*}}, 1
; CHECK-NEXT:  %complex_1{{.*}} = extractvalue %complex_64bit %{{.*}}, 1
; CHECK-NEXT:  %mul{{.*}} = fmul float %complex_0, %complex_0{{.*}}
; CHECK-NEXT:  %mul{{.*}} = fmul float %complex_1, %complex_1{{.*}}
; CHECK-NEXT:  %mul{{.*}} = fmul float %complex_0, %complex_1{{.*}}
; CHECK-NEXT:  %mul{{.*}} = fmul float %complex_1, %complex_0{{.*}}
; CHECK-NEXT:  %sub{{.*}} = fsub float %mul{{.*}}, %mul{{.*}}
; CHECK-NEXT:  %add{{.*}} = fadd float %mul{{.*}}, %mul{{.*}}
; CHECK-NEXT:  %insertval{{.*}} = insertvalue %complex_64bit zeroinitializer, float %sub{{.*}}, 0
; CHECK-NEXT:  %insertval{{.*}} = insertvalue %complex_64bit %insertval{{.*}}, float %add{{.*}}, 1
; CHECK-NEXT:  store %complex_64bit %insertval{{.*}}, %complex_64bit* %red.cpy.dest.ptr{{.*}}


bb21:                                             ; preds = %bb20
  %func_result20 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast18, i64 1)
  store %complex_64bit %insertval16, %complex_64bit* %func_result20
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bb22

bb18:                                             ; preds = %bb17
  %ptr_cast21 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb19

bb22:                                             ; preds = %bb21
  store %complex_64bit { float 0x3FBF7CEDA0000000, float 0x3FBF7CEDA0000000 }, %complex_64bit* %"omp_test_$TMP"
  br label %bb23

bb23:                                             ; preds = %bb22
  %"omp_test_$TMP_fetch" = load %complex_64bit, %complex_64bit* %"omp_test_$TMP"
  %"omp_test_$NUM_THREADS_fetch22" = load i32, i32* @"omp_test_$NUM_THREADS"
  %int_cast23 = sitofp i32 %"omp_test_$NUM_THREADS_fetch22" to float
  %insertval24 = insertvalue %complex_64bit zeroinitializer, float %int_cast23, 0
  %insertval25 = insertvalue %complex_64bit %insertval24, float 0.000000e+00, 1
  %"omp_test_$TMP_fetch_comp_0" = extractvalue %complex_64bit %"omp_test_$TMP_fetch", 0
  %insertval_comp_0 = extractvalue %complex_64bit %insertval25, 0
  %"omp_test_$TMP_fetch_comp_1" = extractvalue %complex_64bit %"omp_test_$TMP_fetch", 1
  %insertval_comp_1 = extractvalue %complex_64bit %insertval25, 1
  %mul26 = fmul float %"omp_test_$TMP_fetch_comp_1", %insertval_comp_1
  %mul27 = fmul float %"omp_test_$TMP_fetch_comp_0", %insertval_comp_0
  %sub28 = fsub float %mul27, %mul26
  %mul29 = fmul float %"omp_test_$TMP_fetch_comp_1", %insertval_comp_0
  %mul30 = fmul float %"omp_test_$TMP_fetch_comp_0", %insertval_comp_1
  %add31 = fadd float %mul30, %mul29
  %insertval32 = insertvalue %complex_64bit zeroinitializer, float %sub28, 0
  %insertval33 = insertvalue %complex_64bit %insertval32, float %add31, 1
  store %complex_64bit %insertval33, %complex_64bit* %"omp_test_$TMP"
  br label %bb24

bb25:                                             ; preds = %bb24
  %func_result35 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast149, i64 1)
  %func_result35_fetch = load %complex_64bit, %complex_64bit* %func_result35
  %"omp_test_$TMP_fetch37" = load %complex_64bit, %complex_64bit* %"omp_test_$TMP"
  %func_result35_fetch_comp_0 = extractvalue %complex_64bit %func_result35_fetch, 0
  %"omp_test_$TMP_fetch37_comp_0" = extractvalue %complex_64bit %"omp_test_$TMP_fetch37", 0
  %sub39 = fsub float %func_result35_fetch_comp_0, %"omp_test_$TMP_fetch37_comp_0"
  %func_result35_fetch_comp_1 = extractvalue %complex_64bit %func_result35_fetch, 1
  %"omp_test_$TMP_fetch37_comp_1" = extractvalue %complex_64bit %"omp_test_$TMP_fetch37", 1
  %sub41 = fsub float %func_result35_fetch_comp_1, %"omp_test_$TMP_fetch37_comp_1"
  %insertval43 = insertvalue %complex_64bit zeroinitializer, float %sub39, 0
  %insertval45 = insertvalue %complex_64bit %insertval43, float %sub41, 1
  %insertval45_comp_0 = extractvalue %complex_64bit %insertval45, 0
  %insertval45_comp_1 = extractvalue %complex_64bit %insertval45, 1
  %mul47 = fmul float %insertval45_comp_1, %insertval45_comp_1
  %mul49 = fmul float %insertval45_comp_0, %insertval45_comp_0
  %add51 = fadd float %mul49, %mul47
  br label %bb26

bb26:                                             ; preds = %bb25
  br label %bb27

bb27:                                             ; preds = %bb26
  %func_result53 = call float @sqrtf(float %add51)
  br label %bb28

bb28:                                             ; preds = %bb27
  %ptr_cast55 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb29

bb29:                                             ; preds = %bb28
  %func_result57 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast55, i64 1)
  %func_result57_fetch = load %complex_64bit, %complex_64bit* %func_result57
  %func_result57_fetch_comp_0 = extractvalue %complex_64bit %func_result57_fetch, 0
  %func_result57_fetch_comp_1 = extractvalue %complex_64bit %func_result57_fetch, 1
  %mul59 = fmul float %func_result57_fetch_comp_1, %func_result57_fetch_comp_1
  %mul61 = fmul float %func_result57_fetch_comp_0, %func_result57_fetch_comp_0
  %add63 = fadd float %mul61, %mul59
  br label %bb30

bb30:                                             ; preds = %bb29
  br label %bb31

bb31:                                             ; preds = %bb30
  %func_result65 = call float @sqrtf(float %add63)
  %"omp_test_$TMP_fetch67" = load %complex_64bit, %complex_64bit* %"omp_test_$TMP"
  %"omp_test_$TMP_fetch67_comp_0" = extractvalue %complex_64bit %"omp_test_$TMP_fetch67", 0
  %"omp_test_$TMP_fetch67_comp_1" = extractvalue %complex_64bit %"omp_test_$TMP_fetch67", 1
  %mul69 = fmul float %"omp_test_$TMP_fetch67_comp_1", %"omp_test_$TMP_fetch67_comp_1"
  %mul71 = fmul float %"omp_test_$TMP_fetch67_comp_0", %"omp_test_$TMP_fetch67_comp_0"
  %add73 = fadd float %mul71, %mul69
  br label %bb32

bb32:                                             ; preds = %bb31
  br label %bb33

bb33:                                             ; preds = %bb32
  %func_result75 = call float @sqrtf(float %add73)
  %add77 = fadd float %func_result65, %func_result75
  %mul79 = fmul float 0x3EE4F8B580000000, %add77
  %rel = fcmp ogt float %func_result53, %mul79
  %int_zext = zext i1 %rel to i32
  %int_zext148 = trunc i32 %int_zext to i1
  br i1 %int_zext148, label %bb35_then, label %bb52_else

bb36:                                             ; preds = %bb35_then
  %BLKFIELD_ = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i32 0, i32 0
  store i64 8, i64* %BLKFIELD_
  %BLKFIELD_81 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i32 0, i32 1
  store i8* getelementptr inbounds ([8 x i8], [8 x i8]* @3, i32 0, i32 0), i8** %BLKFIELD_81
  br label %bb37

bb37:                                             ; preds = %bb36
  %ptr_cast83 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast85 = bitcast [4 x i8]* %addressof to i8*
  %ptr_cast87 = bitcast { i64, i8* }* %ARGBLOCK_0 to i8*
  %func_result89 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast83, i32 -1, i64 1239157112576, i8* %ptr_cast85, i8* %ptr_cast87)
  br label %bb38

bb38:                                             ; preds = %bb37
  %ptr_cast91 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb39

bb39:                                             ; preds = %bb38
  %func_result93 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast91, i64 1)
  %func_result93_fetch = load %complex_64bit, %complex_64bit* %func_result93
  store [4 x i8] c"\1D\01\02\00", [4 x i8]* %addressof94
  br label %bb40

bb40:                                             ; preds = %bb39
  %BLKFIELD_96 = getelementptr inbounds { %complex_64bit }, { %complex_64bit }* %ARGBLOCK_1, i32 0, i32 0
  store %complex_64bit %func_result93_fetch, %complex_64bit* %BLKFIELD_96
  br label %bb41

bb41:                                             ; preds = %bb40
  %ptr_cast98 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast100 = bitcast [4 x i8]* %addressof94 to i8*
  %ptr_cast102 = bitcast { %complex_64bit }* %ARGBLOCK_1 to i8*
  %func_result104 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast98, i8* %ptr_cast100, i8* %ptr_cast102)
  store [4 x i8] c"8\04\02\00", [4 x i8]* %addressof105
  br label %bb42

bb42:                                             ; preds = %bb41
  %BLKFIELD_107 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_2, i32 0, i32 0
  store i64 16, i64* %BLKFIELD_107
  %BLKFIELD_109 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_2, i32 0, i32 1
  store i8* getelementptr inbounds ([16 x i8], [16 x i8]* @2, i32 0, i32 0), i8** %BLKFIELD_109
  br label %bb43

bb43:                                             ; preds = %bb42
  %ptr_cast111 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast113 = bitcast [4 x i8]* %addressof105 to i8*
  %ptr_cast115 = bitcast { i64, i8* }* %ARGBLOCK_2 to i8*
  %func_result117 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast111, i8* %ptr_cast113, i8* %ptr_cast115)
  %"omp_test_$TMP_fetch119" = load %complex_64bit, %complex_64bit* %"omp_test_$TMP"
  store [4 x i8] c"\1D\01\01\00", [4 x i8]* %addressof120
  br label %bb44

bb44:                                             ; preds = %bb43
  %BLKFIELD_122 = getelementptr inbounds { %complex_64bit }, { %complex_64bit }* %ARGBLOCK_3, i32 0, i32 0
  store %complex_64bit %"omp_test_$TMP_fetch119", %complex_64bit* %BLKFIELD_122
  br label %bb45

bb45:                                             ; preds = %bb44
  %ptr_cast124 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast126 = bitcast [4 x i8]* %addressof120 to i8*
  %ptr_cast128 = bitcast { %complex_64bit }* %ARGBLOCK_3 to i8*
  %func_result130 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast124, i8* %ptr_cast126, i8* %ptr_cast128)
  br label %bb34

bb34:                                             ; preds = %bb45
  br label %bb47

bb48:                                             ; preds = %bb47
  %BLKFIELD_133 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_4, i32 0, i32 0
  store i64 19, i64* %BLKFIELD_133
  %BLKFIELD_135 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_4, i32 0, i32 1
  store i8* getelementptr inbounds ([19 x i8], [19 x i8]* @1, i32 0, i32 0), i8** %BLKFIELD_135
  br label %bb49

bb49:                                             ; preds = %bb48
  %ptr_cast137 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast139 = bitcast [4 x i8]* %addressof131 to i8*
  %ptr_cast141 = bitcast { i64, i8* }* %ARGBLOCK_4 to i8*
  %func_result143 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast137, i32 -1, i64 1239157112576, i8* %ptr_cast139, i8* %ptr_cast141)
  br label %bb46

bb46:                                             ; preds = %bb49
  %strlit144 = load [0 x i8], [0 x i8]* @6
  br label %bb50

bb47:                                             ; preds = %bb34
  store [4 x i8] c"8\04\01\00", [4 x i8]* %addressof131
  br label %bb48

bb50:                                             ; preds = %bb46
  br label %bb51

bb51:                                             ; preds = %bb50
  %func_result146 = call i32 (i8*, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(i8* getelementptr inbounds ([0 x i8], [0 x i8]* @6, i32 0, i32 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0)
  br label %bb53_endif

bb35_then:                                        ; preds = %bb33
  store [4 x i8] c"8\04\02\00", [4 x i8]* %addressof
  br label %bb36

bb52_else:                                        ; preds = %bb33
  br label %bb53_endif

bb53_endif:                                       ; preds = %bb52_else, %bb51
  br label %bb55

bb24:                                             ; preds = %bb23
  %ptr_cast149 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb25

bb56:                                             ; preds = %bb55
  %BLKFIELD_152 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_5, i32 0, i32 0
  store i64 19, i64* %BLKFIELD_152
  %BLKFIELD_154 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_5, i32 0, i32 1
  store i8* getelementptr inbounds ([19 x i8], [19 x i8]* @0, i32 0, i32 0), i8** %BLKFIELD_154
  br label %bb57

bb57:                                             ; preds = %bb56
  %ptr_cast156 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast158 = bitcast [4 x i8]* %addressof150 to i8*
  %ptr_cast160 = bitcast { i64, i8* }* %ARGBLOCK_5 to i8*
  %func_result162 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast156, i32 -1, i64 1239157112576, i8* %ptr_cast158, i8* %ptr_cast160)
  br label %bb54

bb54:                                             ; preds = %bb57
  br label %bb1

bb55:                                             ; preds = %bb53_endif
  store [4 x i8] c"8\04\01\00", [4 x i8]* %addressof150
  br label %bb56

bb1:                                              ; preds = %bb54
  ret void
}

declare i32 @for_set_reentrancy(i32*)

declare void @omp_set_dynamic_(i32*)

declare void @omp_set_num_threads_(i32*)

; Function Attrs: nounwind readnone speculatable
declare %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8, i64, i64, %complex_64bit*, i64) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare float @sqrtf(float)

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...)

declare i32 @for_write_seq_lis_xmit(i8*, i8*, i8*)

declare i32 @for_stop_core_quiet(i8*, i32, i32, i64, i32, i32, ...)

attributes #0 = { "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{}

; end INTEL_CUSTOMIZATION
