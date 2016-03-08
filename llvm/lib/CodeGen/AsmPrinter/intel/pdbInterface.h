/**
***  Copyright  (C) 2015 Intel Corporation. All rights reserved.
***
***  The information and source code contained herein is the exclusive
***  property of Intel Corporation and may not be disclosed, examined
***  or reproduced in whole or in part without explicit written authorization
***  from the company.
**/

#ifndef __PDB_INTERFACE_H__
#define __PDB_INTERFACE_H__
#include <stddef.h>

// Open the pdb file from the given file name
// Do initialization as required
// return TRUE if successful

typedef unsigned long long PDB_ULL;
typedef int dgi_bool;

extern dgi_bool pdb_open(const char *name);

//extern PDB_ULL pdb_get_version(void);

//extern PDB_ULL pdb_get_signature(void);

//      Close the pdb
//      If error_close is TRUE we just do a simple close
//      If it is false we check the validity of the
//      pdb by reopening it and making sure we can read something 
//      from the Tpi and then work-around any corruption that
//      occured on the close (this is a work-around for a known 
//      MS pdb writer bug)

extern void pdb_close();

//      Write an id. Returns TRUE is successful
//      assigned_index, of not NULLP gets index assigned
//      buf is bytes for id, with length on front

extern dgi_bool pdb_write_id(const char * buf, 
                        unsigned long 
                        *assigned_index);

//      Write a type. Returns TRUE is successful
//      assigned_index, of not NULLP gets index assigned
//      buf is bytes for type, with length on front

extern dgi_bool pdb_write_type(const char * buf, 
                        unsigned long 
                        *assigned_index);

extern size_t  pdb_get_signature(unsigned char * buf, size_t maxlen);
extern size_t  pdb_get_age(unsigned char *buf, size_t maxlen);

//  Set the default name of the pdb dll to load
//  We will attempt to use this one first before trying the
//  inbuilt list
extern void pdb_set_default_dll_name(const char * dll_name);


extern void pdb_free_path(const char *path);

#endif
