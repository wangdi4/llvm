; extract from polyhedron/capacita
; CMPLRLLVM-28038
; For performance reasons, we don't want to hoist the repeated sdivs to the
; entry block. Disabling hoisting is worth 6% on this benchmarks.
; In llorg, this is happening in the late-LTO SimplifyCFG pass.
; The hoisting is disabled in xmain in the pass builder.

<<<<<<< HEAD
; This test fails when the new pass manager is enabled by default.
; In the case where clang is invoked (which it shouldn't be here), the
; LowerSubscriptIntrinsicPass inserts poison and the whole function eventually
; gets reduced to unreachable by SimplifyCFG.
; CMPLRLLVM-37054
; XFAIL: new_pm_default
; REQUIRES: DEBUG

=======
>>>>>>> e1e9b112731a4414282193c853b7ca47725feda4
; RUN: opt -S -passes="lto<O3>" %s | FileCheck %s
; RUN: clang -flegacy-pass-manager -O3 -o - -S -flto -emit-llvm %s | FileCheck %s
; RUN: clang -fno-legacy-pass-manager -mllvm -loopopt=1 -O3 -o - -S -flto -emit-llvm %s | FileCheck %s

; CHECK-LABEL: alloca_16
; CHECK-NOT: sdiv
; CHECK-LABEL: bb_new989_then

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%complex_64bit*$rank1$.x577.631" = type { %complex_64bit.x630*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%complex_64bit.x630 = type { float, float }

; Function Attrs: nounwind uwtable
define void @solv_cap_mp_fourir_(%"QNCA_a0$%complex_64bit*$rank1$.x577.631"* noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %A, i32* noalias readonly dereferenceable(4) %NTOT, i32* noalias readonly dereferenceable(4) %USEOLD) #0 {
alloca_16:
  %"solv_cap_mp_fourir_$EH" = alloca %complex_64bit.x630, align 8
  %"solv_cap_mp_fourir_$INC" = alloca i32, align 8
  %"var$342" = alloca i32, align 4
  %USEOLD_fetch.2108 = load i32, i32* %USEOLD, align 1
  %rel.418 = icmp eq i32 %USEOLD_fetch.2108, 0
  %int_zext = zext i1 %rel.418 to i32
  %int_zext30 = trunc i32 %int_zext to i1
  br i1 %int_zext30, label %bb_new989_then, label %bb507_endif

bb_new989_then:                                   ; preds = %alloca_16
  %NTOT_fetch.2109 = load i32, i32* %NTOT, align 1
  %div.38 = sdiv i32 %NTOT_fetch.2109, 4
  %sub.244 = sub nsw i32 %div.38, 1
  store i32 %sub.244, i32* %"var$342", align 1
  %"var$342_fetch.2110" = load i32, i32* %"var$342", align 1
  %rel.419 = icmp slt i32 %"var$342_fetch.2110", 1
  call void @llvm.assume(i1 %rel.419)
  br label %bb507_endif

bb507_endif:                                      ; preds = %bb_new989_then, %alloca_16
  br label %bb508

bb508:                                            ; preds = %bb_new1018_else, %bb508, %bb507_endif
  br i1 undef, label %bb508, label %bb_new1018_else

bb_new1018_else:                                  ; preds = %bb508
  %NTOT_fetch.2292 = load i32, i32* %NTOT, align 1
  %div.46 = sdiv i32 %NTOT_fetch.2292, 4
  %sub.259 = sub nsw i32 %div.46, 0
  %int_sext291 = sext i32 %sub.259 to i64
  %"E.addr_a0$_fetch.2290[]" = call %complex_64bit.x630* @llvm.intel.subscript.p0s_complex_64bit.x630s.i64.i64.p0s_complex_64bit.x630s.i64(i8 0, i64 1, i64 undef, %complex_64bit.x630* elementtype(%complex_64bit.x630) undef, i64 %int_sext291)
  %"E.addr_a0$_fetch.2290[]_fetch.2296" = load %complex_64bit.x630, %complex_64bit.x630* %"E.addr_a0$_fetch.2290[]", align 1
  %"E.addr_a0$_fetch.2290[]_fetch.2296_comp_1" = extractvalue %complex_64bit.x630 %"E.addr_a0$_fetch.2290[]_fetch.2296", 1
  %neg.44 = fneg fast float %"E.addr_a0$_fetch.2290[]_fetch.2296_comp_1"
  %insertval296 = insertvalue %complex_64bit.x630 zeroinitializer, float %neg.44, 1
  store %complex_64bit.x630 %insertval296, %complex_64bit.x630* %"solv_cap_mp_fourir_$EH", align 1
  %"A.addr_a0$_fetch.2316[]" = call %complex_64bit.x630* @llvm.intel.subscript.p0s_complex_64bit.x630s.i64.i64.p0s_complex_64bit.x630s.i64(i8 0, i64 0, i64 undef, %complex_64bit.x630* elementtype(%complex_64bit.x630) undef, i64 undef)
  %"A.addr_a0$_fetch.2316[]_fetch.2321" = load %complex_64bit.x630, %complex_64bit.x630* %"A.addr_a0$_fetch.2316[]", align 1
  %"solv_cap_mp_fourir_$EH_fetch.2322" = load %complex_64bit.x630, %complex_64bit.x630* %"solv_cap_mp_fourir_$EH", align 1
  %"A.addr_a0$_fetch.2316[]_fetch.2321_comp_1" = extractvalue %complex_64bit.x630 %"A.addr_a0$_fetch.2316[]_fetch.2321", 1
  %"solv_cap_mp_fourir_$EH_fetch.2322_comp_1" = extractvalue %complex_64bit.x630 %"solv_cap_mp_fourir_$EH_fetch.2322", 1
  %mul.354 = fmul fast float %"A.addr_a0$_fetch.2316[]_fetch.2321_comp_1", %"solv_cap_mp_fourir_$EH_fetch.2322_comp_1"
  %sub.263 = fsub fast float 0.000000e+00, %mul.354
  %insertval335 = insertvalue %complex_64bit.x630 zeroinitializer, float %sub.263, 0
  %insertval336 = insertvalue %complex_64bit.x630 %insertval335, float undef, 1
  %insertval_comp_0337 = extractvalue %complex_64bit.x630 %insertval336, 0
  %add.590 = fadd fast float 0.000000e+00, %insertval_comp_0337
  %insertval339 = insertvalue %complex_64bit.x630 zeroinitializer, float %add.590, 0
  %insertval340 = insertvalue %complex_64bit.x630 %insertval339, float undef, 1
  %"A.addr_a0$341" = getelementptr inbounds %"QNCA_a0$%complex_64bit*$rank1$.x577.631", %"QNCA_a0$%complex_64bit*$rank1$.x577.631"* %A, i32 0, i32 0
  %"A.addr_a0$_fetch.2323" = load %complex_64bit.x630*, %complex_64bit.x630** %"A.addr_a0$341", align 1
  %"A.addr_a0$_fetch.2323[]" = call %complex_64bit.x630* @llvm.intel.subscript.p0s_complex_64bit.x630s.i64.i64.p0s_complex_64bit.x630s.i64(i8 0, i64 0, i64 undef, %complex_64bit.x630* elementtype(%complex_64bit.x630) %"A.addr_a0$_fetch.2323", i64 undef)
  store %complex_64bit.x630 %insertval340, %complex_64bit.x630* %"A.addr_a0$_fetch.2323[]", align 1
  br label %bb508
}

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

; Function Attrs: nounwind readnone speculatable
declare %complex_64bit.x630* @llvm.intel.subscript.p0s_complex_64bit.x630s.i64.i64.p0s_complex_64bit.x630s.i64(i8, i64, i64, %complex_64bit.x630*, i64) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
