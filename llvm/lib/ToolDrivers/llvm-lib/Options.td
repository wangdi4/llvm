// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
include "llvm/Option/OptParser.td"

// lib.exe accepts options starting with either a dash or a slash.

// Flag that takes no arguments.
class F<string name> : Flag<["/", "-", "/?", "-?"], name>;

// Flag that takes one argument after ":".
class P<string name, string help> :
      Joined<["/", "-", "/?", "-?"], name#":">, HelpText<help>;

// Boolean flag which can be suffixed by ":no". Using it unsuffixed turns the
// flag on and using it suffixed by ":no" turns it off.
multiclass B<string name, string help_on, string help_off> {
  def "" : F<name>, HelpText<help_on>;
  def _no : F<name#":no">, HelpText<help_off>;
}

def ignore : P<"ignore", "Specify warning codes to ignore">;
def libpath: P<"libpath", "Object file search path">;

// Can't be called "list" since that's a keyword.
def lst    : F<"list">, HelpText<"List contents of .lib file on stdout">;
def out    : P<"out", "Path to file to write output">;

def llvmlibthin : F<"llvmlibthin">,
    HelpText<"Make .lib point to .obj files instead of copying their contents">;

def llvmlibempty : F<"llvmlibempty">,
    HelpText<"When given no contents, produce an empty .lib file">;

def machine: P<"machine", "Specify target platform">;

defm WX : B<"WX", "Treat warnings as errors",
            "Don't treat warnings as errors (default)">;

def help : F<"help">;

// /?? and -?? must be before /? and -? to not confuse lib/Options.
def help_q : Flag<["/??", "-??", "/?", "-?"], "">, Alias<help>;

#ifdef INTEL_CUSTOMIZATION
// This option enables to process libraries with opaque pointers
def opaque_pointers : F<"opaque-pointers">;
#endif // INTEL_CUSTOMIZATION

//==============================================================================
// The flags below do nothing. They are defined only for lib.exe compatibility.
//==============================================================================

def ltcg : F<"ltcg">;
def nodefaultlib: P<"nodefaultlib", "">;
def nodefaultlib_all: F<"nodefaultlib">;
def nologo : F<"nologo">;
def subsystem : P<"subsystem", "">;
def verbose : F<"verbose">;
