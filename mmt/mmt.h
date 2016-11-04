#ifndef __MMT_H
#define __MMT_H

#include "valgrind.h"

/* !! ABIWARNING !! ABIWARNING !! ABIWARNING !! ABIWARNING !!
   This enum comprises an ABI exported by Valgrind to programs
   which use client requests.  DO NOT CHANGE THE ORDER OF THESE
   ENTRIES, NOR DELETE ANY -- add new ones at the end. */
typedef enum {
    _VG_USERREQ__MMT_START_TEMPCMDBUF = VG_USERREQ_TOOL_BASE('M','T'),
    _VG_USERREQ__MMT_END_TEMPCMDBUF,
} Mmt_TCheckClientRequest;

#endif /* __HELGRIND_H */

