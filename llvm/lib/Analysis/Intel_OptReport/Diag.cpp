//===- Diag.cpp - Implements the OptReportDiag strings --------------------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the OptReportDiag strings.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/Diag.h"
#include <cassert>

using namespace llvm;

/// NOTE:
/// Vec-report ID starts at 15300. 15552 and below are vectorizer remarks from
/// ICC, the rest is xmain-specific. Loop-report ID starts at 25045. Unless a
/// different agreement is made w/ Vector Advisor team, all not vectorized
/// messages need to stay within 8-bit offset from 15300.
const DenseMap<unsigned, const char *> OptReportDiag::Diags = {
    {15300, "LOOP WAS VECTORIZED"},
    {15301, "SIMD LOOP WAS VECTORIZED"},
    {15302, "routine skipped: vectorization disabled due to -mno-sse, "
            "-mno-sse2, -mia32, and/or -no-vec flag"},
    {15303,
     "routine skipped: asynchronous exception handling prevents vectorization"},
    {15304, "%s was not vectorized: non-vectorizable loop instance from "
            "multiversioning"},
    {15305, "vectorization support: vector length %s"},
    {15306,
     "%s was not vectorized: inner loop was transformed to memset or memcpy"},
    {15307, "%s was not vectorized: top test could not be found"},
    {15308, "%s was not vectorized: explicitly compute the iteration count "
            "before executing the loop or try using canonical loop form from "
            "OpenMP specification%s"},
    {15309, "vectorization support: normalized vectorization overhead %s"},
    {15310, "%s was not vectorized: operation cannot be vectorized"},
    {15311, "%s was not vectorized: mixed data types"},
    {15312, "%s was not vectorized: data type unsupported on given target "
            "architecture"},
    {15313, "%s was not vectorized: unsupported data type"},
    {15314, "%s was not vectorized: global scalar assignment in vector "
            "function is prohibited"},
    {15315, "%s was not vectorized: low trip count"},
    {15316, "%s was not vectorized: scalar assignment in simd loop is "
            "prohibited, consider private, lastprivate or reduction clauses"},
    {15317, "%s was not vectorized: subscript too complex"},
    {15318, "%s was not vectorized: dereference too complex"},
    {15319, "%s was not vectorized: novector directive used"},
    {15320, "routine skipped: loop optimizations disabled"},
    {15321, "DO NOT USE. RETIRED MESSAGE. %s was not vectorized: unsupported "
            "loop structure"},
    {15322, "%s was not vectorized: nested conditional statements"},
    {15323, "%s was not vectorized: condition too complex"},
    {15324, "%s was not vectorized: unsigned condition unsupported"},
    {15325, "%s was not vectorized: condition may protect exception"},
    {15326, "%s was not vectorized: implied FP exception model prevents "
            "vectorization. Consider changing compiler flags and/or directives "
            "in the source to enable fast FP model and to mask FP exceptions"},
    {15327, "%s was not vectorized: type conversion prohibits vectorization"},
    {15328, "%s was not vectorized: vector type could not be determined"},
    {15329, "%s was not vectorized: shift operation too complex"},
    {15330,
     "%s was not vectorized: the reduction operator is not supported yet"},
    {15331, "%s was not vectorized: precise FP model implied by the command "
            "line or a directive prevents vectorization. Consider using fast "
            "FP model"},
    {15332, "%s was not vectorized: loop is not within user-defined range"},
    {15333, "%s was not vectorized: exception handling for a call prevents "
            "vectorization"},
    {15334,
     "DO NOT USE. RETIRED MESSAGE. %s was not vectorized: PAROPT problem"},
    {15335, "%s was not vectorized: vectorization possible but seems "
            "inefficient. Use vector always directive or -vec-threshold0 to "
            "override "},
    {15336, "%s was not vectorized: conditional assignment to a scalar"},
    {15337, "%s was not vectorized: rotation too complex"},
    {15338, "%s was not vectorized: no instance of vectorized math function "
            "satisfies specified math function attributes"},
    {15339, "pragma format ignored"},
    {15340, "pragma supersedes previous setting"},
    {15341, "%s was not vectorized: nonstandard loop is not a vectorization "
            "candidate"},
    {15342,
     "%s was not vectorized: contains unvectorizable statement at line %d"},
    {15343, "%s was not vectorized: loop performs prefetch only"},
    {15344,
     "%s was not vectorized: vector dependence prevents vectorization%s"},
    {15345, "vector dependence: proven %s dependence between %.32s line %d, "
            "and %.32s line %d"},
    {15346, "vector dependence: %s"},
    {15347, "FUNCTION WAS VECTORIZED"},
    {15348, "function was not vectorized: data dependence(s) may exist"},
    {15349, "function was not vectorized: statement cannot be vectorized"},
    {15350, "function was not vectorized: operation cannot be vectorized"},
    {15351, "loop was not vectorized: loop can't be transformed into canonical "
            "form from OpenMP specification. Consider simplifying the starting "
            "value and/or the increment of the loop index"},
    {15352, "loop was not vectorized: implied FP exception model prevents "
            "vectorization on Intel(R) Xeon Phi(TM) co-processor codenamed "
            "Knights Corner. Consider changing compiler flags and/or "
            "directives in the source to enable fast FP model and to mask FP "
            "exceptions"},
    {15353, "loop was not vectorized: loop is not in canonical form from "
            "OpenMP specification, may be as a result of previous "
            "optimization(s)"},
    {15354, "function was not vectorized: operator unsuited for vectorization"},
    {15355, "function was not vectorized: subscript too complex"},
    {15356, "function was not vectorized: dereference too complex"},
    {15357, "function was not vectorized: nonunit stride used"},
    {15358, "function was not vectorized: unsupported loop structure"},
    {15359, "function was not vectorized: nested conditional statements"},
    {15360, "function was not vectorized: condition too complex"},
    {15361, "function was not vectorized: unsigned condition unsupported"},
    {15362, "function was not vectorized: condition may protect exception"},
    {15363, "function was not vectorized: implied FP exception model prevents "
            "vectorization. Consider changing compiler flags and/or directives "
            "in the source to enable fast FP model and to mask FP exceptions"},
    {15364,
     "function was not vectorized: type conversion prohibits vectorization"},
    {15365, "function was not vectorized: vector type could not be determined"},
    {15366, "function was not vectorized: shift operation too complex"},
    {15367, "function was not vectorized: unsupported reduction"},
    {15368, "function was not vectorized: precise FP model implied by the "
            "command line or a directive prevents vectorization. Consider "
            "using fast FP model"},
    {15369,
     "function was not vectorized: loop is not within user-defined range"},
    {15370, "function was not vectorized: prefetching too complex"},
    {15371, "function was not vectorized: PAROPT problem"},
    {15372, "function was not vectorized: conditional assignment to a scalar"},
    {15373, "function was not vectorized: rotation too complex"},
    {15374, "%s was not vectorized: no instance of vectorized math function "
            "satisfies specified math function attributes"},
    {15375, "function was not vectorized: contains unvectorizable statement at "
            "line %d"},
    {15376, "function was not vectorized: existence of vector dependence"},
    {15377, "%s can't be vectorized: too many registers required to return "
            "value (big vector length)"},
    {15378, "%s was not vectorized: -ffreestanding flag prevents vectorization "
            "of integer divide/remainder"},
    {15379, "function was not vectorized: -ffreestanding flag prevents "
            "vectorization of integer divide/remainder"},
    {15380, "Invalid vectorlength clause specified"},
    {15381, "vectorization support: unaligned access used inside loop body"},
    {15382, "vectorization support: call to function %s cannot be vectorized"},
    {15383, "vectorization support: call to function %s cannot be vectorized "
            "with given fp model"},
    {15384,
     "vectorization support: assignment cannot be vectorized for variable %s"},
    {15385, "invalid use of a reduction variable %s"},
    {15386, "vectorization support: vectorization of single precision division "
            "of product and converted double precision is not safe"},
    {15387, "Vector length 1 is not supported for function vectorization, "
            "skipping %s"},
    {15388, "vectorization support: reference %s has aligned access"},
    {15389, "vectorization support: reference %s has unaligned access"},
    {15390, "vectorization support: vectorization of this loop under -Os has "
            "impact on code size"},
    {15391, "vectorization support: %s"},
    {15392, "Unsupported data type for %s in vector function '%s'"},
    {15393, "Place holder #1 to keep vec diagnostics numbering intact"},
    {15394, "Place holder #2 to keep vec diagnostics numbering intact"},
    {15395, "The function '%s' declared as vector function in one module does "
            "not have prototype in another module."},
    {15396, "Vector specification for function '%s' is ignored with specified "
            "optimization options."},
    {15397, "Suitable vector variant of function '%s' was not found"},
    {15398, "%s was not vectorized: loop was transformed to memset or memcpy"},
    {15399, "vectorization support: unroll factor %s"},
    {15400, "vectorization support: type long long is not supported as indices "
            "on given target architecture"},
    {15401,
     "vectorization support: type char is not supported for operation %s"},
    {15402,
     "vectorization support: type short is not supported for operation %s"},
    {15403,
     "vectorization support: type int is not supported for operation %s"},
    {15404,
     "vectorization support: type long long is not supported for operation %s"},
    {15405,
     "vectorization support: type float is not supported for operation %s"},
    {15406,
     "vectorization support: type double is not supported for operation %s"},
    {15407, "vectorization support: type complex float is not supported for "
            "operation %s"},
    {15408, "vectorization support: type complex double is not supported for "
            "operation %s"},
    {15409, "vectorization support: unaligned %s will be scalarized"},
    {15410,
     "vectorization support: conversion from int to float will be emulated"},
    {15411,
     "vectorization support: conversion from float to int will be emulated"},
    {15412, "vectorization support: streaming store was generated for %s"},
    {15413, "expression %s should be reduction"},
    {15414, "%s was not vectorized: nothing to vectorize since loop body "
            "became empty after optimizations"},
    {15415, "vectorization support: gather was generated for the variable %s: "
            "%s %s %s"},
    {15416, "vectorization support: scatter was generated for the variable %s: "
            "%s %s %s"},
    {15417, "vectorization support: number of FP up converts: single precision "
            "to double precision %s"},
    {15418, "vectorization support: number of FP down converts: double "
            "precision to single precision %s"},
    {15419,
     "vectorization support: read-write dependencies on indices were resolved"},
    {15420, "%s"},
    {15421, "%s was not vectorized: memory reference is not naturally aligned"},
    {15422, "%s was not vectorized: predication analysis failed due to complex "
            "control flow. "},
    {15423, "%s was not vectorized: has only one iteration"},
    {15424,
     "Vector length (%s) is too large for function '%s'. Using default."},
    {15425, "Vector length (%s) for function '%s' is truncated to nearest "
            "power of two."},
    {15426, "Alignment (%s) is reduced to a nearest power of two."},
    {15427, "loop was completely unrolled"},
    {15428, "%s was not vectorized: function %s has CilkPlus threading "
            "construct that is not allowed inside of a simd-loop"},
    {15429, "%s was not vectorized: function %s has openmp region that is not "
            "allowed inside simd-loop"},
    {15430, "vectorization report for function: %s "},
    {15431, "loop was vectorized (no peel/no remainder)"},
    {15432, "loop was vectorized (no peel/with remainder)"},
    {15433, "loop was vectorized (with peel/no remainder)"},
    {15434, "loop was vectorized (with peel/with remainder)"},
    {15435, "loop was vectorized with mask due to low trip count"},
    {15436, "loop was not vectorized: %s "},
    {15437, "peel loop was vectorized"},
    {15438, "peel loop was not vectorized: %s "},
    {15439, "remainder loop was vectorized (unmasked)"},
    {15440, "remainder loop was vectorized (masked)"},
    {15441, "remainder loop was not vectorized: %s "},
    {15442, "entire loop may be executed in remainder"},
    {15443, "--- begin vector loop hierarchy summary ---"},
    {15444, "vectorized loop at nesting level: %s "},
    {15445, "loop inside vectorized loop at nesting level: %s "},
    {15446, "--- end vector loop hierarchy summary ---"},
    {15447, "--- begin vector loop memory reference summary ---"},
    {15448, "unmasked aligned unit stride loads: %s "},
    {15449, "unmasked aligned unit stride stores: %s "},
    {15450, "unmasked unaligned unit stride loads: %s "},
    {15451, "unmasked unaligned unit stride stores: %s "},
    {15452, "unmasked strided loads: %s "},
    {15453, "unmasked strided stores: %s "},
    {15454, "masked aligned unit stride loads: %s "},
    {15455, "masked aligned unit stride stores: %s "},
    {15456, "masked unaligned unit stride loads: %s "},
    {15457, "masked unaligned unit stride stores: %s "},
    {15458, "masked indexed (or gather) loads: %s "},
    {15459, "masked indexed (or scatter) stores: %s "},
    {15460, "masked strided loads: %s "},
    {15461, "masked strided stores: %s "},
    {15462, "unmasked indexed (or gather) loads: %s "},
    {15463, "unmasked indexed (or scatter) stores: %s "},
    {15464, "unmasked broadcast loads: %s "},
    {15465, "masked broadcast loads: %s "},
    {15466, "unmasked aligned streaming loads: %s "},
    {15467, "unmasked aligned streaming stores: %s "},
    {15468, "unmasked unaligned streaming loads: %s "},
    {15469, "unmasked unaligned streaming stores: %s "},
    {15470, "masked aligned streaming loads: %s "},
    {15471, "masked aligned streaming stores: %s "},
    {15472, "masked unaligned streaming loads: %s "},
    {15473, "masked unaligned streaming stores: %s "},
    {15474, "--- end vector loop memory reference summary ---"},
    {15475, "--- begin vector loop cost summary ---"},
    {15476, "scalar cost: %s "},
    {15477, "vector cost: %s "},
    {15478, "estimated potential speedup: %s "},
    {15479, "lightweight vector operations: %s "},
    {15480, "medium-overhead vector operations: %s "},
    {15481, "heavy-overhead vector operations: %s "},
    {15482, "vectorized math library calls: %s "},
    {15483, "non-vectorized math library calls: %s "},
    {15484, "vector function calls: %s "},
    {15485, "serialized function calls: %s"},
    {15486, "divides: %s "},
    {15487, "type converts: %s "},
    {15488, "--- end vector loop cost summary ---"},
    {15489, "--- begin vector function matching report ---"},
    {15490, "Function call: %s with simdlen=%s, actual parameter types: (%s)"},
    {15491, "A suitable vector variant was found (out of %s) redefined by "
            "function: %s%s"},
    {15492, "A suitable vector variant was found (out of %s) with %s, formal "
            "parameter types: (%s)"},
    {15493, "--- end vector function matching report ---"},
    {15494, "--- begin vector idiom recognition report ---"},
    {15495, "minimum value and minimum value loop index: %s "},
    {15496, "maximum value and maximum value loop index: %s "},
    {15497, "vector compress: %s "},
    {15498, "vector expand: %s "},
    {15499, "histogram: %s "},
    {15500, "saturating downconvert: %s "},
    {15501, "saturating add/subtract: %s "},
    {15502, "byte permute: %s "},
    {15503, "--- end vector idiom recognition report ---"},
    {15504, "--- begin vectorizer fails report"},
    {15505, "--- end vectorizer fails report"},
    {15506, "vplan loop number: %s"},
    {15507, "Incorrect parameter type of vector variant '%s' of function '%s' "
            "at position %d.\n The correct prototype is: '%s'."},
    {15508, "Incorrect return type of vector variant '%s' of function '%s' at "
            "position %d.\n The correct prototype is: '%s'."},
    {15509, "Extraneous parameter of vector variant '%s' of function '%s' at "
            "position %d.\n The correct prototype is: '%s'."},
    {15510, "Extraneous return value of vector variant '%s' of function '%s' "
            "at position %d.\n The correct prototype is: '%s'."},
    {15511, "Not enough parameters of vector variant '%s' of function '%s' at "
            "position %d.\n The correct prototype is: '%s'."},
    {15512, "Not enough return values of vector variant '%s' of function '%s' "
            "at position %d.\n The correct prototype is: '%s'."},
    {15513, "Incorrect type of mask parameter of vector variant '%s' of "
            "function '%s' at position %d.\n The correct prototype is: '%s'."},
    {15514, "Need more blend_to parameter(s) in vector variant '%s' of "
            "function '%s' at position %d.\n The correct prototype is: '%s'."},
    {15515, "Incorrect type of blend_to parameter of vector variant '%s' of "
            "function '%s' at position %d."},
    {15516, "%s was not vectorized: cost model has chosen vectorlength of 1 -- "
            "maybe possible to override via pragma/directive with vectorlength "
            "clause"},
    {15517, "loops in this subroutine cannot be vectorized due to use of "
            "EBX/RBX register in inline ASM."},
    {15518, "loops in this subroutine are not good vectorization candidates "
            "(try compiling with O3 and/or IPO)."},
    {15519, "A part of code was serialized due to operation on %s data"},
    {15520, "%s was not vectorized: loop with multiple exits cannot be "
            "vectorized unless it meets search loop idiom criteria"},
    {15521, "%s was not vectorized: loop control variable was not identified. "
            "Explicitly compute the iteration count before executing the loop "
            "or try using canonical loop form from OpenMP specification%s"},
    {15522, "%s was not vectorized: loop control flow is too complex. Try "
            "using canonical loop form from OpenMP specification%s"},
    {15523, "%s was not vectorized: loop control variable %s was found, but "
            "loop iteration count cannot be computed before executing the "
            "loop%s"},
    {15524, "%s was not vectorized: search loop cannot be vectorized unless "
            "all memory references can be aligned vector load"},
    {15525, "call to function '%s' is serialized"},
    {15526, "%s was not vectorized: ASM code cannot be vectorized"},
    {15527, "%s was not vectorized: function call to %s cannot be vectorized"},
    {15528, "%s was not vectorized: goto statement was not vectorized. Try "
            "simplifying control flow."},
    {15529, "%s was not vectorized: volatile assignment was not vectorized. "
            "Try using non-volatile assignment."},
    {15530, "%s was not vectorized: setjmp()/longjmp() cannot be vectorized"},
    {15531, "A portion of SIMD %s is serialized."},
    {15532, "%s was not vectorized: compile time constraints prevent loop "
            "optimization. Consider using -O3."},
    {15533, "%s was not vectorized: loop exceeds complexity limits. Consider "
            "overriding limits (-override-limits) or using simd directive."},
    {15534, "%s was not vectorized: loop contains arithmetic if or computed "
            "goto. Consider using if-then-else statement. "},
    {15535, "%s was not vectorized: loop contains switch statement. Consider "
            "using if-else statement."},
    {15536, "%s was not vectorized: inner loop throttling prevents "
            "vectorization of this outer loop. Refer to inner loop message for "
            "more details."},
    {15537, "%s was not vectorized: implied FP exception model prevents usage "
            "of SVML library needed for truncation or integer "
            "divide/remainder. Consider changing compiler flags and/or "
            "directives in the source to enable fast FP model and to mask FP "
            "exceptions"},
    {15538, "%s was not vectorized: multi-statement reduction"},
    {15539, "%s was not vectorized: exception handling support code cannot be "
            "vectorized"},
    {15540,
     "%s was not vectorized: auto-vectorization is disabled with -no-vec flag"},
    {15541, "outer %s was not auto-vectorized: consider using SIMD directive"},
    {15542, "%s was not vectorized: inner loop was already vectorized"},
    {15543, "%s was not vectorized: loop with function call not considered an "
            "optimization candidate."},
    {15544,
     "Suitable vector variant functions were not found (out of %s) %s%s"},
    {15545, "SIMD annotation was not seen, consider adding 'declare simd' "
            "directives at function declaration %s%s%s"},
    {15546,
     "Masked function call: %s with simdlen=%s, actual parameter types: (%s)"},
    {15547, "%s was not vectorized: code size was too large for vectorization. "
            "Consider reducing the number of distinct variables used."},
    {15548, "loop was vectorized along with the outer loop"},
    {15549, "loop was vectorized along with the enclosing function"},
    {15550, "%s was not vectorized with \"vector always assert\""},
    {15551, "%s was not vectorized with \"simd assert\""},
    {15552, "%s was not vectorized with \"simd\""},
    {15553, "loop was not vectorized: outer loop is not an auto-vectorization "
            "candidate."},
    // Four entries below semantically belong to "vector loop memory reference
    // summary". Be sure to update their mentiones under respective section
    // above.
    {15554, "Unmasked VLS-optimized loads (each part of the group counted "
            "separately): %s"},
    {15555, "Masked VLS-optimized loads (each part of the group counted "
            "separately): %s"},
    {15556, "Unmasked VLS-optimized stores (each part of the group counted "
            "separately): %s"},
    {15557, "Masked VLS-optimized stores (each part of the group counted "
            "separately): %s"},
    // Entries below semantically belong to  "vector loop cost summary".
    // Remarks for reasons of generated serialized code and scalar instructions.
    {15558, "Call to function '%s' was serialized due to no suitable vector "
            "variants were found."},
    {15559, "Call to function '%s' was serialized due to no vector variants "
            "were found. Consider adding #pragma omp declare simd."},
    {15560, "Indirect call cannot be vectorized."},
    {15561, "Call to function '%s' was serialized due to operating on scalar "
            "operand(s)."},
    {15562, "Call '%s' cannot be vectorized for current context."},
    {15563, "Load/store instruction was serilalized due to operating on "
            "non-vectorizable types."},
    {15564, "Extract/Insert element instruction was serialized due to "
            "non-const index."},
    {15565, "Masked Extract/Insert element instruction is serialized."},
    {15566, "'%s': division was scalarized due to fp-model requirements."},
    // Entries below semantically belong to "vector loop memory reference
    // summary".
    // Remarks for reasons of generated gather/scatter instructions.
    {15567, "Gathers are generated due to non-unit stride index of the "
            "corresponding loads."},
    {15568, "Scatters are generated due to non-unit stride index of the "
            "corresponding stores."},
    {15569, "Compiler has chosen to target XMM/YMM vector. Try using "
            "-mprefer-vector-width=512 to override."},
    {15570, "using scalar loop trip count: %s"},
    // End "vector loop memory reference summary" entries.
    {15571, "%s was not vectorized: loop contains a recurrent computation "
            "that could not be identified as an induction or reduction.  "
            "Try using #pragma omp simd reduction/linear/private to clarify "
            "recurrence."},
    {15572, "%s was not vectorized: loop contains a live-out value that could "
            "not be identified as an induction or reduction.  Try using "
            "#pragma omp simd reduction/linear/private to clarify recurrence."},
    {15573, "%s was not vectorized: a reduction or induction of a vector "
            "type is not supported."},
    {15574, "%s was not vectorized: unsupported nested OpenMP (simd) loop or "
            "region."},
    {15575, "peel loop is static"},
    {15576, "peel loop is dynamic"},
    {15577, "estimated number of scalar loop iterations peeled: %s"},

    {25018, "Total number of lines prefetched=%d"},
    {25019, "Number of spatial prefetches=%d, default dist=%d"},
    {25033, "Number of indirect prefetches=%d, default dist=%d"},
    {25045, "Fused Loops: %s"},
    {25046, "Loop lost in Fusion"},
    {25147, "Using directive-based hint=%d, distance=%d for prefetching "
            "spatial memory reference"},
    {25150, "Using directive-based hint=%d, distance=%d for indirect memory "
            "reference"},
    {25228, "Loop multiversioned for Data Dependence"},
    {25258, "Loop peeled using condition%s"},
    {25259, "Loop optimized away using condition%s"},
    {25264, "Loop rerolled by %d"},
    {25397, "Materialized a loop with a trip count of %d"},
    {25399, "memcopy generated"},
    {25408, "memset generated"},
    {25422, "Invariant Condition%s hoisted out of this loop"},
    {25423, "Invariant If condition%s hoisted out of this loop"},
    {25424, "Invariant Switch condition%s hoisted out of this loop"},
    {25426, "Loop distributed (%d way) for perfect loopnest formation"},
    {25427,
     "Loop distributed (%d way) for enabling vectorization on some chunks"},
    {25428, "Loop stripmined by %d"},
    {25436, "Loop completely unrolled by %d"},
    {25438, "Loop unrolled without remainder by %d"},
    {25439, "Loop unrolled with remainder by %d"},
    {25444, "Loopnest interchanged: %s"},
    {25445, "Loop interchange not done due to: %s"},
    {25446, "Dependencies found between following statements:"},
    {25447, "Loop interchange: %s"},
    {25451, "Advice: Loop interchange, if possible, might help loopnest. "
            "Suggested Permutation: %s"},
    {25459, "Loopnest replaced by matmul intrinsic"},
    {25474, "Multiversioned v%d"},
    {25476, "Predicate Optimized v%d"},
    {25478, "While loop unrolled by %d"},
    {25481, "Distribute point pragma not processed"},
    {25482, "No Distribution as requested by pragma"},
    {25483, "Distribute point pragma processed"},
    {25484,
     "Distribute point pragma not processed: Unsupported constructs in loops"},
    {25485,
     "Distribute point pragma not processed: Loopnest too large for stripmine"},
    {25486,
     "Distribute point pragma not processed: Too many Distribute points"},
    {25487, "Loop peeled to eliminate data dependence"},
    {25488, "LOOP WAS REROLLED"},
    {25489, "MEMOP WAS GENERATED FOR THIS LOOP"},
    {25490, "LOOP STMTS WERE REORDERED"},
    {25491, "Remainder loop"},
    {25492, "AUTO PARALLELIZED LOOP"},
    {25493, "OPENMP LOOP"},
    {25494, "CILK FOR LOOP"},
    {25495, " LOOPS WERE COLLAPSED TO FORM THIS LOOP"},
    {25496, "MULTIVERSIONED FOR DATA DEPENDENCE"},
    {25497, "MULTIVERSIONED FOR TRIP COUNT "},
    {25498, "MULTIVERSIONED FOR SAME BASE ARRAYS"},
    {25499, "MULTIVERSIONED FOR SCALAR REPLACEMENT "},
    {25500, "MULTIVERSIONED FOR STRIDES IN ASSUMED SHAPE ARRAYS"},
    {25501, "MULTIVERSIONED FOR ADDRESS PREDICATES"},
    {25502, "MULTIVERSIONED FOR ASSUMED DEPENDENCIES IN STRIDES"},
    {25503, ", VER "},
    {25504, "LOOP WAS DISTRIBUTED"},
    {25505, ", CHUNK "},
    {25506, "LOOP WAS PREDICATE OPTIMIZED"},
    {25507, ", VER "},
    {25508, "Loop completely unrolled"},
    {25509, "LOOP WAS UNROLLED"},
    {25510, "LOOP WAS UNROLLED AND JAMMED"},
    {25511, " BY "},
    {25512, "LOOP WAS BLOCKED"},
    {25513, "LOOP WAS STRIPMINED"},
    {25514, " BY "},
    {25515, "LOOP WAS PREFETCHED"},
    {25516, ", LINES = "},
    {25517, "LOOP WAS VECTORIZED"},
    {25518, "Peeled loop for vectorization"},
    {25519, "Remainder loop for vectorization"},
    {25520, "SIMD LOOP"},
    {25521, "SIMD ENABLED VECTOR FUNCTION"},
    {25522, "CILK PLUS ARRAY NOTATION LOOP"},
    {25523, "MASKED VECTORIZATION"},
    {25524, "VECTORIZATION HAS UNALIGNED MEMORY REFERENCES"},
    {25525, "VECTORIZATION SPEEDUP COEFFECIENT"},
    {25526, "ALTERNATE ALIGNMENT VECTOR LOOP"},
    {25527, "VECTOR TRIP COUNT IS ESTIMATED CONSTANT"},
    {25528, "Load/Store of reduction at line %d sinked after loop"},
    {25529, "Dead stores eliminated in loop"},
    {25530, "Stmt at line %d sinked after loop using last value computation"},
    {25531, "LOOP WITH USER VECTOR INTRINSICS"},
    {25540, "Loop unrolled and jammed by %d"},
    {25562, "The loop has been multiversioned for the small trip count"},
    {25563, "Load hoisted out of the loop"},
    {25564, "Store sinked out of the loop"},
    {25565, "Blocking using Pragma directives"},
    {25566, "blocked by %d"},
    {25567, "%d loops have been collapsed"},
    {25579, "Loop was reversed"},
    {25580, "Induction variable range split using condition%s"},
    {25581, "Loop has been row-wise multiversioned"},
    {25583, "Number of Array Refs Scalar Replaced In Loop: %d"},
    {25584, "Inner loop sums optimized with sum window reuse"},
    {25585, "Loop converted to switch"},
    {25586, "Peeled loop for fusion"},
    {25587, "Loop has reduction"},
    {25588, "Loop has SIMD reduction"},
};

const char *OptReportDiag::getMsg(unsigned Id) {
  auto DiagMapIt = Diags.find(Id);

  // Sanity check
  assert(DiagMapIt != Diags.end() && "Invalid diagnostic ID.");
  if (DiagMapIt == Diags.end())
    return nullptr;

  return (*DiagMapIt).second;
}
