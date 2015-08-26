// RUN: %clang_cc1 -fsyntax-only -triple=i386-pc-windows-msvc -fintel-compatibility -verify %s

class CVssWriter
{
public:
  __stdcall CVssWriter() {}
  virtual __stdcall ~CVssWriter() {} // expected-note {{overridden virtual function is here}}
};

class CVssWriterEx : public CVssWriter // expected-warning {{virtual function '~CVssWriterEx' has different calling convention attributes ('void () __attribute__((thiscall))') than the function it overrides (which has calling convention 'void () __attribute__((stdcall))'). New attribute ignored.}}
{};

