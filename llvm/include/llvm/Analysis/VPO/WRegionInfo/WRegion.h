//===----------------- WRegion.h - W-Region node ----------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file defines the classes derived from WRegionNode that
//   correspond to each construct type in WRegionNode::WRegionNodeKind
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGION_H
#define LLVM_ANALYSIS_VPO_WREGION_H

#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionClause.h"
#include <set>
#include <iterator>

namespace llvm {

class LoopInfo;

namespace vpo {

typedef WRContainerTy::iterator               wrn_iterator;
typedef WRContainerTy::const_iterator         wrn_const_iterator;
typedef WRContainerTy::reverse_iterator       wrn_reverse_iterator;
typedef WRContainerTy::const_reverse_iterator wrn_const_reverse_iterator;

/// \brief WRegion is an intermediate class to work around circular build 
/// problem that happens if we have WRContainerTy (for the "Children" member)
/// directly in WRegionNode. This intemediate class should never be 
/// instantiated directly. All specialized WRN classes are derived from WRegion.
class WRegion : public WRegionNode {
private:
  WRContainerTy Children;

protected:
  WRegion(unsigned SCID);
  WRegion(WRegionNode *W);

public:
  /// Iterators to iterate over WRegionNodes in the "Children" container

  /// \brief Children iterator methods
  wrn_iterator         wrn_child_begin() { return Children.begin(); }
  wrn_iterator         wrn_child_end()   { return Children.end(); }

  wrn_const_iterator   wrn_child_begin() const { return Children.begin(); }
  wrn_const_iterator   wrn_child_end()   const { return Children.end(); }

  wrn_reverse_iterator wrn_child_rbegin() { return Children.rbegin(); }
  wrn_reverse_iterator wrn_child_rend()   { return Children.rend(); }

  wrn_const_reverse_iterator wrn_child_rbegin() const {
    return Children.rbegin();
  }
  wrn_const_reverse_iterator wrn_child_rend() const {
    return Children.rend();
  }

  /// \brief Returns true if it has children.
  bool hasChildren() const { return !Children.empty(); }

  /// \brief Returns the number of children.
  unsigned getNumChildren() const { return Children.size(); }

  /// \brief Returns the Children container (by ref)
  WRContainerTy &getChildren() { return Children ; }

  /// \brief Returns the first child if it exists, otherwise returns null.
  WRegionNode *getFirstChild();

  /// \brief Returns the last child if it exists, otherwise returns null.
  WRegionNode *getLastChild();

  /// \brief Prints children.
  void printChildren(formatted_raw_ostream &OS, unsigned Depth) const;

}; // class WRegion


//
// Classes below represent REGIONS, one for each type in
// WRegionNode::WRegionNodeKind
//
// All of them are derived classes of the base WRegionNode
//
// For completeness, there's a class for each WRegionNodeKind, but depending
// on implementation, some classes may not be needed (eg, #pragma omp section
// was not represented in the icc implementation)
//
//    WRegionNodeKind:    Class name:             Example OpenMP pragma
//
//    WRNParallel         WRNParallelNode         #pragma omp parallel
//    WRNParallelLoop     WRNParallelLoopNode     #pragma omp parallel for
//    WRNParallelSections WRNParallelSectionsNode #pragma omp parallel sections
//    WRNTask             WRNTaskNode             #pragma omp task
//    WRNVecLoop          WRNVecLoopNode          #pragma omp simd
//    WRNWksLoop          WRNWksLoopNode          #pragma omp for
//    WRNWksSections      WRNWksSectionsNode      #pragma omp sections
//    WRNSection          WRNSectionNode          #pragma omp section
//    WRNSingle           WRNSingleNode           #pragma omp single
//    WRNMaster           WRNMasterNode           #pragma omp master
//    WRNAtomic           WRNAtomicNode           #pragma omp atomic
//    WRNBarrier          WRNBarrierNode          #pragma omp barrier
//    WRNCancel           WRNCancelNode           #pragma omp cancel
//    WRNCritical         WRNCriticalNode         #pragma omp critical
//    WRNFlush            WRNFlushNode            #pragma omp flush
//    WRNOrdered          WRNOrderedNode          #pragma omp ordered
//    WRNTaskgroup        WRNTaskgroupNode        #pragma omp taskgroup


//
// WRNParallelNode
//
// #pragma omp parallel
//
class WRNParallelNode : public WRegion
{
  private:
    SharedClause           *Shared;
    PrivateClause          *Priv;
    FirstprivateClause     *Fpriv;
    ReductionClause        *Reduction;
    CopyinClause           *Copyin;
    EXPR                   *IfExpr;
    EXPR                   *NumThreads;
    VPODefaultKind         DefaultType;    
    VPOProcBindKind        ProcBindType;   

  protected:
    void setShared(SharedClause *S)      { Shared = S;       }
    void setPriv(PrivateClause *P)       { Priv = P;         }
    void setFpriv(FirstprivateClause *F) { Fpriv = F;        }
    void setRed(ReductionClause *R)      { Reduction = R;    }
    void setCopyin(CopyinClause *C)      { Copyin = C;       }
    void setIf(EXPR *E)                  { IfExpr = E;       }
    void setNumThreads(EXPR *E)          { NumThreads = E;   }
    void setDefault(VPODefaultKind T)    { DefaultType = T;  }
    void setProcBind(VPOProcBindKind P)  { ProcBindType = P; }

