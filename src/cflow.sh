#!/bin/bash

COMMON=common
CORE=core
PMEM2=libpmem2
PMEMOBJ=libpmemobj
OS_DIMM=ndctl

### include ../core/pmemcore.inc

PMEMCORE_SOURCE=" \
	$CORE/alloc.c \
	$CORE/fs_posix.c \
	$CORE/membuf.c \
	$CORE/os_posix.c \
	$CORE/os_thread_posix.c \
	$CORE/out.c \
	$CORE/ravl.c \
	$CORE/ravl_interval.c \
	$CORE/util.c \
	$CORE/util_posix.c \
"
SOURCE="$SOURCE $PMEMCORE_SOURCE"

### include ../common/pmemcommon.inc

PMEMCOMMON_SOURCE=" \
	$COMMON/bad_blocks.c\
	$COMMON/set_badblocks.c\
	$COMMON/ctl.c\
	$COMMON/ctl_prefault.c\
	$COMMON/ctl_sds.c\
	$COMMON/ctl_fallocate.c\
	$COMMON/ctl_cow.c\
	$COMMON/file.c\
	$COMMON/file_posix.c\
	$COMMON/mmap.c\
	$COMMON/mmap_posix.c\
	$COMMON/os_deep_linux.c\
	$COMMON/pool_hdr.c\
	$COMMON/rand.c\
	$COMMON/set.c\
	$COMMON/shutdown_state.c\
	$COMMON/uuid.c\
	$PMEM2/pmem2_utils.c\
	$PMEM2/config.c\
	$PMEM2/persist_posix.c\
	$PMEM2/badblocks.c\
	$PMEM2/badblocks_ndctl.c\
	$PMEM2/usc_ndctl.c\
	$PMEM2/source.c\
	$PMEM2/source_posix.c
	$PMEM2/auto_flush_linux.c\
	$PMEM2/deep_flush_linux.c\
	$PMEM2/extent_linux.c\
	$PMEM2/pmem2_utils_linux.c\
	$PMEM2/pmem2_utils_ndctl.c
	$PMEM2/region_namespace_ndctl.c\
	$PMEM2/numa_ndctl.c \
"
# $(call osdep, $(COMMON)/uuid,.c)\

SOURCE="$SOURCE $PMEMCOMMON_SOURCE"

### libpmemobj/Makefile

PMEMOBJ_SOURCE=" \
	$PMEMOBJ/alloc_class.c \
	$PMEMOBJ/bucket.c \
	$PMEMOBJ/container_ravl.c \
	$PMEMOBJ/container_seglists.c \
	$PMEMOBJ/critnib.c \
	$PMEMOBJ/ctl_debug.o \
	$PMEMOBJ/heap.c \
	$PMEMOBJ/lane.c \
	$PMEMOBJ/libpmemobj.c \
	$PMEMOBJ/list.c \
	$PMEMOBJ/memblock.c \
	$PMEMOBJ/memops.c \
	$PMEMOBJ/obj.c \
	$PMEMOBJ/palloc.c \
	$PMEMOBJ/pmalloc.c \
	$PMEMOBJ/recycler.c \
	$PMEMOBJ/sync.c \
	$PMEMOBJ/tx.c \
	$PMEMOBJ/stats.c \
	$PMEMOBJ/ulog.c \
"

SOURCE="$SOURCE $PMEMOBJ_SOURCE"

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

DAOS_USES="
	pmemobj_alloc \
	pmemobj_cancel \
	pmemobj_close \
	pmemobj_create \
	pmemobj_ctl_get \
	pmemobj_ctl_set \
	pmemobj_defer_free \
	pmemobj_direct \
	pmemobj_errormsg \
	pmemobj_flush \
	pmemobj_free \
	pmemobj_memcpy_persist \
	pmemobj_open \
	pmemobj_reserve \
	pmemobj_root \
	pmemobj_tx_abort \
	pmemobj_tx_add_range \
	pmemobj_tx_add_range_direct \
	pmemobj_tx_begin \
	pmemobj_tx_commit \
	pmemobj_tx_end \
	pmemobj_tx_free \
	pmemobj_tx_publish \
	pmemobj_tx_stage \
	pmemobj_tx_xadd_range \
	pmemobj_tx_xalloc \
"

STARTS=
for start in $DAOS_USES; do
	STARTS="$STARTS --start $start"
done

cflow $STARTS $IGNORE_STR -o cflow.txt  $SOURCE 2> cflow_err.txt
