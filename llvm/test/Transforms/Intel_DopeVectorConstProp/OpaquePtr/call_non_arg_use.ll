; RUN: opt < %s -opaque-pointers -dope-vector-local-const-prop=false -disable-output -dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2

; The test checks correctness of OpaquePointerTypeMapper constructor
; when a CallBase is a User of a pointer, but the pointer is not an argument of the CallBase.
; The test should compile w/o an error

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define void @test() #1 {
  %1 = alloca i32, align 8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(ptr %1, i32 1) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { "intel-lang"="fortran" }
