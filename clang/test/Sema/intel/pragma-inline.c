// RUN: %clang_cc1 -verify -fsyntax-only -fintel-compatibility -verify %s

int foo(int j)
{
  return j;
}

int bar(int i)
{
  return  foo(i);
}

int main ()
{
  int i;
  #pragma forceinline recursive
  for (i=0; i<10;++i)
    { bar(i);}
  #pragma inline recursive
  for (i=0; i<10;++i)
    {bar(i);}
  #pragma noinline
  for (i=0; i<10;++i)
    {bar(i);}
  #pragma forceinline
  for (i=0; i<10;++i)
    {bar(i);}
  #pragma inline
  for (i=0; i<10;++i)
    {bar(i);}
  #pragma forceinline goo // expected-error {{recursive keyword expected after a pragma inline directive}}
  for (i=0; i<10;++i)
    { bar(i);}
  #pragma inline hoo // expected-error {{recursive keyword expected after a pragma inline directive}}
  bar(6);
  #pragma inline
}   // expected-error {{expected statement}}

