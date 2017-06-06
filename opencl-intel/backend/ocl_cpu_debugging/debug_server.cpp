/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2014 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "debug_server.h"
#include "debugservermessages.pb.h"
#include "debug_communicator.h"
#include "debuginfo_utils.h"
#include "cl_env.h"
#include "google/protobuf/text_format.h"

// These can be defined as macros in earlier headers and can interfere with
// similarly named methods in LLVM headers
//
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "llvm/Support/MutexGuard.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <string>
#include <deque>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

static const unsigned DEBUG_SERVER_PORT_DEFAULT = 56203;
static const uint64_t MAX_MEMORY_RANGE_SIZE = 16 * 1024;

using namespace std;
using namespace llvm;
using namespace debugservermessages;
using namespace Intel::OpenCL::DeviceBackend;

#ifdef _WIN32
// Get a string value from the registry.
//   top_hkey: one of standard the HKEY_* constants
//   path: path to the key in the registry
//   value_name: name for which to obtain the value from the key
//
// On success, return true and place the value in 'value'.
// On failure, return false.
//
static bool get_reg_value_string(HKEY top_hkey, 
                                 const string& path,
                                 const string& value_name,
                                 string& value)
{
    const DWORD DATABUF_SIZE = 2048;
    CHAR databuf[DATABUF_SIZE] = {0};
    DWORD databuf_size = DATABUF_SIZE;
    HKEY hkey;

    // Open the registry path. hkey will hold the entry
    LONG rc = RegOpenKeyExA(
        top_hkey,           // hkey
        path.c_str(),       // lpSubKey
        0,                  // ulOptions
        KEY_READ,           // samDesired
        &hkey               // phkResult
        );

    if (rc == ERROR_SUCCESS) {
        // Get the value by name from the key
        rc = RegQueryValueExA(
            hkey,                   // hkey
            value_name.c_str(),     // lpValueName
            0,                      // lpReserved
            NULL,                   // lpType
            (LPBYTE)databuf,        // lpData
            &databuf_size           // lpcbData
            );

        // Close the key - we don't need it any more
        RegCloseKey(hkey);

        if (rc == ERROR_SUCCESS && databuf_size < DATABUF_SIZE - 1) {
            value = databuf;
            return true;
        }
    }

    return false;
}

