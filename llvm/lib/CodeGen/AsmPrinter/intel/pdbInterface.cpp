//
//  Copyright  (C) 1993-2015 Intel Corporation. All rights reserved.
//
//  The information and source code contained herein is the exclusive
//  property of Intel Corporation and may not be disclosed, examined
//  or reproduced in whole or in part without explicit written authorization
//  from the company.
//

//
// cvs_id[] = "$Id"
//

#include "pdbInterface.h"
#include "llvm/Config/llvm-config.h"

#ifdef LLVM_ON_WIN32

#include "pdb.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <assert.h>


#define ASSERT_PROD(cond,posn,id,str,...) /*assert(cond && str)*/
#define ASSERT_DEBUG(cond,posn,id,str,...) /*assert(cond && str)*/

#ifdef __cplusplus
extern "C" {
#endif
//
// converts the input null-terminated character string into
// windows wide char null terminated string.  The string returned
// has been allocated via malloc, and must be freed by the client
// calling free once it is done with the value.
//
wchar_t * utf8_to_wchar(
    const char *in
) {
  const size_t cSize = strlen(in)+1;
    wchar_t *wc = (wchar_t*)malloc(cSize*sizeof(wchar_t));
    mbstowcs (wc, in, cSize);

    return wc;
}
#ifdef __cplusplus
}
#endif

const bool DEBUG_show_pdb_trace = false;


static HMODULE the_dll_handle = NULLP;
static PDB  *  pdb_handle = NULLP;
static TPI*  ipi_handle = NULLP;
static TPI*  tpi_handle = NULLP;

static const char * pdb_errors[] = {
    /* EC_OK */                  "no problem",
    /* EC_USAGE */               "invalid parameter or call order",
    /* EC_OUT_OF_MEMORY */       "out of RAM",
    /* EC_FILE_SYSTEM */         " can't write file, out of disk, etc.",
    /* EC_NOT_FOUND */           " PDB file not found",
    /* EC_INVALID_SIG */         " PDB::OpenValidate() and its clients only",
    /* EC_INVALID_AGE */         " PDB::OpenValidate() and its clients only",
    /* EC_PRECOMP_REQUIRED */    "obj name, PDBMod::AddTypes() only",
    /* EC_OUT_OF_TI */           " TPI::QueryTiForCVRecord() only",
    /* EC_NOT_IMPLEMENTED */     "Unknown error",
    /* EC_V1_PDB */              " PDB::Open* only (obsolete)",
                                 "pdb created by newer version",
    /* EC_FORMAT */              "accessing pdb with obsolete format",
    /* EC_LIMIT */               "resources not available",
    /* EC_CORRUPT */             "cv info corrupt, recompile mod",
    /* EC_TI16 */                "no 16-bit type interface present",
    /* EC_ACCESS_DENIED */       "PDB file read-only",
    /* EC_RESERVED_1 */          "Unknown error",
    /* EC_INVALID_EXECUTABLE */  "not recogized as a valid executable",
    /* EC_DBG_NOT_FOUND */       "A required .DBG file was not found",
    /* EC_NO_DEBUG_INFO */       "No recognized debug info found",
    /* EC_INVALID_EXE_TIMESTAMP */ "Invalid timestamp on Openvalidate of exe",
    /* EC_CORRUPT_TYPEPOOL */    "A corrupted type record was found in a PDB",
    /* EC_DEBUG_INFO_NOT_IN_PDB */ "returned by OpenValidateX",
    /* EC_RPC */                 "Error occured during RPC",
    /* EC_UNKNOWN */             "Unknown error",
    /* EC_BAD_CACHE_PATH */      "bad cache location specified with symsrv",
    /* EC_CACHE_FULL */          "symsrv cache is full",
    /* EC_TOO_MANY_MOD_ADDTYPE */ "Addtype is called more then once per mod"
};

static const char *get_error_reason(int ec)
{
    if (ec < 0 || ec >= (int) EC_MAX) {
        return "unknown error";
    }
    return pdb_errors[ec];
}


// A list of entry points terminated by a NULLP entry
// Matched with this is an array of proc pointers that
// are filled in by open_dll

typedef int BOOL;