  public:
    WRNParallelNode();
    WRNParallelNode(WRNParallelNode *W);
    //WRNParallelNode(const WRNParallelNode &W);  // copy constructor

    SharedClause       * getShared()     const { return Shared;       }
    PrivateClause      * getPriv()       const { return Priv;         }
    FirstprivateClause * getFpriv()      const { return Fpriv;        }
    ReductionClause    * getRed()        const { return Reduction;    }
    CopyinClause       * getCopyin()     const { return Copyin;       }
    EXPR               * getIf()         const { return IfExpr;       }
    EXPR               * getNumThreads() const { return NumThreads;   }
    VPODefaultKind     getDefault()      const { return DefaultType;  }
    VPOProcBindKind    getProcBind()     const { return ProcBindType; }

    void print(formatted_raw_ostream &OS, unsigned Depth) const ;

    /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
    static bool classof(const WRegionNode *W) {
      return W->getWRegionKindID() == WRegionNode::WRNParallel;
    }
};

//
// WRNParallelLoopNode
//
// #pragma omp parallel loop
//
class WRNParallelLoopNode : public WRegion
{
  private:
    SharedClause           *Shared;
    PrivateClause          *Priv;
    FirstprivateClause     *Fpriv;
    ReductionClause        *Reduction;
    CopyinClause           *Copyin;
    EXPR                   *IfExpr;
    EXPR                   *NumThreads;
    VPODefaultKind         DefaultType;    
    VPOProcBindKind        ProcBindType;   

  protected:
    void setShared(SharedClause *S)      { Shared = S;       }
    void setPriv(PrivateClause *P)       { Priv = P;         }
    void setFpriv(FirstprivateClause *F) { Fpriv = F;        }
    void setRed(ReductionClause *R)      { Reduction = R;    }
    void setCopyin(CopyinClause *C)      { Copyin = C;       }
    void setIf(EXPR *E)                  { IfExpr = E;       }
    void setNumThreads(EXPR *E)          { NumThreads = E;   }
    void setDefault(VPODefaultKind T)    { DefaultType = T;  }
    void setProcBind(VPOProcBindKind P)  { ProcBindType = P; }

  public:
    WRNParallelLoopNode();
    WRNParallelLoopNode(WRNParallelLoopNode *W);
    //WRNParallelLoopNode(const WRNParallelLoopNode &W);  // copy constructor

    SharedClause       * getShared()     const { return Shared;       }
    PrivateClause      * getPriv()       const { return Priv;         }
    FirstprivateClause * getFpriv()      const { return Fpriv;        }
    ReductionClause    * getRed()        const { return Reduction;    }
    CopyinClause       * getCopyin()     const { return Copyin;       }
    EXPR               * getIf()         const { return IfExpr;       }
    EXPR               * getNumThreads() const { return NumThreads;   }
    VPODefaultKind     getDefault()      const { return DefaultType;  }
    VPOProcBindKind    getProcBind()     const { return ProcBindType; }

    void print(formatted_raw_ostream &OS, unsigned Depth) const ;

    /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
    static bool classof(const WRegionNode *W) {
      return W->getWRegionKindID() == WRegionNode::WRNParallelLoop;
    }
};

//
// WRNVecLoopNode
//
// #pragma omp simd
//
class WRNVecLoopNode : public WRegion
{
  private:
    PrivateClause      *Priv;
    LastprivateClause  *Lpriv;
    ReductionClause    *Reduction;
    LinearClause       *Linear;
    AlignedClause      *Aligned;
    int                Simdlen;  
    int                Safelen;  
    int                Collapse;
    bool               IsAutoVec;
    LoopInfo           *Loop;

  protected:
    void setPriv(PrivateClause *P)      { Priv = P;         }
    void setLpriv(LastprivateClause *L) { Lpriv = L;        }
    void setRed(ReductionClause *R)     { Reduction = R;    }
    void setLinear(LinearClause *L)     { Linear = L;       }
    void setAligned(AlignedClause *A)   { Aligned = A;      }
    void setSimdlen(int N)              { Simdlen = N;      }
    void setSafelen(int N)              { Safelen = N;      }
    void setCollapse(int N)             { Collapse = N;     }
    void setIsAutoVec(bool Flag)        { IsAutoVec = Flag; }
    void setLoopInfo(LoopInfo *L)       { Loop = L;         }

  public:

    WRNVecLoopNode();
    WRNVecLoopNode(WRNVecLoopNode *W);
    //WRNVecLoopNode(const WRNVecLoopNode &W);  // copy constructor

    PrivateClause     * getPriv()    { return Priv;      }
    LastprivateClause * getLpriv()   { return Lpriv;     }
    ReductionClause   * getRed()     { return Reduction; }
    LinearClause      * getLinear()  { return Linear;    }
    AlignedClause     * getAligned() { return Aligned;   }

    int  getSimdlen()       const { return Simdlen;   }
    int  getSafelen()       const { return Safelen;   }
    int  getCollapse()      const { return Collapse;  }
    bool getIsAutoVec()     const { return IsAutoVec; }
    LoopInfo *getLoopInfo() const { return Loop;      }
    
    void print(formatted_raw_ostream &OS, unsigned Depth) const ;

    /// \brief Method to support type inquiry through isa, cast, and dyn_cast.
    static bool classof(const WRegionNode *W) {
      return W->getWRegionKindID() == WRegionNode::WRNVecLoop;
    }
};

} // End namespace vpo

} // End namespace llvm

#endif
