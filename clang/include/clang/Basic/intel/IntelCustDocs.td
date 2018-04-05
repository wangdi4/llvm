//===--- IntelCustDocs.td - Documentation for intel-specific attrs --------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

// To test that the documentation builds cleanly, you must run clang-tblgen to
// convert the .td file into a .rst file, and then run sphinx to convert the
// .rst file into an HTML file. After completing testing, you should revert the
// generated .rst file so that the modified version does not get checked in to
// version control.
//
// To run clang-tblgen to generate the .rst file:
// clang-tblgen -gen-intel-docs -I <root>/llvm/tools/clang/include
//   <root>/llvm/tools/clang/include/clang/Basic/intel/IntelCustDocs.td -o
//   <root>/llvm/tools/clang/docs/IntelCustomizations.rst
//
// To run sphinx to generate the .html files (note that sphinx-build must be
// available on the PATH):
// Windows (from within the clang\docs directory):
//   make.bat html
// Non-Windows (from within the clang\docs directory):
//   make -f Makefile.sphinx html

// The documentation is organized by category.
class DocumentationCategory<string name> {
  string Name = name;
  code Content = [{}];
}

// New categories can be added by adding a new record here.
def DocCatMSCompat : DocumentationCategory<"Microsoft Compatibility">;
def DocCatIntelCompat : DocumentationCategory<"Intel Compatibility">;

// Specifies the documentation to be associated with the given category.
class Documentation {
  DocumentationCategory Category;
  code Content;
  // The Option is optional.  If used it allows splitting based on a tag.
  // This works well for options like IntelCompat which control many small
  // "features".
  string Option = "";
  // The remaining fields are TBD. Items can be added (with an initialized
  // string) or removed here without the need to change the tblgen code.
  string BugTracker = "";
  string Customer = "";
  string RegressionTest = "";
  string Owner = "";
  string CompatVersions = "";
}

def StringCharStarCatchableDocs : Documentation {
  let Category = DocCatMSCompat;
  let Option = "IntelCompat";
  let BugTracker = "tr38183,CQ407554,CMPLRS-25559";
  let RegressionTest = "cfeCpp/tr38183";
  let Owner = "mprice1";
  let CompatVersions = "vs2015";
  let Content = [{
Microsoft allows you to catch a string literal with catch(char \*).

.. code-block:: c++

  int main()
  {
    int R;
    try {
      throw "foo";
    }
    catch (char *) {
      R = 1;
    }
    catch (...) {
      R = 2;
    }
    return R;
  }

.. code-block:: console

  $ cl /nologo /EHsc t.cpp && ./t.exe
  t.cpp
  $ echo $?
  1

.. code-block:: console

  $ clang-cl /nologo /EHsc t.cpp && ./t.exe
  $ echo $?
  2

  }];
}

def UnrollExtensionsDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#371799,CMPLRS-36479";
  let RegressionTest = "regC/tr50768";
  let Owner = "mprice1";
  let CompatVersions = "";
  let Content = [{
#pragma unroll applies to a loop.  The classic Intel compiler allows it on any
statement and applies it to the first loop it encounters.  This would produce
a diagnostic on most other compilers.

Also allows the user to specify multiple #pragma unrolls on a single loop.  The
compiler ignores all but that last one.

In the classic Intel compiler the IL0 directive is emitted at the location the
user specified and the back end does the work to get it applied to the loop.

In clang the front end must deal with it. When an unroll is seen it is saved
and applied when a loop statement is seen.

.. code-block:: c++

  void foo()
  {
    int i;
    #pragma unroll
    i = 100;
    for( i = 0; i < 2; i++ ) { i++; }
  }

.. code-block:: console

  $ icc -c t.c
  $

.. code-block:: console

  $ clang -c t.c
  t.c:5:5: error: expected a for, while, or do-while loop to follow
        '#pragma unroll'
      i = 100;
      ^
  1 error generated.
  $

  }];
}

