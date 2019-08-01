// Test that the switch for generating svml calls is accepted.
// Tests that test the actual translation to svml can be found in llvm/test/Transforms/Intel_MapIntrinToIml.
// RUN: %clang_cc1 -fveclib=SVML -verify %s
// expected-no-diagnostics
