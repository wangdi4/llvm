// RUN: %clang_cl --target=i386 -march=i386 -Qmx87 %s -### 2>&1 | FileCheck -check-prefix=X87 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-x87 %s -### 2>&1 | FileCheck -check-prefix=NO-X87 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qm80387 %s -### 2>&1 | FileCheck -check-prefix=X87 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-80387 %s -### 2>&1 | FileCheck -check-prefix=NO-X87 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-fp-ret-in-387 %s -### 2>&1 | FileCheck -check-prefix=NO-X87 %s
// X87: "-target-feature" "+x87"
// NO-X87: "-target-feature" "-x87"

// RUN: %clang_cl --target=i386 -march=i386 -Qmmmx -Qm3dnow -Qm3dnowa %s -### 2>&1 | FileCheck -check-prefix=MMX %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-mmx -Qmno-3dnow -Qmno-3dnowa %s -### 2>&1 | FileCheck -check-prefix=NO-MMX %s
// MMX: "-target-feature" "+mmx" "-target-feature" "+3dnow" "-target-feature" "+3dnowa"
// NO-MMX: "-target-feature" "-mmx" "-target-feature" "-3dnow" "-target-feature" "-3dnowa"

// RUN: %clang_cl --target=i386 -march=i386 -Qmsse -Qmsse2 -Qmsse3 -Qmssse3 -Qmsse4a -Qmsse4.1 -Qmsse4.2 %s -### 2>&1 | FileCheck -check-prefix=SSE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-sse -Qmno-sse2 -Qmno-sse3 -Qmno-ssse3 -Qmno-sse4a -Qmno-sse4.1 -Qmno-sse4.2 %s -### 2>&1 | FileCheck -check-prefix=NO-SSE %s
// SSE: "-target-feature" "+sse" "-target-feature" "+sse2" "-target-feature" "+sse3" "-target-feature" "+ssse3" "-target-feature" "+sse4a" "-target-feature" "+sse4.1" "-target-feature" "+sse4.2"
// NO-SSE: "-target-feature" "-sse" "-target-feature" "-sse2" "-target-feature" "-sse3" "-target-feature" "-ssse3" "-target-feature" "-sse4a" "-target-feature" "-sse4.1" "-target-feature" "-sse4.2"

// RUN: %clang_cl --target=i386 -march=i386 -Qmsse4 -Qmaes %s -### 2>&1 | FileCheck -check-prefix=SSE4-AES %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-sse4 -Qmno-aes %s -### 2>&1 | FileCheck -check-prefix=NO-SSE4-AES %s
// SSE4-AES: "-target-feature" "+sse4.2" "-target-feature" "+aes"
// NO-SSE4-AES: "-target-feature" "-sse4.1" "-target-feature" "-aes"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx -Qmavx2 -Qmavx512f -Qmavx512cd -Qmavx512er -Qmavx512pf -Qmavx512dq -Qmavx512bw -Qmavx512vl -Qmavx512vbmi -Qmavx512vbmi2 -Qmavx512ifma %s -### 2>&1 | FileCheck -check-prefix=AVX %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx -Qmno-avx2 -Qmno-avx512f -Qmno-avx512cd -Qmno-avx512er -Qmno-avx512pf -Qmno-avx512dq -Qmno-avx512bw -Qmno-avx512vl -Qmno-avx512vbmi -Qmno-avx512vbmi2 -Qmno-avx512ifma %s -### 2>&1 | FileCheck -check-prefix=NO-AVX %s
// AVX: "-target-feature" "+avx" "-target-feature" "+avx2" "-target-feature" "+avx512f" "-target-feature" "+avx512cd" "-target-feature" "+avx512er" "-target-feature" "+avx512pf" "-target-feature" "+avx512dq" "-target-feature" "+avx512bw" "-target-feature" "+avx512vl" "-target-feature" "+avx512vbmi" "-target-feature" "+avx512vbmi2" "-target-feature" "+avx512ifma"
// NO-AVX: "-target-feature" "-avx" "-target-feature" "-avx2" "-target-feature" "-avx512f" "-target-feature" "-avx512cd" "-target-feature" "-avx512er" "-target-feature" "-avx512pf" "-target-feature" "-avx512dq" "-target-feature" "-avx512bw" "-target-feature" "-avx512vl" "-target-feature" "-avx512vbmi" "-target-feature" "-avx512vbmi2" "-target-feature" "-avx512ifma"

