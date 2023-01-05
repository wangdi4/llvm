; REQUIRES: intel_feature_isa_amx_lnc
; RUN: llc -enable-ipra -print-regusage -o /dev/null 2>&1 < %s | FileCheck %s

target triple = "x86_64-unknown-unknown"
declare void @bar1()
define preserve_allcc void @foo()#0 {
;CHECK: foo Clobbered Registers: $cs $df $ds $eflags $eip $eiz $es $esp $fpcw $fpsw $fs $gs $hip $hsp $ip $mxcsr $rip $riz $rsp $sp $sph $spl $ss $ssp $tmmcfg $cr0 $cr1 $cr2 $cr3 $cr4 $cr5 $cr6 $cr7 $cr8 $cr9 $cr10 $cr11 $cr12 $cr13 $cr14 $cr15 $dr0 $dr1 $dr2 $dr3 $dr4 $dr5 $dr6 $dr7 $dr8 $dr9 $dr10 $dr11 $dr12 $dr13 $dr14 $dr15 $fp0 $fp1 $fp2 $fp3 $fp4 $fp5 $fp6 $fp7 $k0 $k1 $k2 $k3 $k4 $k5 $k6 $k7 $mm0 $mm1 $mm2 $mm3 $mm4 $mm5 $mm6 $mm7 $r11 $st0 $st1 $st2 $st3 $st4 $st5 $st6 $st7 $tmm0 $tmm1 $tmm2 $tmm3 $tmm4 $tmm5 $tmm6 $tmm7 $tmm8 $tmm9 $tmm10 $tmm11 $tmm12 $tmm13 $tmm14 $tmm15 $tmm16 $tmm17 $tmm18 $tmm19 $tmm20 $tmm21 $tmm22 $tmm23 $tmm24 $tmm25 $tmm26 $tmm27 $tmm28 $tmm29 $tmm30 $tmm31 $xmm16 $xmm17 $xmm18 $xmm19 $xmm20 $xmm21 $xmm22 $xmm23 $xmm24 $xmm25 $xmm26 $xmm27 $xmm28 $xmm29 $xmm30 $xmm31 $ymm0 $ymm1 $ymm2 $ymm3 $ymm4 $ymm5 $ymm6 $ymm7 $ymm8 $ymm9 $ymm10 $ymm11 $ymm12 $ymm13 $ymm14 $ymm15 $ymm16 $ymm17 $ymm18 $ymm19 $ymm20 $ymm21 $ymm22 $ymm23 $ymm24 $ymm25 $ymm26 $ymm27 $ymm28 $ymm29 $ymm30 $ymm31 $zmm0 $zmm1 $zmm2 $zmm3 $zmm4 $zmm5 $zmm6 $zmm7 $zmm8 $zmm9 $zmm10 $zmm11 $zmm12 $zmm13 $zmm14 $zmm15 $zmm16 $zmm17 $zmm18 $zmm19 $zmm20 $zmm21 $zmm22 $zmm23 $zmm24 $zmm25 $zmm26 $zmm27 $zmm28 $zmm29 $zmm30 $zmm31 $r11b $r11bh $r11d $r11w $r11wh $k0_k1 $k2_k3 $k4_k5 $k6_k7 $tmm0_tmm1 $tmm2_tmm3 $tmm4_tmm5 $tmm6_tmm7 $tmm8_tmm9 $tmm10_tmm11 $tmm12_tmm13 $tmm14_tmm15 $tmm16_tmm17 $tmm18_tmm19 $tmm20_tmm21 $tmm22_tmm23 $tmm24_tmm25 $tmm26_tmm27 $tmm28_tmm29 $tmm30_tmm31 $tmm0_tmm1_tmm2_tmm3 $tmm4_tmm5_tmm6_tmm7 $xmm0_xmm1 $xmm2_xmm3 $xmm4_xmm5 $xmm6_xmm7 $xmm8_xmm9 $xmm10_xmm11 $xmm12_xmm13 $xmm14_xmm15 $xmm16_xmm17 $xmm18_xmm19 $xmm20_xmm21 $xmm22_xmm23 $xmm24_xmm25 $xmm26_xmm27 $xmm28_xmm29 $xmm30_xmm31 $ymm0_ymm1 $ymm2_ymm3 $ymm4_ymm5 $ymm6_ymm7 $ymm8_ymm9 $ymm10_ymm11 $ymm12_ymm13 $ymm14_ymm15 $ymm16_ymm17 $ymm18_ymm19 $ymm20_ymm21 $ymm22_ymm23 $ymm24_ymm25 $ymm26_ymm27 $ymm28_ymm29 $ymm30_ymm31 $zmm16_zmm17_zmm18_zmm19_zmm20_zmm21_zmm22_zmm23_zmm24_zmm25_zmm26_zmm27_zmm28_zmm29_zmm30_zmm31
  call void @bar1()
  call void @bar2()
  ret void
}
declare void @bar2()

@llvm.used = appending global [1 x i8*] [i8* bitcast (void ()* @foo to i8*)]
attributes #0 = {nounwind}
