; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;
;

target datalayout = "e-p:64:64"

%SArray = type { [100 x i64]}

define  void @loopo(%SArray* %p, i32 %i, i32 %j, i64 %v) nounwind {
entry:
; CHECK: movq {{%[a-z]+}}, ({{%[a-z]+}},{{%[a-z]+}},8)
  %vp = getelementptr %SArray* %p, i32 %i, i32 0, i32 %j
  store i64 %v, i64 * %vp
  ret void
}

%RT = type { i8 , [10 x [20 x i32]], i8  }
%ST = type { i32, double, %RT }

define void @foo(%ST* %s, i32 %k, i32 %v) {
entry:
; CHECK: movl {{%[a-z]+}}, 1244({{%[a-z]+}},{{%[a-z]+}},4)
  %reg = getelementptr %ST* %s, i32 1, i32 2, i32 1, i32 5, i32 %k
  store i32 %v, i32 * %reg
  ret void
}



@gbf = common global float zeroinitializer, align 8
@gbs = common global i64 zeroinitializer, align 8


define void @store_gbsi(float %v) nounwind readnone ssp {
entry:
; CHECK: vpackstorelps %zmm0, 20+gbf(%rip){%k1}
  %addr = getelementptr float* @gbf, i32 5
  store float %v, float * %addr, align 8
  ret void
}


define void @store_gbs(i64 %v) nounwind readnone ssp {
entry:
; CHECK: movq      %rdi, gbs(%rip)
  store i64 %v, i64 * @gbs, align 8
  ret void
}

define void @store_i16(i16 * %a, i16 %v) nounwind readnone ssp {
entry:
; CHECK: movw %si, (%rdi)
  store i16 %v, i16 *%a
  ret void
}

define void @store_i8(i8 * %a, i8 %v) nounwind readnone ssp {
entry:
; CHECK:         movb      %sil, (%rdi)
  store i8 %v, i8 *%a
  ret void
}

define void @store_i32(i32 * %a, i32 %v) nounwind readnone ssp {
entry:
; CHECK: movl      %esi, (%rdi)
  store i32 %v, i32 *%a
  ret void
}

define void @store_i64(i64 * %a, i64 %v) nounwind readnone ssp {
entry:
; CHECK: movq      %rsi, (%rdi)
  store i64 %v, i64 *%a
  ret void
}

define void @store_float(float * %a, float %v) nounwind readnone ssp {
entry:
; CHECK: vpackstorelps %zmm0, (%rdi){%k1}
  store float %v, float *%a
  ret void
}

define void @store_double(double * %a, double %v) nounwind readnone ssp {
entry:
; CHECK: vpackstorelpd %zmm0, (%rdi){%k1}
  store double %v, double *%a
  ret void
}

define void @store_16i32(<16 x i32> * %a, <16 x i32> %v) nounwind readnone ssp {
entry:
; CHECK: vmovdqa32 %zmm0, (%rdi)
  store <16 x i32> %v, <16 x i32> *%a
  ret void
}

define void @store_8i64(<8 x i64> * %a, <8 x i64> %v) nounwind readnone ssp {
entry:
; CHECK: vmovdqa64 %zmm0, (%rdi)
  store <8 x i64>  %v, <8 x i64> *%a
  ret void
}

define void @store_16float(<16 x float> * %a, <16 x float> %v) nounwind readnone ssp {
entry:
; CHECK: vmovdqa32 %zmm0, (%rdi)
  store <16 x float>%v, <16 x float> *%a
  ret void
}

define void @store_8double(<8 x double> * %a, <8 x double> %v) nounwind readnone ssp {
entry:
; CHECK: vmovdqa64 %zmm0, (%rdi)
  store <8 x double> %v, <8 x double> *%a
  ret void
}


; non aligned cases

define void @store_i64_na(i64 * %a, i64 %v) nounwind readnone ssp {
entry:
; CHECK: movq      %rsi, (%rdi)
  store i64 %v, i64 *%a, align 1
  ret void
}

;define void @store_float_na(float * %a, float %v) nounwind readnone ssp {
;entry:
;  store float %v, float *%a, align 2
;  ret void
;}

define void @store_double_na4(double * %a, double %v) nounwind readnone ssp {
entry:
; CHECK: movl      $1, %eax
; CHECK: kmov     %eax, %k1
; CHECK: movl      $2, %eax
; CHECK: kmov     %eax, %k2
; CHECK: vpackstoreld %zmm0, (%rdi){%k1}
; CHECK: vpackstoreld %zmm0, 4(%rdi){%k2}
  store double %v, double *%a, align 4
  ret void
}

;define void @store_double_na1(double * %a, double %v) nounwind readnone ssp {
;entry:
;  store double %v, double *%a, align 1
;  ret void
;}

define void @store_16i32_na2(<16 x i32> * %a, <16 x i32>%v) nounwind readnone ssp {
entry:
; CHECK: vpackstorehd
; CHECK: vpackstoreld
  store <16 x i32> %v, <16 x i32> *%a, align 2
  ret void
}

define void @store_8i64_na4(<8 x i64> * %a, <8 x i64> %v) nounwind readnone ssp {
entry:
; CHECK: vpackstoreld
; CHECK: vpackstorehd
  store <8 x i64> %v, <8 x i64> *%a, align 4
  ret void
}

@gk16 = common global <16 x i1> zeroinitializer, align 8
@gk8 = common global <8 x i1> zeroinitializer, align 8

define void @store_gk16 (<16 x i1> %v) nounwind readnone ssp {
entry:
; CHECK:  kmov     %k1, %e[[R0:[a-z]+]]
; CHECK:  movw      %[[R0]], gk16(%rip)
  store <16 x i1> %v, <16 x i1> * @gk16, align 8
  ret void
}

define void @store_v16i1 (<16 x i1> %v, <16 x i1> *%p) nounwind readnone ssp {
entry:
; CHECK:  kmov     %k1, %e[[R0:[a-z]+]]
; CHECK:  movw      %[[R0]], (%rdi)
  store <16 x i1> %v, <16 x i1> * %p, align 8
  ret void
}

define void @store_v8i1 (<8 x i1> %v, <8 x i1> *%p) nounwind readnone ssp {
entry:
; CHECK:  kmov      %k1, %eax
; CHECK:  movb      %al, (%rdi)
  store <8 x i1> %v, <8 x i1> * %p, align 1
  ret void
}

