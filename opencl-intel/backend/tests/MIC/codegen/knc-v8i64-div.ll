; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @rem1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: to remove due to performance bug
; CHECK: rem1:
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: rem1
  %div = sdiv <8 x i64> %a, %b
  ret <8 x i64> %div
}

define <8 x i64> @rem2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: rem2:
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: rem2
  %tmp1 = load <8 x i64>* %a, align 64
  %div = sdiv <8 x i64> %tmp1, %b
  ret <8 x i64> %div
}

define <8 x i64> @rem3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: rem3:
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: rem3
  %tmp2 = load <8 x i64>* %b, align 64
  %div = sdiv <8 x i64> %a, %tmp2
  ret <8 x i64> %div
}

define <8 x i64> @rem4(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: rem4:
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: rem4
  %tmp1 = load <8 x i64>* @gb, align 64
  %div = sdiv <8 x i64> %a, %tmp1
  ret <8 x i64> %div
}

define <8 x i64> @rem5(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: rem5:
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %r{{.*}}
; CHECK: movq -{{[0-9]*}}(%rbp), %r{{.*}}
; CHECK rem5
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %div = sdiv <8 x i64> %a, %tmp2
  ret <8 x i64> %div
}
