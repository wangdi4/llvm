// INTEL_FEATURE_CPU_GLC
// REQUIRES: intel_feature_cpu_glc

// RUN: not %clang_cc1 -triple i386--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86
// X86: error: unknown target CPU 'not-a-cpu'
// X86: note: valid target CPU values are: i386, i486, winchip-c6, winchip2, c3,
// X86-SAME: i586, pentium, pentium-mmx, pentiumpro, i686, pentium2, pentium3,
// X86-SAME: pentium3m, pentium-m, c3-2, yonah, pentium4, pentium4m, prescott,
// X86-SAME: nocona, core2, penryn, bonnell, atom, silvermont, slm, goldmont, goldmont-plus, tremont,
// X86-SAME: nehalem, corei7, westmere, sandybridge, corei7-avx, ivybridge,
// X86-SAME: core-avx-i, haswell, core-avx2, broadwell, common-avx512, skylake, skylake-avx512,
// X86-SAME: skx, cascadelake, cannonlake, icelake-client, icelake-server, goldencove, knl, knm, lakemont, k6, k6-2, k6-3,
// X86-SAME: athlon, athlon-tbird, athlon-xp, athlon-mp, athlon-4, k8, athlon64,
// X86-SAME: athlon-fx, opteron, k8-sse3, athlon64-sse3, opteron-sse3, amdfam10,
// X86-SAME: barcelona, btver1, btver2, bdver1, bdver2, bdver3, bdver4, znver1,
// X86-SAME: znver2, x86-64, geode

// RUN: not %clang_cc1 -triple x86_64--- -target-cpu not-a-cpu -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix X86_64
// X86_64: error: unknown target CPU 'not-a-cpu'
// X86_64: note: valid target CPU values are: nocona, core2, penryn, bonnell,
// X86_64-SAME: atom, silvermont, slm, goldmont, goldmont-plus, tremont, nehalem, corei7, westmere,
// X86_64-SAME: sandybridge, corei7-avx, ivybridge, core-avx-i, haswell,
// X86_64-SAME: core-avx2, broadwell, common-avx512, skylake, skylake-avx512, skx, cascadelake, cannonlake,
// X86_64-SAME: icelake-client, icelake-server, goldencove, knl, knm, k8, athlon64, athlon-fx, opteron, k8-sse3,
// X86_64-SAME: athlon64-sse3, opteron-sse3, amdfam10, barcelona, btver1,
// X86_64-SAME: btver2, bdver1, bdver2, bdver3, bdver4, znver1, znver2, x86-64

// end INTEL_FEATURE_CPU_GLC
