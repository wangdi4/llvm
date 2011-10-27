#include "debug_server.h"
#include "debugservermessages.pb.h"
#include "debug_communicator.h"
#include "debuginfo_utils.h"
#include "cl_env.h"

// These can be defined as macros in earlier headers and can interfere with
// similarly named methods in LLVM headers
//
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "llvm/Analysis/DebugInfo.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Metadata.h"
#include "google/protobuf/text_format.h"
#include <string>
#include <deque>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sstream>

const unsigned DEBUG_SERVER_PORT_DEFAULT = 56203;
const uint64_t MAX_MEMORY_RANGE_SIZE = 16 * 1024;

using namespace std;
using namespace llvm;
using namespace debugservermessages;


void LOG_RECEIVED_MESSAGE(const ClientToServerMessage& msg)
{
    DEBUG_SERVER_LOG("Message received:");
    string s;
    google::protobuf::TextFormat::PrintToString(msg, &s);
    DEBUG_SERVER_LOG(s);
}


// Define the singleton instance
//
DebugServer DebugServer::instance;


// Used to collect the variable declarations visible up to a certain point in 
// a function.
//
struct FunctionStackFrame
{
    struct VarDeclInfo
    {
        VarDeclInfo(void* addr_, const MDNode* description_, bool is_global_ = false)
            : addr(addr_), description(description_), is_global(is_global_)
        {
        }

        void* addr;
        const MDNode* description;
        bool is_global;
    };

    // Vars declared so far in the frame. Not all are visible at the execution
    // point (due to lexical scopes).
    //
    vector<VarDeclInfo> vars;

    // Metadata describing the function
    //
    const MDNode* function_metadata;

    // Metadata for the line of code which invoked the function resulting
    // in this frame
    //
    const MDNode* calling_line_metadata;
};


// Variable descriptions sent to the debug client
//
struct VarDescription
{
    VarDescription(
        const string& name_ = "", const string& type_ = "", 
        const string value_ = "", uint64_t addr_ = 0,
        const VarTypeDescriptor& type_descriptor_ = VarTypeDescriptor())
        : name(name_), type(type_), value(value_), addr(addr_),
          type_descriptor(type_descriptor_)  
    {
    }

    string name;
    string type;
    string value;
    uint64_t addr;
    VarTypeDescriptor type_descriptor;
};

typedef map<string, VarDescription> VarsMapping;


// Merge the vars from 'src' into 'dest'. Only those vars that are not already
// in 'dest' are added.
//
void MergeVarsMapping(VarsMapping& dest, const VarsMapping& src)
{
    for (VarsMapping::const_iterator i = src.begin(); i != src.end(); ++i) {
        if (dest.find(i->first) == dest.end())
            dest[i->first] = i->second;
    }
}


// Dump a mapping for debugging
//
void dump_vars_mapping(VarsMapping mapp)
{
    for (VarsMapping::const_iterator i = mapp.begin(); i != mapp.end(); ++i) {
        cerr << i->first << " ";
    }
    cerr << endl;
}


struct DebugServer::DebugServerImpl 
{
    DebugServerImpl()
        :   m_comm(0), m_initialized(false), m_runningmode(RUNNING_NORMAL),
            m_debugged_global_id(3, 0),
            m_saved_stack_level(0), m_prev_stoppoint_line(0)
    {
    }
    
    ~DebugServerImpl()
    {
        if (m_comm) 
            delete m_comm;
    }

    // Build an error message for the client
    //
    ServerToClientMessage BuildErrorMessage(std::string err);
    
    // Register breakpoints taken from the client message
    //
    void RegisterBreakpoints(const ClientToServerMessage& run_msg);

    // Check if there's a breakpoint registered at the given location
    //
    bool HasBreakpointAt(const std::string& file, unsigned lineno) const;

    // Notify the client that a breakpoint at the given location was hit
    //
    void NotifyBreakpointHit(const std::string& file, unsigned lineno, const VarsMapping& vars);

    // Send a stack trace info message to the client
    //
    void SendStackTraceInfo();

    // Send a memory range info message to the client
    //
    void SendMemoryRangeInfo(const ClientToServerMessage& memory_range_msg);

