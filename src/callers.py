#!/usr/bin/env python3

import argparse
import json

PARSER = argparse.ArgumentParser()
PARSER.add_argument('func')

CALL_STACK_ALL_FILE='call_stacks_all.json'

def load_call_stacks_all():
        with open(CALL_STACK_ALL_FILE, 'r') as file:
                return json.load(file)

def main():
        args = PARSER.parse_args()
        call_stacks = load_call_stacks_all()
        callers = []
        for call_stack in call_stacks:
                if args.func in call_stack['stack']:
                        callers.append(call_stack['stack'][0])
        callers = list(set(callers))
        callers.sort()
        for caller in callers:
                print(caller)

if __name__ == '__main__':
        main()
