#!/usr/bin/env python3

import os
import sys
import subprocess
from concurrent.futures import ThreadPoolExecutor

DefaultWorkspace = 'cpg-llvm'

def get_libpdg_path(prefix='build'):
    root = os.path.dirname(os.path.realpath(__file__))
    root = os.path.dirname(root)
    return os.path.join(root, prefix, 'libpdg.so')

def make_workspace(directory=DefaultWorkspace):
    if not os.path.exists(directory):
        os.makedirs(directory)

    return directory

def get_workspace(directory=DefaultWorkspace):
    return directory

def exec_opt(filename):
    output = os.path.splitext(filename)[0] + '.opt'
    readable = os.path.splitext(output)[0] + '.ll'

    basename = os.path.basename(readable)
    if basename.startswith('.'):
        basename = basename[1:]
    
    readable = os.path.join(get_workspace(), basename)

    args = [
        'opt',
        '-passes=mem2reg',
        filename,
        '-o',
        output
    ]
    subprocess.run(args, capture_output=True)

    args = [
        'llvm-dis',
        output,
        '-o',
        readable
    ]
    subprocess.run(args, capture_output=True)

    args = [
        'opt',
        '-load',
        get_libpdg_path(),
        '--bugpoint-enable-legacy-pm',
        '--disable-output',
        '--dot-ddg',
        output
    ]

    proc = subprocess.run(args, cwd=get_workspace(), capture_output=True)
    return proc.returncode

def main(incoming):
    incoming = os.path.abspath(incoming)

    if not os.path.isdir(incoming):
        exec_opt(incoming)
        return
    
    filenames = []
    for dirpath, _, files in os.walk(incoming):
        filenames.extend([os.path.join(dirpath, f) for f in files])
    
    filenames = list(set(filter(lambda f: f.endswith('.bc'), filenames)))
    
    count = 1
    total = len(filenames)
    with ThreadPoolExecutor() as executor:
        for f, _ in zip(filenames, executor.map(exec_opt, filenames)):
            print(f'[{count} / {total}] Generated {f}')
            count += 1

if __name__ == "__main__":
    make_workspace()
    main(sys.argv[1])