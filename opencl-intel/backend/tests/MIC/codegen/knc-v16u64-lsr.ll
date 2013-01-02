;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @shiftright1(<16 x i64> %a, <16 x i64> %b, <16 x i64>* %ptr) nounwind readnone ssp {
entry:

;shiftright1:
;..B1.1:
;        vpshufd   $160, %zmm2, %zmm7
;        movl      $21845, %eax
;        vpshufd   $160, %zmm3, %zmm13
;        kmov      %eax, %k3
;        vpxord    %zmm14, %zmm14, %zmm14
;        vpsubrd   _const_0(%rip){1to16}, %zmm7, %zmm6
;        vpsubrd   _const_1(%rip){1to16}, %zmm13, %zmm12
;        vpcmpltd  _const_0(%rip){1to16}, %zmm7, %k2
;        vpcmpd    $4, %zmm14, %zmm7, %k1
;        vpcmpltd  _const_1(%rip){1to16}, %zmm13, %k5
;        vpcmpd    $4, %zmm14, %zmm13, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm6, %zmm0, %zmm8
;        kand      %k3, %k1
;        vpsllvd   %zmm12, %zmm1, %zmm15
;        kand      %k5, %k4
;        vpsrlvd   %zmm7, %zmm0, %zmm9
;        kand      %k3, %k4
;        vpsrlvd   %zmm13, %zmm1, %zmm16
;        vmovaps   %zmm14, %zmm4
;        vmovaps   %zmm14, %zmm10
;        vmovdqa32 %zmm0{cdab}, %zmm4{%k3}
;        vpsubd    _const_0(%rip){1to16}, %zmm7, %zmm5
;        vmovdqa32 %zmm1{cdab}, %zmm10{%k3}
;        vpsubd    _const_1(%rip){1to16}, %zmm13, %zmm11
;        vpord     %zmm8{cdab}, %zmm9, %zmm9{%k1}
;        vpord     %zmm15{cdab}, %zmm16, %zmm16{%k4}
;        vpsrlvd   %zmm5, %zmm4, %zmm17
;        vpsrlvd   %zmm11, %zmm10, %zmm18
;        vmovdqa32 %zmm9, %zmm17{%k2}
;        vmovdqa32 %zmm16, %zmm18{%k5}
;        vmovaps   %zmm18, %zmm1
;        vmovdqa64 %zmm17, (%rdi)
;        vmovdqa64 %zmm18, 64(%rdi)
;        ret 

; KNC: shiftright1:
; KNC: vpshufd $160, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpshufd $160, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}

  %shr = lshr <16 x i64> %a, %b
  store <16 x i64> %shr, <16 x i64>* %ptr
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:

;shiftright2:
;..B2.1:
;        vpshufd   $160, %zmm0, %zmm6
;        movl      $21845, %eax
;        vpshufd   $160, %zmm1, %zmm13
;        kmov      %eax, %k3
;        vpxord    %zmm14, %zmm14, %zmm14
;        vmovaps   (%rdi), %zmm4
;        vmovaps   64(%rdi), %zmm11
;        vpsubrd   _const_2(%rip){1to16}, %zmm6, %zmm5
;        vpsubrd   _const_3(%rip){1to16}, %zmm13, %zmm12
;        vpcmpltd  _const_2(%rip){1to16}, %zmm6, %k2
;        vpcmpd    $4, %zmm14, %zmm6, %k1
;        vpcmpltd  _const_3(%rip){1to16}, %zmm13, %k5
;        vpcmpd    $4, %zmm14, %zmm13, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm5, %zmm4, %zmm7
;        kand      %k3, %k1
;        vpsllvd   %zmm12, %zmm11, %zmm15
;        kand      %k5, %k4
;        vpsrlvd   %zmm6, %zmm4, %zmm8
;        kand      %k3, %k4
;        vpsrlvd   %zmm13, %zmm11, %zmm16
;        vmovaps   %zmm14, %zmm2
;        vmovaps   %zmm14, %zmm9
;        vmovdqa32 %zmm4{cdab}, %zmm2{%k3}
;        vpsubd    _const_2(%rip){1to16}, %zmm6, %zmm3
;        vmovdqa32 %zmm11{cdab}, %zmm9{%k3}
;        vpsubd    _const_3(%rip){1to16}, %zmm13, %zmm10
;        vpord     %zmm7{cdab}, %zmm8, %zmm8{%k1}
;        vpord     %zmm15{cdab}, %zmm16, %zmm16{%k4}
;        vpsrlvd   %zmm3, %zmm2, %zmm17
;        vpsrlvd   %zmm10, %zmm9, %zmm1
;        vmovdqa32 %zmm8, %zmm17{%k2}
;        vmovdqa32 %zmm16, %zmm1{%k5}
;        movb      %al, %al
;        vmovdqa64 %zmm17, (%rdi)
;        vmovdqa64 %zmm1, 64(%rdi)
;        ret

