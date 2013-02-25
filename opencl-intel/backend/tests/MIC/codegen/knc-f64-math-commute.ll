; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@f = common global double 0.000000e+00, align 4    ; <double*> [#uses=8]

define double @add1(double %a) nounwind readonly ssp {
entry:
; KNC:  vaddpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp1 = load double* @f                          ; <double> [#uses=1]
  %add = fadd double %tmp1, %a                     ; <double> [#uses=1]
  ret double %add
}

define double @add2(double %a) nounwind readonly ssp {
entry:
; KNC:  vaddpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp = load double* @f                           ; <double> [#uses=1]
  %add = fadd double %a, %tmp                    ; <double> [#uses=1]
  ret double %add
}

define double @mul1(double %a) nounwind readonly ssp {
entry:
; KNC:  vmulpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp1 = load double* @f                          ; <double> [#uses=1]
  %mul = fmul double %tmp1, %a                     ; <double> [#uses=1]
  ret double %mul
}


define double @mul2(double %a) nounwind readonly ssp {
entry:
; KNC:  vmulpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp = load double* @f                           ; <double> [#uses=1]
  %mul = fmul double %a, %tmp                      ; <double> [#uses=1]
  ret double %mul
}

define double @sub1(double %a) nounwind readonly ssp {
entry:
; KNC:  vsubpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp1 = load double* @f                          ; <double> [#uses=1]
  %sub = fsub double %a, %tmp1                     ; <double> [#uses=1]
  ret double %sub
}

define double @sub2(double %a) nounwind readonly ssp {
entry:
; KNC:  vsubrpd    f(%rip){1to8}, %zmm0, %zmm0{%k1}
  %tmp = load double* @f                           ; <double> [#uses=1]
  %sub = fsub double %tmp, %a                      ; <double> [#uses=1]
  ret double %sub
}