    // Set and match global id
    //
    void SetDebuggedGlobalId(unsigned x, unsigned y, unsigned z);
    bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z);

    // Collect all the variables visible from the line in the given stack 
    // frame.
    //
    VarsMapping CollectVisibleVars(
        const MDNode* line_metadata, const FunctionStackFrame& stackframe);
    VarsMapping CollectGlobalVars(
        const FunctionStackFrame& stackframe);
    VarsMapping CollectVarsInScope(
        const DIScope& scope, const FunctionStackFrame& stackframe);
    VarsMapping CollectVarsInLexicalBlock(
        DILexicalBlock block, const FunctionStackFrame& stackframe);
    VarDescription CreateVarDescription(
        const FunctionStackFrame::VarDeclInfo& var_info);

    void DumpFunctionVars();

    enum RunningMode {
        RUNNING_STEP_IN,
        RUNNING_STEP_OVER,
        RUNNING_STEP_OUT,
        RUNNING_NORMAL
    };

    //
    // Data
    //
    DebugCommunicator* m_comm;
    bool m_initialized;
    RunningMode m_runningmode;

    // Holds all registered breakpoints
    // Note: using a map instead of a set here in case we 
    //
    typedef pair<string, unsigned> BreakpointInfo;
    typedef set<BreakpointInfo> BreakpointSet;
    BreakpointSet m_breakpoints;

    // 3-element vector. Holds the global ID
    //
    vector<unsigned> m_debugged_global_id;

    // The shadow frame stack
    //
    typedef deque<FunctionStackFrame> FunctionStack;
    FunctionStack m_stack;

    // State variable that remembers the frame stack level (depth) before 
    // invoking STEP_* commands. Used to correctly step to the right level.
    //
    size_t m_saved_stack_level;

    // State variable that remembers the previous line for which a stoppoint
    // was executed. 
    //
    const MDNode* m_prev_stoppoint_line;
};


ServerToClientMessage DebugServer::DebugServerImpl::BuildErrorMessage(string err)
{
    ServerToClientMessage msg;
    msg.set_type(ServerToClientMessage::CMD_ERROR);
    msg.mutable_cmd_error_msg()->set_description(err);
    return msg;
}


void DebugServer::DebugServerImpl::RegisterBreakpoints(const ClientToServerMessage& run_msg)
{
    assert(run_msg.type() == ClientToServerMessage::RUN);
    m_breakpoints.clear();
    for (int i = 0; i < run_msg.run_msg().breakpoints_size(); ++i) {
        const LineInfo& msg_lineinfo = run_msg.run_msg().breakpoints(i);
        m_breakpoints.insert(make_pair(msg_lineinfo.file(), msg_lineinfo.lineno()));
    }
    DEBUG_SERVER_LOG("Registered " + stringify(m_breakpoints.size()) + " breakpoints");
}


bool DebugServer::DebugServerImpl::HasBreakpointAt(const std::string& file, unsigned lineno) const
{
    BreakpointInfo bpinfo = make_pair(file, lineno);
    return m_breakpoints.find(bpinfo) != m_breakpoints.end();
}


void DebugServer::DebugServerImpl::NotifyBreakpointHit(const std::string& file, unsigned lineno, const VarsMapping& vars)
{
    ServerToClientMessage msg_to_client;
    msg_to_client.set_type(ServerToClientMessage::BP_HIT);
    msg_to_client.mutable_bphit_msg()->mutable_breakpoint()->set_file(file);
    msg_to_client.mutable_bphit_msg()->mutable_breakpoint()->set_lineno(lineno);

    for (VarsMapping::const_iterator ivar = vars.begin(); ivar != vars.end(); ++ivar) {
        ServerToClientMessage::VarInfo* var_info = msg_to_client.mutable_bphit_msg()->add_vars();
        var_info->set_name(ivar->second.name);
        var_info->set_value(ivar->second.value);
        var_info->set_type(ivar->second.type);
        var_info->set_address(ivar->second.addr);
        var_info->mutable_type_descriptor()->CopyFrom(ivar->second.type_descriptor); 
    }

    DEBUG_SERVER_LOG("Sending BP_HIT message");
    m_comm->sendMessage(msg_to_client);
}


