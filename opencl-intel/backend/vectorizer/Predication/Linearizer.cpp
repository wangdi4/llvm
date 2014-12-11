/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "Linearizer.h"
#include "Logger.h"

#include <algorithm>

namespace intel {

/// @brief Deleter helper class used by the for-each loop
template<typename T > struct deleter {
  void operator()(T element) const {
    delete element;
  }
};

SchedulingScope::SchedulingScope(BasicBlock* leader, bool isUCFScope):
  m_blocks(), m_subscope(), m_leader(leader), m_isUCFScope(isUCFScope) {}

  SchedulingScope::~SchedulingScope() {
    // delete all sub-scopes
    std::for_each(m_subscope.begin(), m_subscope.end(),
                  deleter<SchedulingScope*>());
  }

void SchedulingScope::addBasicBlock(BasicBlock* bb) {
  V_ASSERT(bb && "Bad BB");
  m_blocks.push_back(bb);
}

bool SchedulingScope::has(BasicBlock* bb) {
  V_ASSERT(bb && "Bad BB");
  V_ASSERT(m_blocks.size()>0 && "This scope is empty");
  return (std::find(m_blocks.begin(), m_blocks.end(), bb) != m_blocks.end());
}

bool SchedulingScope::isSubsetOf(SchedulingScope* scp) {
  V_ASSERT(scp && "bad scope");
  V_ASSERT(m_blocks.size()>0 && "This scope is empty");
  for (BBVectorIter it = m_blocks.begin(), e = m_blocks.end(); it != e; ++it) {
    if (!scp->has(*it)) return false;
  }
  return true;
}

void SchedulingScope::addSubSchedulingScope(SchedulingScope* scp) {
  V_ASSERT(scp && "bad scope");
  V_ASSERT(scp != this && "Adding self");
  V_ASSERT(scp->isSubsetOf(this) && "adding a non sub-scope");
  m_subscope.push_back(scp);
  compress();
  verify();
}

void SchedulingScope::print(raw_ostream &OS, unsigned indent) {
  for (unsigned i=0; i<indent; i++) OS<<"\t";
  if (m_leader) {
    OS<<"SchedulingScope "<<this<<"("<<m_blocks.size()<<")["
      <<m_leader->getName()<<"]:\n";
  } else {
    OS<<"SchedulingScope "<<this<<"("<<m_blocks.size()<<"):\n";
  }
  for(BBVectorIter it = m_blocks.begin(), e=m_blocks.end() ; it != e; ++it) {
    for (unsigned i=0; i<indent; i++) OS<<"\t";
    OS<<(*it)->getName()<<"\n";
  }

  for (unsigned i=0; i<indent; i++) OS<<"\t";
  OS<<"Sub scopes ("<<m_subscope.size()<<"):\n";
  for (SchedulingScopeSetIter it = m_subscope.begin(), e = m_subscope.end();
       it != e; ++it) {
    (*it)->print(OS, indent+1);
  }
}

void SchedulingScope::verify() {

  for (SchedulingScopeSetIter it = m_subscope.begin(), e = m_subscope.end();
       it != e; ++it) {
    V_ASSERT((*it)->isSubsetOf(this)&& "Not all scopes are subset of this scope");
    (*it)->verify();
  }
}

bool SchedulingScope::hasUnscheduledPreds(const BBVector& schedule,
                                          const BBVector& thisScopeOnly, BasicBlock* bb) {
  V_ASSERT(bb && "invalid bb");

  // This branch guarantees that the linearizer will not get into an infinite loop in case there is a constraint
  // containing a loop header without its latch node.
  if (std::find(schedule.begin(), schedule.end(), bb) != schedule.end())
    return false;

  pred_iterator pred = pred_begin(bb);
  pred_iterator pred_e = pred_end(bb);
  for( ; pred != pred_e ; ++pred) {
    // Ignore unscheduled predecessors which are inside the same UCF scope
    if(m_isUCFScope && std::find(thisScopeOnly.begin(), thisScopeOnly.end(), *pred) != thisScopeOnly.end()) {
      continue;
    }

    if (std::find(schedule.begin(), schedule.end(), *pred) == schedule.end()) {
      return true;
    }
  }
  return false;
}

bool SchedulingScope::hasUnscheduledPreds(const BBVector& schedule) {

  // This branch guarantees that the linearizer will not get into an infinite loop in case there is a constraint
  // containing a loop header without its latch node.
  if (std::find(schedule.begin(), schedule.end(), m_blocks.front()) != schedule.end())
    return false;

  // For each of our instructions
  for (BBVectorIter BB = m_blocks.begin(), BBE = m_blocks.end(); BB != BBE ; ++BB){
    // for each of the preds for the BB
    pred_iterator pred = pred_begin(*BB);
    pred_iterator pred_e = pred_end(*BB);
    for( ; pred != pred_e ; ++pred) {
      // ignore deps within this scope
      if (std::find(m_blocks.begin(), m_blocks.end(), *pred) != m_blocks.end()){
        continue;
      }

      // if unable to find pred within scheduled instructions,
      // then has unscheduled instruction
      if (std::find(schedule.begin(), schedule.end(), *pred) == schedule.end()){
        return true;
      }
    }
  }

  return false;
}

void SchedulingScope::schedule(BBVector& schedule) {
  // Create a work-list of blocks to schedule
  SchedulingScopeSet unscheduled(m_subscope.begin(), m_subscope.end());

  // schedule all BBs which are free and are not a part of any
  // other scheduling constraint scope.
  BBVector thisScopeOnly;
  getNonSchedulingScopedInstructions(thisScopeOnly);

  // first we need to schedule the leader of each scope
  // the leader is used to prevent cycles (in loops).
  // Not all Scheduling scopes have leaders
  if (m_leader) {
    if (std::find(schedule.begin(), schedule.end(), m_leader) ==
        schedule.end()) {
      schedule.push_back(m_leader);
    }
    m_leader = NULL;
  }

  // as long as we have unscented instructions
  while (unscheduled.size() || thisScopeOnly.size()) {
    // schedule free basic blocks
    if (thisScopeOnly.size()) {
      // for each BB in the thisScopeOnly
      BasicBlock* toSched = NULL;
      for(BBVectorIter it = thisScopeOnly.begin(),
          ite=thisScopeOnly.end(); it != ite; ++it) {
        if (!hasUnscheduledPreds(schedule, thisScopeOnly, *it)) {
          toSched = *it;
          break;
        }
      }
      if (toSched) {
        // schedule this instruction
        thisScopeOnly.erase(std::find(thisScopeOnly.begin(), thisScopeOnly.end(), toSched));
        if (std::find(schedule.begin(), schedule.end(), toSched) == schedule.end()) {
          schedule.push_back(toSched);
        }
      }
    }//BB sched

    // schedule ready scopes
    if (unscheduled.size()) {
      SchedulingScope* ready = NULL;
      for (SchedulingScopeSetIter scopeIt = unscheduled.begin(),
           e = unscheduled.end(); scopeIt != e; ++scopeIt) {
        // if found candidate for scheduling
        if (!(*scopeIt)->hasUnscheduledPreds(schedule)) {
          ready = *scopeIt;
          break;
        }
      }
      // found a scope to schedule
      if (ready) {
        // perform scheduling and remove from worklist
        ready->schedule(schedule);
        unscheduled.erase(std::find(unscheduled.begin(), unscheduled.end(), ready));
      }
    }//scopes
  }//while
}

BasicBlock* SchedulingScope::getFirstBlockAfter(const BBVector& schedule) {
  unsigned int last = schedule.size();
  // for all blocks in schedule
  for (unsigned int i=0; i<schedule.size(); ++i) {
    // if this block is in the scope
    if (std::find(m_blocks.begin(), m_blocks.end(), schedule.at(i)) != m_blocks.end()) {
      last = i;
    }
  }
  if ((last+1) < schedule.size()) return schedule.at(last+1);
  return NULL;
}

void SchedulingScope::compress() {

  bool changed = true;

  while(changed) {
    changed = false;
    SchedulingScope* scpToRemove = NULL;
    SchedulingScope* scpToAdd = NULL;
    // Check if the scope of interest is within
    // another existing scope
    for (SchedulingScopeSetIter scp = m_subscope.begin(),
         scp_e = m_subscope.end(); scp != scp_e; ++scp) {
      for (SchedulingScopeSetIter it = m_subscope.begin(),
           e = m_subscope.end(); it != e; ++it) {
        // if scope is not self and it is a subset of another scope
        if (*scp != *it && (*scp)->isSubsetOf(*it)) {
            scpToRemove = *scp;
            scpToAdd = *it;
            changed = true;
            break;
          }
      }//for
    }//for
    if (scpToRemove) {
      V_ASSERT(scpToRemove != scpToAdd);
      m_subscope.erase(std::find(m_subscope.begin(), m_subscope.end(), scpToRemove));
      (scpToAdd)->addSubSchedulingScope(scpToRemove);
    }
  }// changed

}


void SchedulingScope::getNonSchedulingScopedInstructions(
  SchedulingScope::BBVector &thisScopeOnly) {

  // for each instruction we have
  for (BBVectorIter BB = m_blocks.begin(),
       BBE = m_blocks.end(); BB != BBE ; ++BB) {
    bool scoped = false;
    // for each block
    for (SchedulingScopeSetIter it = m_subscope.begin(),
         e = m_subscope.end(); it != e; ++it) {

      if ((*it)->has(*BB)) {
        scoped = true;
        break;
      }
    }
    if (!scoped) thisScopeOnly.push_back(*BB);
  }
}


} //namespace

