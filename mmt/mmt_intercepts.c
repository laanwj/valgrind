/* intercepts for libGAL checking */

#include "pub_tool_basics.h"
#include "pub_tool_redir.h"
#include "pub_tool_clreq.h"
#include "mmt.h"
#include "config.h"

#include <stdio.h>

#define TRACE_MMT_FNS 0
#define  VG_Z_LIBGAL_SONAME         libGALZdso
#define  VG_U_LIBGAL_SONAME         "libGAL.so"
#define MMT_FUNC(ret_ty, f, args...) \
   ret_ty I_WRAP_SONAME_FNNAME_ZZ(VG_Z_LIBGAL_SONAME,f)(args); \
   ret_ty I_WRAP_SONAME_FNNAME_ZZ(VG_Z_LIBGAL_SONAME,f)(args)

static void mmt_init(void) __attribute__((constructor));
static void mmt_init(void)
{
    printf("mmt_init\n");
}

struct temp_cmdbuf
{
    unsigned int offset; /* +0x0 */
    void *buffer; /* +0x4 */
};

/* ugly and completely thread-unsafe */
static struct temp_cmdbuf *tmpbuf;

__attribute__((noinline))
static int gcoBUFFER_StartTEMPCMDBUF_WRK(void *buf, void **out)
{
   int ret;
   OrigFn fn;
   VALGRIND_GET_ORIG_FN(fn);
   if (TRACE_MMT_FNS) {
      fprintf(stderr, "<< gcoBUFFER_StartTEMPCMDBUF wrapper"); fflush(stderr);
   }

   CALL_FN_W_WW(ret, fn, buf, out);
   if (ret == 0 && *out) {
       tmpbuf = (struct temp_cmdbuf *)*out;
       VALGRIND_DO_CLIENT_REQUEST_STMT(_VG_USERREQ__MMT_START_TEMPCMDBUF, tmpbuf->buffer, tmpbuf->offset,0,0,0);
   }
   if (TRACE_MMT_FNS) {
      fprintf(stderr, " :: gcoBUFFER_StartTEMPCMDBUF -> %d >>\n", ret);
   }
   return ret;
}

MMT_FUNC(int, gcoBUFFERZuStartTEMPCMDBUF,
        void *buf, void **out) {
  return gcoBUFFER_StartTEMPCMDBUF_WRK(buf, out);
}

__attribute__((noinline))
static int gcoBUFFER_EndTEMPCMDBUF_WRK(void *buf)
{
   int ret;
   OrigFn fn;
   VALGRIND_GET_ORIG_FN(fn);
   if (TRACE_MMT_FNS) {
      fprintf(stderr, "<< gcoBUFFER_EndTEMPCMDBUF wrapper"); fflush(stderr);
   }
   if (tmpbuf) {
     VALGRIND_DO_CLIENT_REQUEST_STMT(_VG_USERREQ__MMT_END_TEMPCMDBUF, tmpbuf->buffer, tmpbuf->offset,0,0,0);
   }
   tmpbuf = 0;
   CALL_FN_W_W(ret, fn, buf);
   if (TRACE_MMT_FNS) {
      fprintf(stderr, " :: gcoBUFFER_EndTEMPCMDBUF -> %d >>\n", ret);
   }
   return ret;
}

MMT_FUNC(int, gcoBUFFERZuEndTEMPCMDBUF,
        void *buf) {
  return gcoBUFFER_EndTEMPCMDBUF_WRK(buf);
}

/*----------------------------------------------------------------*/
/*--- Replacements for basic string functions, that don't      ---*/
/*--- overrun the input arrays.                                ---*/
/*----------------------------------------------------------------*/

#include "../shared/vg_replace_strmem.c"
