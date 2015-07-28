// RUN: pushd %S; %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s; popd

#pragma include_directory               // expected-warning {{Path to directory is expected}}

#pragma include_directory ""            // expected-warning {{Path to directory is expected}}

#pragma include_directory "asaswq21223" // expected-warning {{Path to directory is expected}}

#pragma include_directory "/tmp" fdfgdfg 

#pragma include_directory "./intel"

#include "ttt.h"


