; RUN: opt < %s -dope-vector-local-const-prop=false -disable-output -dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2

; The test checks correctness of OpaquePointerTypeMapper constructor
; when a CallBase is a User of a pointer, but the pointer is not an argument of the CallBase.
; The test should compile w/o an error

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @test() "intel-lang"="fortran" {
  %1 = alloca i32, align 8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %1, i32 1)]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