const char *  entry_point_names[] = {
    "PDBOpenEx2W",
    "PDBClose",
    "PDBCommit",
    "PDBOpenIpi",
    "PDBOpenTpi",
    "PDBQueryAge",
    "PDBQuerySignature2",
    "PDBQueryLastError",
    "TypesCommit",
    "TypesClose",
    "TypesQueryTiForCVRecordEx",
    NULLP
    };

// Support in debug compilers for capturing
// sequence of commands issued
#if DEBUG >= 2
#define TRACE_COMMAND(command) pdb_trace_command(command, NULLP);
#define TRACE_COMMAND2(command, buffer) \
     pdb_trace_command(command, (const unsigned char *)(buffer))

const char * default_dll_name = NULLP;


static void pdb_trace_command(const char *command, 
                       const unsigned char * buffer)
{
    static dgi_bool  not_enabled = FALSE;
    static FILE * dump_file = NULLP;
    if (not_enabled) {
        return;
    }
    if (dump_file == NULLP) {
        const char * dump_file_name = getenv("PDB_DUMP_FILE");
        if (dump_file_name == NULLP) {
            not_enabled = TRUE;
            return;
        }

        dump_file = fopen(dump_file_name, "a");
    }
    int line_pos = strlen(command) + 1;
    ASSERT_DEBUG(dump_file != NULLP, 0, 0, "could not open dump file");
    fprintf(dump_file, "%s:", command);
    if (buffer != NULLP) {
        size_t len = buffer[0] + 256 * buffer[1] + 2;
        for (int i =0; i < len; i ++) {
            fprintf(dump_file, " %02x",buffer[i]);
            line_pos += 3;
            if (line_pos > 70) {
                fprintf(dump_file, "\n");
                line_pos = 0;
            }
        }
    }
    if (line_pos > 0) {
        fprintf(dump_file, "\n");
    }
    if (strcmp(command, "END_OF_RUN") == 0) {
        fclose(dump_file);
        not_enabled = TRUE;
        dump_file = NULLP;
    }
}
#else
#define TRACE_COMMAND(command)
#define TRACE_COMMAND2(command, buffer)
#endif

extern "C" {
typedef BOOL (__cdecl * t_PDBOpenEx2W) (
            const wchar_t *wszPDB,
            const char *szMode,
            long cbPage,
            long *pec,
            wchar_t *wszError,
            size_t cchErrMax,
            PDB **pppdb);

typedef BOOL (__cdecl *t_PDBClose)(PDB *);
typedef BOOL (__cdecl *t_PDBCommit)(PDB*);
typedef BOOL (__cdecl *t_PDBOpenIpi)(PDB*, _In_z_ const char*, OUT TPI**);
typedef BOOL (__cdecl *t_PDBOpenTpi)(PDB*, _In_z_ const char*, OUT TPI**);
typedef AGE  (__cdecl *t_PDBQueryAge)(PDB*);
typedef BOOL (__cdecl *t_PDBQuerySignature2)(PDB*, PSIG70);
typedef EC   (__cdecl *t_PDBQueryLastError)(PDB *, _Out_opt_cap_(cbErrMax) OUT char szError[cbErrMax]);
typedef BOOL (__cdecl *t_TypesClose)(TPI*);
typedef BOOL (__cdecl *t_TypesCommit)(TPI*);
typedef BOOL (__cdecl *t_TypesQueryTiForCVRecordEx)(TPI*, BYTE*, OUT TI*);
}

static t_PDBOpenEx2W p_PDBOpenEx2W = NULLP;
static t_PDBClose p_PDBClose = NULLP;
static t_PDBCommit p_PDBCommit = NULLP;
static t_PDBOpenIpi p_PDBOpenIpi = NULLP;
static t_PDBOpenTpi p_PDBOpenTpi = NULLP;
static t_PDBQueryAge p_PDBQueryAge = NULLP;
static t_PDBQuerySignature2 p_PDBQuerySignature2 = NULLP;
static t_PDBQueryLastError p_PDBQueryLastError = NULLP;
static t_TypesCommit p_TypesCommit = NULLP;
static t_TypesClose p_TypesClose = NULLP;
static t_TypesQueryTiForCVRecordEx p_TypesQueryTiForCVRecordEx = NULLP;

