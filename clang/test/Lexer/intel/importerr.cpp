// RUN: %clang_cc1 -verify -fintel-ms-compatibility %s
// REQUIRES: system-windows

#import <libid:GC0714F2-3D04-11D1-AE7D-00A0C90F26F4> // expected-error {{could not determine header file for this typelib}} 

#import <libid:AC0714F2-3D04-11D1-AE7D-00A0C90F26F4> version(foo)  // expected-error {{bad attribute value in #import directive}} 
