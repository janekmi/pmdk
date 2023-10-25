#!/bin/bash

cd libpmem

COMMON=../common
CORE=../core
PMEM2=../libpmem2
OS_DIMM=ndctl

### Required headers

HEADERS="
	$CORE/pmemcore.h \
	$COMMON/pmemcommon.h \
"

SOURCE="$SOURCE $HEADERS"

### libpmem/Makefile

SOURCE="\
        $CORE/alloc.c \
	$CORE/fs_posix.c \
	$CORE/os_posix.c \
	$CORE/os_thread_posix.c \
	$CORE/out.c \
	$CORE/util.c \
	$CORE/util_posix.c \
	$COMMON/file.c \
	$COMMON/file_posix.c \
	$COMMON/mmap.c \
	$COMMON/mmap_posix.c \
	$COMMON/os_deep_linux.c \
	libpmem.c \
	$PMEM2/memops_generic.c \
	pmem.c \
	pmem_posix.c \
	$PMEM2/pmem2_utils.c \
	$PMEM2/config.c \
	$PMEM2/persist_posix.c \
	$PMEM2/source.c \
	$PMEM2/source_posix.c"

SOURCE="$SOURCE \
	$PMEM2/pmem2_utils_linux.c \
	$PMEM2/pmem2_utils_$OS_DIMM.c \
	$PMEM2/auto_flush_linux.c \
	$PMEM2/deep_flush_linux.c"

SOURCE="$SOURCE
	$PMEM2/region_namespace_ndctl.c \
	$PMEM2/numa_ndctl.c"

### libpmem2/x86_64/sources.inc

ARCH=../libpmem2/x86_64

LIBPMEM2_ARCH_SOURCE="
        $ARCH/init.c \
	$ARCH/cpu.c \
	$ARCH/memcpy/memcpy_nt_avx.c \
	$ARCH/memcpy/memcpy_nt_sse2.c \
	$ARCH/memcpy/memcpy_t_avx.c \
	$ARCH/memcpy/memcpy_t_sse2.c \
	$ARCH/memset/memset_nt_avx.c \
	$ARCH/memset/memset_nt_sse2.c \
	$ARCH/memset/memset_t_avx.c \
	$ARCH/memset/memset_t_sse2.c"

LIBPMEM2_ARCH_SOURCE="$LIBPMEM2_ARCH_SOURCE
	$ARCH/memcpy/memcpy_nt_avx512f.c \
	$ARCH/memcpy/memcpy_t_avx512f.c \
	$ARCH/memset/memset_nt_avx512f.c \
	$ARCH/memset/memset_t_avx512f.c"

LIBPMEM2_ARCH_SOURCE="$LIBPMEM2_ARCH_SOURCE
	$ARCH/memcpy/memcpy_nt_movdir64b.c \
	$ARCH/memset/memset_nt_movdir64b.c"

SOURCE="$SOURCE $LIBPMEM2_ARCH_SOURCE"

###

# for src in $SOURCE; do
#         echo "> $src"
# done

###

# PREPROCESS='cc -c -I../libpmem2 -I../core -E'
# PREPROCESS='cc -D_FORTIFY_SOURCE=2 -DAVX512F_AVAILABLE=1 -DMOVDIR64B_AVAILABLE=1 -DNDCTL_ENABLED=1 -DSDS_ENABLED -DSTRINGOP_TRUNCATION_SUPPORTED -fno-common -fno-lto -std=gnu99 -U_FORTIFY_SOURCE -Wall -Wcast-function-type -Wconversion -Werror -Wfloat-equal -Wmissing-field-initializers -Wmissing-prototypes -Wpointer-arith -Wsign-compare -Wsign-conversion -Wswitch-default -Wunused-macros -Wunused-parameter -I. -I../../src/../src/libpmem2 -I../../src/../src/libpmem2/x86_64 -I../common/ -I../core/ -I../include -E'
# PREPROCESS='--preprocess="cc -E"'

IGNORE_STR=
# for ignore in __extension__ __endptr __leaf__ __pure__ __artificial__ __restrict __base __nothrow__ __buffer __result __n __src; do
# for ignore in inline __extension__ __leaf__ __endptr __restrict __base __nothrow__ __buffer __result __n  __src __buf __c __s2 __delim __format; do
# 	IGNORE_STR="$IGNORE_STR -D$ignore"
# done

cflow --all $IGNORE_STR -o cflow.txt  $SOURCE 2> cflow_err.txt