// RUN: %clang_cl --target=i386 -march=i386 -Qmpclmul -Qmrdrnd -Qmfsgsbase -Qmbmi -Qmbmi2 %s -### 2>&1 | FileCheck -check-prefix=BMI %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-pclmul -Qmno-rdrnd -Qmno-fsgsbase -Qmno-bmi -Qmno-bmi2 %s -### 2>&1 | FileCheck -check-prefix=NO-BMI %s
// BMI: "-target-feature" "+pclmul" "-target-feature" "+rdrnd" "-target-feature" "+fsgsbase" "-target-feature" "+bmi" "-target-feature" "+bmi2"
// NO-BMI: "-target-feature" "-pclmul" "-target-feature" "-rdrnd" "-target-feature" "-fsgsbase" "-target-feature" "-bmi" "-target-feature" "-bmi2"

// RUN: %clang_cl --target=i386 -march=i386 -Qmlzcnt -Qmpopcnt -Qmtbm -Qmfma -Qmfma4 %s -### 2>&1 | FileCheck -check-prefix=FMA %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-lzcnt -Qmno-popcnt -Qmno-tbm -Qmno-fma -Qmno-fma4 %s -### 2>&1 | FileCheck -check-prefix=NO-FMA %s
// FMA: "-target-feature" "+lzcnt" "-target-feature" "+popcnt" "-target-feature" "+tbm" "-target-feature" "+fma" "-target-feature" "+fma4"
// NO-FMA: "-target-feature" "-lzcnt" "-target-feature" "-popcnt" "-target-feature" "-tbm" "-target-feature" "-fma" "-target-feature" "-fma4"

// RUN: %clang_cl --target=i386 -march=i386 -Qmxop -Qmf16c -Qmrtm -Qmprfchw -Qmrdseed %s -### 2>&1 | FileCheck -check-prefix=XOP %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-xop -Qmno-f16c -Qmno-rtm -Qmno-prfchw -Qmno-rdseed %s -### 2>&1 | FileCheck -check-prefix=NO-XOP %s
// XOP: "-target-feature" "+xop" "-target-feature" "+f16c" "-target-feature" "+rtm" "-target-feature" "+prfchw" "-target-feature" "+rdseed"
// NO-XOP: "-target-feature" "-xop" "-target-feature" "-f16c" "-target-feature" "-rtm" "-target-feature" "-prfchw" "-target-feature" "-rdseed"

// RUN: %clang_cl --target=i386 -march=i386 -Qmsha -Qmpku -Qmadx -Qmcx16 -Qmfxsr %s -### 2>&1 | FileCheck -check-prefix=SHA %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-sha -Qmno-pku -Qmno-adx -Qmno-cx16 -Qmno-fxsr %s -### 2>&1 | FileCheck -check-prefix=NO-SHA %s
// SHA: "-target-feature" "+sha" "-target-feature" "+pku" "-target-feature" "+adx" "-target-feature" "+cx16" "-target-feature" "+fxsr"
// NO-SHA: "-target-feature" "-sha" "-target-feature" "-pku" "-target-feature" "-adx" "-target-feature" "-cx16" "-target-feature" "-fxsr"

// RUN: %clang_cl --target=i386 -march=i386 -Qmxsave -Qmxsaveopt -Qmxsavec -Qmxsaves %s -### 2>&1 | FileCheck -check-prefix=XSAVE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-xsave -Qmno-xsaveopt -Qmno-xsavec -Qmno-xsaves %s -### 2>&1 | FileCheck -check-prefix=NO-XSAVE %s
// XSAVE: "-target-feature" "+xsave" "-target-feature" "+xsaveopt" "-target-feature" "+xsavec" "-target-feature" "+xsaves"
// NO-XSAVE: "-target-feature" "-xsave" "-target-feature" "-xsaveopt" "-target-feature" "-xsavec" "-target-feature" "-xsaves"

// RUN: %clang_cl --target=i386 -march=i386 -Qmclflushopt %s -### 2>&1 | FileCheck -check-prefix=CLFLUSHOPT %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-clflushopt %s -### 2>&1 | FileCheck -check-prefix=NO-CLFLUSHOPT %s
// CLFLUSHOPT: "-target-feature" "+clflushopt"
// NO-CLFLUSHOPT: "-target-feature" "-clflushopt"

