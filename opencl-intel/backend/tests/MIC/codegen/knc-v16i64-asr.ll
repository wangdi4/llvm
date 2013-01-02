; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define void @shiftright1(<16 x i64> %a, <16 x i64> %b, <16 x i64> * %c) nounwind ssp {
entry:
; KNC: shiftright1:
; KNC: vpshufd $160, %zmm2, %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd

  %shr = ashr <16 x i64> %a, %b
  store  <16 x i64> %shr,  <16 x i64> *%c
  ret void
}

define void @shiftright2(<16 x i64>* %a, <16 x i64> %b) nounwind ssp {
entry:
; KNC: shiftright2:
; KNC: vpshufd $160, %zmm0, %zmm{{[0-9]+}}
; KNC: vmov{{[a-z]+}} (%rdi), [[Z1:%zmm[0-9]+]]
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31
; KNC: vmovdqa32 [[Z1]]{cdab}
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>* %a, align 128
  %shr = ashr <16 x i64> %tmp1, %b
  store  <16 x i64> %shr,  <16 x i64> *%a
  ret void
}

define void @shiftright3(<16 x i64> %a, <16 x i64>* %b) nounwind ssp {
entry:
; KNC: shiftright3:
; KNC: vpshufd $160, (%rdi), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd

  %tmp2 = load <16 x i64>* %b, align 128
  %shr = ashr <16 x i64> %a, %tmp2
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

define void @shiftright4(<16 x i64> %a, <16 x i64> *%b) nounwind readonly ssp {
entry:
; KNC: shiftright4:
; KNC: vpshufd $160, gb(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>* @gb, align 128
  %shr = ashr <16 x i64> %a, %tmp1
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

define void @shiftright5(<16 x i64> %a ,<16 x i64> *%b) nounwind readonly ssp {
entry:
; KNC: shiftright5:
; KNC: mov{{[a-z]+}} pgb(%rip), [[R1:%[a-z]+]]
; KNC: vpshufd $160, ([[R1]]), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd

  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %shr = ashr <16 x i64> %a, %tmp2
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

define void @shiftright6(<16 x i64> %a, <16 x i64> *%b) nounwind readnone ssp {
entry:
; KNC: shiftright6:
; KNC: vpshufd $160, _const_{{[0-9]+}}(%rip), %zmm{{[0-9]+}}
; KNC: vpcmpltd 
; KNC: vpsllvd
; KNC: vpsrlvd
; KNC: vpsrad $31, %zmm0, [[Z1:%zmm[0-9]+]]
; KNC: vmovdqa32 %zmm0{cdab}, [[Z1]]
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd
; KNC: vpsravd

  %shr = ashr <16 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

define void @shiftright7(<16 x i64> %a, <16 x i64> *%b) nounwind ssp {
entry:
; KNC: shiftright7:
; KNC: vpslld $28
; KNC: vpslld $28

  %shr = ashr <16 x i64> %a, <i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4, i64 4>
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

define void @shiftright8(<16 x i64> %a, <16 x i64> *%b) nounwind ssp {
entry:
; KNC: shiftright8:
; KNC: vpsrad $31
; KNC: vpsrad $31

  %shr = ashr <16 x i64> %a, <i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37, i64 37>
  store  <16 x i64> %shr,  <16 x i64> *%b
  ret void
}

