; RUN: opt -S -mcpu=skylake-avx512 -passes="vec-clone,vplan-vec" < %s | FileCheck %s
; RUN: opt -S -mcpu=skylake-avx512 -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1| FileCheck %s --check-prefix=HIR
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i1 @foo(i1 %i1) #0 {
entry:
  %v = and i1 %i1, 1
  ret i1 %v
}

; Function Attrs: nounwind uwtable
define dso_local void @baz(ptr %arr, i1 %i1) {
; CHECK-LABEL:  VPlannedBB3:                                      ; preds = %vector.body
; CHECK-NEXT:     [[TMP1:%.*]] = zext <16 x i1> [[TMP0:%.*]] to <16 x i8>
; CHECK-NEXT:     [[MASKEXT:%.*]] = sext <16 x i1> [[TMP0]] to <16 x i8>
; CHECK-NEXT:     [[TMP2:%.*]] = trunc <16 x i8>  [[MASKEXT]] to <16 x i1>
; CHECK-NEXT:     [[TMP3:%.*]] = bitcast <16 x i1> [[TMP2]] to i16
; CHECK-NEXT:     [[TMP4:%.*]] = zext i16 [[TMP3]] to i32
; CHECK-NEXT:     [[TMP5:%.*]] = call x86_regcallcc <16 x i8> @_ZGVZM16v_foo(<16 x i8> %1, i32 [[TMP4]])
; CHECK-NEXT:     [[TRUNC:%.*]] = trunc <16 x i8> [[TMP5]] to <16 x i1>

; CHECK:        define x86_regcallcc <16 x i8> @_ZGVZM16v_foo(<16 x i8> [[I1:%.*]], i32 [[MASK:%.*]])
; CHECK-NOT:    define <16 x i8> @_ZGVZM16v_foo(<16 x i8> [[I1]], <16 x i8> [[MASK]])

; HIR:          + DO i1 = 0, 1023, 16   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:     |   [[VEC:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> > 42;
; HIR-NEXT:     |   [[VEC2:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> > 42;
; HIR-NEXT:     |   [[SEXT:%.*]] = sext.<16 x i1>.<16 x i8>([[VEC]]);
; HIR-NEXT:     |   [[SEXT2:%.*]] = sext.<16 x i1>.<16 x i8>([[VEC2]]);
; HIR-NEXT:     |   [[TRUNC:%.*]] = trunc.<16 x i8>.<16 x i1>([[SEXT2]]);
; HIR-NEXT:     |   [[TMP0:%.*]] = bitcast.<16 x i1>.i16([[TRUNC]]);
; HIR-NEXT:     |   [[ZEXT:%.*]] = zext.i16.i32([[TMP0]]);
; HIR-NEXT:     |   [[FOO:%.*]] = @_ZGVZM16v_foo([[SEXT]],  [[ZEXT]]);
; HIR-NEXT:     + END LOOP


for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.28
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body

for.body:                               ; preds = %for.body.lr.ph, %for.inc
  %iv = phi i64 [ 0, %for.body.lr.ph ], [ %add4, %for.inc ]
  %cmp1 = icmp sgt i64 %iv, 42
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %call.i = call i1 @foo(i1 %cmp1) #3
  br label %for.inc

for.inc:                                ; preds = %if.then, %for.body
  %add4 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add4, 1024
  br i1 %exitcond.not, label %exit, label %for.body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #3
declare void @llvm.directive.region.exit(token) #3
attributes #0 = { "vector-variants"="_ZGVZM16v_foo" }
