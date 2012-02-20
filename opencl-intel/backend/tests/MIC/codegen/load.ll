; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

target datalayout = "e-p:64:64"

%SArray = type { [100 x i64]}

define  i64 @loopo(%SArray* %p, i32 %i, i32 %j) nounwind {
entry:
; CHECKnot: movq ({{%[a-z]+}},{{%[a-z]+}},8), %rax
  %vp = getelementptr %SArray* %p, i32 %i, i32 0, i32 %j
  %v = load i64 * %vp
  ret i64 %v
}

%RT = type { i8 , [10 x [20 x i32]], i8  }
%ST = type { i32, double, %RT }

define i32* @foo(%ST* %s, i32 %k) {
entry:
; CHECK: lea 1244({{%[a-z]+}},{{%[a-z]+}},4), %rax
  %reg = getelementptr %ST* %s, i32 1, i32 2, i32 1, i32 5, i32 %k
  ret i32* %reg
}



@gbf = common global float zeroinitializer, align 8
@gbs = common global i64 zeroinitializer, align 8


define float @load_gbfi() nounwind readnone ssp {
entry:
; CHECK: vloadd 20+gbf(%rip){1to16}, %v0
  %addr = getelementptr float* @gbf, i32 5
  %0 = load float * %addr, align 8
  ret float %0
}


define i64 @load_gbs() nounwind readnone ssp {
entry:
; CHECK: movq gbs(%rip), %rax
  %0 = load i64 * @gbs, align 8
  ret i64 %0
}

define i16 @load_i16(i16 * %a) nounwind readnone ssp {
entry:
; CHECK: movzwl    (%rdi), %eax
  %0 = load i16 *%a
  ret i16 %0
}

define i8 @load_i8(i8 * %a) nounwind readnone ssp {
entry:
; CHECK: movb    (%rdi), %al
  %0 = load i8 *%a
  ret i8 %0
}

define i32 @load_i32(i32 * %a) nounwind readnone ssp {
entry:
; CHECK: movl    (%rdi), %eax
  %0 = load i32 *%a
  ret i32 %0
}

define i64 @load_i64(i64 * %a) nounwind readnone ssp {
entry:
; CHECK: movq      (%rdi), %rax
  %0 = load i64 *%a
  ret i64 %0
}

define float @load_float(float * %a) nounwind readnone ssp {
entry:
; CHECK: vloadd    (%rdi){1to16}, %v0
  %0 = load float *%a
  ret float %0
}

define double @load_double(double * %a) nounwind readnone ssp {
entry:
; CHECK: vloadq    (%rdi){1to8}, %v0
  %0 = load double *%a
  ret double %0
}

define <16 x i32> @load_16i32(<16 x i32> * %a) nounwind readnone ssp {
entry:
; CHECK: vloadq    (%rdi), %v0
  %0 = load <16 x i32> *%a
  ret <16 x i32> %0
}

define <8 x i64> @load_8i64(<8 x i64> * %a) nounwind readnone ssp {
entry:
; CHECK: vloadq    (%rdi), %v0
  %0 = load <8 x i64> *%a
  ret <8 x i64> %0
}

define <16 x float> @load_16float(<16 x float> * %a) nounwind readnone ssp {
entry:
; CHECK: vloadq    (%rdi), %v0
  %0 = load <16 x float> *%a
  ret <16 x float> %0
}

define <8 x double> @load_8double(<8 x double> * %a) nounwind readnone ssp {
entry:
; CHECK: vloadq    (%rdi), %v0
  %0 = load <8 x double> *%a
  ret <8 x double> %0
}


; non aligned cases

define i64 @load_i64_na(i64 * %a) nounwind readnone ssp {
entry:
; CHECK: movq      (%rdi), %rax
  %0 = load i64 *%a, align 1
  ret i64 %0
}

;define float @load_float_na(float * %a) nounwind readnone ssp {
;entry:
;  %0 = load float *%a, align 2
;  ret float %0
;}

define double @load_double_na4(double * %a) nounwind readnone ssp {
entry:
; CHECK: vloadd    4(%rdi){1to16}, %v0
; CHECK: movl      $1, %eax
; CHECK: vkmov     %eax, %k1
; CHECK: vloadd    (%rdi){1to16}, %v0{%k1}
  %0 = load double *%a, align 4
  ret double %0
}

;define double @load_double_na1(double * %a) nounwind readnone ssp {
;entry:
;  %0 = load double *%a, align 1
;  ret double %0
;}

define <16 x i32> @load_16i32_na2(<16 x i32> * %a) nounwind readnone ssp {
entry:
; CHECK: testq     $3, %rdi
; CHECK: je 
; CHECK: movq      (%rdi), %r10
; CHECK: movq      %r10, -64(%rsp)
; CHECK: movq      8(%rdi), %r10
; CHECK: movq      %r10, -56(%rsp)
; CHECK: movq      16(%rdi), %r10
; CHECK: movq      %r10, -48(%rsp)
; CHECK: movq      24(%rdi), %r10
; CHECK: movq      %r10, -40(%rsp)
; CHECK: movq      32(%rdi), %r10
; CHECK: movq      %r10, -32(%rsp)
; CHECK: movq      40(%rdi), %r10
; CHECK: movq      %r10, -24(%rsp)
; CHECK: movq      48(%rdi), %r10
; CHECK: movq      %r10, -16(%rsp)
; CHECK: movq      56(%rdi), %r10
; CHECK: movq      %r10, -8(%rsp)
; CHECK: vloadd    -64(%rsp), %v0
; CHECK: jmp
; CHECK: vloadunpackld (%rdi), %v0
; CHECK: testq     $63, %rdi
; CHECK: je 
; CHECK: vloadunpackhd 64(%rdi), %v0
  %0 = load <16 x i32> *%a, align 2
  ret <16 x i32> %0
}

define <8 x i64> @load_8i64_na4(<8 x i64> * %a) nounwind readnone ssp {
entry:
; CHECK: vloadunpackld (%rdi), %v0
; CHECK: testq     $63, %rdi
; CHECK: je 
; CHECK: vloadunpackhd 64(%rdi), %v0
  %0 = load <8 x i64> *%a, align 4
  ret <8 x i64> %0
}


@gk16 = common global <16 x i1> zeroinitializer, align 8
@gk8 = common global <8 x i1> zeroinitializer, align 8

define <16 x i1> @load_gk16 () nounwind readnone ssp {
entry:
; CHECK:  movl      gk16(%rip), %eax
; CHECK:  vkmov     %eax, %k1
  %0 = load <16 x i1> * @gk16, align 8
  ret <16 x i1> %0
}

define <16 x i1> @load_16i1(<16 x i1> * %a) nounwind readnone ssp {
entry:
; CHECK:  movzwl    (%rdi), %eax
; CHECK:  vkmov     %eax, %k1
  %0 = load <16 x i1> *%a, align 2
  ret <16 x i1> %0
}

define <8 x i1> @load_8i1(<8 x i1> * %a) nounwind readnone ssp {
entry:
; CHECK:  movl      (%rdi), %eax
; CHECK:  vkmov     %eax, %k1
  %0 = load <8 x i1> *%a, align 4
  ret <8 x i1> %0
}

define <8 x i1> @load_8i1_a2(<8 x i1> * %a) nounwind readnone ssp {
entry:
; CHECK:  movb      (%rdi), %al
; CHECK:  vkmov     %eax, %k1
  %0 = load <8 x i1> *%a, align 2
  ret <8 x i1> %0
}

