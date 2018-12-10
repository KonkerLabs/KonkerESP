#!/usr/bin/env python
# encoding: utf-8
import os.path
import subprocess


def _minimal_ext_cmd(cmd):
    # construct minimal environment
    env = {}
    for k in ['SYSTEMROOT', 'PATH']:
        v = os.environ.get(k)
        if v is not None:
            env[k] = v
    # LANGUAGE is used on win32
    env['LANGUAGE'] = 'C'
    env['LANG'] = 'C'
    env['LC_ALL'] = 'C'
    out = subprocess.Popen(cmd, stdout=subprocess.PIPE, env=env).communicate()[0]
    return out

def get_git_hash():
    '''
    Gets the last GIT commit hash and date for the repository, using the
    path to this file.
    '''
    try:
        out = _minimal_ext_cmd(['git', 'rev-parse', '--short', 'HEAD'])
        GIT_REVISION = out.strip().decode('ascii')
    except:
        GIT_REVISION = None

    return GIT_REVISION

if __name__ == '__main__':
    print('\'-DPIO_SRC_REV="%s"\'' % (get_git_hash()))
