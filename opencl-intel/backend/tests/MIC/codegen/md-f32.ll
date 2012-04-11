; XFAIL: win32
; XFAIL: *
; RUN: opt < %p/knc-%b -runtimelib %p/../../../vectorizer/Full/runtime.bc \
; RUN:       -std-compile-opts -inline-threshold=4096 -inline -lowerswitch \
; RUN:       -scalarize -mergereturn -loopsimplify -phicanon -predicate \
; RUN:       -mem2reg -dce -packetize -packet-size=16 -resolve -verify -S \
; RUN:     | llc -O2 -mtriple=x86_64-pc-linux \
; RUN:           -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %p/knc-%b -check-prefix=KNF
