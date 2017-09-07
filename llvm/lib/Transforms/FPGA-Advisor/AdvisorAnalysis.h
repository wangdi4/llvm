#include "AdvisorCommon.h"

// LLVM
#include "llvm/IR/InstVisitor.h"

// boost
#include <boost/graph/adjacency_list.hpp>

// tbb
#include "tbb/concurrent_queue.h"
#include "tbb/task.h"
#include "tbb/task_group.h"
#include "tbb/task_scheduler_init.h"

// Std Lib
#include <list>
#include <map>

namespace llvm {
class BasicBlock;
class Function;
class Instruction;
}

namespace fpga {

class AdvisorAnalysis;
class ModuleScheduler;
class ModuleAreaEstimator;

// TraceGraph vertex property struct representing each individual
// scheduling element (basic block granularity)
class BBSchedElem {
private:
  // cycStart and cycEnd are the actual schedules
  std::vector<int> mutable cycStart;
  std::vector<int> mutable cycEnd;

public:
  void setMinStart(int _start) const { minCycStart = _start; }
  void setMinEnd(int _end) const { minCycEnd = _end; }
  void setStart(int _start, int tid) const { cycStart[tid] = _start; }
  void setEnd(int _end, int tid) const { cycEnd[tid] = _end; }
  int getMinStart() const { return minCycStart; }
  int getMinEnd() const { return minCycEnd; }
  int getStart(int tid) const { return cycStart[tid]; }
  int getEnd(int tid) const { return cycEnd[tid]; }

  BBSchedElem();

  Function *function;
  BasicBlock *basicblock;
  uint64_t ID;
  // the min cycles represent the earliest the basic block may
  // execute, this equates to the scheduling without resource
  // constraint
  int mutable minCycStart;
  int mutable minCycEnd;
  int64_t cpuCycles;
  std::string name;
  // a memory access tuple for each store/load
  // first field stores the starting address, second field stores the width of
  // access in bytes
  std::vector<std::pair<uint64_t, uint64_t>> memoryWriteTuples;
  std::vector<std::pair<uint64_t, uint64_t>> memoryReadTuples;
};

// TraceGraph edge weight property representing transition delay
// between fpga and cpu
typedef boost::property<boost::edge_weight_t, unsigned> TransitionDelay;
// Graph type:
// STL list container for OutEdge List
// STL vector container for vertices
// Use directed edges
// typedef boost::adjacency_list< boost::listS, boost::vecS, boost::directedS >
// digraph;
// trace graph property
// typedef boost::property<boost::vertex_index_t, BBSchedElem> VertexProperty;
// typedef boost::adjacency_list< boost::listS, boost::vecS, boost::directedS,
// VertexProperty > TraceGraph;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS,
                              BBSchedElem, TransitionDelay>
    TraceGraph;
// typedef boost::adjacency_list< boost::listS, boost::vecS, boost::directedS,
// BBSchedElem > TraceGraph;
typedef std::list<TraceGraph> TraceGraphList;
typedef std::map<Function *, TraceGraphList> ExecGraph;

// ExecutionOrder map definition
typedef std::map<BasicBlock *,
                 std::pair<int, std::vector<TraceGraph::vertex_descriptor>>>
    ExecutionOrder;
typedef std::list<ExecutionOrder> ExecutionOrderList;
typedef std::map<Function *, ExecutionOrderList> ExecutionOrderListMap;

// Unconstrained scheduler -- does not account for resource limitations in
// scheduling
class ScheduleVisitor : public boost::default_dfs_visitor {
public:
  TraceGraphList::iterator graph_ref;
  AdvisorAnalysis *parent;
  std::map<BasicBlock *, LatencyStruct> &LT;
  int mutable lastCycle;
  int *lastCycle_ref;
  int tid;

  ScheduleVisitor(TraceGraphList::iterator graph, AdvisorAnalysis *_parent,
                  std::map<BasicBlock *, LatencyStruct> &_LT, int &lastCycle,
                  int _tid)
      : graph_ref(graph), parent(_parent), LT(_LT), lastCycle_ref(&lastCycle),
        tid(_tid) {}

