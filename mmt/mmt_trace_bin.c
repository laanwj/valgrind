/*
   Copyright (C) 2006 Dave Airlie
   Copyright (C) 2007 Wladimir J. van der Laan
   Copyright (C) 2009, 2011, 2014 Marcin Slusarz <marcin.slusarz@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "pub_tool_debuginfo.h"
#include "pub_tool_vkiscnums.h"
#include "coregrind/pub_core_syscall.h"
#include "coregrind/pub_core_threadstate.h"
#include "coregrind/pub_core_stacktrace.h"

#include "mmt_trace_bin.h"

#define NUM_IPS 16
static void mydescribe(Addr inst_addr, char *namestr, int len)
{
/*
	const char* filename;
	UInt line = 0;
	if (VG_(get_filename)(inst_addr, &filename))
	{
		VG_(get_linenum)(inst_addr, &line);
		VG_(snprintf) (namestr, len, "@%08lx (%s:%d)", inst_addr, filename, line);
	}
	else
		VG_(snprintf) (namestr, len, "@%08lx", inst_addr);
*/
        //const HChar* description = VG_(describe_IP)(inst_addr, NULL);
        //VG_(strncpy)(namestr, description, len);
        // Collect first part of stacktrace
        Addr ips[NUM_IPS];
        int num_ips = VG_(get_StackTrace)(VG_(running_tid), ips, NUM_IPS, NULL, NULL, 0);
        int i;
        int ofs = 0;
        len -= 1;
        for (i=0; i<num_ips && ofs<len; ++i)
        {
            //VG_(sprintf(&namestr[i*9], "%08x ", (unsigned int)ips[i]));
            const HChar* description = VG_(describe_IP)(ips[i], NULL);
            int l = VG_(strlen)((const char*)description);
            if ((ofs + l)>=len)
                break;
            VG_(strncpy)(&namestr[ofs], description, len - ofs);
            ofs += l;
            namestr[ofs++] = '\n';
        }
        if (ofs)
            namestr[ofs-1] = 0;
        else
            namestr[0] = 0;
}

#define print_begin(type) do { \
		mmt_bin_write_1(type); \
		mmt_bin_write_4(region->id); \
		mmt_bin_write_4((UInt)(addr - region->start)); \
	} while (0)

#define print_begin2(type) do { \
		mmt_bin_write_1(type); \
		mmt_bin_write_8(addr); \
	} while (0)

#define print_store_begin() do { \
		if (all_mem) \
			print_begin2('W'); \
		else \
			print_begin('w'); \
	} while (0)

#define print_load_begin() do { \
		if (all_mem) \
			print_begin2('R'); \
		else \
			print_begin('r'); \
	} while (0)

#define print_info(type, namestr) do { \
		mmt_bin_write_1(type); \
		print_str(namestr); \
		mmt_bin_end(); \
	} while (0)

#define print_load_info() \
		print_info('s', namestr)

#define print_store_info() \
		print_info('x', namestr)

#define print_1(value) \
		mmt_bin_write_1(1); \
		mmt_bin_write_1(value);

#define print_2(value) \
		mmt_bin_write_1(2); \
		mmt_bin_write_2(value);

#define print_4(value) \
		mmt_bin_write_1(4); \
		mmt_bin_write_4(value);

#define print_4_4(value1, value2) \
		mmt_bin_write_1(8); \
		mmt_bin_write_4(value2); \
		mmt_bin_write_4(value1);

#define print_4_4_4_4(value1, value2, value3, value4) \
		mmt_bin_write_1(16); \
		mmt_bin_write_4(value4); \
		mmt_bin_write_4(value3); \
		mmt_bin_write_4(value2); \
		mmt_bin_write_4(value1);

#define print_8(value) \
		mmt_bin_write_1(8); \
		mmt_bin_write_8(value);

#define print_8_8(value1, value2) \
		mmt_bin_write_1(16); \
		mmt_bin_write_8(value2); \
		mmt_bin_write_8(value1);

#define print_8_8_8_8(value1, value2, value3, value4) \
		mmt_bin_write_1(32); \
		mmt_bin_write_8(value4); \
		mmt_bin_write_8(value3); \
		mmt_bin_write_8(value2); \
		mmt_bin_write_8(value1);

#define print_str(str) mmt_bin_write_str(str)

#define print_store_end() do { mmt_bin_end(); mmt_bin_sync(); } while (0)
#define print_load_end() do { mmt_bin_end(); mmt_bin_sync(); } while (0)

