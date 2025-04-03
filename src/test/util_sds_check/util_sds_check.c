// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2025, Hewlett Packard Enterprise Development LP */

/*
 * util_sds_check.c -- unit test for shutdown_state_check function
 */

#include <stdbool.h>
#include "unittest.h"
#include "shutdown_state.h"
#include "set.h"

void
ut_fatal(const char *file, int line, const char *func,
    const char *fmt, ...)
{
	exit(-103);
}

void
ut_out(const char *file, int line, const char *func,
    const char *fmt, ...)
{
	;
}

int os_part_deep_common(struct pool_replica *rep, unsigned partidx, void *addr,
	size_t len, int flush);
/*
 * os_part_deep_common -- XXX temporally workaround until we will have pmem2
 *		integrated with common
 */
int
os_part_deep_common(struct pool_replica *rep, unsigned partidx, void *addr,
	size_t len, int flush)
{
	return 0;
}

int
pmem2_source_delete(struct pmem2_source **src)
{
	ASSERT(false);
	return PMEM2_E_NOSUPP;
}

int
pmem2_source_from_fd(struct pmem2_source **src, int fd)
{
	ASSERT(false);
	return PMEM2_E_NOSUPP;
}

#define DIRTY_SET 1
#define DIRTY_CLEAR 0

#define SDS_CUSTOM(DIRTY_BIT) \
{ \
	.usc = 1, \
	.uuid = 1, \
	.dirty = DIRTY_BIT, \
	.reserved = {0}, \
	.checksum = 43 \
}

#define SDS_ZERO {0}

#define SET_SDS(DST, VALUE) \
do { \
	struct shutdown_state sds_src = VALUE; \
	(void) memcpy(&DST, &sds_src, sizeof(struct shutdown_state)); \
} while(0)

/* pool_sds - SDS read from the pool header (persistent) */
static struct shutdown_state Pool_sds;
/*
 * SDS calculated ad hoc considering all the involved PMem DIMMs their UUIDs and
 * USCs (volatile)
 */
static struct shutdown_state Curr_sds;
static struct pool_replica Rep;

FUNC_MOCK(shutdown_state_reinit2, int, struct shutdown_state *curr_sds,
	struct shutdown_state *pool_sds, struct pool_replica *rep)
	FUNC_MOCK_RUN(UINT_MAX) {
		return _FUNC_REAL(shutdown_state_reinit2)(curr_sds, pool_sds, rep);
	}
FUNC_MOCK_RUN_DEFAULT {
	ASSERTeq(curr_sds, &Curr_sds);
	ASSERTeq(pool_sds, &Pool_sds);
	ASSERTeq(rep, &Rep);
	return 0;
}
FUNC_MOCK_END

FUNC_MOCK(shutdown_state_init, int, struct shutdown_state *sds, struct pool_replica *rep)
FUNC_MOCK_RUN_DEFAULT {
	UT_ASSERT(false);
	return -101;
}
FUNC_MOCK_END

FUNC_MOCK(util_checksum, int, void *addr, size_t len, uint64_t *csump,
	int insert, size_t skip_off)
FUNC_MOCK_RUN_DEFAULT {
	return _FUNC_REAL(util_checksum)(addr, len, csump, insert, skip_off);
}
FUNC_MOCK_END

/* Test with curr_sds having non-zero values and pool_sds being zeroed out. */
static int
test_case_1()
{
	SET_SDS(Pool_sds, SDS_ZERO);
	SET_SDS(Curr_sds, SDS_CUSTOM(DIRTY_CLEAR));

	int ret = shutdown_state_check(&Curr_sds, &Pool_sds, &Rep);
	ASSERTeq(ret, 0);
	ASSERTeq(RCOUNTER(shutdown_state_reinit2), 1);

	return ret;
}

/*
 * Test with curr_sds and pool_sds having the same values
 * except for the dirty field.
 */
static int
test_case_2()
{
	SET_SDS(Curr_sds, SDS_CUSTOM(DIRTY_CLEAR));
	SET_SDS(Curr_sds, SDS_CUSTOM(DIRTY_SET));

	int ret = shutdown_state_check(&Curr_sds, &Pool_sds, &Rep);
	ASSERTeq(ret, 0);
	ASSERTeq(RCOUNTER(shutdown_state_reinit2), 1);

	return ret;
}

/* Tests with curr_sds and pool_sds having identical values. */
static int
test_case_3()
{
	SET_SDS(Curr_sds, SDS_CUSTOM(DIRTY_SET));
	SET_SDS(Pool_sds, SDS_CUSTOM(DIRTY_SET));

	int ret = shutdown_state_check(&Curr_sds, &Pool_sds, &Rep);
	ASSERTeq(ret, 0);
	ASSERTeq(RCOUNTER(shutdown_state_reinit2), 1);

	return ret;
}

static struct test_case test_cases[] = {
	TEST_CASE(test_case_1),
	TEST_CASE(test_case_2),
	TEST_CASE(test_case_3),
};

#define NTESTS ARRAY_SIZE(test_cases)

static void
log_function(enum core_log_level level, const char *file_name,
	unsigned line_no, const char *function_name, const char *message)
{
	UT_OUT(message);
}

int
main(int argc, char *argv[])
{
	// START(argc, argv, "util_sds_check");
	core_log_set_function(log_function);
	TEST_CASE_PROCESS(argc, argv, test_cases, NTESTS);
	// DONE(NULL);
}