// RUN: %clang_cl --target=i386 -march=i386 -Qmclwb %s -### 2>&1 | FileCheck -check-prefix=CLWB %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-clwb %s -### 2>&1 | FileCheck -check-prefix=NO-CLWB %s
// CLWB: "-target-feature" "+clwb"
// NO-CLWB: "-target-feature" "-clwb"

// RUN: %clang_cl --target=i386 -march=i386 -Qmwbnoinvd %s -### 2>&1 | FileCheck -check-prefix=WBNOINVD %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-wbnoinvd %s -### 2>&1 | FileCheck -check-prefix=NO-WBNOINVD %s
// WBNOINVD: "-target-feature" "+wbnoinvd"
// NO-WBNOINVD: "-target-feature" "-wbnoinvd"

// RUN: %clang_cl --target=i386 -march=i386 -Qmmovbe %s -### 2>&1 | FileCheck -check-prefix=MOVBE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-movbe %s -### 2>&1 | FileCheck -check-prefix=NO-MOVBE %s
// MOVBE: "-target-feature" "+movbe"
// NO-MOVBE: "-target-feature" "-movbe"

// RUN: %clang_cl --target=i386 -march=i386 -Qmshstk %s -### 2>&1 | FileCheck -check-prefix=CETSS %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-shstk %s -### 2>&1 | FileCheck -check-prefix=NO-CETSS %s
// CETSS: "-target-feature" "+shstk"
// NO-CETSS: "-target-feature" "-shstk"

// RUN: %clang_cl --target=i386 -march=i386 -Qmsgx %s -### 2>&1 | FileCheck -check-prefix=SGX %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-sgx %s -### 2>&1 | FileCheck -check-prefix=NO-SGX %s
// SGX: "-target-feature" "+sgx"
// NO-SGX: "-target-feature" "-sgx"

// RUN: %clang_cl --target=i386 -march=i386 -Qmprefetchwt1 %s -### 2>&1 | FileCheck -check-prefix=PREFETCHWT1 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-prefetchwt1 %s -### 2>&1 | FileCheck -check-prefix=NO-PREFETCHWT1 %s
// PREFETCHWT1: "-target-feature" "+prefetchwt1"
// NO-PREFETCHWT1: "-target-feature" "-prefetchwt1"

// RUN: %clang_cl --target=i386 -march=i386 -Qmclzero %s -### 2>&1 | FileCheck -check-prefix=CLZERO %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-clzero %s -### 2>&1 | FileCheck -check-prefix=NO-CLZERO %s
// CLZERO: "-target-feature" "+clzero"
// NO-CLZERO: "-target-feature" "-clzero"

// RUN: %clang_cl --target=i386 -march=i386 -Qmvaes %s -### 2>&1 | FileCheck -check-prefix=VAES %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-vaes %s -### 2>&1 | FileCheck -check-prefix=NO-VAES %s
// VAES: "-target-feature" "+vaes"
// NO-VAES: "-target-feature" "-vaes"

// RUN: %clang_cl --target=i386 -march=i386 -Qmgfni %s -### 2>&1 | FileCheck -check-prefix=GFNI %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-gfni %s -### 2>&1 | FileCheck -check-prefix=NO-GFNI %s
// GFNI: "-target-feature" "+gfni"
// NO-GFNI: "-target-feature" "-gfni

// RUN: %clang_cl --target=i386 -march=i386 -Qmvpclmulqdq %s -### 2>&1 | FileCheck -check-prefix=VPCLMULQDQ %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-vpclmulqdq %s -### 2>&1 | FileCheck -check-prefix=NO-VPCLMULQDQ %s
// VPCLMULQDQ: "-target-feature" "+vpclmulqdq"
// NO-VPCLMULQDQ: "-target-feature" "-vpclmulqdq"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx512bitalg %s -### 2>&1 | FileCheck -check-prefix=BITALG %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx512bitalg %s -### 2>&1 | FileCheck -check-prefix=NO-BITALG %s
// BITALG: "-target-feature" "+avx512bitalg"
// NO-BITALG: "-target-feature" "-avx512bitalg"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx512vnni %s -### 2>&1 | FileCheck -check-prefix=VNNI %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx512vnni %s -### 2>&1 | FileCheck -check-prefix=NO-VNNI %s
// VNNI: "-target-feature" "+avx512vnni"
// NO-VNNI: "-target-feature" "-avx512vnni"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx512vbmi2 %s -### 2>&1 | FileCheck -check-prefix=VBMI2 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx512vbmi2 %s -### 2>&1 | FileCheck -check-prefix=NO-VBMI2 %s
// VBMI2: "-target-feature" "+avx512vbmi2"
// NO-VBMI2: "-target-feature" "-avx512vbmi2"

