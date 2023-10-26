#!/usr/bin/env python3

import re
import json

STACK_USAGE_FILE='stats/stack-usage-filtered.txt'
CALL_STACK_FILE='libpmem/cflow.txt'
NOT_CALLED_WHITELIST_FILE='libpmem/libpmem_not_called_whitelist.json'

ALIASES = {'ERR': 'out_err'}
API = [
        'pmem_map_file',
        'pmem_unmap',
        'pmem_memset',
        'pmem_memmove',
        'pmem_memcpy',
        'pmem_memset_persist'
        'pmem_memmove_persist',
        'pmem_memcpy_persist',
        'pmem_memcpy_nodrain',
        "pmem_deep_persist",
        "pmem_persist",
        "pmem_check_version",
        "libpmem_init",
        "libpmem_fini",
        "pmem_has_hw_drain",
        "pmem_has_auto_flush",
        "pmem_errormsg",
        "pmem_memset_persist",
        "pmem_memmove_persist"
]

def dump(var, name):
        with open(f"{name}.json", "w") as outfile: 
                json.dump(var, outfile, indent = 4)

def load_stack_usage():
        funcs = {}
        with open(STACK_USAGE_FILE, 'r') as file:
                for line in file:
                        # 8432 out_common : src/nondebug/libpmem/out.su:out.c dynamic,bounded
                        found = re.search("([0-9]+) ([a-zA-Z0-9_.]+) : [a-z0-9.:/_]+ ([a-z,]+)", line)
                        # print('{} {} {}'.format(found.group(1), found.group(2), found.group(3)))
                        if found:
                                funcs[found.group(2)] = {'size': found.group(1), 'type': found.group(3)}
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
        return calls

def dict_extend(dict_, key, values):
        if key not in dict_.keys():
                dict_[key] = values
        else:
                dict_[key].extend(values)
        return dict_

