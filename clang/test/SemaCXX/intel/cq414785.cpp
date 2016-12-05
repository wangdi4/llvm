// RUN: %clang_cc1 -verify -fsyntax-only -triple x86_64-pc-linux-gnu %s -Wno-literal-conversion -Wfloat-conversion -DFLOAT_CONVERSION -DZERO -DBOOL -DCONSTANT_BOOL -DOVERFLOW
// RUN: %clang_cc1 -verify -fsyntax-only -triple x86_64-pc-linux-gnu %s -Wno-conversion -Wfloat-overflow-conversion -DOVERFLOW
// RUN: %clang_cc1 -verify -fsyntax-only -triple x86_64-pc-linux-gnu %s -Wno-conversion -Wfloat-zero-conversion -DZERO
// expected-no-diagnostics


int m(int argc)
{
  float f = argc;

  if (f)  	
    return 1;
  else  
    return f
           ? 1 
           : 0;

  if (f+1)  	//Question: also suppress for this expresson?
    return 1;

  while (f) ;

  do ; while (f) ;

  for (; f ; ) ;

  return 0;
}