// RUN: %clang_cl -target i386-linux-gnu -Qmavx512vp2intersect %s -### 2>&1 | FileCheck -check-prefix=VP2INTERSECT %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-avx512vp2intersect %s -### 2>&1 | FileCheck -check-prefix=NO-VP2INTERSECT %s
// VP2INTERSECT: "-target-feature" "+avx512vp2intersect"
// NO-VP2INTERSECT: "-target-feature" "-avx512vp2intersect"

// RUN: %clang_cl --target=i386 -march=i386 -Qmrdpid %s -### 2>&1 | FileCheck -check-prefix=RDPID %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-rdpid %s -### 2>&1 | FileCheck -check-prefix=NO-RDPID %s
// RDPID: "-target-feature" "+rdpid"
// NO-RDPID: "-target-feature" "-rdpid"

// RUN: %clang_cl --target=i386 -march=i386 -Qmrdpru %s -### 2>&1 | FileCheck -check-prefix=RDPRU %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-rdpru %s -### 2>&1 | FileCheck -check-prefix=NO-RDPRU %s
// RDPRU: "-target-feature" "+rdpru"
// NO-RDPRU: "-target-feature" "-rdpru"

// RUN: %clang_cl -target i386-linux-gnu -Qmretpoline %s -### 2>&1 | FileCheck -check-prefix=RETPOLINE %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-retpoline %s -### 2>&1 | FileCheck -check-prefix=NO-RETPOLINE %s
// RETPOLINE: "-target-feature" "+retpoline-indirect-calls" "-target-feature" "+retpoline-indirect-branches"
// NO-RETPOLINE-NOT: retpoline

// RUN: %clang_cl -target i386-linux-gnu -Qmretpoline -Qmretpoline-external-thunk %s -### 2>&1 | FileCheck -check-prefix=RETPOLINE-EXTERNAL-THUNK %s
// RUN: %clang_cl -target i386-linux-gnu -Qmretpoline -Qmno-retpoline-external-thunk %s -### 2>&1 | FileCheck -check-prefix=NO-RETPOLINE-EXTERNAL-THUNK %s
// RETPOLINE-EXTERNAL-THUNK: "-target-feature" "+retpoline-external-thunk"
// NO-RETPOLINE-EXTERNAL-THUNK: "-target-feature" "-retpoline-external-thunk"

// RUN: %clang_cl -target i386-linux-gnu -Qmspeculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=SLH %s
// RUN: %clang_cl -target i386-linux-gnu -Qmretpoline -Qmspeculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=RETPOLINE %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-speculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=NO-SLH %s
// SLH-NOT: retpoline
// SLH: "-target-feature" "+retpoline-indirect-calls"
// SLH-NOT: retpoline
// SLH: "-mspeculative-load-hardening"
// NO-SLH-NOT: retpoline

// RUN: %clang_cl -target i386-linux-gnu -Qmlvi-cfi %s -### 2>&1 | FileCheck -check-prefix=LVICFI %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-lvi-cfi %s -### 2>&1 | FileCheck -check-prefix=NO-LVICFI %s
// LVICFI: "-target-feature" "+lvi-cfi"
// NO-LVICFI-NOT: lvi-cfi

// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-cfi -Qmspeculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=LVICFI-SLH %s
// LVICFI-SLH: error: invalid argument 'mspeculative-load-hardening' not allowed with 'mlvi-cfi'
// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-cfi -Qmretpoline %s -### 2>&1 | FileCheck -check-prefix=LVICFI-RETPOLINE %s
// LVICFI-RETPOLINE: error: invalid argument 'mretpoline' not allowed with 'mlvi-cfi'
// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-cfi -Qmretpoline-external-thunk %s -### 2>&1 | FileCheck -check-prefix=LVICFI-RETPOLINE-EXTERNAL-THUNK %s
// LVICFI-RETPOLINE-EXTERNAL-THUNK: error: invalid argument 'mretpoline-external-thunk' not allowed with 'mlvi-cfi'

