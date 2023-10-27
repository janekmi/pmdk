#!/usr/bin/env python3

import re
import json

STACK_USAGE_FILE='funcs.json'
CALL_STACK_FILE='call_stack.json'

def dump(var, name):
        with open(f"{name}.json", "w") as outfile: 
                json.dump(var, outfile, indent = 4)

def load_stack_usage():
        with open(STACK_USAGE_FILE, 'r') as file:
                return json.load(file)

def load_call_stack():
        with open(CALL_STACK_FILE, 'r') as file:
                return json.load(file)

def main():
        funcs = load_stack_usage()
        call_stack = load_call_stack()
        for func in call_stack['stack']:
                if func in funcs.keys():
                        size = funcs[func]['size']
                else:
                        size = 0
                print(f"{size}\t{func}")

if __name__ == '__main__':
        main()
