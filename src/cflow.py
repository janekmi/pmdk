#!/usr/bin/env python3

import re
import json

STACK_USAGE_FILE='stats/stack-usage-nondebug.txt'
CALL_STACK_FILE='cflow.txt'
NOT_CALLED_WHITELIST_FILE='libpmemobj/not_called_whitelist.txt'

API = [
        'libpmemobj_init',
        'pmem_init',
        'pmemobj_alloc_usable_size',
        'pmemobj_alloc',
        'pmemobj_cancel',
        'pmemobj_check_version',
        'pmemobj_check',
        'pmemobj_close',
        'pmemobj_cond_broadcast',
        'pmemobj_cond_signal',
        'pmemobj_cond_wait',
        'pmemobj_cond_zero',
        'pmemobj_create',
        'pmemobj_ctl_exec',
        'pmemobj_ctl_get',
        'pmemobj_ctl_set',
        'pmemobj_defer_free',
        'pmemobj_defrag',
        'pmemobj_direct',
        'pmemobj_drain',
        'pmemobj_errormsg',
        'pmemobj_first',
        'pmemobj_flush',
        'pmemobj_free',
        'pmemobj_get_user_data',
        'pmemobj_list_insert',
        'pmemobj_list_move',
        'pmemobj_list_remove',
        'pmemobj_memcpy_persist',
        'pmemobj_memcpy',
        'pmemobj_memmove',
        'pmemobj_memset_persist',
        'pmemobj_memset',
        'pmemobj_mutex_timedlock',
        'pmemobj_mutex_trylock',
        'pmemobj_mutex_zero',
        'pmemobj_next',
        'pmemobj_oid',
        'pmemobj_open',
        'pmemobj_persist',
        'pmemobj_publish',
        'pmemobj_realloc',
        'pmemobj_reserve',
        'pmemobj_root_size',
        'pmemobj_root',
        'pmemobj_rwlock_rdlock',
        'pmemobj_rwlock_timedrdlock',
        'pmemobj_rwlock_timedwrlock',
        'pmemobj_rwlock_tryrdlock',
        'pmemobj_rwlock_trywrlock',
        'pmemobj_rwlock_zero',
        'pmemobj_set_funcs',
        'pmemobj_set_user_data',
        'pmemobj_set_value',
        'pmemobj_strdup',
        'pmemobj_tx_abort',
        'pmemobj_tx_add_range_direct',
        'pmemobj_tx_add_range',
        'pmemobj_tx_begin',
        'pmemobj_tx_commit',
        'pmemobj_tx_end',
        'pmemobj_tx_errno',
        'pmemobj_tx_free',
        'pmemobj_tx_get_failure_behavior',
        'pmemobj_tx_get_user_data',
        'pmemobj_tx_lock',
        'pmemobj_tx_log_append_buffer',
        'pmemobj_tx_log_auto_alloc',
        'pmemobj_tx_log_intents_max_size',
        'pmemobj_tx_log_snapshots_max_size',
        'pmemobj_tx_process',
        'pmemobj_tx_publish',
        'pmemobj_tx_realloc',
        'pmemobj_tx_set_failure_behavior',
        'pmemobj_tx_set_user_data',
        'pmemobj_tx_stage',
        'pmemobj_tx_strdup',
        'pmemobj_tx_wcsdup',
        'pmemobj_tx_xadd_range_direct',
        'pmemobj_tx_xadd_range',
        'pmemobj_tx_xalloc',
        'pmemobj_tx_xlock',
        'pmemobj_tx_xlog_append_buffer',
        'pmemobj_tx_xstrdup',
        'pmemobj_tx_xwcsdup',
        'pmemobj_tx_zalloc',
        'pmemobj_tx_zrealloc',
        'pmemobj_type_num',
        'pmemobj_volatile',
        'pmemobj_xreserve',
        'pmemobj_tx_alloc',
        'pmemobj_wcsdup',
        'pmemobj_xalloc',
        'pmemobj_zalloc',
        'pmemobj_zrealloc',
]

DEAD_END = [
        'prealloc'
]

def dump(var, name):
        with open(f"{name}.json", "w") as outfile: 
                json.dump(var, outfile, indent = 4)

def load_stack_usage():
        funcs = {}
        with open(STACK_USAGE_FILE, 'r') as file:
                for line in file:
                        # 8432 out_common : src/nondebug/libpmem/out.su:out.c dynamic,bounded
                        found = re.search("([0-9]+) ([a-zA-Z0-9_]+)(.[a-z0-9.]+)* : [a-z0-9.:/_]+ ([a-z,]+)", line)
                        # print('{} {} {}'.format(found.group(1), found.group(2), found.group(3)))
                        if found:
                                funcs[found.group(2)] = {'size': found.group(1), 'type': found.group(4)}
                        else:
                                # An unexpected line format
                                print(line)
                                exit(1)
        return funcs

def load_call_stack():
        calls = {}
        callers = []
        with open(CALL_STACK_FILE, 'r') as file:
                for line in file:
                        line_copy = line
                        level = 0
                        while line[0] == ' ':
                                level += 1
                                line = line[4:]
# pmem_memset_persist() <void *pmem_memset_persist (void *pmemdest, int c, size_t len) at pmem.c:731>:
                        found = re.search("^([a-zA-Z0-9_]+)\(\)", line)
                        if not found:
                                # An unexpected line format
                                print(line_copy)
                                exit(1)
                        func = found.group(1)
                        callers.insert(level, func)
                        if level == 0:
                                continue
                        callee = func
                        caller = callers[level - 1]
                        if caller == "pmem":
                                print(line_copy)
                                exit(1)
                        if caller in calls.keys():
                                calls[caller].append(callee)
                        else:
                                calls[caller] = [callee]
        # remove duplicates
        calls_unique = {}
        for k, v in calls.items():
                v_unique = list(set(v))
                calls_unique[k] = v_unique
        return calls_unique

def dict_extend(dict_, key, values):
        if key not in dict_.keys():
                dict_[key] = values
        else:
                dict_[key].extend(values)
        return dict_

def inlines(calls):
        calls['core_init'] = ['util_init', 'out_init']
        calls['core_fini'] = ['out_fini']
        calls['ERR'] = ['out_err']
        calls['Print'] = ['out_print_func']
        calls['common_init'] = ['core_init', 'util_mmap_init']
        calls['common_fini'] = ['util_mmap_fini', 'core_fini']
        calls['Last_errormsg_key_alloc'] = ['_Last_errormsg_key_alloc']
        calls['_Last_errormsg_key_alloc'] = ['os_once', 'os_tls_key_create']
        calls['flush_empty'] = ['flush_empty_nolog']

        calls = dict_extend(calls, 'libpmemobj_init', ['common_init'])
        calls = dict_extend(calls, 'out_common', ['out_snprintf'])
        calls = dict_extend(calls, 'run_vg_init', ['run_iterate_used'])

        calls = dict_extend(calls, 'palloc_heap_action_on_unlock', ['palloc_reservation_clear'])
        calls = dict_extend(calls, 'palloc_heap_action_on_cancel', ['palloc_reservation_clear'])

        calls = dict_extend(calls, 'util_uuid_generate', ['util_uuid_from_string'])

        return calls

def function_pointers(calls):
        # block_container_ops
        insert_all = ['container_ravl_insert_block', 'container_seglists_insert_block']
        get_rm_exact_all = ['container_ravl_get_rm_block_exact']
        get_rm_bestfit_all = ['container_ravl_get_rm_block_bestfit', 'container_seglists_get_rm_block_bestfit']
        is_empty_all = ['container_ravl_is_empty', 'container_seglists_is_empty']
        rm_all_all = ['container_ravl_rm_all', 'container_seglists_rm_all']
        destroy_all = ['container_ravl_destroy', 'container_seglists_destroy']

        calls = dict_extend(calls, 'bucket_insert_block', insert_all)

        calls = dict_extend(calls, 'bucket_fini', destroy_all)
        calls = dict_extend(calls, 'bucket_fini', destroy_all)

        # memory_block_ops
        block_size_all = ['huge_block_size', 'run_block_size']
        prep_hdr_all = ['huge_prep_operation_hdr', 'run_prep_operation_hdr']
        get_lock_all = ['huge_get_lock', 'run_get_lock']
        get_state_all = ['huge_get_state', 'run_get_state']
        get_user_data_all = ['block_get_user_data']
        get_real_data_all = ['huge_get_real_data', 'run_get_real_data']
        get_user_size_all = ['block_get_user_size']
        get_real_size_all = ['block_get_real_size']
        write_header_all = ['block_write_header']
        invalidate_all = ['block_invalidate']
        ensure_header_type_all = ['huge_ensure_header_type', 'run_ensure_header_type']
        reinit_header_all = ['block_reinit_header']
        vg_init_all = ['huge_vg_init', 'run_vg_init']
        get_extra_all = ['block_get_extra']
        get_flags_all = ['block_get_flags']
        iterate_free_all = ['huge_iterate_free', 'run_iterate_free']
        iterate_used_all = ['huge_iterate_used', 'run_iterate_used']
        reinit_chunk_all = ['huge_reinit_chunk', 'run_reinit_chunk']
        calc_free_all = ['run_calc_free']
        get_bitmap_all = ['run_get_bitmap']
        fill_pct_all = ['huge_fill_pct', 'run_fill_pct']

        calls = dict_extend(calls, 'heap_free_chunk_reuse', prep_hdr_all)
        calls = dict_extend(calls, 'palloc_heap_action_exec', prep_hdr_all)

        calls = dict_extend(calls, 'alloc_prep_block', write_header_all)

        calls = dict_extend(calls, 'palloc_heap_action_on_cancel', invalidate_all)

        calls = dict_extend(calls, 'heap_get_bestfit_block', ensure_header_type_all)

        calls = dict_extend(calls, 'palloc_vg_register_alloc', reinit_header_all)

        calls = dict_extend(calls, 'heap_vg_open', vg_init_all)

        calls = dict_extend(calls, 'bucket_attach_run', iterate_free_all)

        calls = dict_extend(calls, 'heap_zone_foreach_object', iterate_used_all)

        calls = dict_extend(calls, 'recycler_element_new', calc_free_all)

        # memblock_header_ops
        get_size_all = ['memblock_header_legacy_get_size', 'memblock_header_compact_get_size', 'memblock_header_none_get_size']
        get_extra_all = ['memblock_header_legacy_get_extra', 'memblock_header_compact_get_extra', 'memblock_header_none_get_extra']
        get_flags_all = ['memblock_header_legacy_get_flags', 'memblock_header_compact_get_flags', 'memblock_header_none_get_flags']
        write_all = ['memblock_header_legacy_write', 'memblock_header_compact_write', 'memblock_header_none_write']
        invalidate_all = ['memblock_header_legacy_invalidate', 'memblock_header_compact_invalidate', 'memblock_header_none_invalidate']
        reinit_all = ['memblock_header_legacy_reinit', 'memblock_header_compact_reinit', 'memblock_header_none_reinit']

        calls = dict_extend(calls, 'block_write_header', write_all)
        calls = dict_extend(calls, 'block_invalidate', invalidate_all)
        calls = dict_extend(calls, 'block_reinit_header', reinit_all)

        # action_funcs
        exec_all = ['palloc_heap_action_exec', 'palloc_mem_action_exec']
        on_cancel_all = ['palloc_heap_action_on_cancel', 'palloc_mem_action_noop']
        on_process_all = ['palloc_heap_action_on_process', 'palloc_mem_action_noop']
        on_unlock_all = ['palloc_heap_action_on_unlock', 'palloc_mem_action_noop']

        calls = dict_extend(calls, 'palloc_cancel', on_cancel_all)

        calls = dict_extend(calls, 'palloc_exec_actions', on_process_all + on_unlock_all)

        # DAOS used CTLs
        # pmemobj_ctl_get("stats.heap.curr_allocated") - just an 
        # pmemobj_ctl_get("stats.heap.run_allocated")
        # pmemobj_ctl_get("stats.heap.run_active")
        get_all = ['ctl__persistent_curr_allocated_read', 'ctl__transient_run_allocated_read', 'ctl__transient_run_active_read']
        calls = dict_extend(calls, 'ctl_exec_query_read', get_all)

        # pmemobj_ctl_set("heap.arenas_assignment_type")
        # pmemobj_ctl_set("heap.alloc_class.new.desc", &pmemslab);
        # pmemobj_ctl_set("stats.enabled", &enabled);
        # pmemobj_ctl_set("stats.enabled", &enabled);
        set_all = ['ctl__arenas_assignment_type_write', 'ctl__desc_write', 'ctl__enabled_write']
        calls = dict_extend(calls, 'ctl_exec_query_write', set_all)

        calls = dict_extend(calls, 'ctl_query', ['ctl_exec_query_read', 'ctl_exec_query_write'])
        
        return calls

def is_reachable(func, calls):
        callers = [func]
        while len(callers) > 0:
                callers_new = []
                for callee in callers:
                        for k, v in calls.items():
                                if callee not in v:
                                        continue
                                if k not in API:
                                        callers_new.append(k)
                                return True
                callers = callers_new
        print(func)
        return False

def api_callers(func, calls):
        callers = [func]
        visited = [func] # loop breaker
        apis = []
        while len(callers) > 0:
                callers_new = []
                for callee in callers:
                        for k, v in calls.items():
                                # this caller does not call this callee
                                if callee not in v:
                                        continue
                                # it is part of the API
                                if k in visited:
                                        continue
                                if k in API or k in DEAD_END:
                                        apis.append(k)
                                else:
                                        callers_new.append(k)
                                        visited.append(k)
                callers = list(set(callers_new))
                # print(callers)
                # if len(apis) > 0 and len(callers) > 0:
                #         exit(1)
        # if len(apis) == 0:
        #         print(func)
        # assert(len(apis) > 0)
        return apis

def validate(funcs, calls):
        all_callees = []
        for _, v in calls.items():
                all_callees.extend(v)
        all_callees = list(set(all_callees))
        # dump(all_callees, 'all_callees')

        with open(NOT_CALLED_WHITELIST_FILE, 'r') as file:
                whitelist = json.load(file)

        # All known functions are expected to be called at least once
        not_called = []
        for k, v in funcs.items():
                if k in all_callees:
                        continue
                if k in whitelist:
                        continue
                if k in API:
                        continue
                if int(v['size']) <= 128:
                        continue
                not_called.append(k)
        # dump(not_called, 'not_called')
        assert(len(not_called) == 0)

        # for callee in all_callees:
        #         assert(is_reachable(callee, calls))

        # All mem(move|set) functions are expected to be tracked back to pmem_mem* API calls
        no_api_connection = {}
        for k, v in funcs.items():
        # for k in ['prealloc']:
                if k in whitelist or k in DEAD_END or k in API:
                        continue
                if k in API:
                        continue
                # too complex, ignore
                # if k in ['out_common']:
                #         continue
                # print(k)
                callers = api_callers(k, calls)
                # valid = False
                # for caller in callers:
                #         if re.search("^pmem_mem", caller):
                #                 valid = True
                #                 break
                # if not valid:
                #         print(k)
                if int(v['size']) <= 32: # there is too many of them
                        continue
                if len(callers) == 0:
                        no_api_connection[k] = v['size']
        # dump(no_api_connection, 'no_api_connection')
        assert(len(no_api_connection) == 0)

def generate_call_stacks(func, funcs, rcalls):
        call_stacks = [
                {
                        'stack': [func],
                        'size': int(funcs[func]['size']) if func in funcs.keys() else 0
                }
        ]
        # the main loop
        while True:
                call_stacks_new = []
                call_stacks_new_end = []
                for call_stack in call_stacks:
                        callee = call_stack['stack'][0]
                        if callee in API:
                                call_stacks_new_end.append(call_stack)
                                continue
                        if callee not in rcalls.keys():
                                call_stacks_new_end.append(call_stack)
                                continue
                        for caller in rcalls[callee]:
                                if call_stack['stack'].count(caller) == 2:
                                        continue # loop breaker
                                if caller in funcs.keys():
                                        caller_stack_size = int(funcs[caller]['size'])
                                else:
                                        caller_stack_size = 0
                                call_stacks_new.append({
                                        'stack': [caller] + call_stack['stack'],
                                        'size': call_stack['size'] + caller_stack_size
                                })
                if len(call_stacks_new) == 0:
                        break
                call_stacks = call_stacks_new + call_stacks_new_end
        return call_stacks

# check if a is a substack of b
def is_substack(a, b):
        if len(a['stack']) >= len(b['stack']):
                return False
        for i in range(len(a['stack'])):
                if a['stack'][i] != b['stack'][i]:
                        return False
        return True

def call_stacks_reduce(call_stacks):
        ret_call_stacks = []
        for i in range(len(call_stacks)):
                a = call_stacks[i]
                substack = False
                for j in range(len(call_stacks)):
                        if i == j:
                                continue
                        b = call_stacks[j]
                        if is_substack(a, b):
                                substack = True
                                break
                if not substack:
                        ret_call_stacks.append(a)
        return ret_call_stacks

def call_stack_key(e):
        return e['size']

def generate_all_call_stacks(funcs, calls):
        with open(NOT_CALLED_WHITELIST_FILE, 'r') as file:
                whitelist = json.load(file)
        # preparing a reverse call dictionary
        rcalls = {}
        for caller, callees in calls.items():
                for callee in callees:
                        rcalls = dict_extend(rcalls, callee, [caller])
        # dump(rcalls, 'rcalls')
        print("Reverse call dictionary - done")

        call_stacks = []
        for func in rcalls.keys():
        # for func in ['VALGRIND_ANNOTATE_NEW_MEMORY']:
                if func == 'LOG':
                        continue
                if func in whitelist:
                        continue
                if func in calls.keys():
                        continue
                print(f"Generating call stacks ending at - {func}")
                call_stacks.extend(generate_call_stacks(func, funcs, rcalls))
        # call_stacks = call_stacks_reduce(call_stacks)
        print(len(call_stacks))
        call_stacks.sort(reverse=True, key=call_stack_key)
        dump(call_stacks, 'call_stacks_all')
        # XXX
        # call_stacks = list(filter(lambda call_stack: re.search('^pmem_(mem|persist|flush|drain)', call_stack['stack'][0]), call_stacks))
        # print(len(call_stacks))
        # call_stacks.sort(reverse=True, key=call_stack_key)
        # dump(call_stacks, 'call_stacks_pmem_ops')

def main():
        funcs = load_stack_usage()
        dump(funcs, 'funcs')
        calls = load_call_stack()
        calls = inlines(calls)
        calls = function_pointers(calls)
        dump(calls, 'calls')
        validate(funcs, calls)
        print("Validation - done")

        generate_all_call_stacks(funcs, calls)

if __name__ == '__main__':
        main()