  void discover_vertex(TraceGraph::vertex_descriptor v,
                       const TraceGraph &graph);

}; // end class ScheduleVisitor

class ConstrainedScheduleVisitor : public boost::default_bfs_visitor {
public:
  TraceGraphList::iterator graph_ref;
  std::map<BasicBlock *, LatencyStruct> &LT;
  int mutable lastCycle;
  int64_t *lastCycle_ref;
  int64_t *cpuCycle_ref;
  std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable;
  int tid;

  ConstrainedScheduleVisitor(
      TraceGraphList::iterator graph,
      std::map<BasicBlock *, LatencyStruct> &_LT, int64_t &lastCycle,
      int64_t &cpuCycle,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *_resourceTable,
      int _tid)
      : graph_ref(graph), LT(_LT), lastCycle_ref(&lastCycle),
        cpuCycle_ref(&cpuCycle), resourceTable(_resourceTable), tid(_tid) {}

  void discover_vertex(TraceGraph::vertex_descriptor v,
                       const TraceGraph &graph);

}; // end class ConstrainedScheduleVisitor

class AdvisorAnalysis : public ModulePass, public InstVisitor<AdvisorAnalysis> {
  // node to keep track of where to record trace information
  typedef struct {
    Function *function;
    TraceGraphList::iterator graph;
    TraceGraph::vertex_descriptor vertex;
    ExecutionOrderList::iterator executionOrder;
  } FunctionExecutionRecord;

  typedef struct {
    int blockCount;
    uint64_t latencyStart;
    uint64_t latencyFinish;
    double grad;
  } GradientPoint;

  typedef struct {
    std::vector<GradientPoint> gradientPoints;
    double gradientCoefficient;
  } Gradient;

public:
  static char ID;
  const int THREADS = 16;
  const bool useThreading = true;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<AAResultsWrapperPass>();
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    // AU.addRequired<DependenceGraph>();
    AU.addRequired<ModuleScheduler>();
    AU.addRequired<ModuleAreaEstimator>();
    AU.setPreservesAll();
  }
  AdvisorAnalysis();
  ~AdvisorAnalysis() noexcept(true) {}
  bool runOnModule(Module &M) override;

  void visitFunction(Function &F);
  void visitBasicBlock(BasicBlock &BB);
  void visitInstruction(Instruction &I);

  int get_basic_block_instance_count(BasicBlock *BB);
  void set_basic_block_instance_count(BasicBlock *BB, int value);
  void adjust_all_thread_pool_resource_tables(BasicBlock *BB, int value);
  void set_all_thread_pool_basic_block_instance_counts(BasicBlock *BB,
                                                       int value);
  void set_thread_pool_basic_block_instance_count(BasicBlock *BB, int value);
  int get_thread_pool_basic_block_instance_count(BasicBlock *BB);
  void handle_basic_block_gradient(
      BasicBlock *BB, std::unordered_map<BasicBlock *, double> *gradient,
      int initialLatency, int initialArea);

  // Create a TBB queue to give thread ids to the TBB threads.
  boost::lockfree::stack<int> tidPool;

  // state for each thread. We index these by basic block, such that we might
  // have a thread per basic block.
  std::unordered_map<BasicBlock *, int> bbInstanceCounts;
  std::unordered_map<BasicBlock *, std::unordered_map<BasicBlock *, int> *>
      threadPoolInstanceCounts;
  std::unordered_map<BasicBlock *,
                     std::unordered_map<BasicBlock *, std::vector<unsigned>> *>
      threadPoolResourceTables;

private:
  // thread pool for gradient.
  tbb::task_scheduler_init init;
  tbb::task_group group;
  std::mutex threadPoolMutex;

  unsigned areaConstraint;
  std::vector<double> thresholds;

  // functions
  void find_recursive_functions(Module &M);
  void does_function_recurse(Function *func, CallGraphNode *CGN,
                             std::vector<Function *> &stack);
  void print_recursive_functions();
  bool run_on_function(Function *F);
  bool run_on_module(Module &M);
  bool has_unsynthesizable_construct(Function *F);
  bool is_recursive_function(Function *F);
  bool has_recursive_call(Function *F);
  bool does_function_call_recursive_function(CallGraphNode *CGN);
  bool has_external_call(Function *F);
  bool does_function_call_external_function(CallGraphNode *CGN);
  // void instrument_function(Function *F);
  // void instrument_basicblock(BasicBlock *BB);

  void print_statistics();

