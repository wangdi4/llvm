// RUN: %clang_cc1 %s -verify -fsyntax-only

[[intel::prefer_dsp, intel::prefer_softlogic]] void f1_cxx11(); // expected-error {{'prefer_softlogic' and 'prefer_dsp' attributes are not compatible}} \
                                                                // expected-note {{conflicting attribute is here}}

__attribute__((prefer_dsp, prefer_softlogic)) void f1_gnu(); // expected-error {{'prefer_softlogic' and 'prefer_dsp' attributes are not compatible}} \
                                                             // expected-note {{conflicting attribute is here}}

[[intel::prefer_softlogic, intel::prefer_dsp]] void f2(); // expected-error {{'prefer_dsp' and 'prefer_softlogic' attributes are not compatible}} \
                                                          // expected-note {{conflicting attribute is here}}

[[intel::prefer_dsp]] void f3_cxx11(); // expected-note {{conflicting attribute is here}}
[[intel::prefer_softlogic]] void f3_cxx11(); // expected-error {{'prefer_softlogic' and 'prefer_dsp' attributes are not compatible}}

__attribute__((prefer_dsp)) void f3_gnu(); // expected-note {{conflicting attribute is here}}
__attribute__((prefer_softlogic)) void f3_gnu(); // expected-error {{'prefer_softlogic' and 'prefer_dsp' attributes are not compatible}}

[[intel::prefer_dsp]] void f3_cxx11_gnu(); // expected-note {{conflicting attribute is here}}
__attribute__((prefer_softlogic)) void f3_cxx11_gnu(); // expected-error {{'prefer_softlogic' and 'prefer_dsp' attributes are not compatible}}

[[intel::prefer_softlogic]] void f4(); // expected-note {{conflicting attribute is here}}
[[intel::prefer_dsp]] void f4(); // expected-error {{'prefer_dsp' and 'prefer_softlogic' attributes are not compatible}}

[[intel::prefer_dsp("no")]] void f5_cxx11(); // expected-error {{'prefer_dsp' attribute takes no arguments}}
[[intel::prefer_softlogic("no")]] void f6_cxx11(); // expected-error {{'prefer_softlogic' attribute takes no arguments}}
[[intel::propagate_dsp_preference("no")]] void f7_cxx11(); // expected-error {{'propagate_dsp_preference' attribute takes no arguments}}

__attribute__((prefer_dsp ("no"))) void f5_gnu(); // expected-error {{'prefer_dsp' attribute takes no arguments}}
__attribute__((prefer_softlogic ("no"))) void f6_gnu(); // expected-error {{'prefer_softlogic' attribute takes no arguments}}
__attribute__((propagate_dsp_preference ("no"))) void f7_gnu(); // expected-error {{'propagate_dsp_preference' attribute takes no arguments}}

[[intel::prefer_dsp]] int x1_cxx11; // expected-error {{'prefer_dsp' attribute only applies to functions}}
[[intel::prefer_softlogic]] int x2_cxx11; // expected-error {{'prefer_softlogic' attribute only applies to functions}}
[[intel::propagate_dsp_preference]] int x3_cxx11; // expected-error {{'propagate_dsp_preference' attribute only applies to functions}}

__attribute__((prefer_dsp)) int x1_gnu; // expected-error {{'prefer_dsp' attribute only applies to functions}}
__attribute__((prefer_softlogic)) int x2_gnu; // expected-error {{'prefer_softlogic' attribute only applies to functions}}
__attribute__((propagate_dsp_preference)) int x3_gnu; // expected-error {{'propagate_dsp_preference' attribute only applies to functions}}
