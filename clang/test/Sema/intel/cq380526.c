// CQ#380526
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c89 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c90 %s

// expected-no-diagnostics

int main (int argc, char *argv<::>)
  <%
  return 0;
  %>

