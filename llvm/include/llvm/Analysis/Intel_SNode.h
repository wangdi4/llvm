//===- IntelSNode.h - Structure Node Analysis ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This is the interface for LLVM's structure node analysis for LLVM IR.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_ANALYSIS_INTEL_SNODE_H
#define LLVM_ANALYSIS_INTEL_SNODE_H
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Casting.h"
#include <functional>
#include <map>
#include <vector>
#include <list>

namespace llvm {
class Loop;
class LoopInfo;
class LoopBlocksTraversal;
class SNode;
class BlockSNode;
class SNodeAnalysis;
typedef std::map<BasicBlock *, BlockSNode *> BB2SNodeMapTy;
typedef std::map<Loop *, SNode *> Loop2SNodeMapTy;
typedef std::list<SNode *> SNodeListTy;
typedef std::list<SNode *>::iterator snode_list_iterator;
typedef std::list<SNode *>::const_iterator const_snode_list_iterator;
typedef std::vector<SNode *> SNodeVectorTy;
typedef std::vector<SNode *>::iterator snode_vector_iterator;
typedef std::vector<SNode *>::const_iterator const_snode_vector_iterator;


// The existing LLVM compiler only provides incomplete hierarchical 
// information such as loopInfo and regionInfo. It does not provide
// any information to model the if branches. It does not provide accurate 
// information to understand whether the loop is do-while loop, 
// while loop or do-loop either. This information is essential for 
// compiler analysis, transformations and program understanding. 
// In order to overcome these issues, the SNodeInfo is designed to 
// provide the complete hierarchical information. 
//
// An SNode represents a single structured control flow element, 
// e.g. if-then, if-then-else, do-while-loop, while-loop, do-loop, 
// switch, or condition. In the extreme case, it represents one 
// basic block. At the top level, SNodes are linked to each other 
// via the Pred/Succ lists to form a complete CFG for the routine. 
// Each SNode consists of one or more structural sub-elements, or 
// Children. For example, an if-then-else has Children representing 
// the conditional SNode, the then SNode, and the else SNode. Each 
// child is itself at the top of an arbitrarily complex tree of 
// SNodes.
//
// The basic structure of SNode is as follows. The first member 
// is the unique ID of a SNode. The second member is the type of 
// SNode. For example, it can be a BlockSNode, ListSNode
// or IfThenElseSNode and etc. If the SNode is inside a tree, it has 
// parent and children. The pred or the succ of such SNode is empty.
// For the SNode at the top level of the hierarchy, i.e. at the routine 
// CFG level, the Parent field will point to itself.
//
// class SNode {
//   unsigned int SNodeNum;
//   SNodeOp Op;
//   SNode *Parent;
//   iplist<SNode> Children;
//   std::list<SNode *> Pred;
//   std::list<SNode *> Succ;
// };
//
//
// Given an example in C code and the corresponding LLVM IR, it is hard 
// to figure out the control flow inside the loop from the LLVM IR.
// After the SNodeInfo is built on top of the LLVM IR, it is easy to 
// understand there is a if-then statement inside a loop.
//
// int bar(int i);
// void foo(int m, int n, int *b)
// {
//   for (int i=0;i<m;i++) {
//     if (bar(i))
//       b[i] = i;
//     else
//       b[i] += i;
//   }
// }
//
//
// LLVM IR for the example
//
// define void @foo(i32 %m, i32 %n, i32* nocapture %b) #0 {
// entry:
//   %cmp.8 = icmp sgt i32 %m, 0
//   br i1 %cmp.8, label %for.body, label %for.cond.cleanup
// 
// for.cond.cleanup:                           ; preds = %for.inc, %entry
//   ret void
//
//  for.body:                                  ; preds = %entry, %for.inc
//   %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %entry ]
//   %0 = trunc i64 %indvars.iv to i32
//   %call = tail call i32 @bar(i32 %0) #3
//   %tobool = icmp eq i32 %call, 0
//   %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
//   br i1 %tobool, label %if.else, label %for.inc
//
// if.else:                                    ; preds = %for.body
//   %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
//   %add = add nsw i32 %1, %0
//   br label %for.inc
//
// for.inc:                                    ; preds = %for.body, %if.else
//   %storemerge = phi i32 [ %add, %if.else ], [ %0, %for.body ]
//   store i32 %storemerge, i32* %arrayidx, align 4, !tbaa !1
//   %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
//   %lftr.wideiv = trunc i64 %indvars.iv.next to i32
//   %exitcond = icmp eq i32 %lftr.wideiv, %m
//   br i1 %exitcond, label %for.cond.cleanup, label %for.body
// }
//
// SNode Graph
//
// SN[9]  pred:  succ: 
// SN_LIST SN[9]
//     SN_IF_THEN SN[8]
//         SN_BLOCK SN[0] B[%entry]
//         SN_LOOP SN[7]
//             SN_LIST SN[6]
//                 SN_IF_THEN SN[5]
//                     SN_BLOCK SN[2] B[%for.body]
//                     SN_BLOCK SN[3] B[%if.else]
//                 END  SN_IF_THEN SN[5]
//                 SN_BLOCK SN[4] B[%for.inc]
//             END  SN_LIST SN[6]
//         END  SN_LOOP SN[7]
//     END  SN_IF_THEN SN[8]
//     SN_BLOCK SN[1] B[%for.cond.cleanup]
// END  SN_LIST SN[9]
//
//
//
// Pretty print of SNode Graph
//
//   /* B[%entry]  */
//   %cmp.8 = icmp sgt i32 %m, 0
//   br i1 %cmp.8, label %for.body, label %for.cond.cleanup
//   if (%cmp.8) {
//      do {
//         /* B[%for.body]  */
//         %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %entry ]
//         %0 = trunc i64 %indvars.iv to i32
//         %call = tail call i32 @bar(i32 %0) #3
//         %tobool = icmp eq i32 %call, 0
//         %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv:
//         br i1 %tobool, label %if.else, label %for.inc
//         if (%tobool) {
//
//            /* B[%if.else]  */
//            %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
//            %add = add nsw i32 %1, %0
//            br label %for.inc
//
//         }
//
//         /* B[%for.inc]  */
//         %storemerge = phi i32 [ %add, %if.else ], [ %0, %for.body ]
//         store i32 %storemerge, i32* %arrayidx, align 4, !tbaa !1
//         %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
//         %lftr.wideiv = trunc i64 %indvars.iv.next to i32
//         %exitcond = icmp eq i32 %lftr.wideiv, %m
//         br i1 %exitcond, label %for.cond.cleanup, label %for.body
//
//
//      } while (%exitcond == false)
//   }
//
//   /* B[%for.cond.cleanup]  */
//   ret void
//
// The proposed framework, which is a rules based system, 
// uses pattern match technology to discover the new structured 
// control flows in the control flow graph. The users can add 
// new rules to facilitate their uses. 
//

template <> struct ilist_traits<SNode> : public ilist_default_traits<SNode> {
private:
  mutable ilist_half_node<SNode> Sentinel;

public:
  SNode *createSentinel() const;
  static void destroySentinel(SNode *) {};

  SNode *provideInitialHead() const { return createSentinel(); };
  SNode *ensureHead(SNode *) const { return createSentinel(); };
  static void noteHead(SNode *, SNode *) {};
  static void deleteNode(SNode *){};
};

typedef iplist<SNode> SNodeChildrenListTy;
typedef SNodeChildrenListTy::iterator snode_children_iterator;
typedef SNodeChildrenListTy::reverse_iterator reverse_snode_children_iterator;
typedef SNodeChildrenListTy::const_iterator const_snode_children_iterator;

// The ilist_node is used as the data structure for SNode since I 
// believe the SNode can be used for high level loop transformation 
// such as loop unrolling, where the insert and delete operation 
// would be heavily used. 
class SNode : public ilist_node<SNode> {
friend class SNodeAnalysis;
public:
  enum SNodeOp {
    SN_BLOCK,                   // sn_block { basic block }
    SN_LIST,                    // sn1, sn2, ..., snN
    SN_IF_THEN_ELSE,            // if (sn1) { sn2; } else { sn3;}
  };

private:
  // Unique ID for the SNode
  unsigned int SNodeNum;