; KNC: shiftright2:
; KNC: vpshufd $160, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpshufd $160, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vmovaps (%rdi), %zmm{{[0-9]+}}
; KNC: vmovaps 64(%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}

  %tmp1 = load <16 x i64>* %a
  %shr = lshr <16 x i64> %tmp1, %b
  store <16 x i64> %shr, <16 x i64>* %a
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:

;shiftright3:
;..B3.1:
;        vpshufd   $160, (%rdi), %zmm5
;        movl      $21845, %eax
;        vpshufd   $160, 64(%rdi), %zmm11
;        vpxord    %zmm12, %zmm12, %zmm12
;        kmov      %eax, %k3
;        vpsubrd   _const_4(%rip){1to16}, %zmm5, %zmm4
;        vpsubrd   _const_5(%rip){1to16}, %zmm11, %zmm10
;        vpcmpltd  _const_4(%rip){1to16}, %zmm5, %k2
;        vpcmpd    $4, %zmm12, %zmm5, %k1
;        vpcmpltd  _const_5(%rip){1to16}, %zmm11, %k5
;        vpcmpd    $4, %zmm12, %zmm11, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm4, %zmm0, %zmm6
;        kand      %k3, %k1
;        vpsllvd   %zmm10, %zmm1, %zmm13
;        kand      %k5, %k4
;        vpsrlvd   %zmm5, %zmm0, %zmm7
;        kand      %k3, %k4
;        vpsrlvd   %zmm11, %zmm1, %zmm14
;        vmovaps   %zmm12, %zmm2
;        vmovaps   %zmm12, %zmm8
;        vmovdqa32 %zmm0{cdab}, %zmm2{%k3}
;        vpsubd    _const_4(%rip){1to16}, %zmm5, %zmm3
;        vmovdqa32 %zmm1{cdab}, %zmm8{%k3}
;        vpsubd    _const_5(%rip){1to16}, %zmm11, %zmm9
;        vpord     %zmm6{cdab}, %zmm7, %zmm7{%k1}
;        vpord     %zmm13{cdab}, %zmm14, %zmm14{%k4}
;        vpsrlvd   %zmm3, %zmm2, %zmm15
;        vpsrlvd   %zmm9, %zmm8, %zmm16
;        vmovdqa32 %zmm7, %zmm15{%k2}
;        vmovdqa32 %zmm14, %zmm16{%k5}
;        vmovaps   %zmm16, %zmm1
;        vmovdqa64 %zmm15, (%rdi)
;        vmovdqa64 %zmm16, 64(%rdi)
;        ret

