; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check reduction operation with complex type.
;      PROGRAM OMP_TEST
;
;        IMPLICIT NONE
;
;        COMPLEX R( 1 )
;        INTEGER ::NUM_THREADS=4
;
;
;        CALL OMP_SET_DYNAMIC( .FALSE. )
;
;
;        CALL OMP_SET_NUM_THREADS( NUM_THREADS )
;        R( 1 ) = CMPLX(0.123, 0.123)
;C$OMP   PARALLEL REDUCTION( + : R )
;          R( 1 ) = R( 1 ) + 1
;C$OMP   END PARALLEL
;
;        IF( ABS( R( 1 ) - CMPLX(0.123 + NUM_THREADS, 0.123) ) .GT.
;     +      ( 1.0E-05 * ( ABS( R( 1 ) ) +
;     +      ABS( CMPLX(0.123 + NUM_THREADS, 0.123) ) ) ) ) THEN
;          PRINT *, 'R( 1 ): ', R( 1 ), 'expected value: ',
;     +          CMPLX(0.123 + NUM_THREADS, 0.123)
;          PRINT *, '*** TEST FAILED ***'
;          STOP
;        ENDIF
;
;        PRINT *, '*** TEST PASSED ***'
;
;      END



source_filename = "reduction_complex_add.f"
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
  %"omp_test_$R" = alloca [1 x %complex_64bit], align 16
  %"var$2" = alloca i32, align 4
  %addressof = alloca [4 x i8]
  %ARGBLOCK_0 = alloca { i64, i8* }
  %addressof82 = alloca [4 x i8]
  %ARGBLOCK_1 = alloca { %complex_64bit }
  %addressof93 = alloca [4 x i8]
  %ARGBLOCK_2 = alloca { i64, i8* }
  %addressof116 = alloca [4 x i8]
  %ARGBLOCK_3 = alloca { %complex_64bit }
  %"var$3" = alloca i32, align 4
  %addressof127 = alloca [4 x i8]
  %ARGBLOCK_4 = alloca { i64, i8* }
  %"var$4" = alloca i32, align 4
  %addressof146 = alloca [4 x i8]
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:CMPLX"([1 x %complex_64bit]* %"omp_test_$R") ]
  br label %bb18

; CHECK:  store %complex_64bit zeroinitializer, %complex_64bit* %red.cpy.dest.ptr

bb16:                                             ; preds = %bb12
  %ptr_cast = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb17

bb19:                                             ; preds = %bb18
  %func_result4 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast13, i64 1)
  %func_result4_fetch = load %complex_64bit, %complex_64bit* %func_result4
  %func_result4_fetch_comp_0 = extractvalue %complex_64bit %func_result4_fetch, 0
  %add = fadd float %func_result4_fetch_comp_0, 1.000000e+00
  %func_result4_fetch_comp_1 = extractvalue %complex_64bit %func_result4_fetch, 1
  %add6 = fadd float %func_result4_fetch_comp_1, 0.000000e+00
  %insertval = insertvalue %complex_64bit zeroinitializer, float %add, 0
  %insertval8 = insertvalue %complex_64bit %insertval, float %add6, 1
  br label %bb20

bb20:                                             ; preds = %bb19
  %ptr_cast10 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb21

;CHECK:       %{{.*}} = load %complex_64bit, %complex_64bit* %red.cpy.src.ptr{{.*}}
;CHECK-NEXT:  %{{.*}} = load %complex_64bit, %complex_64bit* %red.cpy.dest.ptr{{.*}}
;CHECK-NEXT:  %complex_0{{.*}} = extractvalue %complex_64bit %{{.*}}, 0
;CHECK-NEXT:  %complex_0{{.*}} = extractvalue %complex_64bit %{{.*}}, 0
;CHECK-NEXT:  %add{{.*}} = fadd float %complex_0, %complex_0{{.*}}
;CHECK-NEXT:  %complex_1 = extractvalue %complex_64bit %{{.*}}, 1
;CHECK-NEXT:  %complex_1{{.*}} = extractvalue %complex_64bit %{{.*}}, 1
;CHECK-NEXT:  %add{{.*}} = fadd float %complex_1, %complex_1{{.*}}
;CHECK-NEXT:  %insertval{{.*}} = insertvalue %complex_64bit zeroinitializer, float %add{{.*}}, 0
;CHECK-NEXT:  %insertval{{.*}} = insertvalue %complex_64bit %insertval{{.*}}, float %add{{.*}}, 1
;CHECK-NEXT:  store %complex_64bit %insertval{{.*}}, %complex_64bit* %red.cpy.dest.ptr{{.*}}