// RUN: %clang_cl -target i386-linux-gnu -Qmlvi-hardening %s -### 2>&1 | FileCheck -check-prefix=LVIHARDENING %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-lvi-hardening %s -### 2>&1 | FileCheck -check-prefix=NO-LVIHARDENING %s
// LVIHARDENING: "-target-feature" "+lvi-load-hardening" "-target-feature" "+lvi-cfi"
// NO-LVIHARDENING-NOT: "+lvi-

// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-hardening -Qmspeculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=LVIHARDENING-SLH %s
// LVIHARDENING-SLH: error: invalid argument 'mspeculative-load-hardening' not allowed with 'mlvi-hardening'
// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-hardening -Qmretpoline %s -### 2>&1 | FileCheck -check-prefix=LVIHARDENING-RETPOLINE %s
// LVIHARDENING-RETPOLINE: error: invalid argument 'mretpoline' not allowed with 'mlvi-hardening'
// RUN: not %clang_cl -target i386-linux-gnu -Qmlvi-hardening -Qmretpoline-external-thunk %s -### 2>&1 | FileCheck -check-prefix=LVIHARDENING-RETPOLINE-EXTERNAL-THUNK %s
// LVIHARDENING-RETPOLINE-EXTERNAL-THUNK: error: invalid argument 'mretpoline-external-thunk' not allowed with 'mlvi-hardening'

// RUN: %clang_cl -target i386-linux-gnu -Qmseses %s -### 2>&1 | FileCheck -check-prefix=SESES %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-seses %s -### 2>&1 | FileCheck -check-prefix=NO-SESES %s
// SESES: "-target-feature" "+seses"
// SESES: "-target-feature" "+lvi-cfi"
// NO-SESES-NOT: seses
// NO-SESES-NOT: lvi-cfi

// RUN: %clang_cl -target i386-linux-gnu -Qmseses -Qmno-lvi-cfi %s -### 2>&1 | FileCheck -check-prefix=SESES-NOLVICFI %s
// SESES-NOLVICFI: "-target-feature" "+seses"
// SESES-NOLVICFI-NOT: lvi-cfi

// RUN: not %clang_cl -target i386-linux-gnu -Qmseses -Qmspeculative-load-hardening %s -### 2>&1 | FileCheck -check-prefix=SESES-SLH %s
// SESES-SLH: error: invalid argument 'mspeculative-load-hardening' not allowed with 'mseses'
// RUN: not %clang_cl -target i386-linux-gnu -Qmseses -Qmretpoline %s -### 2>&1 | FileCheck -check-prefix=SESES-RETPOLINE %s
// SESES-RETPOLINE: error: invalid argument 'mretpoline' not allowed with 'mseses'
// RUN: not %clang_cl -target i386-linux-gnu -Qmseses -Qmretpoline-external-thunk %s -### 2>&1 | FileCheck -check-prefix=SESES-RETPOLINE-EXTERNAL-THUNK %s
// SESES-RETPOLINE-EXTERNAL-THUNK: error: invalid argument 'mretpoline-external-thunk' not allowed with 'mseses'

// RUN: not %clang_cl -target i386-linux-gnu -Qmseses -Qmlvi-hardening %s -### 2>&1 | FileCheck -check-prefix=SESES-LVIHARDENING %s
// SESES-LVIHARDENING: error: invalid argument 'mlvi-hardening' not allowed with 'mseses'

// RUN: %clang_cl -target i386-linux-gnu -Qmwaitpkg %s -### 2>&1 | FileCheck -check-prefix=WAITPKG %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-waitpkg %s -### 2>&1 | FileCheck -check-prefix=NO-WAITPKG %s
// WAITPKG: "-target-feature" "+waitpkg"
// NO-WAITPKG: "-target-feature" "-waitpkg"

// RUN: %clang_cl --target=i386 -march=i386 -Qmmovdiri %s -### 2>&1 | FileCheck -check-prefix=MOVDIRI %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-movdiri %s -### 2>&1 | FileCheck -check-prefix=NO-MOVDIRI %s
// MOVDIRI: "-target-feature" "+movdiri"
// NO-MOVDIRI: "-target-feature" "-movdiri"

