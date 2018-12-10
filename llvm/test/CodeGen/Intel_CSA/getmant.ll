; RUN: llc -mtriple=csa < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define double @getmantf(double %v) #0 {
entry:
  %0 = tail call double asm "getmantf64 $0, $1, SIGNCTL_FORCE, INTERVAL1", "=d,d"(double %v) #0
; CHECK: getmantf
; CHECK: getmantf64 %{{[a-z0-9_]+}}, %{{[a-z0-9_]+}}, SIGNCTL_FORCE, INTERVAL1
  ret double %0
}

define double @getmantf2(double %v) #0 {
entry:
  %0 = tail call double asm "getmantf64 $0, $1, SIGNCTL_PROP, INTERVAL3", "=d,d"(double %v) #0
; CHECK: getmantf2
; CHECK: getmantf64 %{{[a-z0-9_]+}}, %{{[a-z0-9_]+}}, SIGNCTL_PROP, INTERVAL3
  ret double %0
}

attributes #0 = { nounwind readnone }
