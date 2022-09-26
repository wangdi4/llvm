// RUN: %clang_cc1 -fintel-compatibility -std=c89 -verify=c89 %s
// RUN: %clang_cc1 -fintel-compatibility -std=c99 -verify=c99 %s
// RUN: %clang_cc1 -std=c89 -verify=c89 %s
// RUN: %clang_cc1 -std=c99 -verify=c99 %s

// c89-no-diagnostics

// In c99 this is a keyword, so these look like bizarre nonsense.
int restrict = 0; // c99-error{{expected identifier or '('}}

int foo() {
  int restrict; // c99-error{{restrict requires a pointer or reference}}\
                // c99-warning{{declaration does not declare anything}}
  restrict = 1; // c99-error{{expected identifier or '('}}
  return 0;
}