void ** entry_point[] = {
    (void **)&p_PDBOpenEx2W,
    (void **)&p_PDBClose,
    (void **)&p_PDBCommit,
    (void **)&p_PDBOpenIpi,
    (void **)&p_PDBOpenTpi,
    (void **)&p_PDBQueryAge,
    (void **)&p_PDBQuerySignature2,
    (void **)&p_PDBQueryLastError,
    (void **)&p_TypesCommit,
    (void **)&p_TypesClose,
    (void **)&p_TypesQueryTiForCVRecordEx,
    NULLP  
    };

static void process_error() 
{
    char szErrPDB[cbErrMax];
    EC ec;

    if (pdb_handle == NULL) {
        fprintf(stderr, "Unable to retrieve error code.\n");
        return;
    }

    ec = (p_PDBQueryLastError)(pdb_handle, szErrPDB);
    fprintf(stderr, "last error was %08X %s\n", (int)ec, szErrPDB);
}

// The basic memory ops are failing to compile because they are marked deprecated
static size_t move_bytes(unsigned char * to, unsigned char * from, size_t len)
{
    int i;
    for (i = 0;i < len; i ++) {
        to[i] = from[i];
    }
    return len;
}

// attempt to open the given dll
// MS has a strange algorithm for opening dlls
// that causes it to fail when we have multiple
// versions. So that we can inspect all possible
// versions, we first try the MS method and then
// if that fails, we walk down the path looking
// at alternatives


//      Attempt to open a dll in the given directory
//      
static HMODULE dgil_open_dll(const char * filename)
{
    wchar_t *wfilename = utf8_to_wchar(filename);

    if (wfilename == NULLP) {
        // translation to wide char failed, just return that we
        // couldn't open the dll.
        return NULLP;
    }

    HMODULE handle = LoadLibraryW(wfilename);

    //
    // We are done with the wide file name.  Be sure to free it since
    // it was malloced by utf8_to_wchar
    //
    if (wfilename != NULLP) {
        free(wfilename);
        wfilename = NULLP;
    }

    if (handle == NULLP) {
        if (DEBUG_show_pdb_trace) {
            process_error();
        }
        return NULLP;
    }

    if (DEBUG_show_pdb_trace) {
        fprintf(stderr, " Using dll: %s\n", filename);
    }

    int i = 0;
    while (entry_point_names[i] != NULLP) {
        void *p = GetProcAddress(handle, entry_point_names[i]);
        if (p == NULLP) {
            CloseHandle(handle);
            return NULLP;
        }
        *(entry_point[i]) = p;
        i++;
    }

    return handle;
}

//      Attempt to open a dll with the given name and dirname
//      The dirname is not null terminated. Rather an explicit length
//      is provided
//      parameter three gives a list of alternative names for the dll

static HMODULE dgil_open_dll_in_dir(
        const char *dir_name,     // dirname to apply (may be NULLP)
        size_t dir_name_len,      // size of dirname
        const char *dll_names[],  // NULLP terminated array of possible dll names
        size_t max_name)          // length of longest dll name (for buffer allocation)
{
    static char * buffer = NULLP;
    static size_t buff_size;

    size_t buff_size_needed = dir_name_len + max_name + 2;  // 1 for the null and 1 for possible "/"

    if (buffer == NULLP || buff_size < buff_size_needed) {
        buffer = (char *)malloc(buff_size_needed + 200); // trying to do this only once so add room
        buff_size = buff_size_needed + 200;
        ASSERT_PROD(buffer != NULLP, 0, 0, "out of memory in dgil_open_dll");
    }
    size_t pos = 0;
    if (dir_name != NULLP && dir_name_len > 0) {
        strncpy(buffer, dir_name, dir_name_len);
        pos = dir_name_len;
        if (buffer[pos - 1] != '/' && 
            buffer[pos - 1] != '\\') {
            buffer[pos ++] = '\\';
        }
    }
    

    for (int name_idx =0; dll_names[name_idx] != NULLP; name_idx ++)
    {
        strcpy(buffer + pos, dll_names[name_idx]);
        HMODULE handle = dgil_open_dll(buffer);
        if (handle != NULLP) {
            return handle;
        }
    }
    return NULLP;
}


//   These next two arrays must be identical (except for the extra
//   NULLP on pdb_dll_names, which is required). pdb_dll_names
//   gets recreated from knwon_pdb_dll_names when a new default
//   dll name is given
static const char * pdb_dll_names[] = {
               "mspdb80.dll",
               "mspdb100.dll",
               "mspdb110.dll",
               "mspdb120.dll",
               "mspdb140.dll",
               NULLP,
               NULLP};