bb21:                                             ; preds = %bb20
  %func_result12 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast10, i64 1)
  store %complex_64bit %insertval8, %complex_64bit* %func_result12
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bb22

bb18:                                             ; preds = %bb17
  %ptr_cast13 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb19

bb23:                                             ; preds = %bb22
  %func_result15 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast145, i64 1)
  %func_result15_fetch = load %complex_64bit, %complex_64bit* %func_result15
  %"omp_test_$NUM_THREADS_fetch" = load i32, i32* @"omp_test_$NUM_THREADS"
  %int_cast = sitofp i32 %"omp_test_$NUM_THREADS_fetch" to float
  %add17 = fadd float 0x3FBF7CEDA0000000, %int_cast
  %insertval19 = insertvalue %complex_64bit zeroinitializer, float %add17, 0
  %insertval21 = insertvalue %complex_64bit %insertval19, float 0x3FBF7CEDA0000000, 1
  %func_result15_fetch_comp_0 = extractvalue %complex_64bit %func_result15_fetch, 0
  %insertval21_comp_0 = extractvalue %complex_64bit %insertval21, 0
  %sub = fsub float %func_result15_fetch_comp_0, %insertval21_comp_0
  %func_result15_fetch_comp_1 = extractvalue %complex_64bit %func_result15_fetch, 1
  %insertval21_comp_1 = extractvalue %complex_64bit %insertval21, 1
  %sub23 = fsub float %func_result15_fetch_comp_1, %insertval21_comp_1
  %insertval25 = insertvalue %complex_64bit zeroinitializer, float %sub, 0
  %insertval27 = insertvalue %complex_64bit %insertval25, float %sub23, 1
  %insertval27_comp_0 = extractvalue %complex_64bit %insertval27, 0
  %insertval27_comp_1 = extractvalue %complex_64bit %insertval27, 1
  %mul = fmul float %insertval27_comp_1, %insertval27_comp_1
  %mul29 = fmul float %insertval27_comp_0, %insertval27_comp_0
  %add31 = fadd float %mul29, %mul
  br label %bb24

bb24:                                             ; preds = %bb23
  br label %bb25

bb25:                                             ; preds = %bb24
  %func_result33 = call float @sqrtf(float %add31)
  br label %bb26

bb26:                                             ; preds = %bb25
  %ptr_cast35 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb27

bb27:                                             ; preds = %bb26
  %func_result37 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast35, i64 1)
  %func_result37_fetch = load %complex_64bit, %complex_64bit* %func_result37
  %func_result37_fetch_comp_0 = extractvalue %complex_64bit %func_result37_fetch, 0
  %func_result37_fetch_comp_1 = extractvalue %complex_64bit %func_result37_fetch, 1
  %mul39 = fmul float %func_result37_fetch_comp_1, %func_result37_fetch_comp_1
  %mul41 = fmul float %func_result37_fetch_comp_0, %func_result37_fetch_comp_0
  %add43 = fadd float %mul41, %mul39
  br label %bb28

bb28:                                             ; preds = %bb27
  br label %bb29