  bool get_program_trace(std::string fileIn);
  bool process_time(const std::string &line,
                    TraceGraphList::iterator lastTraceGraph,
                    TraceGraph::vertex_descriptor lastVertex, bool start);
  bool
  process_function_return(const std::string &line, Function **function,
                          std::stack<FunctionExecutionRecord> &stack,
                          TraceGraphList::iterator &lastTraceGraph,
                          TraceGraph::vertex_descriptor &lastVertex,
                          ExecutionOrderList::iterator &lastExecutionOrder);
  bool process_load(const std::string &line, Function *function,
                    TraceGraphList::iterator lastTraceGraph,
                    TraceGraph::vertex_descriptor lastVertex);
  bool process_store(const std::string &line, Function *function,
                     TraceGraphList::iterator lastTraceGraph,
                     TraceGraph::vertex_descriptor lastVertex);
  bool
  process_basic_block_entry(const std::string &line, Function *latestFunction,
                            int &ID, TraceGraphList::iterator lastTraceGraph,
                            TraceGraph::vertex_descriptor &lastVertex,
                            ExecutionOrderList::iterator lastExecutionOrder);
  bool
  process_function_entry(const std::string &line, Function **function,
                         TraceGraphList::iterator &latestTraceGraph,
                         TraceGraph::vertex_descriptor &latestVertex,
                         ExecutionOrderList::iterator &latestExecutinoOrder,
                         std::stack<FunctionExecutionRecord> &stack);
  void getCPULatencyTable(Function *F,
                          std::map<BasicBlock *, LatencyStruct> *LT,
                          ExecutionOrderList &executionOrderList,
                          TraceGraphList &executionGraphList);
  void getGlobalCPULatencyTable(Module &M,
                                std::map<BasicBlock *, LatencyStruct> *LT,
                                ExecutionOrder executionOrder,
                                TraceGraph executionGraph);

  bool check_trace_sanity();
  BasicBlock *find_basicblock_by_name(std::string funcName, std::string bbName);
  Function *find_function_by_name(std::string funcName);

  // functions that do analysis on trace
  bool find_maximal_configuration_for_all_calls(Function *F,
                                                unsigned &fpgaOnlyLatency,
                                                unsigned &fpgaOnlyArea);
  bool find_maximal_configuration_for_module(Module &M,
                                             unsigned &fpgaOnlyLatency,
                                             unsigned &fpgaOnlyArea);
  bool find_maximal_configuration_for_call(
      Function *F, TraceGraphList::iterator graph,
      ExecutionOrderList::iterator execOrder,
      std::vector<TraceGraph::vertex_descriptor> &rootVertices);
  bool find_maximal_configuration_global(
      TraceGraphList::iterator graph, ExecutionOrderList::iterator execOrder,
      std::vector<TraceGraph::vertex_descriptor> &rootVertices);
  bool prune_basic_block_configuration_to_device_area(Function *F);
  bool prune_basic_block_configuration_to_device_area_global(Module &M);
  int get_total_basic_block_instances(Function *F);
  int get_total_basic_block_instances_global(Module &M);

