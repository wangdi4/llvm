// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
#pragma region wwww
#pragma endregion
#pragma endregion  // expected-warning {{the #pragma region for this #pragma endregion is missing}}
#pragma endregion  // expected-warning {{the #pragma region for this #pragma endregion is missing}}