  // Type of the SNode
  SNodeOp Op;

  // Parent of the SNode if the SNode is under the hierarchical tree.
  SNode *Parent;

  // The immediate loop where the SNode stays. 
  Loop *Lp;

  // The children SNode in a tree SNode such as ListSNode, IfThenElseSNode.
  SNodeChildrenListTy Children;
  
  // The Preds of the SNode
  SNodeListTy Pred;

  // The succs of the SNode
  SNodeListTy Succ;

  // If the number of predecessors is 2, returns true 
  // and updates the *Pred1 and *Pred2 with the predecessors.
  bool hasTwoPred(SNode **Pred1, SNode **Pred2);
  
  // If the number of successors is 2, returns true
  // and updates the *Succ1 and *Succ2 with the successors.
  bool hasTwoSucc(SNode **Succ1, SNode **Succ2);
  
  // If the number of succssors is more than 2, returns true
  // and replaces the list SnSuccs with the successors.
  // This routine returns false when there are not more than 
  // two unique successors. So, there could be three successors 
  // arcs, but if there are only two actual unique successor 
  // blocks, then this should return false.
  bool hasMoreThanTwoSucc(SNodeListTy *SnSuccs);
  
  // If the number of successors is 1, returns true
  // and updates *Succ1 with the unique successor.
  bool hasOneSucc(SNode **Succ1);
  