static const char * known_pdb_dll_names[] = {
               "mspdb80.dll",
               "mspdb100.dll",
               "mspdb110.dll",
               "mspdb120.dll",
               "mspdb140.dll",
               NULLP};

//  Set the default name of the pdb dll to load
//  We put the new name on the front of the list then
//  recreate the rest of ths list making sure we skip dups
void pdb_set_default_dll_name(const char * dll_name)
{
    // simplest way to do this is recreate the table ab initio
    // with the new name added to the front
    int pos = 0;
    pdb_dll_names[pos++] = dll_name;
    for (int i = 0; known_pdb_dll_names[i] != NULLP; i++) {
        if (strcmp(dll_name, known_pdb_dll_names[i]) != 0) {
            pdb_dll_names[pos++] = known_pdb_dll_names[i];
        }
    }
    pdb_dll_names[pos] = NULLP;
}

static dgi_bool open_dll(void)
{
    size_t maxlen = 0;
    // pre-calc the length of the longest simple dll file name 
    for (int i = 0; pdb_dll_names[i] != NULLP; i++) {
        size_t size = strlen(pdb_dll_names[i]);
        if (size >= maxlen) {
            maxlen = size + 1;
        }
    }

    // start by trying to open the DLL "at large"

    the_dll_handle = dgil_open_dll_in_dir(NULLP, 0, pdb_dll_names, maxlen);
    if (the_dll_handle != NULLP) {
        return TRUE;
    }
    const char * path = getenv("PATH");
    if (path == NULLP) {
        return FALSE;
    }

    // now work way down path attempting to open dll

    size_t start_pos = 0;
    while (path[start_pos] != 0) {
        size_t end_pos = start_pos;
        while (path[end_pos] != ';' &&
               path[end_pos] != 0) {
            end_pos ++;
        }
        the_dll_handle = dgil_open_dll_in_dir(
            path + start_pos,
            end_pos - start_pos,
            pdb_dll_names,
            maxlen);
        if (the_dll_handle != NULLP) {
            return TRUE;
        }
        start_pos = end_pos;
        if (path[start_pos] != 0) {
            start_pos ++;
        }
    }
    return FALSE;
}

dgi_bool pdb_open(const char *name)
{

    // start by loading the dll 
    // we do this dynamically as we want the 
    // compiler to be able to run without the dll 
    // being present. 
    // Also, this potentially gives us the opportunity
    // to adapt this code for multiple versions of MSVC
    TRACE_COMMAND("PDB_OPEN");

    if (the_dll_handle != NULLP &&
        ipi_handle != NULLP &&
        tpi_handle != NULLP &&
        pdb_handle != NULLP) {
        return TRUE; // already open
    }

    if (the_dll_handle == NULLP) {
        if (!open_dll()) {
            //DIAG_Message(DIAG_SEVERITY_WARNING,
            //     NULLP, -1, -1, NULLP,
            //     DGI_NO_DLL_MESSAGE, NULLP);
            ASSERT_DEBUG(FALSE, 0, 0, "could not open dll");
            return FALSE;
        }
    }

    // clear fields in case we fail
    ipi_handle = NULLP;
    tpi_handle = NULLP;
    pdb_handle = NULLP;

    // open the pdb
    // TODO - its possible we should
    // sometimes delete the pdb and recreate it
    // This is done by the MS sample code

    long ErrorCode;
    wchar_t errmsg[256];
    BOOL ok;
    int retry_count = 5;     // retry on errors as many are transient
    while (retry_count-- > 0) {
        wchar_t * wname = utf8_to_wchar(name);

        ok = (p_PDBOpenEx2W)(
            wname,                // name of pdb to open
            "iw",                 // or could be "s" for shared
            0x1000,
            &ErrorCode,
            errmsg,
            256,
            &pdb_handle
            );

        if (wname != NULLP) {
            free(wname);
        }

        if (!ok && retry_count == 0) {
            if (ErrorCode == 11 || ErrorCode == 3) { // special nice message for cq#156179
                //DIAG_Message(DIAG_SEVERITY_ERROR,
                //         NULLP, -1, -1, NULLP,
                //         DGI_CORRUPT_PDB_MESSAGE,
                //         name);
            }
            fprintf(stderr, "error code %ld (%s) opening pdb %s\n", 
                ErrorCode, 
                get_error_reason(ErrorCode),
                name);
            ASSERT_DEBUG(FALSE, 0, 0, "could not open pdb");
            return FALSE;
        }

        if (ok) {
            // open the id manager interface
            ok = (p_PDBOpenIpi)(pdb_handle, pdbWrite pdbGetTiOnly, &ipi_handle);

            if (!ok) {
                // if we could not open the pdb delete it and create a new one
                // as it is correupt. This will result in incomplete debug info
                // but it is what the Microsoft compiler appears to do
#if DEBUG  > 0
                process_error();
#endif
                pdb_close();
                if (retry_count == 0) {
                    //DIAG_Message(DIAG_SEVERITY_ERROR, 
                    //         NULLP, -1, -1, NULLP, 
                    //         DGI_CORRUPT_PDB_MESSAGE,
                    //         name);
                    exit(-1);
                }
                Sleep(2000);   // wait for transient problem to clear itself
            }
        }

        if (ok) {
            // open the type manager interface
            ok = (p_PDBOpenTpi)(pdb_handle, pdbWrite pdbGetTiOnly, &tpi_handle);
            if (ok) break;

            // if we could not open the pdb delete it and create a new one
            // as it is correupt. This will result in incomplete debug info
            // but it is what the Microsoft compiler appears to do
#if DEBUG  > 0
            process_error();
#endif
            pdb_close();
            if (retry_count == 0) {
                //DIAG_Message(DIAG_SEVERITY_ERROR, 
                //         NULLP, -1, -1, NULLP, 
                //         DGI_CORRUPT_PDB_MESSAGE,
                //         name);
                exit(-1);
            }
            Sleep(2000);   // wait for transient problem to clear itself
        }
    }
    ASSERT_DEBUG(ok, 0, 0, "TPI open error");
    return ok;
}