bb29:                                             ; preds = %bb28
  %func_result45 = call float @sqrtf(float %add43)
  %"omp_test_$NUM_THREADS_fetch47" = load i32, i32* @"omp_test_$NUM_THREADS"
  %int_cast49 = sitofp i32 %"omp_test_$NUM_THREADS_fetch47" to float
  %add51 = fadd float 0x3FBF7CEDA0000000, %int_cast49
  %insertval53 = insertvalue %complex_64bit zeroinitializer, float %add51, 0
  %insertval55 = insertvalue %complex_64bit %insertval53, float 0x3FBF7CEDA0000000, 1
  %insertval55_comp_0 = extractvalue %complex_64bit %insertval55, 0
  %insertval55_comp_1 = extractvalue %complex_64bit %insertval55, 1
  %mul57 = fmul float %insertval55_comp_1, %insertval55_comp_1
  %mul59 = fmul float %insertval55_comp_0, %insertval55_comp_0
  %add61 = fadd float %mul59, %mul57
  br label %bb30

bb30:                                             ; preds = %bb29
  br label %bb31

bb31:                                             ; preds = %bb30
  %func_result63 = call float @sqrtf(float %add61)
  %add65 = fadd float %func_result45, %func_result63
  %mul67 = fmul float 0x3EE4F8B580000000, %add65
  %rel = fcmp ogt float %func_result33, %mul67
  %int_zext = zext i1 %rel to i32
  %int_zext144 = trunc i32 %int_zext to i1
  br i1 %int_zext144, label %bb33_then, label %bb50_else

bb34:                                             ; preds = %bb33_then
  %BLKFIELD_ = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i32 0, i32 0
  store i64 8, i64* %BLKFIELD_
  %BLKFIELD_69 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_0, i32 0, i32 1
  store i8* getelementptr inbounds ([8 x i8], [8 x i8]* @3, i32 0, i32 0), i8** %BLKFIELD_69
  br label %bb35

bb35:                                             ; preds = %bb34
  %ptr_cast71 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast73 = bitcast [4 x i8]* %addressof to i8*
  %ptr_cast75 = bitcast { i64, i8* }* %ARGBLOCK_0 to i8*
  %func_result77 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast71, i32 -1, i64 1239157112576, i8* %ptr_cast73, i8* %ptr_cast75)
  br label %bb36

bb36:                                             ; preds = %bb35
  %ptr_cast79 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb37

bb37:                                             ; preds = %bb36
  %func_result81 = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) %ptr_cast79, i64 1)
  %func_result81_fetch = load %complex_64bit, %complex_64bit* %func_result81
  store [4 x i8] c"\1D\01\02\00", [4 x i8]* %addressof82
  br label %bb38

bb38:                                             ; preds = %bb37
  %BLKFIELD_84 = getelementptr inbounds { %complex_64bit }, { %complex_64bit }* %ARGBLOCK_1, i32 0, i32 0
  store %complex_64bit %func_result81_fetch, %complex_64bit* %BLKFIELD_84
  br label %bb39

bb39:                                             ; preds = %bb38
  %ptr_cast86 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast88 = bitcast [4 x i8]* %addressof82 to i8*
  %ptr_cast90 = bitcast { %complex_64bit }* %ARGBLOCK_1 to i8*
  %func_result92 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast86, i8* %ptr_cast88, i8* %ptr_cast90)
  store [4 x i8] c"8\04\02\00", [4 x i8]* %addressof93
  br label %bb40

bb40:                                             ; preds = %bb39
  %BLKFIELD_95 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_2, i32 0, i32 0
  store i64 16, i64* %BLKFIELD_95
  %BLKFIELD_97 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_2, i32 0, i32 1
  store i8* getelementptr inbounds ([16 x i8], [16 x i8]* @2, i32 0, i32 0), i8** %BLKFIELD_97
  br label %bb41

bb41:                                             ; preds = %bb40
  %ptr_cast99 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast101 = bitcast [4 x i8]* %addressof93 to i8*
  %ptr_cast103 = bitcast { i64, i8* }* %ARGBLOCK_2 to i8*
  %func_result105 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast99, i8* %ptr_cast101, i8* %ptr_cast103)
  %"omp_test_$NUM_THREADS_fetch107" = load i32, i32* @"omp_test_$NUM_THREADS"
  %int_cast109 = sitofp i32 %"omp_test_$NUM_THREADS_fetch107" to float
  %add111 = fadd float 0x3FBF7CEDA0000000, %int_cast109
  %insertval113 = insertvalue %complex_64bit zeroinitializer, float %add111, 0
  %insertval115 = insertvalue %complex_64bit %insertval113, float 0x3FBF7CEDA0000000, 1
  store [4 x i8] c"\1D\01\01\00", [4 x i8]* %addressof116
  br label %bb42