  // If the number of predecessors is 1, returns true
  // and updates *Pred1 with the unique predecessor.
  bool hasOnePred(SNode **Pred1);

  // Release the memory after the SNode is no longer used.
  void releaseMemory() {
    Pred.clear();
    Succ.clear();
    Children.clear();
  };

  // The current SNode (this) is a newly created root SNode.
  // It inherits its preds from Child1 by moving Child1's preds 
  // to the current SNode, mapping each pred to its parent to 
  // properly handle preds that are now children of this.
  // It also inherits its succs from Child2 by moving Child2's 
  // succs to the current SNode, mapping each succ to its parent 
  // to properly handle succs that are now children of this.
  //
  void inheritPredsNSuccs(SNode *Child1, SNode *Child2) {
    assert(Pred.empty() && Succ.empty() &&
           "Pred and Succ are expected to be empty");

    for (snode_list_iterator I = Child1->snPredBegin(),
                               E = Child1->snPredEnd();
         I != E; ++I) {
      SNode *Sn = *I;
      addPred(Sn->getParentSn());
    }

    for (snode_list_iterator I = Child2->snSuccBegin(),
                               E = Child2->snSuccEnd();
         I != E; ++I) {
      SNode *Sn = *I;
      addSucc(Sn->getParentSn());
    }

    for (snode_list_iterator I = Child1->snPredBegin(),
                               E = Child1->snPredEnd();
         I != E; ++I) {
      SNode *Sn = *I;
      if (Sn->getParentSn() != getParentSn()) 
        Sn->getParentSn()->replaceSucc(Child1, getParentSn());
    }

    for (snode_list_iterator I = Child2->snSuccBegin(),
                               E = Child2->snSuccEnd();
         I != E; ++I) {
      SNode *Sn = *I;
      if (Sn->getParentSn() != getParentSn()) 
        Sn->getParentSn()->replacePred(Child2, getParentSn());
    }

    Child1->clearPred();
    Child2->clearSucc();
  }
 
  // After the SNode A is placed in the new SNode B, for every
  // successor SNode D of SNode A,  the SNode A needs 
  // to be removed from the predecessor set of SNode D.
  void deletePredsFromSucc(SNode *Node) {
    for (snode_list_iterator I = snSuccBegin(), E = snSuccEnd(); I != E;
         ++I) {
      SNode *Sn = *I;
      Sn->removePred(Node);
    }
  }
  
public:
  SNode(const SNode &) = delete;
  void operator=(const SNode &) = delete;
  virtual ~SNode(){};
  explicit SNode(SNodeOp OP) {
    Op = OP;
    Lp = nullptr;
    Parent = nullptr;
  };

  // Returns the true target basic block for BlockSNode. This information 
  // is only valid for blockSNode and orSNode.
  virtual const SNode *getTrueTarget() const = 0;

  // Returns the false target basic block for BlockSNode. This information 
  // is only valid for blockSNode and orSNode.
  virtual const SNode *getFalseTarget() const = 0;

  // Sets the id for the SNode
  void setSNodeNum(unsigned int Num) { SNodeNum = Num; }

  // Returns the id of the SNode
  unsigned int getSNodeNum() const { return SNodeNum; }

  // Returns the type of the SNode
  SNodeOp getOp() const { return Op; }
  
  // Sets the SNode's type
  void setOp(SNodeOp OP) { Op = OP; }
  
  // Sets the parent of the SNode
  void setParentSn(SNode *Par) { Parent = Par; }
  
  // Returns the parent of the SNode
  SNode *getParentSn() const { return Parent; }
  
