/****************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OclBuiltinsHeaderGen.h

\****************************************************************************/

#ifndef __OCLBUILTINS_HEADER_GEN_H__
#define __OCLBUILTINS_HEADER_GEN_H__

#include "llvm/TableGen/TableGenBackend.h"

namespace llvm {

////////////////////////////////////////////////////
//Name: OclBuiltinsHeaderGen
//Purpose: generates an array of strings, contining all the mangled names of
//the builting in the given td file.
////////////////////////////////////////////////////
class OclBuiltinsHeaderGen: public TableGenBackend {
public:
  explicit OclBuiltinsHeaderGen(RecordKeeper&);
  void run(raw_ostream&);
protected:
  RecordKeeper& m_recordKeeper;
};
}
#endif//__OCLBUILTINS_HEADER_GEN_H__