  // bool find_maximal_configuration_for_call(Function *F,
  // TraceGraphList_iterator graph_it, std::vector<TraceGraph_vertex_descriptor>
  // &rootVertices);
  bool basicblock_is_dependent(BasicBlock *child, BasicBlock *parent,
                               TraceGraph &graph);
  bool instruction_is_dependent(Instruction *inst1, Instruction *inst2);
  bool true_dependence_exists(Instruction *inst1, Instruction *inst2);
  bool basicblock_control_flow_dependent(BasicBlock *child, BasicBlock *parent,
                                         TraceGraph &graph);
  void find_new_parents(std::vector<TraceGraph::vertex_descriptor> &newParents,
                        TraceGraph::vertex_descriptor child,
                        TraceGraph::vertex_descriptor parent,
                        TraceGraph &graph);
  bool annotate_schedule_for_call(Function *F,
                                  TraceGraphList::iterator graph_it,
                                  int &lastCycle);
  bool find_maximal_resource_requirement(
      Function *F, TraceGraphList::iterator graph_it,
      std::vector<TraceGraph::vertex_descriptor> &rootVertices, int lastCycle);
  bool latest_parent(TraceGraph::out_edge_iterator edge,
                     TraceGraphList::iterator graph);
  void modify_resource_requirement(Function *F,
                                   TraceGraphList::iterator graph_it);
  void find_optimal_configuration_for_all_calls(Function *F,
                                                unsigned &cpuOnlyLatency,
                                                unsigned fpgaOnlyLatency,
                                                unsigned fpgaOnlyArea);
  void find_optimal_configuration_for_module(Module &M,
                                             unsigned &cpuOnlyLatency,
                                             unsigned fpgaOnlyLatency,
                                             unsigned fpgaOnlyArea);
  bool incremental_gradient_descent(
      Function *F, std::unordered_map<BasicBlock *, double> &gradient,
      std::unordered_map<BasicBlock *, int> &removeBBs, int64_t &deltaDelay,
      unsigned cpuOnlyLatency, unsigned fpgaOnlyLatency, unsigned fpgaOnlyArea,
      int64_t &initialLatency);
  bool incremental_gradient_descent_global(
      Module &M, std::unordered_map<BasicBlock *, double> &gradient,
      std::unordered_map<BasicBlock *, int> &removeBBs, int64_t &deltaDelay,
      unsigned cpuOnlyLatency, unsigned fpgaOnlyLatency, unsigned fpgaOnlyArea,
      int64_t &initialLatency);
  void initialize_basic_block_instance_count(Function *F);
  void initialize_basic_block_instance_count_global(Module &M);
  bool decrement_basic_block_instance_count(BasicBlock *BB);
  bool decrement_thread_pool_basic_block_instance_count(BasicBlock *BB);
  bool increment_basic_block_instance_count(BasicBlock *BB);
  bool increment_thread_pool_basic_block_instance_count(BasicBlock *BB);
  void update_transition(BasicBlock *BB);
  bool
  decrement_basic_block_instance_count_and_update_transition(BasicBlock *BB);
  bool decrease_basic_block_instance_count_and_update_transition(
      std::unordered_map<BasicBlock *, int> &removeBBs);
  bool
  increment_basic_block_instance_count_and_update_transition(BasicBlock *BB);
  void
  decrement_all_basic_block_instance_count_and_update_transition(Function *F);
  void decrement_all_basic_block_instance_count_and_update_transition_global(
      Module &M);
  void find_root_vertices(std::vector<TraceGraph::vertex_descriptor> &roots,
                          TraceGraphList::iterator graph_it);
  void dumpImplementationCounts(Function *F);
  void dumpBlockCounts(Function *F, unsigned cpuLatency);
  void dumpBlockCountsGlobal(unsigned cpuLatency);
  uint64_t schedule_with_resource_constraints(
      TraceGraphList::iterator graph_it, Function *F,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable,
      int tid);
  uint64_t schedule_with_resource_constraints_global(
      TraceGraphList::iterator graph_it,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable,
      int tid);
  uint64_t schedule_without_resource_constraints(
      TraceGraphList::iterator graph_it, Function *F,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable);
  uint64_t schedule_without_resource_constraints_global(
      TraceGraphList::iterator graph_it,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable);
  uint64_t schedule_cpu(TraceGraphList::iterator graph_it, Function *F);
  uint64_t schedule_cpu_global(TraceGraphList::iterator graph_it);
  void initialize_resource_table(
      Function *F,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable,
      bool cpuOnly);
  void initialize_resource_table_global(
      Module &M,
      std::unordered_map<BasicBlock *, std::vector<unsigned>> *resourceTable,
      bool cpuOnly);
  unsigned get_cpu_only_latency(Function *F);
  unsigned get_cpu_only_latency_global(Module &M);
  unsigned get_area_requirement(Function *F);
  unsigned get_area_requirement_global(Module &M);
  void update_transition_delay(TraceGraphList::iterator graph);
  unsigned get_transition_delay(BasicBlock *source, BasicBlock *target,
                                bool CPUToHW);
  void remove_redundant_dynamic_dependencies(
      TraceGraphList::iterator graph,
      std::vector<TraceGraph::vertex_descriptor> &dynamicDeps);
  void recursively_remove_redundant_dynamic_dependencies(
      TraceGraphList::iterator graph,
      std::vector<TraceGraph::vertex_descriptor> &dynamicDeps,
      std::vector<TraceGraph::vertex_descriptor>::iterator search,
      TraceGraph::vertex_descriptor v);

