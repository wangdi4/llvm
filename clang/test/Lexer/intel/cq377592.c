// RUN: %clang -target i686-windows-msvc -Xclang -fintel-compatibility -fsyntax-only %s

_Static_assert(!__is_identifier(static_assert), "static_assert");