void DebugServer::DebugServerImpl::SendStackTraceInfo()
{
    ServerToClientMessage msg_to_client;
    msg_to_client.set_type(ServerToClientMessage::STACK_TRACE_INFO);

    for (FunctionStack::iterator i = m_stack.begin(); i != m_stack.end(); ++i) {
        DISubprogram subprogram_descriptor(i->function_metadata);
        string function_name = subprogram_descriptor.getName().str();
        ServerToClientMessage::StackFrameInfo* frameinfo = 
            msg_to_client.mutable_stack_trace_info_msg()->add_frames();
        frameinfo->set_func_name(function_name);
        LineInfo* call_line_info = frameinfo->mutable_call_line();

        if (i->calling_line_metadata) {
            DILocation loc(i->calling_line_metadata);
            call_line_info->set_file(loc.getFilename());
            call_line_info->set_lineno(loc.getLineNumber());
        }
        else {
            call_line_info->set_file("<unknown>");
            call_line_info->set_lineno(0);
        }

        // For stack frames other than the first one, collect variable 
        // information visible at the point of call to the frame.
        // 'i' is the current stack frame. 'iprev' is the previous stack frame,
        // so the call was made from 'i' to 'iprev', hence the calling line
        // metadata of 'iprev' is the place we need to check visibility from.
        //
        if (i != m_stack.begin()) {
            FunctionStack::iterator iprev = i - 1;
            if (iprev->calling_line_metadata) {
                VarsMapping vars_info = CollectVisibleVars(
                    iprev->calling_line_metadata, *i);
                
                VarsMapping::const_iterator ivar = vars_info.begin();
                for (; ivar != vars_info.end(); ++ivar) {
                    ServerToClientMessage::VarInfo* var_msg_info = frameinfo->add_vars();
                    var_msg_info->set_name(ivar->second.name);
                    var_msg_info->set_value(ivar->second.value);
                    var_msg_info->set_type(ivar->second.type);
                    var_msg_info->set_address(ivar->second.addr);
                    var_msg_info->mutable_type_descriptor()->CopyFrom(
                        ivar->second.type_descriptor);
                }
            }
        }
    }

    DEBUG_SERVER_LOG("Sending STACK_TRACE_INFO message");
    m_comm->sendMessage(msg_to_client);
}


// Safely query the value at the given address in the current process.
// Return '\x00' for invalid addresses.
//
static char safe_query_addr(uint64_t addr)
{
    char* memptr = reinterpret_cast<char*>(addr);
    char value;
#ifdef _WIN32
    __try {
        value = *memptr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = '\x00';
    }
#else
    value = *memptr;
#endif // _WIN32
    return value;
}


bool InitDebugServer()
{
    // Check if debugging is enabled at all
    //
    string val;
    cl_err_code rc = Intel::OpenCL::Utils::GetEnvVar(val, "CL_CONFIG_DBG_ENABLE");
    if (rc != CL_SUCCESS || val != "1")
        return true;

    // Debugging enabled: try to initialize the server.
    // Note: no Linux support for now.
    //
#ifdef _WIN32
    if (!DebugServer::GetInstance().Init())
        return false;
    DebugServer::GetInstance().WaitForStartCommand();
#endif // _WIN32
    return true;
}


void DebugServer::DebugServerImpl::SendMemoryRangeInfo(const ClientToServerMessage& memory_range_msg)
{
    uint64_t start_addr = memory_range_msg.get_memory_range_msg().start_addr();
    uint64_t end_addr = memory_range_msg.get_memory_range_msg().end_addr();
    uint64_t range_len = end_addr - start_addr + 1;

    // Sanity check on parameters
    //
    if (start_addr > end_addr || range_len > MAX_MEMORY_RANGE_SIZE) {
        ServerToClientMessage error_msg = BuildErrorMessage(
            "Invalid GET_MEMORY_RANGE parameters");
        m_comm->sendMessage(error_msg);
    }

    
    string buf = "";
    for (uint64_t addr = start_addr; addr <= end_addr; ++addr) {
        buf += safe_query_addr(addr);
    }

    ServerToClientMessage msg_to_client;
    msg_to_client.set_type(ServerToClientMessage::MEMORY_RANGE_INFO);
    msg_to_client.mutable_memory_range_info_msg()->set_buf(buf);
    DEBUG_SERVER_LOG("Sending MEMORY_RANGE_INFO message");
    m_comm->sendMessage(msg_to_client);
}


void DebugServer::DebugServerImpl::SetDebuggedGlobalId(unsigned x, unsigned y, unsigned z)
{
    m_debugged_global_id[0] = x;
    m_debugged_global_id[1] = y;
    m_debugged_global_id[2] = z;
}


