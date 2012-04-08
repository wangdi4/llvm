; This test should cover the MIC-specific changes done in X86TargetLowering::LowerSINT_TO_FP
; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

define float @A(i16* %a) nounwind readnone ssp {
entry:
  %i = load i16* %a
  %conv = sitofp i16 %i to float
  ret float %conv
}

define float @B(i8* %a) nounwind readnone ssp {
entry:
  %i = load i8* %a
  %conv = sitofp i8 %i to float
  ret float %conv
}

define double @C(i16* %a) nounwind readnone ssp {
entry:
  %i = load i16* %a
  %conv = sitofp i16 %i to double
  ret double %conv
}

define double @D(i8* %a) nounwind readnone ssp {
entry:
  %i = load i8* %a
  %conv = sitofp i8 %i to double
  ret double %conv
}

define <16 x float> @E(<16 x i16>* %a) nounwind readnone ssp {
entry:
  %i = load <16 x i16>* %a
  %conv = sitofp <16 x i16> %i to <16 x float>
  ret <16 x float> %conv
}

define <16 x float> @F(<16 x i8>* %a) nounwind readnone ssp {
entry:
  %i = load <16 x i8>* %a
  %conv = sitofp <16 x i8> %i to <16 x float>
  ret <16 x float> %conv
}

define <8 x double> @G(<8 x i16>* %a) nounwind readnone ssp {
entry:
  %i = load <8 x i16>* %a
  %conv = sitofp <8 x i16> %i to <8 x double>
  ret <8 x double> %conv
}

define <8 x double> @H(<8 x i8>* %a) nounwind readnone ssp {
entry:
  %i = load <8 x i8>* %a
  %conv = sitofp <8 x i8> %i to <8 x double>
  ret <8 x double> %conv
}

define float @Au(i16* %a) nounwind readnone ssp {
entry:
  %i = load i16* %a
  %conv = uitofp i16 %i to float
  ret float %conv
}

define float @Bu(i8* %a) nounwind readnone ssp {
entry:
  %i = load i8* %a
  %conv = uitofp i8 %i to float
  ret float %conv
}

define double @Cu(i16* %a) nounwind readnone ssp {
entry:
  %i = load i16* %a
  %conv = uitofp i16 %i to double
  ret double %conv
}

define double @Du(i8* %a) nounwind readnone ssp {
entry:
  %i = load i8* %a
  %conv = uitofp i8 %i to double
  ret double %conv
}

define <16 x float> @Eu(<16 x i16>* %a) nounwind readnone ssp {
entry:
  %i = load <16 x i16>* %a
  %conv = uitofp <16 x i16> %i to <16 x float>
  ret <16 x float> %conv
}

define <16 x float> @Fu(<16 x i8>* %a) nounwind readnone ssp {
entry:
  %i = load <16 x i8>* %a
  %conv = uitofp <16 x i8> %i to <16 x float>
  ret <16 x float> %conv
}

define <8 x double> @Gu(<8 x i16>* %a) nounwind readnone ssp {
entry:
  %i = load <8 x i16>* %a
  %conv = uitofp <8 x i16> %i to <8 x double>
  ret <8 x double> %conv
}

define <8 x double> @Hu(<8 x i8>* %a) nounwind readnone ssp {
entry:
  %i = load <8 x i8>* %a
  %conv = uitofp <8 x i8> %i to <8 x double>
  ret <8 x double> %conv
}