// RUN: %clang_cl --target=i386 -march=i386 -Qmmovdir64b %s -### 2>&1 | FileCheck -check-prefix=MOVDIR64B %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-movdir64b %s -### 2>&1 | FileCheck -check-prefix=NO-MOVDIR64B %s
// MOVDIR64B: "-target-feature" "+movdir64b"
// NO-MOVDIR64B: "-target-feature" "-movdir64b"

// RUN: %clang_cl --target=i386 -march=i386 -Qmpconfig %s -### 2>&1 | FileCheck -check-prefix=PCONFIG %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-pconfig %s -### 2>&1 | FileCheck -check-prefix=NO-PCONFIG %s
// PCONFIG: "-target-feature" "+pconfig"
// NO-PCONFIG: "-target-feature" "-pconfig"

// RUN: %clang_cl --target=i386 -march=i386 -Qmptwrite %s -### 2>&1 | FileCheck -check-prefix=PTWRITE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-ptwrite %s -### 2>&1 | FileCheck -check-prefix=NO-PTWRITE %s
// PTWRITE: "-target-feature" "+ptwrite"
// NO-PTWRITE: "-target-feature" "-ptwrite"

// RUN: %clang_cl --target=i386 -march=i386 -Qminvpcid %s -### 2>&1 | FileCheck -check-prefix=INVPCID %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-invpcid %s -### 2>&1 | FileCheck -check-prefix=NO-INVPCID %s
// INVPCID: "-target-feature" "+invpcid"
// NO-INVPCID: "-target-feature" "-invpcid"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx512bf16 %s -### 2>&1 | FileCheck -check-prefix=AVX512BF16 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx512bf16 %s -### 2>&1 | FileCheck -check-prefix=NO-AVX512BF16 %s
// AVX512BF16: "-target-feature" "+avx512bf16"
// NO-AVX512BF16: "-target-feature" "-avx512bf16"

// RUN: %clang_cl --target=i386 -march=i386 -Qmenqcmd %s -### 2>&1 | FileCheck --check-prefix=ENQCMD %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-enqcmd %s -### 2>&1 | FileCheck --check-prefix=NO-ENQCMD %s
// ENQCMD: "-target-feature" "+enqcmd"
// NO-ENQCMD: "-target-feature" "-enqcmd"

// RUN: %clang_cl --target=i386 -march=i386 -Qmvzeroupper %s -### 2>&1 | FileCheck --check-prefix=VZEROUPPER %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-vzeroupper %s -### 2>&1 | FileCheck --check-prefix=NO-VZEROUPPER %s
// VZEROUPPER: "-target-feature" "+vzeroupper"
// NO-VZEROUPPER: "-target-feature" "-vzeroupper"

// RUN: %clang_cl --target=i386 -march=i386 -Qmserialize %s -### 2>&1 | FileCheck -check-prefix=SERIALIZE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-serialize %s -### 2>&1 | FileCheck -check-prefix=NO-SERIALIZE %s
// SERIALIZE: "-target-feature" "+serialize"
// NO-SERIALIZE: "-target-feature" "-serialize"

// RUN: %clang_cl --target=i386 -march=i386 -Qmtsxldtrk %s -### 2>&1 | FileCheck --check-prefix=TSXLDTRK %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-tsxldtrk %s -### 2>&1 | FileCheck --check-prefix=NO-TSXLDTRK %s
// TSXLDTRK: "-target-feature" "+tsxldtrk"
// NO-TSXLDTRK: "-target-feature" "-tsxldtrk"

// RUN: %clang_cl -target i386-linux-gnu -Qmkl_ %s -### 2>&1 | FileCheck -check-prefix=KL %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-kl %s -### 2>&1 | FileCheck -check-prefix=NO-KL %s
// KL: "-target-feature" "+kl"
// NO-KL: "-target-feature" "-kl"

// RUN: %clang_cl -target i386-linux-gnu -Qmwidekl %s -### 2>&1 | FileCheck -check-prefix=WIDE_KL %s
// RUN: %clang_cl -target i386-linux-gnu -Qmno-widekl %s -### 2>&1 | FileCheck -check-prefix=NO-WIDE_KL %s
// WIDE_KL: "-target-feature" "+widekl"
// NO-WIDE_KL: "-target-feature" "-widekl"

