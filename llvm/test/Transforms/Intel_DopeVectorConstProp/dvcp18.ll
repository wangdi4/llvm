; REQUIRES: asserts
; RUN: opt < %s -dope-vector-local-const-prop=false -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -dope-vector-local-const-prop=false -opaque-pointers -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: NO FORTRAN FUNCTION
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

define dso_local i32 @main() #0 {
entry:
  ret i32 0
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }


