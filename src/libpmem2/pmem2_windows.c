/*
 * Copyright 2019, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmem2_windows.c -- pmem2 entry points for libpmem2 (Windows)
 */

#include "libpmem2.h"
#include "pmem2.h"
#include "pmem2_utils.h"

/*
 * pmem2_map -- XXX
 */
int
pmem2_map(struct pmem2_config *cfg, struct pmem2_map **mapp)
{
	int ret = PMEM2_E_OK;
	struct pmem2_map *map;
	map = (struct pmem2_map *)pmem2_zalloc(&ret, sizeof(*map));
	if (!map)
		ret;
	map->cfg = (struct pmem2_config *)pmem2_zalloc(&ret, sizeof(*map->cfg));
	if (!map->cfg) {
		goto err_free_map;
	}
	memcpy(map->cfg, cfg, sizeof(*cfg));
	map->addr = NULL;
	*mapp = map;

	DWORD access = FILE_MAP_ALL_ACCESS;
	size_t max_size = cfg->length + cfg->offset;
	HANDLE fh = CreateFileMapping(cfg->handle,
		NULL, /* security attributes */
		PAGE_READWRITE,
		(DWORD)(max_size >> 32),
		(DWORD)(max_size & 0xFFFFFFFF),
		NULL);

	if (!(fh) || fh == INVALID_HANDLE_VALUE) {
		ret = GetLastError();
		if (ret == ERROR_ACCESS_DENIED) {
			fh = CreateFileMapping(cfg->handle,
				NULL, /* security attributes */
				PAGE_READONLY,
				(DWORD)(max_size >> 32),
				(DWORD)(max_size & 0xFFFFFFFF),
				NULL);
			access = FILE_MAP_READ;
		}
		if (!(fh) || fh == INVALID_HANDLE_VALUE) {
			ret = PMEM2_E_INVALID_HANDLE;
			goto err_free_map_cfg;
		}
	}

	void *base = MapViewOfFileEx(fh,
		access,
		(DWORD)(cfg->offset >> 32),
		(DWORD)(cfg->offset & 0xFFFFFFFF),
		cfg->length,
		(*mapp)->addr); /* hint address */

	if (base == NULL) {
		ret = PMEM2_E_MAP_FAILED;
		goto err_free_map_cfg;
	}

	return ret;

err_free_map_cfg:
	free(map->cfg);
err_free_map:
	free(map);
	return ret;
}