  // Sets the SNode's loop.
  void setLoop(Loop *L) { Lp = L; }

  // Sets the SNode's loop by copying the loop from SNode From.
  void setLoop(SNode *From);
  
  // Returns the SNode's loop if exists
  Loop *getLoop() const { return Lp; }

  // Adds the Node as one of its children
  void addChild(SNode *Node) { Children.push_back(Node); }

  // Returns the number of predecessors
  unsigned int snPredSize() const { return Pred.size(); }
  
  // Returns the number of successors
  unsigned int snSuccSize() const { return Succ.size(); }
  
  // Adds the Node as the last predecessor
  void addPred(SNode *Node) { 
    Pred.push_back(Node);
  }
  
  // Adds the Node as the last successor
  void addSucc(SNode *Node) {
    Succ.push_back(Node);
  }

  // Replaces the SNode Old with the SNode New in its succs
  void replaceSucc(SNode *Old, SNode *New) {
    snode_list_iterator I, E;
    I = snSuccBegin();
    E = snSuccEnd();
    for (; I != E; ++I) {
      if (Old == *I) {
        Succ.insert(I, New);
        Succ.erase(I);
        return;
      }
    }
    llvm_unreachable("Old succ is expected to be found");
  }

  // Replaces the SNode Old with the SNode New in its preds
  void replacePred(SNode *Old, SNode *New) {
    snode_list_iterator I, E;
    I = snPredBegin();
    E = snPredEnd();
    for (; I != E; ++I) {
      if (Old == *I) {
        Pred.insert(I, New);
        Pred.erase(I);
        return;
      }
    }
    llvm_unreachable("Old pred is expected to be found");
  }
  
  // Returns true if the children set is empty. Otherwise returns false.
  bool isChildrenEmpty() const { return Children.empty(); }

  snode_children_iterator childBegin() { return Children.begin(); }
  snode_children_iterator childEnd() { return Children.end(); }
  const_snode_children_iterator childCBegin() const { return Children.begin(); }
  const_snode_children_iterator childCEnd() const { return Children.end(); }
  
  // Returns the number of children.
  unsigned int numChildren() const { return Children.size(); }
  
  // Returns the Nth children
  const SNode *getChild(int II) const {
    int Count = 0;
    for (const_snode_children_iterator I = Children.begin(), 
         E = Children.end(); I != E;
         ++I, ++Count) {
      if (Count == II) {
        return &*I;
      }
    }
    llvm_unreachable("invalid children count");
  }
  
  // Clears the pred set
  void clearPred() { Pred.clear(); }
  
  // Clears the succ set
  void clearSucc() { Succ.clear(); }
  
  // Checks whether Node is one of the predecessors
  bool hasPred(SNode *Node) const { 
    for (const_snode_list_iterator I = Pred.begin(), E = Pred.end(); I != E;
         ++I) {
      if (Node == *I) {
        return true;
      }
    }
    return false;
  }
  
  // Checks whether Node is one of the successors
  bool hasSucc(SNode *Node) const {
    for (const_snode_list_iterator I = Succ.begin(), E = Succ.end(); I != E;
         ++I) {
      if (Node == *I) {
        return true;
      }
    }
    return false;
  }
  
  // Returns true if predecessor set is empty
  bool predIsEmpty() const { return Pred.empty(); }
  
  // Returns true if successor set is empty
  bool succIsEmpty() const { return Succ.empty(); }
  
  snode_list_iterator snPredBegin() { return Pred.begin(); }
  snode_list_iterator snPredEnd() { return Pred.end(); }
  snode_list_iterator snSuccBegin() { return Succ.begin(); }
  snode_list_iterator snSuccEnd() { return Succ.end(); }

  const_snode_list_iterator snPredCBegin() const { return Pred.cbegin(); }
  const_snode_list_iterator snPredCEnd() const { return Pred.cend(); }
  const_snode_list_iterator snSuccCBegin() const { return Succ.cbegin(); }
  const_snode_list_iterator snSuccCEnd() const { return Succ.cend(); }

  // Returns the first child SNode
  const SNode *getFirstChild() const { return &Children.front(); }
  
  // Returns the last child SNode
  const SNode *getLastChild() const { return &Children.back(); }

  // Adds Node as the first child
  void addAsFirstChild(SNode *Node) {
    Children.push_front(Node);
  }
  
