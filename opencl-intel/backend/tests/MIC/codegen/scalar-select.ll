; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x float> @test0(i1 %x, <16 x float> %v1, <16 x float> %v2) nounwind readnone {
entry:
; KNF: test0:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
	%.0 = select i1 %x, <16 x float> %v1, <16 x float> %v2		; <i32> [#uses=1]
	ret <16 x float> %.0
}

define float @test11(i1 %x, float %v1, float %v2) nounwind readnone {
entry:
; KNF: test11:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, float %v1, float %v2		; <i32> [#uses=1]
	ret float %.0
}

define float @test12(i1 %x, float %v1, float %v2) nounwind readnone {
entry:
; KNF: test12:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, float %v2, float %v1		; <i32> [#uses=1]
	ret float %.0
}

define float @test21(i1 %x, float %v0) nounwind readnone {
entry:
; KNF: test21:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, float 12.0, float %v0
	ret float %.0
}

define float @test22(i1 %x, float %v0) nounwind readnone {
entry:
; KNF: test22:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, float %v0, float 12.0
	ret float %.0
}

define float @test31(i1 %x, float %v1, float* %vp2) nounwind readonly {
entry:
; KNF: test31:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %v2 = load float* %vp2
	%.0 = select i1 %x, float %v1, float %v2		; <i32> [#uses=1]
	ret float %.0
}

define float @test32(i1 %x, float %v1, float* %vp2) nounwind readonly {
entry:
; KNF: test32:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %v2 = load float* %vp2
	%.0 = select i1 %x, float %v2, float %v1		; <i32> [#uses=1]
	ret float %.0
}

; in the following 2 tests the select is promoted to work on teh addresses
; rather than the value
define float @test41(i1 %x, float* %vp0) nounwind readonly {
entry:
; KNF: test41:
; KNF: vloadd    (%{{[a-z]+}}){1to16}, %v0
  %v0 = load float* %vp0
	%.0 = select i1 %x, float 12.0, float %v0
	ret float %.0
}

define float @test42(i1 %x, float* %vp0) nounwind readonly {
entry:
; KNF: test42:
; KNF: vloadd    (%{{[a-z]+}}){1to16}, %v0
  %v0 = load float* %vp0
	%.0 = select i1 %x, float %v0, float 12.0
	ret float %.0
}

define <8 x double> @testd0(i1 %x, <8 x double> %v1, <8 x double> %v2) nounwind readnone {
entry:
; KNF: testd0:
; KNF: vorpq {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+{%k[1-9]}}}
	%.0 = select i1 %x, <8 x double> %v1, <8 x double> %v2		; <i32> [#uses=1]
	ret <8 x double> %.0
}

; in the following tests, since the whole vector is moved vorpi is 
; as good as vorpq
define double @testd11(i1 %x, double %v1, double %v2) nounwind readnone {
entry:
; KNF: testd11:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, double %v1, double %v2		; <i32> [#uses=1]
	ret double %.0
}

define double @testd12(i1 %x, double %v1, double %v2) nounwind readnone {
entry:
; KNF: testd12:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, double %v2, double %v1		; <i32> [#uses=1]
	ret double %.0
}

define double @testd21(i1 %x, double %v0) nounwind readnone {
entry:

; KNF: testd21:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, double 4.5e+15, double %v0
	ret double %.0
}

define double @testd22(i1 %x, double %v0) nounwind readnone {
entry:

; KNF: testd22:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
	%.0 = select i1 %x, double %v0, double 4.5e+15
	ret double %.0
}

define double @testd31(i1 %x, double %v1, double* %vp2) nounwind readonly {
entry:

; KNF: testd31:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %v2 = load double* %vp2
	%.0 = select i1 %x, double %v1, double %v2		; <i32> [#uses=1]
	ret double %.0
}

define double @testd32(i1 %x, double %v1, double* %vp2) nounwind readonly {
entry:

; KNF: testd32:
; KNF: vorpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %v2 = load double* %vp2
	%.0 = select i1 %x, double %v2, double %v1		; <i32> [#uses=1]
	ret double %.0
}

define double @testd41(i1 %x, double* %vp0) nounwind readonly {
entry:
; KNF: testd41:
; KNF: vloadq    (%{{[a-z]+}}){1to8}, %v0
  %v0 = load double* %vp0
	%.0 = select i1 %x, double 4.5e+15, double %v0
	ret double %.0
}

define double @testd42(i1 %x, double* %vp0) nounwind readonly {
entry:
; KNF: testd42:
; KNF: vloadq    (%{{[a-z]+}}){1to8}, %v0
  %v0 = load double* %vp0
	%.0 = select i1 %x, double %v0, double 4.5e+15
	ret double %.0
}

define i64 @testq0(i1 %x, i64 %v0, i64 %v1) nounwind readnone {
entry:
; KNF: testq0:
; KNF: jne
	%.0 = select i1 %x, i64 %v0, i64 %v1
	ret i64 %.0
}