VG_REGPARM(2)
void mmt_trace_store_bin_1(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_1(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_1_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_1(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_2(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_2(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_2_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_2(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_4(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_4(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_4_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_4(value);
	print_store_end();
}

#ifdef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_store_bin_8(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_8(value);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_8_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_8(value);
	print_store_end();
}
#endif

VG_REGPARM(2)
void mmt_trace_store_bin_4_4(Addr addr, UWord value1, UWord value2)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_4_4(value1, value2);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_4_4_ia(Addr addr, UWord value1, UWord value2, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_4_4(value1, value2);
	print_store_end();
}

#ifdef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_store_bin_8_8(Addr addr, UWord value1, UWord value2)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_8_8(value1, value2);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_8_8_ia(Addr addr, UWord value1, UWord value2, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_8_8(value1, value2);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_8_8_8_8(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_8_8_8_8(value1, value2, value3, value4);
	print_store_end();
}

VG_REGPARM(2)
void mmt_trace_store_bin_8_8_8_8_ia(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4, UWord inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_8_8_8_8(value1, value2, value3, value4);
	print_store_end();
}
#endif

#ifndef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_store_bin_4_4_4_4(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_store_begin();
	print_4_4_4_4(value1, value2, value3, value4);
	print_store_end();
}
VG_REGPARM(2)
void mmt_trace_store_bin_4_4_4_4_ia(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_store_info();
	print_store_begin();
	print_4_4_4_4(value1, value2, value3, value4);
	print_store_end();
}
#endif

VG_REGPARM(2)
void mmt_trace_load_bin_1(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_1(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_1_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_1(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_2(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_2(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_2_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_2(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_4(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_4(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_4_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_4(value);
	print_load_end();
}

#ifdef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_load_bin_8(Addr addr, UWord value)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_8(value);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_8_ia(Addr addr, UWord value, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_8(value);
	print_load_end();
}
#endif

VG_REGPARM(2)
void mmt_trace_load_bin_4_4(Addr addr, UWord value1, UWord value2)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_4_4(value1, value2);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_4_4_ia(Addr addr, UWord value1, UWord value2, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_4_4(value1, value2);
	print_load_end();
}

#ifdef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_load_bin_8_8(Addr addr, UWord value1, UWord value2)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_8_8(value1, value2);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_8_8_ia(Addr addr, UWord value1, UWord value2, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_8_8(value1, value2);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_8_8_8_8(Addr addr, UWord value1, UWord value2, UWord value3, UWord value4)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_8_8_8_8(value1, value2, value3, value4);
	print_load_end();
}

VG_REGPARM(2)
void mmt_trace_load_bin_8_8_8_8_ia(Addr addr, UWord value1, UWord value2, UWord value3, UWord value4, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_8_8_8_8(value1, value2, value3, value4);
	print_load_end();
}
#endif

#ifndef MMT_64BIT
VG_REGPARM(2)
void mmt_trace_load_bin_4_4_4_4(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4)
{
	struct mmt_mmap_data *region = NULL;

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	print_load_begin();
	print_4_4_4_4(value1, value2, value3, value4);
	print_load_end();
}
VG_REGPARM(2)
void mmt_trace_load_bin_4_4_4_4_ia(Addr addr, UWord value1, UWord value2,
		UWord value3, UWord value4, Addr inst_addr)
{
	struct mmt_mmap_data *region = NULL;
	char namestr[256];

	if (LIKELY(!all_mem))
	{
		region = find_mmap(addr);
		if (LIKELY(!region))
			return;
	}

	mydescribe(inst_addr, namestr, 256);

	print_load_info();
	print_load_begin();
	print_4_4_4_4(value1, value2, value3, value4);
	print_load_end();
}
#endif

#define BUF_SIZE 64 * 1024
static char buffer[BUF_SIZE];
static int written = 0;

void mmt_bin_flush(void)
{
	VG_(write)(VG_(log_output_sink).fd, buffer, written);
	written = 0;
}

void mmt_bin_flush_and_sync(void)
{
	mmt_bin_flush();
	VG_(do_syscall1)(__NR_fdatasync, VG_(log_output_sink).fd);
}

void mmt_bin_write_1(UChar u8)
{
	if (BUF_SIZE - written < 1)
		mmt_bin_flush();
	VG_(memcpy)(buffer + written, &u8, 1);
	written++;
}
void mmt_bin_write_2(UShort u16)
{
	if (BUF_SIZE - written < 2)
		mmt_bin_flush();
	VG_(memcpy)(buffer + written, &u16, 2);
	written += 2;
}
void mmt_bin_write_4(UInt u32)
{
	if (BUF_SIZE - written < 4)
		mmt_bin_flush();
	VG_(memcpy)(buffer + written, &u32, 4);
	written += 4;
}
void mmt_bin_write_8(ULong u64)
{
	if (BUF_SIZE - written < 8)
		mmt_bin_flush();
	VG_(memcpy)(buffer + written, &u64, 8);
	written += 8;
}
#define MIN(a, b) ((a) < (b) ? (a) : (b))
void mmt_bin_write_str(const char *str)
{
	UInt len = VG_(strlen)(str) + 1;
	mmt_bin_write_4(len);

	do
	{
		if (BUF_SIZE - written < len)
			mmt_bin_flush();
		int cur = MIN(BUF_SIZE - written, len);
		VG_(memcpy)(buffer + written, str, cur);
		written += cur;
		len -= cur;
		str += cur;
	}
	while (len > 0);
}
void mmt_bin_write_buffer(const UChar *buf, int len)
{
	mmt_bin_write_4(len);
	if (len == 0)
		return;

	do
	{
		if (BUF_SIZE - written < len)
			mmt_bin_flush();
		int cur = MIN(BUF_SIZE - written, len);
		VG_(memcpy)(buffer + written, buf, cur);
		written += cur;
		len -= cur;
		buf += cur;
	}
	while (len > 0);
}
