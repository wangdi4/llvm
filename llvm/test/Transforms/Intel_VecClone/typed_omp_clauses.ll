; RUN: opt -vec-clone -vec-clone-typed-omp -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -vec-clone-typed-omp -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.pair = type { i32, i32 }


define i32 @test1(%struct.pair* byval(%struct.pair) %x) #0 {
; CHECK: _ZGVbN4u_test1(%struct.pair* byval(%struct.pair) %x)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(%struct.pair* %x, %struct.pair zeroinitializer, i32 1) ]
; CHECK: _ZGVbN4v_test1(<4 x %struct.pair*> byval(%struct.pair) %x)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  %fst.p = getelementptr inbounds %struct.pair, %struct.pair* %x, i32 0, i32 0
  %snd.p = getelementptr inbounds %struct.pair, %struct.pair* %x, i32 0, i32 1
  %fst = load i32, i32* %fst.p, align 4
  %snd = load i32, i32* %snd.p, align 4
  %sum = add i32 %fst, %snd
  ret i32 %sum
}

define i64 @test2(i64 %a) #1 {
; CHECK: _ZGVbN4u_test2(i64 %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(i64* %alloca.a, i64 0, i32 1) ]
; CHECK: _ZGVbN4v_test2(<4 x i64> %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
; CHECK: _ZGVbN4l_test2(i64 %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED"(i64* %alloca.a, i64 0, i32 1, i64 1) ]
; CHECK: _ZGVbN4l4_test2(i64 %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED"(i64* %alloca.a, i64 0, i32 1, i64 4) ]
  %add = add i64 %a, 42
  ret i64 %add
}

define float @test3(float %a) #2 {
; CHECK: _ZGVbN4u_test3(float %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(float* %alloca.a, float 0.000000e+00, i32 1) ]
; CHECK: _ZGVbN4v_test3(<4 x float> %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  %add = fadd float %a, 42.0
  ret float %add
}

define i64 @test4(i64 %a) #3 {
; CHECK: _ZGVbN4v_test4(<4 x i64> %a)
; CHECK:    @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.PRIVATE:TYPED"(i64* %priv, i64 0, i32 1) ]
  %priv = alloca i64
  %x = add i64 %a, 42
  store i64 %x, i64 *%priv
  %ld = load i64, i64 *%priv
  %add2 = add i64 %ld, 2
  ret i64 %add2
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4u_test1,_ZGVbN4v_test1" }
attributes #1 = { nounwind uwtable "vector-variants"="_ZGVbN4u_test2,_ZGVbN4v_test2,_ZGVbN4l_test2,_ZGVbN4l4_test2" }
attributes #2 = { nounwind uwtable "vector-variants"="_ZGVbN4u_test3,_ZGVbN4v_test3" }
attributes #3 = { nounwind uwtable "vector-variants"="_ZGVbN4v_test4" }
