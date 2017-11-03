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
extra_build = True


def get_root_dir():
    return os.path.dirname(os.path.realpath(__file__))


if not actrie_dir:
    actrie_dir = get_root_dir()

if not alib_dir:
    alib_dir = os.path.join(actrie_dir, "../alib")


def build_library():
    os.system(os.path.join(actrie_dir, "scripts/build.sh"))


library_sources = [
    os.path.join(alib_dir, 'src/amem.c'),
    os.path.join(alib_dir, 'src/aobj.c'),
    os.path.join(alib_dir, 'src/astr.c'),
    os.path.join(alib_dir, 'src/dynastr.c'),
    os.path.join(alib_dir, 'src/acom.c'),
    os.path.join(alib_dir, 'src/list.c'),
    os.path.join(alib_dir, 'src/dlnk.c'),
    os.path.join(alib_dir, 'src/dynapool.c'),
    os.path.join(alib_dir, 'src/avl.c'),
    os.path.join(alib_dir, 'src/stream.c'),
    os.path.join(alib_dir, 'src/dynabuf.c'),
    os.path.join(alib_dir, 'src/utf8.c'),
    os.path.join(actrie_dir, 'src/configure.c'),
    os.path.join(actrie_dir, 'src/vocab.c'),
    os.path.join(actrie_dir, 'src/dict.c'),
    os.path.join(actrie_dir, 'src/actrie.c'),
    os.path.join(actrie_dir, 'src/acdat.c'),
    os.path.join(actrie_dir, 'src/mdimap.c'),
    os.path.join(actrie_dir, 'src/disambi.c'),
    os.path.join(actrie_dir, 'src/distance.c'),
    os.path.join(actrie_dir, 'src/matcher.c'),
]

warp_sources = [
    os.path.join(actrie_dir, 'actrie/src/wrap.c'),
]

if not extra_build:
    sources = library_sources + warp_sources
    library_dirs = []
    libraries = []
else:
    sources = warp_sources
    # build_library()
    library_dirs = [os.path.join(actrie_dir, 'lib')]
    libraries = ['pcre', 'alib', 'actrie']

include_dir = [
    os.path.join(alib_dir, 'include'),
    os.path.join(actrie_dir, 'include'),
    # os.path.join(alib_dir, 'third_part/pcre/include')
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
data = open(os.path.join(actrie_dir, 'actrie/__init__.py')).read()
kwds['version'] = eval(pat.search(data).group(1))

setup(name="actrie",
      description="actrie",
      author="James Yin",
      author_email="ywhjames@hotmail.com",
      packages=['actrie', 'actrie.tests'],
      ext_modules=[actrie],
      **kwds)