static void strToLower(string& str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// Get the PID of the current process as a string
//
static string get_my_pid_string()
{
    return stringify(GetCurrentProcessId());
}

// Resolves the absolute path (strips /, .. and .\)
// for the given dir and file name.
//
static string getAbsPath(string file, string dir)
{
    string concat_path(dir);
    concat_path += '\\';
    concat_path += file;
    string path = sys::fs::exists(concat_path) ? concat_path : file;

    char absPath[MAX_PATH] = {0};
    char* res = _fullpath(absPath, path.c_str(), MAX_PATH);
    if (NULL == res)
    {
        return file;
    }

    string absPathStr(absPath);
    strToLower(absPathStr);
    // the breakpoints database we have contains paths with forward
    // slashes only, therefore we need to be consistent with it
    replace(absPathStr.begin(), absPathStr.end(), '\\', '/');
    return absPathStr;
}

#endif // _WIN32

static void LOG_RECEIVED_MESSAGE(const ClientToServerMessage& msg)
{
    DEBUG_SERVER_LOG("Message received:");
    string s;
    google::protobuf::TextFormat::PrintToString(msg, &s);
    DEBUG_SERVER_LOG(s);
}

// Define the singleton instance
//
DebugServer DebugServer::instance;

namespace {
// Used to collect the variable declarations visible up to a certain point in 
// a function.
//
struct FunctionStackFrame
{
    struct VarDeclInfo
    {
        VarDeclInfo(void* addr_, const MDNode* description_, const MDNode* expression_)
            : addr(addr_), description(description_), expression(expression_)
        {
            if (expression) {
              assert(dyn_cast<DIExpression>(expression) && "DIExpression is expected");
              const DIExpression* di_expr = cast<DIExpression>(expression);
              unsigned N = di_expr->getNumElements();
              for (unsigned i = 0; i < N; ++i) {
                uint64_t Element = di_expr->getElement(i);
                if (Element == dwarf::DW_OP_plus) {
                  uint64_t a = reinterpret_cast<uint64_t>(addr);
                  a += di_expr->getElement(++i);
                  addr = reinterpret_cast<void*>(a);
                } else if (Element == dwarf::DW_OP_deref) {
                  addr = *(reinterpret_cast<void**>(addr));
                } else {
                  // [LLVM 3.6 UPGRADE] FIXME: What about DW_OP_bit_piece?
                  // See http://llvm.org/docs/LangRef.html#diexpression for more details
                  // Note that 3.6.0 already has DIExpression but this is not described in the release related pages
                  llvm_unreachable("unknown complex address opcode");
                }
              }
            }
        }

        void* addr;
        const MDNode* description;
        const MDNode* expression;
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

} // namespace

typedef map<string, VarDescription> VarsMapping;


// Merge the vars from 'src' into 'dest'. Only those vars that are not already
// in 'dest' are added.
//
static void MergeVarsMapping(VarsMapping& dest, const VarsMapping& src)
{
    for (VarsMapping::const_iterator i = src.begin(); i != src.end(); ++i) {
        if (dest.find(i->first) == dest.end())
            dest[i->first] = i->second;
    }
}


// Dump a mapping for debugging
//
static void dump_vars_mapping(VarsMapping mapp)
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
        TerminateCommunicator();
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
        const DILexicalBlock& block, const FunctionStackFrame& stackframe);
    VarDescription CreateVarDescription(
        const FunctionStackFrame::VarDeclInfo& var_info);

    void DumpFunctionVars();

    void TerminateCommunicator();

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
        string filename = msg_lineinfo.file();
#ifdef _WIN32
        strToLower(filename);
#endif
        m_breakpoints.insert(make_pair(filename, msg_lineinfo.lineno()));
        stringstream ss;
        ss << "Registered " << filename << ":" << msg_lineinfo.lineno() << "\n";
        DEBUG_SERVER_LOG(ss.str());
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
        assert(dyn_cast<DISubprogram>(i->function_metadata) && "DISubprogram is expected");
        const DISubprogram* subprogram_descriptor = cast<DISubprogram>(i->function_metadata);
        string function_name = subprogram_descriptor->getName().str();
        ServerToClientMessage::StackFrameInfo* frameinfo =
            msg_to_client.mutable_stack_trace_info_msg()->add_frames();
        frameinfo->set_func_name(function_name);
        LineInfo* call_line_info = frameinfo->mutable_call_line();

        if (i->calling_line_metadata) {
            assert(dyn_cast<DILocation>(i->calling_line_metadata) && "DILocation is expected");
            const DILocation* loc = cast<DILocation>(i->calling_line_metadata);
            call_line_info->set_file(loc->getFilename());
            call_line_info->set_lineno(loc->getLine());
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


bool DebuggingIsEnabled()
{
    string val;
    cl_err_code rc = Intel::OpenCL::Utils::GetEnvVar(val, "CL_CONFIG_DBG_ENABLE");
    if (rc != CL_SUCCESS || val != "1")
        return false;

#ifdef _WIN32
    // On Windows only, we also check in the registry. Either a PID-specific
    // or global key must exist, with value OCL_DBG_CFG_ENABLE=1.
    // If the value is wrong or neither key exists, debugging is disabled.
    //
    static const char* REG_BASE = "Software\\Intel\\OpenCL\\Debugger\\";
    static const char* REG_VAL_NAME = "CL_CONFIG_DBG_ENABLE";

    string pid_path = string(REG_BASE) + get_my_pid_string();
    string global_path = string(REG_BASE) + "__global";
    if (get_reg_value_string(HKEY_CURRENT_USER, pid_path, REG_VAL_NAME, val)) {
        return val == "1";
    }
    else if (get_reg_value_string(HKEY_CURRENT_USER, global_path, REG_VAL_NAME, val)) {
        return val == "1";
    }
    else 
        return false;
#endif //_WIN32

    return true;
}


bool InitDebugServer(unsigned int port_number)
{
#ifndef _WIN32
    if (!DebuggingIsEnabled())
        return true;
#endif

    // Debugging enabled: try to initialize the server.
    //
    if (!DebugServer::GetInstance().Init(port_number))
        return false;
    DebugServer::GetInstance().WaitForStartCommand();
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
        if (i->expression)
            i->description->dump();
    }

    cerr << ">--------------- Global vars\n";
    for (   vector<FunctionStackFrame::VarDeclInfo>::iterator i = m_stack.front().vars.begin();
            i != m_stack.front().vars.end(); ++i)
    {
        if (!i->expression)
            i->description->dump();
    }
}


VarsMapping DebugServer::DebugServerImpl::CollectVisibleVars(
    const MDNode* line_metadata, const FunctionStackFrame& stackframe)
{
    VarsMapping visiblevars;

    assert(line_metadata->getNumOperands() == 1);
    if (const MDNode* scope = dyn_cast<const MDNode>(line_metadata->getOperand(0))) {
        if (dyn_cast<DILexicalBlock>(scope)) {
            auto descriptor = cast<DILexicalBlock>(scope);
            visiblevars = CollectVarsInLexicalBlock(*descriptor, stackframe);
        }
        else {
            assert(dyn_cast<DISubprogram>(scope) && dyn_cast<DIScope>(scope));
            const DIScope* descriptor = cast<DIScope>(scope);
            visiblevars = CollectVarsInScope(*descriptor, stackframe);
        }
    }
    else
        assert(0);

    VarsMapping globalvars = CollectGlobalVars(stackframe);
    MergeVarsMapping(visiblevars, globalvars);
    
    return visiblevars;
}


VarsMapping DebugServer::DebugServerImpl::CollectVarsInLexicalBlock(
    const DILexicalBlock& block, const FunctionStackFrame& stackframe)
{
    VarsMapping myvars = CollectVarsInScope(block, stackframe);

    // Look up declarations in the parent (context) of this block
    //
    VarsMapping parent_vars;
    DIScope* block_context = block.getScope();
    if (auto parent_block = dyn_cast<DILexicalBlock>(block_context)) {
        parent_vars = CollectVarsInLexicalBlock(*parent_block, stackframe);
    }
    else if (auto parent_subprogram = dyn_cast<DISubprogram>(block_context)) {
        parent_vars = CollectVarsInScope(*parent_subprogram, stackframe);
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
        if (!i->expression) // if this is a global variable declaration
            continue;

        assert(dyn_cast<DIVariable>(i->description) && "DIVariable is expected");
        const DIVariable* var_di = cast<DIVariable>(i->description);

        // Find the context scope of the declaration
        //
        DIScope* var_scope = var_di->getScope();
        MDNode* var_scope_md = static_cast<MDNode*>(var_scope);

        // Is this the same block as the one we're collecting from?
        //
        if (cast<MDNode>(&scope) == var_scope_md) {
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
        if (!i->expression) { // if this is a global variable declaration
            VarDescription var_description = CreateVarDescription(*i);
            globalvars[var_description.name] = var_description;
        }
    }

    return globalvars;
}


VarDescription DebugServer::DebugServerImpl::CreateVarDescription(
                            const FunctionStackFrame::VarDeclInfo& var_info)
{
    string var_name_str;
    DIType* di_type = nullptr;

    if (!var_info.expression) { // if this is a global variable declaration
        assert(dyn_cast<DIGlobalVariable>(var_info.description)
               && "DIGlobalVariable is expected");
        const DIGlobalVariable* di_global_type =
              cast<DIGlobalVariable>(var_info.description);
        var_name_str = di_global_type->getName().str();
        di_type = di_global_type->getType().resolve();
    }
    else {
        assert(dyn_cast<DIVariable>(var_info.description)
               && "DIVariable is expected");
        const DIVariable* di_local_type = cast<DIVariable>(var_info.description);
        var_name_str = di_local_type->getName().str();
        di_type = di_local_type->getType().resolve();
    }

    string var_type_str = DescribeVarType(di_type);
    string var_value_str = DescribeVarValue(di_type, var_info.addr, var_type_str);
    VarTypeDescriptor var_type_descriptor = GenerateVarTypeDescriptor(*di_type);
    
    return VarDescription(
        var_name_str, 
        var_type_str, 
        var_value_str,
        reinterpret_cast<uint64_t>(var_info.addr),
        var_type_descriptor);
}


void DebugServer::DebugServerImpl::TerminateCommunicator()
{
    if (m_comm)  {
        delete m_comm;
        m_comm = 0;
    }
    m_initialized = false;
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


bool DebugServer::Init(unsigned int port_number)
{
    llvm::MutexGuard lock(m_Lock);
    if (d->m_initialized)
        return true;
    
    unsigned port_num = DEBUG_SERVER_PORT_DEFAULT;
#ifdef _WIN32
    port_num = port_number;
#else
    if (!DebuggingIsEnabled())
        return true;

    string port_val_str;
    cl_err_code rc = Intel::OpenCL::Utils::GetEnvVar(port_val_str, "CL_CONFIG_DBG_PORT_NUMBER");

    if (rc == CL_SUCCESS) {
        char c;
        stringstream ss(port_val_str);
        ss >> port_num;
        if (ss.fail() || ss.get(c) || port_num > 0xFFFF)
            port_num = DEBUG_SERVER_PORT_DEFAULT;
    }
#endif

    d->m_comm = new DebugCommunicator(port_num);
#ifdef _WIN32
    // On Windows, as part of the handshake with the MSVC plugin, we send a 
    // Windows event when the server starts listening on the port.
    //
    static const char* EVENT_NAME_PREFIX = "icldbgevent_";

    d->m_comm->waitForListen();
    string eventName = EVENT_NAME_PREFIX + get_my_pid_string();
    HANDLE e = CreateEventA(0, false, false, eventName.c_str());
    SetEvent(e);
#endif

    DEBUG_SERVER_LOG("Server waiting for connection on port " + stringify(port_num));
    d->m_comm->waitForConnection();
    DEBUG_SERVER_LOG("Initialized successfully");

    d->m_saved_stack_level = 0;
    d->m_prev_stoppoint_line = 0;

    d->m_initialized = true;
    return true;
}


void DebugServer::WaitForStartCommand()
{
    // Receive a START_SESSION message and reply to it
    //
    llvm::MutexGuard lock(m_Lock);
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

    // Receive a RUN or STEP_IN message.
    //
    msg = d->m_comm->receiveMessage();
    LOG_RECEIVED_MESSAGE(msg);

    if (msg.type() == ClientToServerMessage::RUN) {
        DEBUG_SERVER_LOG("RUN received... starting");
        d->m_runningmode = DebugServerImpl::RUNNING_NORMAL;
        d->RegisterBreakpoints(msg);
    }
    else if (msg.type() == ClientToServerMessage::SINGLE_STEP_IN) {
        DEBUG_SERVER_LOG("SINGLE_STEP_IN received... starting");
        d->m_runningmode = DebugServerImpl::RUNNING_STEP_IN;
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
  assert(dyn_cast<DILocation>(line_metadata) && "DILocation is expected");
    const DILocation* loc = dyn_cast<DILocation>(line_metadata);
    StringRef file  = loc->getFilename();
    StringRef dir   = loc->getDirectory();
    unsigned lineno = loc->getLine();

#ifdef _WIN32
    // Resolve full path.
    // This is required when .cl files include other .cl files,
    // In which case the "file" argument contains only relative path
    // while our breakpoints database deals with absolute paths.
    string absPath = getAbsPath(file, dir);
#else
    string absPath = file;
#endif

    llvm::MutexGuard lock(m_Lock);
    d->m_prev_stoppoint_line = line_metadata;
    bool stopped = false;


    if (d->HasBreakpointAt(absPath, lineno))
    {
        // If there's a breakpoint here, stop anyway, no matter in which 
        // running state we are.
        //
        stringstream ss;
        ss << "Breakpoint hit at " << absPath << ":" << lineno << "\n";
        DEBUG_SERVER_LOG(ss.str());
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
    llvm::MutexGuard lock(m_Lock);
    return d->DebuggedGlobalIdMatch(x, y, z);
}


void DebugServer::EnterFunction(const llvm::MDNode* subprogram_mdn)
{
    FunctionStackFrame stack_frame = FunctionStackFrame();
    stack_frame.function_metadata = subprogram_mdn;
    stack_frame.calling_line_metadata = d->m_prev_stoppoint_line;
    llvm::MutexGuard lock(m_Lock);
    d->m_stack.push_front(stack_frame);
}


void DebugServer::ExitFunction(const llvm::MDNode* subprogram_mdn)
{
    assert(d->m_stack.size() > 0);
    llvm::MutexGuard lock(m_Lock);
    d->m_stack.pop_front();
}


void DebugServer::DeclareLocal(void* addr, const llvm::MDNode* description, const llvm::MDNode* expression)
{
    FunctionStackFrame::VarDeclInfo varinfo(addr, description, expression);
    assert(d->m_stack.size() > 0);
    llvm::MutexGuard lock(m_Lock);
    d->m_stack.front().vars.push_back(varinfo);
}


void DebugServer::DeclareGlobal(void* addr, const llvm::MDNode* description)
{
    FunctionStackFrame::VarDeclInfo varinfo(addr, description, nullptr);
    assert(d->m_stack.size() > 0);
    llvm::MutexGuard lock(m_Lock);
    d->m_stack.front().vars.push_back(varinfo);
}


void DebugServer::TerminateConnection()
{
    llvm::MutexGuard lock(m_Lock);
    d->TerminateCommunicator();
}


DEBUG_SERVICE_API bool InitDebuggingService(unsigned int port_number)
{
    return InitDebugServer(port_number);
}


DEBUG_SERVICE_API ICLDebuggingService* DebuggingServiceInstance()
{
    return &DebugServer::GetInstance();
}


DEBUG_SERVICE_API void TerminateDebuggingService()
{
    DebugServer::GetInstance().TerminateConnection();
}
