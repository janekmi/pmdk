# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2025, Hewlett Packard Enterprise Development LP

#
# src/test/util_sds_check/Makefile -- shutdown_state_check unit test
#
TOP = ../../..
vpath %.c $(TOP)/src/common/
vpath %.c $(TOP)/src/core/
vpath %.c $(TOP)/src/libpmem2/

BUILD_STATIC_DEBUG=n
BUILD_STATIC_NONDEBUG=n

TARGET = util_sds_check
OBJS = util_sds_check.o\
	shutdown_state.o\
	usc_none.o\
	util.o\
	alloc.o os_posix.o os_thread_posix.o

include ../Makefile.inc

CFLAGS += -DDEBUG -DNONSTATIC_REINIT

LDFLAGS += $(call extract_funcs, util_sds_check.c)