  bool dynamic_memory_dependence_exists(TraceGraph::vertex_descriptor child,
                                        TraceGraph::vertex_descriptor parent,
                                        TraceGraphList::iterator graph);
  bool memory_accesses_conflict(std::pair<uint64_t, uint64_t> &access1,
                                std::pair<uint64_t, uint64_t> &access2);

  void print_basic_block_configuration(Function *F, raw_ostream *out);
  void print_optimal_configuration_for_all_calls(Function *F);
  void print_execution_order(ExecutionOrderList::iterator execOrder);
  bool functionInTrace(Function *F) {
    std::unordered_set<Function *>::const_iterator funcIter =
        functionsSeen.find(F);
    return (funcIter != functionsSeen.end());
  }

  // dependence graph construction
  bool get_dependence_graph_from_file(std::string fileName, DepGraph **depGraph,
                                      std::string funcName, bool is_global);

  // define some data structures for collecting statistics
  std::vector<Function *> functionList;
  std::vector<Function *> recursiveFunctionList;
  // std::vector<std::pair<Loop *, bool> > loopList;

  // recursive and external functions are included
  std::unordered_map<Function *, FunctionInfo *> functionMap;

  Module *mod;
  CallGraph *callGraph;
  ExecutionOrderList::iterator globalExecutionOrder;
  TraceGraphList::iterator globalTraceGraph;
  std::unordered_set<Function *> functionsSeen;

  raw_ostream *outputLog;

  raw_ostream *outputFile;

  // exeuctionTrace contains the execution traces separated by function
  // the value for each key (function) is a vector, where each vector element
  // represents the basicblock execution of one call to that function
  // std::map<Function *, std::list<std::list<BBSchedElem> > > executionTrace;

  ExecGraph executionGraph;

  ExecutionOrderListMap executionOrderListMap;

  std::unordered_map<BasicBlock *, Gradient *> gradients;

  // DepGraph depGraph;

}; // end class AdvisorAnalysis

// TraceGraph custom vertex writer for execution trace graph output to dotfile
template <class TraceGraph> class TraceGraphVertexWriter {
  AdvisorAnalysis *parent;

public:
  TraceGraphVertexWriter(TraceGraph &_graph, AdvisorAnalysis *_parent)
      : parent(_parent), graph(_graph) {}
  template <class TraceGraph_vertex_descriptor>
  void operator()(std::ostream &out,
                  const TraceGraph_vertex_descriptor &v) const {
    /*
     out << "[shape = \"record\" label=\"<r0 fontcolor=Red> " <<
     graph[v].cycStart << "| <r1>"
     << graph[v].name << "| <r2>"
     << graph[v].cycEnd << "\"]";
     */
    out << "[shape=\"none\" label=<<table border=\"0\" cellspacing=\"0\">";
    out << "<tr><td bgcolor=\"#AEFDFD\" border=\"1\"> "
        << graph[v].getMinStart() << "</td></tr>";
    if (parent->get_basic_block_instance_count(graph[v].basicblock) > 0) {
      // if (get_basic_block_instance_count_meta(graph[v].basicblock) > 0) {
      out << "<tr><td bgcolor=\"#FFFF33\" border=\"1\"> " << graph[v].name
          << " (" << v << ")"
          << "</td></tr>";
    } else {
      out << "<tr><td bgcolor=\"#FFFFFF\" border=\"1\"> " << graph[v].name
          << " (" << v << ") "
          << "</td></tr>";
    }
    out << "<tr><td bgcolor=\"#AEFDFD\"  border=\"1\"> " << graph[v].getMinEnd()
        << "</td></tr>";
    out << "</table>>]";
  }

private:
  TraceGraph &graph;
}; // end class TraceGraphVertexWriter

template <class TraceGraph> class TraceGraphEdgeWriter {
public:
  TraceGraphEdgeWriter(TraceGraph &_graph) : graph(_graph) {}
  template <class TraceGraph_edge_descriptor>
  void operator()(std::ostream &out,
                  const TraceGraph_edge_descriptor &e) const {
    unsigned delay = boost::get(boost::edge_weight_t(), graph, e);
    if (delay > 0) {
      out << "[color=\"red\" penwidth=\"4\" label=\"" << delay << "\"]";
    }
  }

private:
  TraceGraph &graph;
}; // end class TraceGraphEdgeWriter

} // end namespace fpga