def function_pointers(calls):
        fence_all = ['fence_empty', 'memory_barrier']
        calls = dict_extend(calls, 'pmem_drain', fence_all)
        
        flush_all = ['flush_empty', 'flush_clflush', 'flush_clflushopt', 'flush_clwb']
        calls = dict_extend(calls, 'pmem_deep_flush', flush_all)
        calls = dict_extend(calls, 'pmem_flush', flush_all)

        # '.static' suffix added to differentiate between libpmem API function and a static helper.
        memmove_nodrain_all = ['memmove_nodrain_libc', 'memmove_nodrain_generic', 'pmem_memmove_nodrain.static', 'pmem_memmove_nodrain_eadr.static']
        calls = dict_extend(calls, 'pmem_memmove', memmove_nodrain_all)
        calls = dict_extend(calls, 'pmem_memcpy', memmove_nodrain_all)
        calls = dict_extend(calls, 'pmem_memmove_nodrain', memmove_nodrain_all)
        calls = dict_extend(calls, 'pmem_memcpy_nodrain', memmove_nodrain_all)
        calls = dict_extend(calls, 'pmem_memmove_persist', memmove_nodrain_all)
        calls = dict_extend(calls, 'pmem_memcpy_persist', memmove_nodrain_all)

        memset_nodrain_all = ['memset_nodrain_libc', 'memset_nodrain_generic', 'pmem_memset_nodrain.static', 'pmem_memset_nodrain_eadr.static']
        calls = dict_extend(calls, 'pmem_memset', memset_nodrain_all)
        calls = dict_extend(calls, 'pmem_memset_nodrain', memset_nodrain_all)
        calls = dict_extend(calls, 'pmem_memset_persist', memset_nodrain_all)

        memmove_funcs = {
                't': {
                        func: [ f'memmove_mov_{trick}_{func}'
                                for trick in ['sse2', 'avx', 'avx512f']
                        ] for func in ['noflush', 'empty'] 
                },
                'nt': {
                        'empty': [ f'memmove_movnt_{trick}_empty_{drain}'
                                for trick in ['sse2', 'avx']
                                        for drain in ['wcbarrier', 'nobarrier'] 
                        ],
                        'flush': [ f'memmove_movnt_{trick}_{flush}_{drain}'
                                for trick in ['sse2', 'avx']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                                                for drain in ['wcbarrier', 'nobarrier']
                        ]
                }
        }
        memmove_funcs_extras = {
                't': {
                        'flush': [ f'memmove_mov_{trick}_{flush}'
                                for trick in ['sse2', 'avx', 'avx512f']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                        ]
                },
                'nt': {
                        'empty': [ f'memmove_movnt_{trick}_empty'
                                for trick in ['avx512f', 'movdir64b']
                        ],
                        'flush': [ f'memmove_movnt_{trick}_{flush}'
                                for trick in ['avx512f', 'movdir64b']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                        ]
                }
        }
        memmove_funcs['t']['flush'] = memmove_funcs_extras['t']['flush']
        memmove_funcs['nt']['empty'].extend(memmove_funcs_extras['nt']['empty'])
        memmove_funcs['nt']['flush'].extend(memmove_funcs_extras['nt']['flush'])

        calls = dict_extend(calls, 'pmem_memmove_nodrain.static', memmove_funcs['t']['noflush'])
        calls = dict_extend(calls, 'pmem_memmove_nodrain.static', memmove_funcs['nt']['flush'])
        calls = dict_extend(calls, 'pmem_memmove_nodrain.static', memmove_funcs['t']['flush'])

        calls = dict_extend(calls, 'pmem_memmove_nodrain_eadr.static', memmove_funcs['t']['noflush'])
        calls = dict_extend(calls, 'pmem_memmove_nodrain_eadr.static', memmove_funcs['nt']['empty'])
        calls = dict_extend(calls, 'pmem_memmove_nodrain_eadr.static', memmove_funcs['t']['empty'])

        memsetfuncs = {
                't': {
                        func: [ f'memset_mov_{trick}_{func}'
                                for trick in ['sse2', 'avx', 'avx512f']
                        ] for func in ['noflush', 'empty'] 
                },
                'nt': {
                        'empty': [ f'memset_movnt_{trick}_empty_{drain}'
                                for trick in ['sse2', 'avx']
                                        for drain in ['wcbarrier', 'nobarrier'] 
                        ],
                        'flush': [ f'memset_movnt_{trick}_{flush}_{drain}'
                                for trick in ['sse2', 'avx']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                                                for drain in ['wcbarrier', 'nobarrier']
                        ]
                }
        }
        memsetfuncs_extras = {
                't': {
                        'flush': [ f'memset_mov_{trick}_{flush}'
                                for trick in ['sse2', 'avx', 'avx512f']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                        ]
                },
                'nt': {
                        'empty': [ f'memset_movnt_{trick}_empty'
                                for trick in ['avx512f', 'movdir64b']
                        ],
                        'flush': [ f'memset_movnt_{trick}_{flush}'
                                for trick in ['avx512f', 'movdir64b']
                                        for flush in ['clflush', 'clflushopt', 'clwb']
                        ]
                }
        }
        memsetfuncs['t']['flush'] = memsetfuncs_extras['t']['flush']
        memsetfuncs['nt']['empty'].extend(memsetfuncs_extras['nt']['empty'])
        memsetfuncs['nt']['flush'].extend(memsetfuncs_extras['nt']['flush'])

        calls = dict_extend(calls, 'pmem_memset_nodrain.static', memsetfuncs['t']['noflush'])
        calls = dict_extend(calls, 'pmem_memset_nodrain.static', memsetfuncs['nt']['flush'])
        calls = dict_extend(calls, 'pmem_memset_nodrain.static', memsetfuncs['t']['flush'])

        calls = dict_extend(calls, 'pmem_memset_nodrain_eadr.static', memsetfuncs['t']['noflush'])
        calls = dict_extend(calls, 'pmem_memset_nodrain_eadr.static', memsetfuncs['nt']['empty'])
        calls = dict_extend(calls, 'pmem_memset_nodrain_eadr.static', memsetfuncs['t']['empty'])

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
        apis = []
        while len(callers) > 0:
                callers_new = []
                for callee in callers:
                        for k, v in calls.items():
                                # this caller does not call this callee
                                if callee not in v:
                                        continue
                                # it is part of the API
                                if k in API:
                                        apis.append(k)
                                callers_new.append(k)
                callers = callers_new
        assert(len(apis) > 0)
        # if len(apis) == 0:
        #         print(func)
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
        for k, _ in funcs.items():
                if k in all_callees:
                        continue
                if k in whitelist:
                        continue
                if k in API:
                        continue
                not_called.append(k)
        # dump(not_called, 'not_called')
        assert(len(not_called) == 0)

        # for callee in all_callees:
        #         assert(is_reachable(callee, calls))

        # All mem(move|set) functions are expected to be tracked back to pmem_mem* API calls
        no_api_connection = {}
        for k, _ in funcs.items():
                if not re.search("^mem(set|move)", k):
                        continue
                if k in whitelist:
                        continue
                if k in API:
                        continue
                callers = api_callers(k, calls)
                valid = False
                for caller in callers:
                        if re.search("^pmem_mem", caller):
                                valid = True
                                break
                if not valid:
                        # print(k)
                        no_api_connection[k] = callers
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
        call_stacks = []
        for func in rcalls.keys():
                if func == 'LOG':
                        continue
                if func in whitelist:
                        continue
                if func in calls.keys():
                        continue
                call_stacks.extend(generate_call_stacks(func, funcs, rcalls))
        # call_stacks = call_stacks_reduce(call_stacks)
        print(len(call_stacks))
        call_stacks.sort(reverse=True, key=call_stack_key)
        dump(call_stacks, 'call_stacks_all')
        # XXX
        call_stacks = list(filter(lambda call_stack: re.search('^pmem_(mem|persist|flush|drain)', call_stack['stack'][0]), call_stacks))
        print(len(call_stacks))
        call_stacks.sort(reverse=True, key=call_stack_key)
        dump(call_stacks, 'call_stacks_pmem_ops')

def main():
        funcs = load_stack_usage()
        # dump(funcs, 'funcs')
        calls = load_call_stack()
        calls = inlines(calls)
        calls = function_pointers(calls)
        dump(calls, 'calls')
        validate(funcs, calls)

        generate_all_call_stacks(funcs, calls)

if __name__ == '__main__':
        main()