bool DebugServer::DebugServerImpl::DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z)
{
    return (m_initialized &&
        m_debugged_global_id[0] == x && 
        m_debugged_global_id[1] == y &&
        m_debugged_global_id[2] == z);
}


void DebugServer::DebugServerImpl::DumpFunctionVars()
{
    DEBUG_SERVER_LOG("Dumping function vars ===============");
    cerr << ">--------------- Local vars\n";
    for (   vector<FunctionStackFrame::VarDeclInfo>::iterator i = m_stack.front().vars.begin();
            i != m_stack.front().vars.end(); ++i)
    {
        if (!i->is_global)
            i->description->dump();
    }

    cerr << ">--------------- Global vars\n";
    for (   vector<FunctionStackFrame::VarDeclInfo>::iterator i = m_stack.front().vars.begin();
            i != m_stack.front().vars.end(); ++i)
    {
        if (i->is_global)
            i->description->dump();
    }
}


VarsMapping DebugServer::DebugServerImpl::CollectVisibleVars(
    const MDNode* line_metadata, const FunctionStackFrame& stackframe)
{
    VarsMapping visiblevars;

    assert(line_metadata->getNumOperands() == 4);
    if (const MDNode* scope = dyn_cast<const MDNode>(line_metadata->getOperand(2))) {
        DILexicalBlock descriptor(scope);
        if (descriptor.isLexicalBlock()) {
            visiblevars = CollectVarsInLexicalBlock(descriptor, stackframe);
        }
        else {
            DIScope descriptor(scope);
            assert(descriptor.isSubprogram());
            visiblevars = CollectVarsInScope(descriptor, stackframe);
        }
    }
    else
        assert(0);

    VarsMapping globalvars = CollectGlobalVars(stackframe);
    MergeVarsMapping(visiblevars, globalvars);
    
    return visiblevars;
}


VarsMapping DebugServer::DebugServerImpl::CollectVarsInLexicalBlock(
    DILexicalBlock block, const FunctionStackFrame& stackframe)
{
    VarsMapping myvars = CollectVarsInScope(block, stackframe);

    // Look up declarations in the parent (context) of this block
    //
    VarsMapping parent_vars;
    DIScope block_context = block.getContext();
    if (block_context.isLexicalBlock()) {
        const MDNode* context_mdnode = static_cast<const MDNode*>(block_context);
        DILexicalBlock parent_block(context_mdnode);
        parent_vars = CollectVarsInLexicalBlock(parent_block, stackframe);
    }
    else if (block_context.isSubprogram()) {
        parent_vars = CollectVarsInScope(block_context, stackframe);
    }
    else {
        assert(0);
    }

    //cerr << "-- In Lexical block on line " << block.getLineNumber() << " --" << endl;
    //cerr << "> My vars:\n";
    //dump_vars_mapping(myvars);
    //cerr << "> Parent vars:\n";
    //dump_vars_mapping(parent_vars);

    // Merge parent's vars into myvars.
    // We go over the hierarchy from children to parents. Parent vars
    // that were redefined in children are ignored due to the lexical scoping
    // rules of C
    //
    MergeVarsMapping(myvars, parent_vars);

    //cerr << "> Merged vars:\n";
    //dump_vars_mapping(myvars);

    return myvars;
}


VarsMapping DebugServer::DebugServerImpl::CollectVarsInScope(
    const DIScope& scope, const FunctionStackFrame& stackframe)
{
    VarsMapping myvars;

    // Go over all local declarations in the given stack frame.
    // Find out whether the declaration was done in *scope*.
    //
    vector<FunctionStackFrame::VarDeclInfo>::const_iterator i = stackframe.vars.begin();
    for (; i != stackframe.vars.end(); ++i) {
        if (i->is_global)
            continue;

        DIVariable var_di(i->description);
        assert(var_di.isVariable());

        // Find the context scope of the declaration
        //
        DIScope var_scope = var_di.getContext();
        MDNode* var_scope_md = static_cast<MDNode*>(var_scope);

        // Is this the same block as the one we're collecting from?
        //
        if (static_cast<MDNode*>(scope) == var_scope_md) {
            VarDescription var_description = CreateVarDescription(*i);
            myvars[var_description.name] = var_description;
        }
    }

    return myvars;
}


