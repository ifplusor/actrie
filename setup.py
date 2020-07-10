#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
import platform
import re
from setuptools import setup, Extension

python_version = platform.python_version()
system_name = platform.system()
print("build for python{} on {}".format(python_version, system_name))

# Arguments
actrie_dir = ""
alib_dir = ""


def get_root_dir():
    return os.path.dirname(os.path.realpath(__file__))


if not actrie_dir:
    actrie_dir = get_root_dir()

if not alib_dir:
    alib_dir = os.path.join(actrie_dir, 'deps', 'alib')


def build_library():
    os.system(os.path.join(actrie_dir, "utils", "build.sh"))


# build_library()


warp_sources = [
    os.path.join(actrie_dir, 'actrie', 'src', 'wrap.c')
]

compile_args = []
if system_name == "Windows":
    compile_args.append("/utf-8")
else:
    compile_args.append("-fno-strict-aliasing")

library_dirs = [
    # os.path.join(alib_dir, 'lib'),
    os.path.join(actrie_dir, 'lib')
]

libraries = ['actrie', 'alib']

include_dirs = [
    os.path.join(alib_dir, 'include'),
    os.path.join(actrie_dir, 'include')
]

actrie = Extension('actrie._actrie',
                   sources=warp_sources,
                   extra_compile_args=compile_args,
                   include_dirs=include_dirs,
                   library_dirs=library_dirs,
                   libraries=libraries)

kwds = {}
# Read version from bitarray/__init__.py
pat = re.compile(r'__version__\s*=\s*(\S+)', re.M)
data = open(os.path.join(actrie_dir, 'actrie', '__init__.py')).read()
kwds['version'] = eval(pat.search(data).group(1))

setup(name="actrie",
      description="Aho-Corasick automation for large-scale multi-pattern matching.",
      author="James Yin",
      author_email="ywhjames@hotmail.com",
      url="https://github.com/ifplusor/actrie",
      license="BSD",
      packages=['actrie', 'actrie.example'],
      ext_modules=[actrie],
      classifiers=[
          "Programming Language :: C",
          "Programming Language :: Python :: 2",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python :: Implementation :: CPython",
          "License :: OSI Approved :: BSD License",
          "Operating System :: OS Independent",
          "Topic :: Utilities"
      ],
      keywords=["matcher", "trie", "aho-corasick automation", "ac-automation",
                "string matching", "string search", "string matcher"],
      zip_safe=False,
      **kwds)
