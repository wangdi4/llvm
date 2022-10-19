; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-outofboundsok=false -disable-output %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -disable-output %s

target triple = "x86_64-unknown-linux-gnu"

; Test that loading a pointer value from a location that the
; pointer type analyzer did not identify as a pointer location
; does not crash the DTrans safety analyzer. This is a trimmed
; down test from a failure seen in 502.gcc (CMPLRLLVM-36517)

define fastcc void @test01() {
  %i = alloca [0 x i8]

  ; &(i[0])
  %i2 = getelementptr inbounds [0 x i8], ptr %i, i64 0, i64 0

  ; &(i[N])
  %i4 = getelementptr inbounds ptr, ptr %i2, i64 undef

  ; i[N]
  %i5 = load ptr, ptr %i2, align 8

  ; Use i[N] as if it were a pointer type.
  %i7 = load ptr, ptr %i5, align 8

  ; Load an integer from a memory location pointed to by the value in i[0].
  %i8 = load i32, ptr %i7, align 8
  ret void
}

!intel.dtrans.types = !{}
