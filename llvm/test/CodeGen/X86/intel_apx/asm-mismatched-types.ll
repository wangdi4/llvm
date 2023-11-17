; RUN: llc -o - %s -no-integrated-as | FileCheck %s
target triple = "x86_64--"

; Allow to specify any of the 8/16/32/64 register names interchangeably in
; constraints

; Produced by C-programs like this:
; void foo(int p) { register int reg __asm__("r16") = p;
; __asm__ __volatile__("# REG: %0" : : "r" (reg)); }

; TODO: Merge to asm-mismatched-types.ll after disclose.

; CHECK-LABEL: reg64_as_32:
; CHECK: # REG: %r16d
define void @reg64_as_32(i32 %p) {
  call void asm sideeffect "# REG: $0", "{r16}"(i32 %p)
  ret void
}

; CHECK-LABEL: reg64_as_32_float:
; CHECK: # REG: %r16d
define void @reg64_as_32_float(float %p) {
  call void asm sideeffect "# REG: $0", "{r16}"(float %p)
  ret void
}

; CHECK-LABEL: reg64_as_16:
; CHECK: # REG: %r17w
define void @reg64_as_16(i16 %p) {
  call void asm sideeffect "# REG: $0", "{r17}"(i16 %p)
  ret void
}

; CHECK-LABEL: reg64_as_8:
; CHECK: # REG: %r21b
define void @reg64_as_8(i8 %p) {
  call void asm sideeffect "# REG: $0", "{r21}"(i8 %p)
  ret void
}

; CHECK-LABEL: reg32_as_16:
; CHECK: # REG: %r21w
define void @reg32_as_16(i16 %p) {
  call void asm sideeffect "# REG: $0", "{r21d}"(i16 %p)
  ret void
}

; CHECK-LABEL: reg32_as_8:
; CHECK: # REG: %r20b
define void @reg32_as_8(i8 %p) {
  call void asm sideeffect "# REG: $0", "{r20d}"(i8 %p)
  ret void
}

; CHECK-LABEL: reg16_as_8:
; CHECK: # REG: %r17b
define void @reg16_as_8(i8 %p) {
  call void asm sideeffect "# REG: $0", "{r17w}"(i8 %p)
  ret void
}

; CHECK-LABEL: reg32_as_64:
; CHECK: # REG: %r21
define void @reg32_as_64(i64 %p) {
  call void asm sideeffect "# REG: $0", "{r21d}"(i64 %p)
  ret void
}

; CHECK-LABEL: reg32_as_64_float:
; CHECK: # REG: %r21
define void @reg32_as_64_float(double %p) {
  call void asm sideeffect "# REG: $0", "{r21d}"(double %p)
  ret void
}

; CHECK-LABEL: reg16_as_64:
; CHECK: # REG: %r21
define void @reg16_as_64(i64 %p) {
  call void asm sideeffect "# REG: $0", "{r21w}"(i64 %p)
  ret void
}

; CHECK-LABEL: reg16_as_64_float:
; CHECK: # REG: %r21
define void @reg16_as_64_float(double %p) {
  call void asm sideeffect "# REG: $0", "{r21w}"(double %p)
  ret void
}

; CHECK-LABEL: reg8_as_64:
; CHECK: # REG: %r16
define void @reg8_as_64(i64 %p) {
  call void asm sideeffect "# REG: $0", "{r16b}"(i64 %p)
  ret void
}

; CHECK-LABEL: reg8_as_64_float:
; CHECK: # REG: %r16
define void @reg8_as_64_float(double %p) {
  call void asm sideeffect "# REG: $0", "{r16b}"(double %p)
  ret void
}

; CHECK-LABEL: reg16_as_32:
; CHECK: # REG: %r19d
define void @reg16_as_32(i32 %p) {
  call void asm sideeffect "# REG: $0", "{r19w}"(i32 %p)
  ret void
}

; CHECK-LABEL: reg16_as_32_float:
; CHECK: # REG: %r19d
define void @reg16_as_32_float(float %p) {
  call void asm sideeffect "# REG: $0", "{r19w}"(float %p)
  ret void
}

; CHECK-LABEL: reg8_as_32:
; CHECK: # REG: %r17d
define void @reg8_as_32(i32 %p) {
  call void asm sideeffect "# REG: $0", "{r17b}"(i32 %p)
  ret void
}

; CHECK-LABEL: reg8_as_32_float:
; CHECK: # REG: %r17d
define void @reg8_as_32_float(float %p) {
  call void asm sideeffect "# REG: $0", "{r17b}"(float %p)
  ret void
}

; CHECK-LABEL: reg8_as_16:
; CHECK: # REG: %r18w
define void @reg8_as_16(i16 %p) {
  call void asm sideeffect "# REG: $0", "{r18b}"(i16 %p)
  ret void
}