bb42:                                             ; preds = %bb41
  %BLKFIELD_118 = getelementptr inbounds { %complex_64bit }, { %complex_64bit }* %ARGBLOCK_3, i32 0, i32 0
  store %complex_64bit %insertval115, %complex_64bit* %BLKFIELD_118
  br label %bb43

bb43:                                             ; preds = %bb42
  %ptr_cast120 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast122 = bitcast [4 x i8]* %addressof116 to i8*
  %ptr_cast124 = bitcast { %complex_64bit }* %ARGBLOCK_3 to i8*
  %func_result126 = call i32 @for_write_seq_lis_xmit(i8* %ptr_cast120, i8* %ptr_cast122, i8* %ptr_cast124)
  br label %bb32

bb32:                                             ; preds = %bb43
  br label %bb45

bb46:                                             ; preds = %bb45
  %BLKFIELD_129 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_4, i32 0, i32 0
  store i64 19, i64* %BLKFIELD_129
  %BLKFIELD_131 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_4, i32 0, i32 1
  store i8* getelementptr inbounds ([19 x i8], [19 x i8]* @1, i32 0, i32 0), i8** %BLKFIELD_131
  br label %bb47

bb47:                                             ; preds = %bb46
  %ptr_cast133 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast135 = bitcast [4 x i8]* %addressof127 to i8*
  %ptr_cast137 = bitcast { i64, i8* }* %ARGBLOCK_4 to i8*
  %func_result139 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast133, i32 -1, i64 1239157112576, i8* %ptr_cast135, i8* %ptr_cast137)
  br label %bb44

bb44:                                             ; preds = %bb47
  %strlit140 = load [0 x i8], [0 x i8]* @6
  br label %bb48

bb45:                                             ; preds = %bb32
  store [4 x i8] c"8\04\01\00", [4 x i8]* %addressof127
  br label %bb46

bb48:                                             ; preds = %bb44
  br label %bb49

bb49:                                             ; preds = %bb48
  %func_result142 = call i32 (i8*, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(i8* getelementptr inbounds ([0 x i8], [0 x i8]* @6, i32 0, i32 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0)
  br label %bb51_endif

bb33_then:                                        ; preds = %bb31
  store [4 x i8] c"8\04\02\00", [4 x i8]* %addressof
  br label %bb34

bb50_else:                                        ; preds = %bb31
  br label %bb51_endif

bb51_endif:                                       ; preds = %bb50_else, %bb49
  br label %bb53

bb22:                                             ; preds = %bb21
  %ptr_cast145 = bitcast [1 x %complex_64bit]* %"omp_test_$R" to %complex_64bit*
  br label %bb23

bb54:                                             ; preds = %bb53
  %BLKFIELD_148 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_5, i32 0, i32 0
  store i64 19, i64* %BLKFIELD_148
  %BLKFIELD_150 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %ARGBLOCK_5, i32 0, i32 1
  store i8* getelementptr inbounds ([19 x i8], [19 x i8]* @0, i32 0, i32 0), i8** %BLKFIELD_150
  br label %bb55

bb55:                                             ; preds = %bb54
  %ptr_cast152 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast154 = bitcast [4 x i8]* %addressof146 to i8*
  %ptr_cast156 = bitcast { i64, i8* }* %ARGBLOCK_5 to i8*
  %func_result158 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast152, i32 -1, i64 1239157112576, i8* %ptr_cast154, i8* %ptr_cast156)
  br label %bb52

bb52:                                             ; preds = %bb55
  br label %bb1

bb53:                                             ; preds = %bb51_endif
  store [4 x i8] c"8\04\01\00", [4 x i8]* %addressof146
  br label %bb54

bb1:                                              ; preds = %bb52
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
