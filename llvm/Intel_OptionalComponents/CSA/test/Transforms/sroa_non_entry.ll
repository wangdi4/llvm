; RUN: opt -sroa -S -o - %s | FileCheck %s
;
; Check that SROA registerizes alloca instructions in non-entry blocks.

define float @test(float* %p, i32 %c) {
entry:
  br label %non.entry.block

non.entry.block:
; CHECK-NOT: alloca
  %f = alloca float, align 4
  %f.gep = getelementptr inbounds float, float* %f, i64 0
  store float 1.000000e+00, float* %f.gep, align 4
  %c.nz = icmp ne i32 %c, 0
  br i1 %c.nz, label %if.then, label %if.end

if.then:
  %p.i32 = bitcast float* %p to i32*
  %p.val = load i32, i32* %p.i32, align 4
  %f.i32 = bitcast float* %f.gep to i32*
  store i32 %p.val, i32* %f.i32, align 4
  br label %if.end

if.end:
  %f.val = load float, float* %f.gep, align 4
  ret float %f.val
}