// RUN: %clang_cl --target=i386 -march=i386 -Qmamx-tile %s -### 2>&1 | FileCheck --check-prefix=AMX-TILE %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-amx-tile %s -### 2>&1 | FileCheck --check-prefix=NO-AMX-TILE %s
// AMX-TILE: "-target-feature" "+amx-tile"
// NO-AMX-TILE: "-target-feature" "-amx-tile"

// RUN: %clang_cl --target=i386 -march=i386 -Qmamx-bf16 %s -### 2>&1 | FileCheck --check-prefix=AMX-BF16 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-amx-bf16 %s -### 2>&1 | FileCheck -check-prefix=NO-AMX-BF16 %s
// AMX-BF16: "-target-feature" "+amx-bf16"
// NO-AMX-BF16: "-target-feature" "-amx-bf16"

// RUN: %clang_cl --target=i386 -march=i386 -Qmamx-int8 %s -### 2>&1 | FileCheck --check-prefix=AMX-INT8 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-amx-int8 %s -### 2>&1 | FileCheck --check-prefix=NO-AMX-INT8 %s
// AMX-INT8: "-target-feature" "+amx-int8"
// NO-AMX-INT8: "-target-feature" "-amx-int8"

// RUN: %clang_cl --target=i386 -march=i386 -Qmhreset %s -### 2>&1 | FileCheck -check-prefix=HRESET %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-hreset %s -### 2>&1 | FileCheck -check-prefix=NO-HRESET %s
// HRESET: "-target-feature" "+hreset"
// NO-HRESET: "-target-feature" "-hreset"

// RUN: %clang_cl --target=i386 -march=i386 -Qmuintr %s -### 2>&1 | FileCheck -check-prefix=UINTR %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-uintr %s -### 2>&1 | FileCheck -check-prefix=NO-UINTR %s
// UINTR: "-target-feature" "+uintr"
// NO-UINTR: "-target-feature" "-uintr"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavxvnni %s -### 2>&1 | FileCheck --check-prefix=AVX-VNNI %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avxvnni %s -### 2>&1 | FileCheck --check-prefix=NO-AVX-VNNI %s
// AVX-VNNI: "-target-feature" "+avxvnni"
// NO-AVX-VNNI: "-target-feature" "-avxvnni"

// RUN: %clang_cl --target=i386 -march=i386 -Qmavx512fp16 %s -### 2>&1 | FileCheck -check-prefix=AVX512FP16 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-avx512fp16 %s -### 2>&1 | FileCheck -check-prefix=NO-AVX512FP16 %s
// AVX512FP16: "-target-feature" "+avx512fp16"
// NO-AVX512FP16: "-target-feature" "-avx512fp16"

// RUN: %clang_cl --target=i386 -march=i386 -Qmcrc32 %s -### 2>&1 | FileCheck -check-prefix=CRC32 %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmno-crc32 %s -### 2>&1 | FileCheck -check-prefix=NO-CRC32 %s
// CRC32: "-target-feature" "+crc32"
// NO-CRC32: "-target-feature" "-crc32"

// RUN: %clang_cl --target=i386 -march=i386 -Qmharden-sls=return %s -### -o %t.o 2>&1 | FileCheck -check-prefixes=SLS-RET,NO-SLS %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmharden-sls=indirect-jmp %s -### -o %t.o 2>&1 | FileCheck -check-prefixes=SLS-IJMP,NO-SLS %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmharden-sls=none -Qmharden-sls=all %s -### -o %t.o 2>&1 | FileCheck -check-prefixes=SLS-IJMP,SLS-RET %s
// RUN: %clang_cl --target=i386 -march=i386 -Qmharden-sls=all -Qmharden-sls=none %s -### -o %t.o 2>&1 | FileCheck -check-prefix=NO-SLS %s
// RUN: not %clang_cl --target=i386 -march=i386 -Qmharden-sls=return,indirect-jmp %s -### -o %t.o 2>&1 | FileCheck -check-prefix=BAD-SLS %s
// NO-SLS-NOT: "+harden-sls-
// SLS-RET-DAG: "-target-feature" "+harden-sls-ret"
// SLS-IJMP-DAG: "-target-feature" "+harden-sls-ijmp"
// NO-SLS-NOT: "+harden-sls-
// BAD-SLS: unsupported argument '{{[^']+}}' to option '-mharden-sls='
