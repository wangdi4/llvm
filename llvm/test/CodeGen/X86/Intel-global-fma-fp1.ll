; INTEL_CUSTOMIZATION:
; This test checks that Global FMA optimizes an arbitrary (1 of ~40k) test case
; using 1.0 fp const.
; For the input expression:
;   +acd+a+b
; the output code must have 2 arithmetic instructions:
;   F0=+c*d+1; F1=+F0*a+b;

; The tests are started with AVX2 and with SKX flags. The generated code is
; different now because the FMA form selection optimization is not enabled for
; SKX opcodes. the code should become identical for AVX2 and SKX after the
; FMA form selection is enabled for SKX.

; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=core-avx2 -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=small -relocation-model=pic | FileCheck --check-prefix=AVX2SML %s
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=skx       -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=small -relocation-model=pic | FileCheck --check-prefix=SKXSML %s

; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=core-avx2 -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=large | FileCheck --check-prefix=AVX2LRG %s
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=skx       -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=large | FileCheck --check-prefix=SKXLRG %s

; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=core-avx2 -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=kernel | FileCheck --check-prefix=AVX2OTHER %s
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=skx       -do-x86-global-fma -fp-contract=fast -enable-unsafe-fp-math -enable-misched=0 -code-model=kernel | FileCheck --check-prefix=SKXOTHER %s

attributes #0 = { nounwind }