def AllowMissingTypenameDocs : Documentation {
  let Category = DocCatIntelCompat;
  let Option = "IntelCompat";
  let BugTracker = "CQ#368310,CMPLRS-25573";
  let RegressionTest = "cfeCpp/cq117771";
  let Owner = "";
  let CompatVersions = "";
  let Content = [{
Microsoft allows you to leave off typename in some contexts.  Clang also
allows this in MSVCCompat mode.  See isMicrosoftMissingTypename in
SemaDecl.cpp.

The Intel compiler allows this in more cases than Clang and in some cases
more than Microsoft.

Recommendation: Run full testing without this customization.  If tests fail
the same with Microsoft and GCC remove it.

.. code-block:: c++

  template <class T>
  struct X {
    typedef T type1;
    type1 foo2();
  };

  template<class T>
  X<T>::type1 X<T>::foo2() {
    type1 t;
    return t;
  }

  int main() {
    X<float> x;
    return x.foo2();
  }

.. code-block:: console

  $ clang-cl -c -GX -GR  -Tpt.cpp
  t.cpp(8,1):  error: missing 'typename' prior to dependent type name 'X<T>::type1'
  X<T>::type1 X<T>::foo2() {
  ^~~~~~~~~~~
  typename
  1 error generated.

.. code-block:: console

  [0:windev-lab:test]$ icx -c -nologo t.cpp
  t.cpp
  t.cpp:8:1: warning: missing 'typename' prior to dependent type name 'X<T>::type1' [-Wtypename-missing]
  X<T>::type1 X<T>::foo2() {
  ^~~~~~~~~~~
  typename
  1 warning generated.

  }];
}

def GlobalDocumentation {
  code Intro =[{..
  -------------------------------------------------------------------
  NOTE: This file is automatically generated by running
  clang-tblgen -gen-intel-cust-docs. Do not edit this file by hand!!
  -------------------------------------------------------------------

===============================
Intel-only Clang Customizations
===============================
.. contents::
   :local:

Introduction
============

This page lists the Intel customizations we've added to Clang. All
customizations can be described here in a single location and the description
can be reviewed with the code implementing the customization.  Making this
part of the code review means we can ensure all changes are documented
and will never be lost in bug tracking or source control changes.

Each customization is associated with a tag both here and in the sources.  When
encountering this customization during development or resolving a merge
conflict we can go directly here for information.

It also allows us a means to enumerate all our changes so they can be reviewed
from time to time.  For example if we are ready to release a new version of
the compiler where support for X is no longer needed, we can look at these
and remove any related to X.

Each customization has an associated category and additional categories can
be added as needed.

Customizations will have whatever properties we decide we need.  Some
possibilities:

 - BugTracker - Jira number in case more investigation is needed. We want it
   to be rare to go here though.
 - Customer - If an important customer requested this.
 - RegressionTest - To easily find some real code for this feature.
 - Owner - Who implemented it or someone who knows something about it if it
   goes wrong or needs to be merged with community changes.
 - CompatVersions - If this is some compatibility fix which does it apply to.
   Possibly update this field when new MS/GCC compilers are released and
   remove if we don't care about it anymore.

The Content should describe the basic functionality, give a small code snippet,
and describe any unusual or tricky parts of the implementation.

Using Tags
==========

Each documented item has a tag.  The tag can be used in the sources to point
the reader to this documentation.

So instead of::

  #if INTEL_CUSTOMIZATION
    <code implementing some extension>
  #endif // INTEL_CUSTOMIZATION

We can add the tag along with the code::

  #if INTEL_CUSTOMIZATION
    // IntelBlahBlahFeature
    <code implementing some extension>
  #endif // INTEL_CUSTOMIZATION

If we come across this block reading code or resolving a merge conflict we can
easily open the docs and find out why it is here.

In addition when used with an option like IntelCompat we can use the tag as
part of the condition that checks the flag::

  if (getContext().getLangOpts().isIntelCompat(LangOptions::XYZ))

Not only does this identify the customization it allows us to individually
control that customization.  If we are triaging a bug and think this one is
to blame we can easily test it with a command line option::

  icx -Xclang -fintel-compatibility -fintel-compatibility-disable=XYZ

Or if we want to just enable that one item::

  icx -Xclang -fintel-compatibility-enable=XYZ

These options are for internal use only.  They are for compiler developers to
more easily understand and maintain the code.  We don't want customers using
them and we don't want anyone relying on specific names.  We can expect to
remove or rename them when it makes sense.

A list of current tags can be shown with::

 icx -Xclang -fintel-compatibility-help

Using these in normal development should be very easy.  When implementing
a new extension you add the documentation to IntelCustDocs.td and then just
use the tag in your code.  The documentation and code are developed and
reviewed together.
}];
}