void pdb_close()
{
    TRACE_COMMAND("PDB_CLOSE");
    // close everything in an orderly fashion
    // start with the types
    int retry_count = 5;
    BOOL ok = TRUE;
    while (retry_count-- > 0 && 
            (ipi_handle != NULLP ||
            tpi_handle != NULLP ||
            pdb_handle != NULLP)) {
        ok = TRUE;
        if (ok && ipi_handle != NULLP) {
            ok = (p_TypesCommit)(ipi_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not commit ipi info\n");
        }
        if (ok && ipi_handle != NULLP) {
            ok = (p_TypesClose)(ipi_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not close id handle");
            if(ok)ipi_handle = NULLP;
        }
        if (ok && tpi_handle != NULLP) {
            ok = (p_TypesCommit)(tpi_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not commit tpi info\n");
        }
        if (ok && tpi_handle != NULLP) {
            ok = (p_TypesClose)(tpi_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not close type handle");
            if(ok)tpi_handle = NULLP;
        }
        if (ok && pdb_handle != NULLP) {
            ok = (p_PDBCommit)(pdb_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not commit pdb");
        }
    
        if (ok && pdb_handle != NULLP) {
            ok = (p_PDBClose)(pdb_handle);
            ASSERT_PROD(ok || retry_count > 0, 0, 0, 
                "Could not close pdb file");
            if (ok)pdb_handle = NULLP;
        }
        if (ok)break;
        Sleep(2000);   // wait for possible transient problem
    }
    if(DEBUG_show_pdb_trace)fprintf(stderr, " pdb successfully closed\n");

    // we have no explicit "end of run" point so we end it here
    // This is distinct to the PDB_CLOSE command so that we can
    // add more granularity, even in the pdb_close routine, if we wish
    TRACE_COMMAND("END_OF_RUN");
}


static BOOL is_printable(unsigned char x) 
{
    return (x >= 32 && x <= 126);
}

//      Write a type. Returns TRUE is successful
//      assigned_index, of not NULLP gets index assigned
//      buf is bytes for type, with length on front

static void dump_record(unsigned char *buf)
{
    int len = buf[0] + 256 * buf[1];
    if (len < 0 || len > 512) {
        len = 512;
    }
    len += 2;

    int i;
    for (i=0;i<len;i+=16) {
        int j;
        int k = i + 16;
        if (k > len)k = len;
        fprintf(stderr,"%04x:",i);
        for (j=i;j<k;j++)fprintf(stderr," %02X",buf[j]);
        for (j=i;j<k;j++) {
            unsigned char c=buf[j];
            if (!is_printable(c)) {
                c='.';
            }
            fprintf(stderr,"  %c",c);
        }
        fprintf(stderr,"\n");
    }
    i = len -1;
    while (i > 0 && !is_printable(buf[i]))i--;
    if (is_printable(buf[i]) && i < len -1 && buf[i+1] == 0) {
        while (i>0 && is_printable(buf[i]))i--;
        if (i>0) i++;
    
        fprintf(stderr, "     name = %s\n", (char *)buf + i);
    }
}

static unsigned int get_field(const char *buff, int offset, int size)
{
    unsigned int val = 0;
    while (size > 0) {
        val = (val << 8) | (unsigned char)buff[offset + size - 1];
        size --;
    }
    return val;
}


static dgi_bool pdb_write_type_or_id(TPI*  handle, const char * buf, unsigned long *assigned_index)
{
    if (handle == NULLP) {
         return FALSE;
    }

    TRACE_COMMAND2("WRITE_TYPE", buf);

    TI ti;

    BOOL ok;

    ok = (p_TypesQueryTiForCVRecordEx)(handle, (BYTE*)buf, &ti);

    if (DEBUG_show_pdb_trace) {
        dump_record((unsigned char *)buf);
    }

    if (!ok) {
        if (DEBUG_show_pdb_trace) {
            fprintf(stderr, "Could not create type in TypesQueryTiForCVRecordEx\n");
            process_error();
       
        
            dump_record((unsigned char *)buf);
        }
        pdb_close();
        // *assigned_index = (unsigned long)last_index;
        ASSERT_DEBUG(0, 0, 0, "Could not output type record\n");
        return FALSE;
    }
    if (DEBUG_show_pdb_trace)fprintf(stderr,"     type index %04x\n\n",(int)ti);
    *assigned_index = (unsigned long)ti;
    return TRUE;
}

dgi_bool pdb_write_id(const char * buf, unsigned long *assigned_index)
{
    return pdb_write_type_or_id(ipi_handle, buf, assigned_index);
}

dgi_bool pdb_write_type(const char * buf, unsigned long *assigned_index)
{
    return pdb_write_type_or_id(tpi_handle, buf, assigned_index);
}

size_t  pdb_get_signature(unsigned char * buf, size_t maxlen) 
{
    TRACE_COMMAND("QUERY_SIG");
    ASSERT_PROD(maxlen > sizeof(SIG70), 0, 0, "buffer overflow in pdb_get_signature\n");
    ASSERT_PROD(pdb_handle != NULLP, 0, 0, "pdb_get_signature called when pdb not open\n");
    SIG70 sig;
    BOOL ok;

    ok = (p_PDBQuerySignature2)(pdb_handle, &sig);
    ASSERT_PROD(ok, 0, 0, "could not query signature version 2\n");
    move_bytes(buf, (unsigned char *)&sig,sizeof(sig));
    return sizeof(sig);
}
    
size_t  pdb_get_age(unsigned char *buf, size_t maxlen)
{
    TRACE_COMMAND("GET_PDB_AGE");
    ASSERT_PROD(maxlen > sizeof(AGE), 0, 0, "buffer overflow in pdb_get_age\n");
    ASSERT_PROD(pdb_handle != NULLP, 0, 0, "pdb_get_agenature called when pdb not open\n");
    AGE age;

    age = (p_PDBQueryAge)(pdb_handle);
    move_bytes(buf, (unsigned char *)&age,sizeof(age));
    return sizeof(age);
}

#else // LLVM_ON_WIN32

dgi_bool pdb_open(const char *name) { return 0; }

void pdb_close() {}

dgi_bool pdb_write_id(const char * buf, unsigned long *assigned_index) {
  return 0;
}

dgi_bool pdb_write_type(const char * buf, unsigned long *assigned_index) {
  return 0;
}

size_t pdb_get_signature(unsigned char * buf, size_t maxlen) { return 0; }

size_t pdb_get_age(unsigned char *buf, size_t maxlen) {  return 0; }

void pdb_set_default_dll_name(const char * dll_name) {}

#endif // LLVM_ON_WIN32
