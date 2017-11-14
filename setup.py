#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
import platform
import re
from distutils.core import setup, Extension

PYTHON_VERSION = platform.python_version()
print("build for python" + PYTHON_VERSION)

# Arguments
actrie_dir = ""
alib_dir = ""


def get_root_dir():
    return os.path.dirname(os.path.realpath(__file__))


if not actrie_dir:
    actrie_dir = get_root_dir()

if not alib_dir:
    alib_dir = os.path.join(actrie_dir, '..', 'alib')


def build_library():
    os.system(os.path.join(actrie_dir, "scripts", "build.sh"))


warp_sources = [
    os.path.join(actrie_dir, 'actrie', 'src', 'wrap.c'),
    os.path.join(actrie_dir, 'actrie', 'src', 'utf8ctx.c')
]

sources = warp_sources
# build_library()
library_dirs = [os.path.join(actrie_dir, 'lib')]
libraries = ['actrie', 'alib', 'pcre']

include_dir = [
    os.path.join(alib_dir, 'include'),
    os.path.join(actrie_dir, 'include'),
]

actrie = Extension('actrie._actrie',
                   sources=sources,
                   # extra_compile_args=["/utf-8"],
                   include_dirs=include_dir,
                   library_dirs=library_dirs,
                   libraries=libraries)

kwds = {}
# Read version from bitarray/__init__.py
pat = re.compile(r'__version__\s*=\s*(\S+)', re.M)
data = open(os.path.join(actrie_dir, 'actrie', '__init__.py')).read()
kwds['version'] = eval(pat.search(data).group(1))

setup(name="actrie",
      description="actrie",
      author="James Yin",
      author_email="ywhjames@hotmail.com",
      packages=['actrie', 'actrie.tests'],
      ext_modules=[actrie],
      **kwds)