  // Deletes the last child
  void removeLastChild() { Children.pop_back(); }
  
  // Deletes Node from the predecessor set
  void removePred(SNode *Node) {
    Pred.remove(Node);
  }
  
  // Deletes Node from the successor set
  void removeSucc(SNode *Node) {
    Succ.remove(Node);
  }
  
  // Returns the first basic block of the SNode
  BasicBlock *getFirstBlock() const;
 
  // Returns the last basic block of the last child SNode. 
  // The significance of this block varies depending on the SNode type.
  BasicBlock *getLastBlock() const;

  // Returns the first pred SNode
  const SNode* getFirstPred() const { return Pred.front(); }

  // Returns the last pred SNode
  const SNode* getLastPred() const { return Pred.back(); }

  // Returns the first succ SNode
  const SNode* getFirstSucc() const { return Succ.front(); }

  // Returns the last succ SNode
  const SNode* getLastSucc() const { return Succ.back(); }

};


// The BlockSNode is a counter part of basic block. It is a 
// derived class of SNode since the BlockSNode needs to 
// have predecessors and successors.
class BlockSNode : public SNode {
private:
  BasicBlock *B;

public:
  BlockSNode() : SNode(SN_BLOCK) { B = nullptr; };
  ~BlockSNode(){};
  
  // Sets the basic block 
  void setBblock(BasicBlock *BB) { B = BB; }
  
  // Returns the basic block
  BasicBlock *getBblock() const { return B; }

  // Returns the true target SNode.
  const SNode *getTrueTarget() const;

  // Returns the false target basic block
  const SNode *getFalseTarget() const;
};

// The ListSNode is composed of a sequence of child SNodes X_1,
// X_2, ..., X_n. For each child SNode X_i, the succ of X_i is 
// X_i+1 and the pred of X_i+1 is X_i.
// Similarly, it is a derived class of SNode since the 
// ListSNode needs to have predecessors and successors.
//
class ListSNode : public SNode {
public:
  ListSNode() : SNode(SN_LIST){};
  ~ListSNode(){};

  // The TrueTarget and FalseTarget flag can only be applied to the 
  // SNodes such as BlockSNode and OrSNode which has two outgoing
  // edges. There is only one outgoing edge for ListSNode.
  const SNode *getTrueTarget() const { return nullptr; }
  const SNode *getFalseTarget() const { return nullptr; }
};


// The IfThenElseSNode is composed of three SNodes
// in the form of if-then-else. Similarly, it is a derived 
// class of SNode since the IfThenElseSNode
// needs to have predecessors and successors
class IfThenElseSNode : public SNode {
public:
  IfThenElseSNode() : SNode(SN_IF_THEN_ELSE){};
  ~IfThenElseSNode(){};
  
  // Returns the SNode where the if condition stays
  const SNode *getIfCond() const { return getChild(0); }
  
  // Returns the SNode where the then body stays
  const SNode *getThenBody() const { return getChild(1); }
  
  // Returns the SNode where the else body stays
  const SNode *getElseBody() const { return getChild(2); }

  // The TrueTarget and FalseTarget flag can only be applied to the 
  // SNodes such as BlockSNode and OrSNode which has two outgoing
  // edges. There is only one outgoing edge for IfThenElseSNode.
  const SNode *getTrueTarget() const { return nullptr; }
  const SNode *getFalseTarget() const { return nullptr; }
};

// The class SNodeAnalysis is the main driver to generate
// the hierarchical control flow graph based on the 
// regular control flow graph.
class SNodeAnalysis : public FunctionPass {
private:
  // the LoopInfo
  LoopInfo *LI;
  
  // the Function
  Function *F;
  
  // the map table between the basic block and the SNode
  BB2SNodeMapTy BB2SNodeMap;
  
  // the map table between the loop and the LoopSNode
  Loop2SNodeMapTy Loop2SNodeMap;

  // container of all the allocated SNodes.
  SNodeVectorTy SNodes;
  
  // the first SNode after hierarchical cfg is formed
  SNode *EntrySNode;
  
  void operator=(const SNodeAnalysis &) = delete;
  SNodeAnalysis(const SNodeAnalysis &) = delete;
  
  // Prints the indents
  void printIndent(int Indent, raw_ostream &OS) const;
  
  // Prints out the id of the SNode 
  void printSNodeNum(const SNode *Node, raw_ostream &OS) const;
  
