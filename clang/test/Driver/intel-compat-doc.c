// Check output from -fintel-compatibility-doc using all valid values.
//
// RUN: %clang -c -Xclang -fintel-compatibility-doc=AllowExtraArgument,AllowFewerMacroArgs,AllowMissingTypename,FArgumentNoalias,FakeLoad,IMFAttributes,IntelTBAABF,IntelTBAA,IntrinsicPromotion,PragmaBlockLoop,PragmaFusion,PragmaLoopCount,PragmaNoFusion,PragmaNoUnrollAndJam,PragmaNoVector,PragmaUnrollAndJam,PragmaVector,PredeclareAlignValT,RelaxSpirCCNoProtoDiag,StringCharStarCatchable,UnrollZero,VaArgPack,VolatileInOMPRegions,WholeProgramVTableWrap %s 2>&1 | FileCheck --match-full-lines %s

// CHECK:AllowExtraArgument
// CHECK-NEXT: ==================
// CHECK-NEXT: Microsoft allows, with warnings, extra arguments to function calls. Disabled
// CHECK-NEXT: by default (generates errors).

// CHECK:AllowFewerMacroArgs
// CHECK-NEXT: ===================
// CHECK-NEXT: Microsoft allows passing fewer arguments to function-like macros, replacing
// CHECK-NEXT: each missing argument with the empty string. A warning is given. This behavior
// CHECK-NEXT: is off-by-default.

// CHECK: AllowMissingTypename
// CHECK-NEXT: ====================
// CHECK-NEXT: Enables Microsoft compatibility by allowing a missing typename in some contexts.

// CHECK: FArgumentNoalias
// CHECK-NEXT: ================
// CHECK-NEXT: Add support for -fargument-noalias, which speicfies that arguments do not
// CHECK-NEXT: alias each other.

// CHECK: FakeLoad
// CHECK-NEXT: ========
// CHECK-NEXT: Adds an intrinsic to improve alias analysis for pointers returned from a
// CHECK-NEXT: function.

// CHECK: IMFAttributes
// CHECK-NEXT: =============
// CHECK-NEXT: Attaches IMF attributes specified by the user to the applicable LLVM math
// CHECK-NEXT: functions.

// CHECK: IntelTBAABF
// CHECK-NEXT: ===========
// CHECK-NEXT: When loading (or storing) a bitfield, other bitfields may share the same
// CHECK-NEXT: location, which inhibits optimization. This option enables type-based alias
// CHECK-NEXT: analysis, which allows the compiler to better optimize these references.

// CHECK: IntelTBAA
// CHECK-NEXT: =========
// CHECK-NEXT: Enables type-based alias analysis for when an array is nested in a struct.
// CHECK-NEXT: Allows assumption that if the address of a struct member is taken, it is valid
// CHECK-NEXT: to do arithmetic which yields addresses to other fields within the struct.

// CHECK: IntrinsicPromotion
// CHECK-NEXT: ==================
// CHECK-NEXT: This change automatically promotes the target features of functions that use
// CHECK-NEXT: intrinsics or intrinsic builtins that would otherwise be disallowed due to
// CHECK-NEXT: always-inlining. The error message that this should prevent is '<intrin-name>
// CHECK-NEXT: needs target feature <feature>.' Only intrinsics defined in the system headers
// CHECK-NEXT: will cause this promotion to happen automatically.

// CHECK: PragmaBlockLoop
// CHECK-NEXT: ===============
// CHECK-NEXT: The ``block_loop`` pragma lets you exert greater control over optimizations on
// CHECK-NEXT: a specific loop inside a nested loop.

// CHECK: PragmaFusion
// CHECK-NEXT: ============
// CHECK-NEXT: Pragma fusion instructs the compiler to fuse the following loop with adjacent
// CHECK-NEXT: loops.

// CHECK: PragmaLoopCount
// CHECK-NEXT: ===============
// CHECK-NEXT: The ``loop_count`` pragma specifies the minimum, maximum, or average number of
// CHECK-NEXT: iterations for a for loop. In addition, a list of commonly occurring values
// CHECK-NEXT: can be specified to help the compiler generate multiple versions and perform
// CHECK-NEXT: complete unrolling.

// CHECK: PragmaNoFusion
// CHECK-NEXT: ==============
// CHECK-NEXT: The ``nofusion`` pragma instructs the compiler not to fuse the loop with
// CHECK-NEXT: adjacent loops.

// CHECK: PragmaNoUnrollAndJam
// CHECK-NEXT: ====================
// CHECK-NEXT: The ``nounroll_and_jam`` pragma instructs the compiler to disable unrolling
// CHECK-NEXT: and jamming loops.

// CHECK: PragmaNoVector
// CHECK-NEXT: ==============
// CHECK-NEXT: Accept #pragma novector to indicate loop should never be vectorized.

// CHECK: PragmaUnrollAndJam
// CHECK-NEXT: ==================
// CHECK-NEXT: The ``unroll_and_jam`` pragma instructs the compiler to enable unrolling and
// CHECK-NEXT: jamming loops.

// CHECK: PragmaVector
// CHECK-NEXT: ============
// CHECK-NEXT: Accept #pragma vector - indicates a loop should be vectorized according to the
// CHECK-NEXT: argument keywords 'assert', 'aligned', 'unaligned', 'vecremainder', and
// CHECK-NEXT: 'novecremainder'.

// CHECK: PredeclareAlignValT
// CHECK-NEXT: ===================
// CHECK-NEXT: Enables support for automatically-aligned dynamic allocation per C++17
// CHECK-NEXT: std::align_val_t.

// CHECK: RelaxSpirCCNoProtoDiag
// CHECK-NEXT: ======================
// CHECK-NEXT: This change relaxes the 'function with no prototype cannot use the X calling
// CHECK-NEXT: convention' diagnostic from an error to a warning. This diagnostic is
// CHECK-NEXT: triggered for unprototyped function declarations for calling conventions that
// CHECK-NEXT: do not support variadic calls.

// CHECK: StringCharStarCatchable
// CHECK-NEXT: =======================
// CHECK-NEXT: Microsoft allows you to catch a string literal with catch (char *). Disabled
// CHECK-NEXT: by default.

// CHECK: UnrollZero
// CHECK-NEXT: ==========
// CHECK-NEXT: Allow unroll value of zero, withe same semantics as nounroll.

// CHECK: VaArgPack
// CHECK-NEXT: =========
// CHECK-NEXT: Support GCC __builtin_va_arg_pack and __builtin_va_arg_pack_len builtins.

// CHECK: VolatileInOMPRegions
// CHECK-NEXT: ====================
// CHECK-NEXT: Community clang removes volatile from variables captured in OpenMP regions.
// CHECK-NEXT: This extension, controlled by -fintel-compatibility, retains volatile.

// CHECK: WholeProgramVTableWrap
// CHECK-NEXT: ======================
// CHECK-NEXT: When used with ``-fwhole-program-vtables`` option, this optimization enables
// CHECK-NEXT: devirtualization with whole program analysis without hidden LTO visibility.