define float @test_ss(float %a32, float %b32, float %c32, float %d32) #0 {
; AVX2SML-LABEL: test_ss:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213ss {{.*}}(%rip), %xmm2, %xmm0
; AVX2SML-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_ss:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vmovss {{.*#+}} xmm4 = mem[0],zero,zero,zero
; SKXSML-NEXT:    vfmadd213ss %xmm4, %xmm2, %xmm0
; SKXSML-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_ss:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213ss (%rax), %xmm2, %xmm0
; AVX2LRG-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_ss:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vmovss {{.*#+}} xmm4 = mem[0],zero,zero,zero
; SKXLRG-NEXT:    vfmadd213ss %xmm4, %xmm2, %xmm0
; SKXLRG-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_ss:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; AVX2OTHER-NEXT:    vmovd %eax, %xmm4
; AVX2OTHER-NEXT:    vfmadd213ss %xmm4, %xmm2, %xmm0
; AVX2OTHER-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_ss:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; SKXOTHER-NEXT:    vmovd %eax, %xmm4
; SKXOTHER-NEXT:    vfmadd213ss %xmm4, %xmm2, %xmm0
; SKXOTHER-NEXT:    vfmadd213ss %xmm1, %xmm3, %xmm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast float %a32, %d32
  %mul1 = fmul fast float %mul, %c32
  %add = fadd fast float %mul1, %d32
  %add2 = fadd fast float %add, %b32
  ret float %add2
}

define double @test_sd(double %a64, double %b64, double %c64, double %d64) #0 {
; AVX2SML-LABEL: test_sd:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213sd {{.*}}(%rip), %xmm2, %xmm0
; AVX2SML-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_sd:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vmovsd {{.*#+}} xmm4 = mem[0],zero
; SKXSML-NEXT:    vfmadd213sd %xmm4, %xmm2, %xmm0
; SKXSML-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_sd:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213sd (%rax), %xmm2, %xmm0
; AVX2LRG-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_sd:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vmovsd {{.*#+}} xmm4 = mem[0],zero
; SKXLRG-NEXT:    vfmadd213sd %xmm4, %xmm2, %xmm0
; SKXLRG-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_sd:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; AVX2OTHER-NEXT:    vmovd %rax, %xmm4
; AVX2OTHER-NEXT:    vfmadd213sd %xmm4, %xmm2, %xmm0
; AVX2OTHER-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_sd:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; SKXOTHER-NEXT:    vmovd %rax, %xmm4
; SKXOTHER-NEXT:    vfmadd213sd %xmm4, %xmm2, %xmm0
; SKXOTHER-NEXT:    vfmadd213sd %xmm1, %xmm3, %xmm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast double %a64, %d64
  %mul1 = fmul fast double %mul, %c64
  %add = fadd fast double %mul1, %d64
  %add2 = fadd fast double %add, %b64
  ret double %add2
}

define <4 x float> @test_ps(<4 x float> %a32, <4 x float> %b32, <4 x float> %c32, <4 x float> %d32) #0 {
; AVX2SML-LABEL: test_ps:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213ps {{.*}}(%rip), %xmm2, %xmm0
; AVX2SML-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_ps:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vfmadd213ps {{.*}}(%rip), %xmm2, %xmm0
; SKXSML-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_ps:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213ps (%rax), %xmm2, %xmm0
; AVX2LRG-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_ps:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vfmadd213ps (%rax), %xmm2, %xmm0
; SKXLRG-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_ps:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; AVX2OTHER-NEXT:    vmovd %eax, %xmm4
; AVX2OTHER-NEXT:    vbroadcastss %xmm4, %xmm4
; AVX2OTHER-NEXT:    vfmadd213ps %xmm4, %xmm2, %xmm0
; AVX2OTHER-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_ps:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; SKXOTHER-NEXT:    vpbroadcastd %eax, %xmm4
; SKXOTHER-NEXT:    vfmadd213ps %xmm4, %xmm2, %xmm0
; SKXOTHER-NEXT:    vfmadd213ps %xmm1, %xmm3, %xmm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast <4 x float> %a32, %d32
  %mul1 = fmul fast <4 x float> %mul, %c32
  %add = fadd fast <4 x float> %mul1, %d32
  %add2 = fadd fast <4 x float> %add, %b32
  ret <4 x float> %add2
}

define <2 x double> @test_pd(<2 x double> %a32, <2 x double> %b32, <2 x double> %c32, <2 x double> %d32) #0 {
; AVX2SML-LABEL: test_pd:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213pd {{.*}}(%rip), %xmm2, %xmm0
; AVX2SML-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_pd:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vfmadd213pd {{.*}}(%rip), %xmm2, %xmm0
; SKXSML-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_pd:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213pd (%rax), %xmm2, %xmm0
; AVX2LRG-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_pd:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vfmadd213pd (%rax), %xmm2, %xmm0
; SKXLRG-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_pd:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; AVX2OTHER-NEXT:    vmovd %rax, %xmm4
; AVX2OTHER-NEXT:    vunpcklpd {{.*#+}} xmm4 = xmm4[0,0]
; AVX2OTHER-NEXT:    vfmadd213pd %xmm4, %xmm2, %xmm0
; AVX2OTHER-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_pd:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; SKXOTHER-NEXT:    vpbroadcastq %rax, %xmm4
; SKXOTHER-NEXT:    vfmadd213pd %xmm4, %xmm2, %xmm0
; SKXOTHER-NEXT:    vfmadd213pd %xmm1, %xmm3, %xmm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast <2 x double> %a32, %d32
  %mul1 = fmul fast <2 x double> %mul, %c32
  %add = fadd fast <2 x double> %mul1, %d32
  %add2 = fadd fast <2 x double> %add, %b32
  ret <2 x double> %add2
}

define <8 x float> @test_ps256(<8 x float> %a32, <8 x float> %b32, <8 x float> %c32, <8 x float> %d32) #0 {
; AVX2SML-LABEL: test_ps256:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213ps {{.*}}(%rip), %ymm2, %ymm0
; AVX2SML-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_ps256:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vfmadd213ps {{.*}}(%rip), %ymm2, %ymm0
; SKXSML-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_ps256:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213ps (%rax), %ymm2, %ymm0
; AVX2LRG-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_ps256:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vfmadd213ps (%rax), %ymm2, %ymm0
; SKXLRG-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_ps256:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; AVX2OTHER-NEXT:    vmovd %eax, %xmm4
; AVX2OTHER-NEXT:    vbroadcastss %xmm4, %ymm4
; AVX2OTHER-NEXT:    vfmadd213ps %ymm4, %ymm2, %ymm0
; AVX2OTHER-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_ps256:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; SKXOTHER-NEXT:    vpbroadcastd %eax, %ymm4
; SKXOTHER-NEXT:    vfmadd213ps %ymm4, %ymm2, %ymm0
; SKXOTHER-NEXT:    vfmadd213ps %ymm1, %ymm3, %ymm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast <8 x float> %a32, %d32
  %mul1 = fmul fast <8 x float> %mul, %c32
  %add = fadd fast <8 x float> %mul1, %d32
  %add2 = fadd fast <8 x float> %add, %b32
  ret <8 x float> %add2
}

define <4 x double> @test_pd256(<4 x double> %a32, <4 x double> %b32, <4 x double> %c32, <4 x double> %d32) #0 {
; AVX2SML-LABEL: test_pd256:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213pd {{.*}}(%rip), %ymm2, %ymm0
; AVX2SML-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_pd256:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vfmadd213pd {{.*}}(%rip), %ymm2, %ymm0
; SKXSML-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_pd256:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213pd (%rax), %ymm2, %ymm0
; AVX2LRG-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_pd256:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vfmadd213pd (%rax), %ymm2, %ymm0
; SKXLRG-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_pd256:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; AVX2OTHER-NEXT:    vmovd %rax, %xmm4
; AVX2OTHER-NEXT:    vbroadcastsd %xmm4, %ymm4
; AVX2OTHER-NEXT:    vfmadd213pd %ymm4, %ymm2, %ymm0
; AVX2OTHER-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_pd256:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movabsq $4607182418800017408, %rax # imm = 0x3FF0000000000000
; SKXOTHER-NEXT:    vpbroadcastq %rax, %ymm4
; SKXOTHER-NEXT:    vfmadd213pd %ymm4, %ymm2, %ymm0
; SKXOTHER-NEXT:    vfmadd213pd %ymm1, %ymm3, %ymm0
; SKXOTHER-NEXT:    retq
entry:
  %mul = fmul fast <4 x double> %a32, %d32
  %mul1 = fmul fast <4 x double> %mul, %c32
  %add = fadd fast <4 x double> %mul1, %d32
  %add2 = fadd fast <4 x double> %add, %b32
  ret <4 x double> %add2
}

attributes #1 = { nounwind "target-cpu"="skx" "target-features"="+avx512f,+fma" }

define <16 x float> @test_ps512(<16 x float> %a32, <16 x float> %b32, <16 x float> %c32, <16 x float> %d32) #1 {
; AVX2SML-LABEL: test_ps512:
; AVX2SML:       # BB#0: # %entry
; AVX2SML-NEXT:    vfmadd213ps {{.*}}(%rip), %zmm2, %zmm0
; AVX2SML-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; AVX2SML-NEXT:    retq
;
; SKXSML-LABEL: test_ps512:
; SKXSML:       # BB#0: # %entry
; SKXSML-NEXT:    vfmadd213ps {{.*}}(%rip), %zmm2, %zmm0
; SKXSML-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; SKXSML-NEXT:    retq
;
; AVX2LRG-LABEL: test_ps512:
; AVX2LRG:       # BB#0: # %entry
; AVX2LRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; AVX2LRG-NEXT:    vfmadd213ps (%rax), %zmm2, %zmm0
; AVX2LRG-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; AVX2LRG-NEXT:    retq
;
; SKXLRG-LABEL: test_ps512:
; SKXLRG:       # BB#0: # %entry
; SKXLRG-NEXT:    movabsq ${{\.LCPI.*}}, %rax
; SKXLRG-NEXT:    vfmadd213ps (%rax), %zmm2, %zmm0
; SKXLRG-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; SKXLRG-NEXT:    retq
;
; AVX2OTHER-LABEL: test_ps512:
; AVX2OTHER:       # BB#0: # %entry
; AVX2OTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; AVX2OTHER-NEXT:    vpbroadcastd %eax, %zmm4
; AVX2OTHER-NEXT:    vfmadd213ps %zmm4, %zmm2, %zmm0
; AVX2OTHER-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; AVX2OTHER-NEXT:    retq
;
; SKXOTHER-LABEL: test_ps512:
; SKXOTHER:       # BB#0: # %entry
; SKXOTHER-NEXT:    movl $1065353216, %eax # imm = 0x3F800000
; SKXOTHER-NEXT:    vpbroadcastd %eax, %zmm4
; SKXOTHER-NEXT:    vfmadd213ps %zmm4, %zmm2, %zmm0
; SKXOTHER-NEXT:    vfmadd213ps %zmm1, %zmm3, %zmm0
; SKXOTHER-NEXT:    retq
entry:

  %mul = fmul fast <16 x float> %a32, %d32
  %mul1 = fmul fast <16 x float> %mul, %c32
  %add = fadd fast <16 x float> %mul1, %d32
  %add2 = fadd fast <16 x float> %add, %b32
  ret <16 x float> %add2
}