  // Prints the succs' id of a SNode.
  void printSNodeListStructureForSucc(const SNode *Node, raw_ostream &OS) const;
  
  // Prints the preds'id of a SNode.
  void printSNodeListStructureForPred(const SNode *Node, raw_ostream &OS) const;
  
  // Prints the SNodes in hierarchical structure.
  void printSNodeStructure(const SNode *Node, int Level, raw_ostream &OS) const;
  
  
  // Prints the basic block name.
  void printBlockName(const BasicBlock *BB, raw_ostream &OS) const;
  
  SNode *genSNode(SNode::SNodeOp Op);
  
  // Creates the SNode graph based on the basic block's CFG.
  void makeSNodeGraph(BasicBlock *Block);
  
  // Returns true if the last SNode is sn_block or sn_or.
  bool TailSnIsSnblockOrSnor(SNode *Node);

  // Given SNode NodeA, checks whether it has unique successor SNode 
  // which can form a list SNode with itself or not. If so, *NodeB is 
  // updated as this successor SNode and returns true. Otherwise, 
  // returns false
  bool isSNodeList(SNode *NodeA, SNode **NodeB);

  // Given SNode NodeA and NodeB which is the unique successor of NodeA, 
  // generates a list SNode whose children are in the order of NodeA 
  // and NodeB.
  // The preds and succs of new list SNode will be updated 
  // according to the preds and succs of NodeA and NodeB.
  // The succ of NodeA needs to be cleared. Same for NodeB's pred.
  // TODO: If NodeB is list SNode, it should only add its children 
  // into the new list SNode.
  SNode *genSNodeList(SNode *NodeA, SNode *NodeB);

  // Given SNode Node, looks for its two successors to 
  // see whether Node and its two successors can form an if-then-else 
  // control flow. If so, returns true and updates *SnThen and 
  // *SnElse with its two succssors. Otherwise returns false.
  bool isSNodeIfThenElse(SNode *Node, SNode **SnThen, SNode **SnElse);
 
  // Given three SNodes SnIf, SnThen and SnElse, generates a sn_if_then_else 
  // whose children are in the order of SnIf, SnThen and SnElse. 
  // The preds and succs of new if-then-else SNode will be updated 
  // according to that of SnIf, SnThen and SnElse.
  SNode *genSNodeIfThenElse(SNode *SnIf, SNode *SnThen, SNode *SnElse);
  
  // Returns a new sn_block for the given basic block.
  SNode *makeSnBlock(BasicBlock *Block);
  
  // Genereates the SNode block for every basic block and updates
  // the preds and succs based on the CFG.
  void createSNodeBlocks();
  
  // This is the kernel function of the SNodeInfo analysis. The basic idea
  // is to do patten match, generate the structured SNode and keep
  void doRoutineLevelSNodeAnalyses();

  // Updates the preds of the sn_block according to its 
  // basic block's preds.
  void makeSNodePredList(BlockSNode *Snode);
  
  // Updates the succs of the sn_block according to its 
  // basic block's succs.
  void makeSNodeSuccList(BlockSNode *Snode);
  
  // Returns the string name for the SNode type
  static std::string snOpName(const SNode::SNodeOp OpIndex) {
    static std::string SnOpTable[20] = {
      "SN_BLOCK",          
      "SN_LIST",         
      "SN_IF_THEN_ELSE",
    };

    return SnOpTable[OpIndex];
  }
  
public:
  static char ID;
  SNodeAnalysis();
  ~SNodeAnalysis() {};
  void releaseMemory() override;
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const;
  
  // Returns the corresponding SNode for the given basic block.
  BlockSNode *getSNodeForBlock(BasicBlock *Block) const;

  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void printSeq(raw_ostream &OS, const Module *M) const;

  // Prints the SNodes in hierarchical structure recursively.
  void dumpSNodeStructure(SNode *Snode, raw_ostream &OS,
                                       SmallPtrSetImpl<SNode *> &Visited);
  // Prints out single SNode information.
  void dumpSingleSNode(const SNode *Snode, raw_ostream &OS) const;
    
  SNode *getEntrySNode() const { return EntrySNode; };
  const_snode_vector_iterator begin() const { return SNodes.begin(); }
  const_snode_vector_iterator end() const { return SNodes.end(); }

};

FunctionPass *createSNodeAnalysisPass();

// the main driver to print out the snode graph hierarchically.
}

#endif // LLVM_ANALYSIS_INTEL_SNODE_H