; KNC: shiftright3:
; KNC: vpshufd $160, (%rdi), %zmm{{[0-9]+}}
; KNC: vpshufd $160, 64(%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpsrlvd

  %tmp2 = load <16 x i64>* %b
  %shr = lshr <16 x i64> %a, %tmp2
  store <16 x i64> %shr, <16 x i64>* %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright4(<16 x i64> %a, <16 x i64>* %ptr) nounwind readonly ssp {
entry:

;shiftright4:
;..B4.1:
;        vpshufd   $160, 64+gb(%rip), %zmm5
;        vpshufd   $160, gb(%rip), %zmm11
;        vpxord    %zmm12, %zmm12, %zmm12
;        movl      $21845, %eax
;        vpsubrd   _const_6(%rip){1to16}, %zmm5, %zmm4
;        vpsubrd   _const_7(%rip){1to16}, %zmm11, %zmm10
;        vpcmpltd  _const_6(%rip){1to16}, %zmm5, %k2
;        vpcmpd    $4, %zmm12, %zmm5, %k1
;        kmov      %eax, %k3
;        vpcmpltd  _const_7(%rip){1to16}, %zmm11, %k5
;        vpcmpd    $4, %zmm12, %zmm11, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm4, %zmm1, %zmm6
;        kand      %k3, %k1
;        vpsllvd   %zmm10, %zmm0, %zmm13
;        kand      %k5, %k4
;        vpsrlvd   %zmm5, %zmm1, %zmm7
;        kand      %k3, %k4
;        vpsrlvd   %zmm11, %zmm0, %zmm14
;        vmovaps   %zmm12, %zmm2
;        vmovaps   %zmm12, %zmm8
;        vmovdqa32 %zmm1{cdab}, %zmm2{%k3}
;        vpsubd    _const_6(%rip){1to16}, %zmm5, %zmm3
;        vmovdqa32 %zmm0{cdab}, %zmm8{%k3}
;        vpsubd    _const_7(%rip){1to16}, %zmm11, %zmm9
;        vpord     %zmm6{cdab}, %zmm7, %zmm7{%k1}
;        vpord     %zmm13{cdab}, %zmm14, %zmm14{%k4}
;        vpsrlvd   %zmm3, %zmm2, %zmm16
;        vpsrlvd   %zmm9, %zmm8, %zmm15
;        vmovdqa32 %zmm7, %zmm16{%k2}
;        vmovdqa32 %zmm14, %zmm15{%k5}
;        vmovaps   %zmm16, %zmm1
;        vmovdqa64 %zmm16, 64(%rdi)
;        vmovdqa64 %zmm15, (%rdi)
;        ret 

; KNC: shiftright4:
; KNC: vpshufd $160, 64+gb(%rip), %zmm{{[0-9]+}}
; KNC: vpshufd $160, gb(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vpsrlvd
; KNC: vpsrlvd

  %tmp1 = load <16 x i64>* @gb, align 128
  %shr = lshr <16 x i64> %a, %tmp1
  store <16 x i64> %shr, <16 x i64>* %ptr
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright5(<16 x i64> %a) nounwind readonly ssp {
entry:

;shiftright5:
;..B5.1:
;        vpxord    %zmm12, %zmm12, %zmm12
;        movq      pgb(%rip), %rax
;        vpshufd   $160, (%rax), %zmm5
;        movl      $21845, %edx
;        vpshufd   $160, 64(%rax), %zmm11
;        vpsubrd   _const_8(%rip){1to16}, %zmm5, %zmm4
;        vpsubrd   _const_9(%rip){1to16}, %zmm11, %zmm10
;        vpcmpltd  _const_8(%rip){1to16}, %zmm5, %k2
;        vpcmpd    $4, %zmm12, %zmm5, %k1
;        kmov      %edx, %k3
;        vpcmpltd  _const_9(%rip){1to16}, %zmm11, %k5
;        vpcmpd    $4, %zmm12, %zmm11, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm4, %zmm0, %zmm6
;        kand      %k3, %k1
;        vpsllvd   %zmm10, %zmm1, %zmm13
;        kand      %k5, %k4
;        vpsrlvd   %zmm5, %zmm0, %zmm7
;        kand      %k3, %k4
;        vpsrlvd   %zmm11, %zmm1, %zmm14
;        vmovaps   %zmm12, %zmm2
;        vmovaps   %zmm12, %zmm8
;        vmovdqa32 %zmm0{cdab}, %zmm2{%k3}
;        vpsubd    _const_8(%rip){1to16}, %zmm5, %zmm3
;        vmovdqa32 %zmm1{cdab}, %zmm8{%k3}
;        vpsubd    _const_9(%rip){1to16}, %zmm11, %zmm9
;        vpord     %zmm6{cdab}, %zmm7, %zmm7{%k1}
;        vpord     %zmm13{cdab}, %zmm14, %zmm14{%k4}
;        vpsrlvd   %zmm3, %zmm2, %zmm15
;        vpsrlvd   %zmm9, %zmm8, %zmm16
;        vmovdqa32 %zmm7, %zmm15{%k2}
;        vmovdqa32 %zmm14, %zmm16{%k5}
;        vmovaps   %zmm16, %zmm1
;        vmovdqa64 %zmm15, (%rax)
;        vmovdqa64 %zmm16, 64(%rax)
;        ret 

; KNC: shiftright5:
; KNC: mov{{[a-z]+}} pgb(%rip), [[R1:%[a-z]+]]
; KNC: vpshufd $160, ([[R1]]), %zmm{{[0-9]+}}
; KNC: vpshufd $160, 64([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}

  %tmp1 = load <16 x i64>** @pgb
  %tmp2 = load <16 x i64>* %tmp1
  %shr = lshr <16 x i64> %a, %tmp2
  store <16 x i64> %shr, <16 x i64>* %tmp1
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright6(<16 x i64> %a, <16 x i64>* %ptr) nounwind readnone ssp {
entry:

;shiftright6:
;..B6.1:
;        vpshufd   $160, _const_10(%rip), %zmm5
;        vpshufd   $160, _const_11(%rip), %zmm11
;        vpxord    %zmm12, %zmm12, %zmm12
;        movl      $21845, %eax
;        vpsubrd   _const_12(%rip){1to16}, %zmm5, %zmm4
;        vpsubrd   _const_13(%rip){1to16}, %zmm11, %zmm10
;        vpcmpltd  _const_12(%rip){1to16}, %zmm5, %k2
;        vpcmpd    $4, %zmm12, %zmm5, %k1
;        kmov      %eax, %k3
;        vpcmpltd  _const_13(%rip){1to16}, %zmm11, %k5
;        vpcmpd    $4, %zmm12, %zmm11, %k4
;        kand      %k2, %k1
;        vpsllvd   %zmm4, %zmm0, %zmm6
;        kand      %k3, %k1
;        vpsllvd   %zmm10, %zmm1, %zmm13
;        kand      %k5, %k4
;        vpsrlvd   %zmm5, %zmm0, %zmm7
;        kand      %k3, %k4
;        vpsrlvd   %zmm11, %zmm1, %zmm14
;        vmovaps   %zmm12, %zmm2
;        vmovaps   %zmm12, %zmm8
;        vmovdqa32 %zmm0{cdab}, %zmm2{%k3}
;        vpsubd    _const_12(%rip){1to16}, %zmm5, %zmm3
;        vmovdqa32 %zmm1{cdab}, %zmm8{%k3}
;        vpsubd    _const_13(%rip){1to16}, %zmm11, %zmm9
;        vpord     %zmm6{cdab}, %zmm7, %zmm7{%k1}
;        vpord     %zmm13{cdab}, %zmm14, %zmm14{%k4}
;        vpsrlvd   %zmm3, %zmm2, %zmm15
;        vpsrlvd   %zmm9, %zmm8, %zmm16
;        vmovdqa32 %zmm7, %zmm15{%k2}
;        vmovdqa32 %zmm14, %zmm16{%k5}
;        vmovaps   %zmm16, %zmm1
;        vmovdqa64 %zmm15, (%rdi)
;        vmovdqa64 %zmm16, 64(%rdi)
;        ret

; KNC: shiftright6:
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrlvd
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}

  %shr = lshr <16 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  store <16 x i64> %shr, <16 x i64>* %ptr
  ret <16 x i64> %shr
}


define <16 x i64> @shiftleft7(<16 x i64> %a, <16 x i64>* %ptr) nounwind readnone ssp {
entry:

;shiftleft7:
;..B7.1:
;        vpslld    $28, %zmm0, %zmm2
;        movl      $21845, %eax
;        vpslld    $28, %zmm1, %zmm3
;        kmov      %eax, %k1
;        vpsrld    $4, %zmm0, %zmm4
;        vpsrld    $4, %zmm1, %zmm5
;        vpord     %zmm2{cdab}, %zmm4, %zmm4{%k1}
;        vpord     %zmm3{cdab}, %zmm5, %zmm5{%k1}
;        vmovaps   %zmm5, %zmm1
;        vmovdqa64 %zmm4, (%rdi)
;        vmovdqa64 %zmm5, 64(%rdi)
;        ret

; KNC: shiftleft7:
; KNC: vpslld $28, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpslld $28, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpsrld $4, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: vpsrld $4, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}

  %shr = lshr <16 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  store <16 x i64> %shr, <16 x i64>* %ptr
  ret <16 x i64> %shr
}

define <16 x i64> @shiftleft8(<16 x i64> %a, <16 x i64>* %ptr) nounwind readnone ssp {
entry:

;shiftleft8:
;..B8.1:
;        vpxord    %zmm3, %zmm3, %zmm3
;        movl      $21845, %eax
;        vmovaps   %zmm3, %zmm2
;        kmov      %eax, %k1
;        vmovdqa32 %zmm0{cdab}, %zmm2{%k1}
;        vmovdqa32 %zmm1{cdab}, %zmm3{%k1}
;        vpsrld    $5, %zmm2, %zmm4
;        vpsrld    $5, %zmm3, %zmm1
;        movb      %al, %al
;        vmovdqa64 %zmm4, (%rdi)
;        vmovdqa64 %zmm1, 64(%rdi)
;        ret

; KNC: shiftleft8:
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}{%k1}
; KNC: vmovdqa32 %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}{%k1}
; KNC: vpslld $5, %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}
; KNC: vpslld $5, %zmm{{[0-9]+}}{cdab}, %zmm{{[0-9]+}}

  %shr = lshr <16 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  store <16 x i64> %shr, <16 x i64>* %ptr
  ret <16 x i64> %shr
}