VarsMapping DebugServer::DebugServerImpl::CollectGlobalVars(
    const FunctionStackFrame& stackframe)
{
    VarsMapping globalvars;

    vector<FunctionStackFrame::VarDeclInfo>::const_iterator i = stackframe.vars.begin();
    for (; i != stackframe.vars.end(); ++i) {
        if (i->is_global) {
            VarDescription var_description = CreateVarDescription(*i);
            globalvars[var_description.name] = var_description;
        }
    }

    return globalvars;
}


VarDescription DebugServer::DebugServerImpl::CreateVarDescription(const FunctionStackFrame::VarDeclInfo& var_info)
{
    string var_name_str;
    DIType di_type;

    if (var_info.is_global) {
        DIGlobalVariable di_global_type(var_info.description);
        var_name_str = di_global_type.getName().str();
        di_type = di_global_type.getType();
    }
    else {
        DIVariable di_local_type(var_info.description);
        var_name_str = di_local_type.getName().str();
        di_type = di_local_type.getType();
    }

    string var_type_str = DescribeVarType(di_type);
    string var_value_str = DescribeVarValue(di_type, var_info.addr, var_type_str);
    VarTypeDescriptor var_type_descriptor = GenerateVarTypeDescriptor(di_type);
    
    return VarDescription(
        var_name_str, 
        var_type_str, 
        var_value_str,
        reinterpret_cast<uint64_t>(var_info.addr),
        var_type_descriptor);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


DebugServer::DebugServer()
    : d(new DebugServerImpl) 
{
}


DebugServer::~DebugServer()
{
}


bool DebugServer::Init()
{
    if (d->m_initialized)
        return true;
    
    unsigned port_num = DEBUG_SERVER_PORT_DEFAULT;
    string port_val_str;
    cl_err_code rc = Intel::OpenCL::Utils::GetEnvVar(port_val_str, "CL_CONFIG_DBG_PORT_NUMBER");

    if (rc == CL_SUCCESS) {
        char c;
        stringstream ss(port_val_str);
        ss >> port_num;
        if (ss.fail() || ss.get(c) || port_num > 0xFFFF)
            port_num = DEBUG_SERVER_PORT_DEFAULT;
    }

    d->m_comm = new DebugCommunicator(port_num);
    DEBUG_SERVER_LOG("Server waiting for connection on port " + stringify(port_num));
    d->m_comm->waitForConnection();
    DEBUG_SERVER_LOG("Initialized successfully");

    d->m_initialized = true;
    return true;
}


void DebugServer::WaitForStartCommand()
{
    // Receive a START_SESSION message and reply to it
    //
    ClientToServerMessage msg = d->m_comm->receiveMessage();
    LOG_RECEIVED_MESSAGE(msg);

    if (msg.type() == ClientToServerMessage::START_SESSION && msg.has_start_session_msg()) {
        DEBUG_SERVER_LOG("START_SESSION received");
        d->SetDebuggedGlobalId(
            msg.start_session_msg().global_id_x(),
            msg.start_session_msg().global_id_y(),
            msg.start_session_msg().global_id_z());

        // Report the received global ID for debugging
        //
        string s;
        for (size_t i = 0; i < 3; ++i) {
            s += ' ';
            s += stringify(d->m_debugged_global_id[i]);
        }
        DEBUG_SERVER_LOG("Debugged global id:" + s);

        ServerToClientMessage start_ack_msg;
        start_ack_msg.set_type(ServerToClientMessage::START_SESSION_ACK);
        start_ack_msg.mutable_start_session_ack_msg()->set_sizeof_size_t(
            sizeof(size_t));
        d->m_comm->sendMessage(start_ack_msg);
    }
    else {
        DEBUG_SERVER_LOG("Invalid message received while waiting for START_SESSION");
        ServerToClientMessage error_msg = d->BuildErrorMessage(
            "Expected a START_SESSION command");
        d->m_comm->sendMessage(error_msg);
        return;
    }

    // Receive a RUN message and register the breakpoints
    //
    msg = d->m_comm->receiveMessage();
    LOG_RECEIVED_MESSAGE(msg);

    if (msg.type() == ClientToServerMessage::RUN && msg.has_run_msg()) {
        DEBUG_SERVER_LOG("RUN received");
        d->RegisterBreakpoints(msg);
    }
    else {
        DEBUG_SERVER_LOG("Invalid message received while waiting for RUN");
        ServerToClientMessage error_msg = d->BuildErrorMessage(
            "Expected a RUN command");
        d->m_comm->sendMessage(error_msg);
        return;
    }
}


void DebugServer::Stoppoint(const MDNode* line_metadata)
{
    DILocation loc(line_metadata);
    StringRef file = loc.getFilename();
    StringRef dir = loc.getDirectory();
    unsigned lineno = loc.getLineNumber();

    d->m_prev_stoppoint_line = line_metadata;
    bool stopped = false;

    if (d->HasBreakpointAt(file, lineno)) {
        // If there's a breakpoint here, stop anyway, no matter in which 
        // running state we are.
        //
        stopped = true;
    }
    else {
        switch (d->m_runningmode) {
            case DebugServerImpl::RUNNING_NORMAL:
                // Don't stop - since a previous HasBreakpointAt test failed.
                //
                stopped = false;
                break;
            case DebugServerImpl::RUNNING_STEP_IN:
                stopped = true;
                break;
            case DebugServerImpl::RUNNING_STEP_OVER:
                // When stepping over, we won't stop inside a function call
                // made from this line. The next stoppoint can, however, end up
                // higher in the stack (maybe the stopoint from which we're
                // stepping over is on a 'return'), so check for at most as
                // deep as saved level.
                //
                if (d->m_stack.size() <= d->m_saved_stack_level) {
                    stopped = true;
                }
                break;
            case DebugServerImpl::RUNNING_STEP_OUT:
                // When stepping out, we want to end up strictly less deep in
                // the stack.
                //
                if (d->m_stack.size() < d->m_saved_stack_level) {
                    stopped = true;
                }
                break;
            default:
                assert(0 && "unreachable");
                break;
        }
    }

    if (stopped) {
        assert(d->m_stack.size() > 0);
        VarsMapping vars_info = d->CollectVisibleVars(line_metadata, d->m_stack.front());
        d->NotifyBreakpointHit(file, lineno, vars_info);
        bool stuck = true;
        while (stuck) {
            // Block until message is received from client
            //
            ClientToServerMessage msg = d->m_comm->receiveMessage();
            LOG_RECEIVED_MESSAGE(msg);
            
            switch (msg.type()) {
                case ClientToServerMessage::RUN:
                    d->m_runningmode = DebugServerImpl::RUNNING_NORMAL;
                    d->RegisterBreakpoints(msg);
                    stuck = false;
                    break;
                case ClientToServerMessage::SINGLE_STEP_IN:
                    d->m_runningmode = DebugServerImpl::RUNNING_STEP_IN;
                    stuck = false;
                    break;
                case ClientToServerMessage::SINGLE_STEP_OVER:
                    d->m_runningmode = DebugServerImpl::RUNNING_STEP_OVER;
                    stuck = false;
                    d->m_saved_stack_level = d->m_stack.size();
                    break;
                case ClientToServerMessage::SINGLE_STEP_OUT:
                    d->m_runningmode = DebugServerImpl::RUNNING_STEP_OUT;
                    stuck = false;
                    d->m_saved_stack_level = d->m_stack.size();
                    break;
                case ClientToServerMessage::GET_STACK_TRACE:
                    d->SendStackTraceInfo();
                    break;
                case ClientToServerMessage::GET_MEMORY_RANGE:
                    d->SendMemoryRangeInfo(msg);
                    break;
                default:
                    assert(0);
            }
        }
    }
}


bool DebugServer::DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z)
{
    return d->DebuggedGlobalIdMatch(x, y, z);
}


void DebugServer::EnterFunction(const llvm::MDNode* subprogram_mdn)
{
    FunctionStackFrame stack_frame = FunctionStackFrame();
    stack_frame.function_metadata = subprogram_mdn;
    stack_frame.calling_line_metadata = d->m_prev_stoppoint_line;
    d->m_stack.push_front(stack_frame);
}


void DebugServer::ExitFunction(const llvm::MDNode* subprogram_mdn)
{
    assert(d->m_stack.size() > 0);
    d->m_stack.pop_front();
}


void DebugServer::DeclareLocal(void* addr, const llvm::MDNode* description)
{
    FunctionStackFrame::VarDeclInfo varinfo(addr, description, false);
    assert(d->m_stack.size() > 0);
    d->m_stack.front().vars.push_back(varinfo);
}


void DebugServer::DeclareGlobal(void* addr, const llvm::MDNode* description)
{
    FunctionStackFrame::VarDeclInfo varinfo(addr, description, true);
    assert(d->m_stack.size() > 0);
    d->m_stack.front().vars.push_back(varinfo);
}

