#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import os
from distutils.core import setup, Extension

alib_dir = '../alib/'
actrie_dir = '.'

# alib = Extension('alib',
#                  sources=[
#                      os.path.join(alib_dir, 'src/amem.c'),
#                      os.path.join(alib_dir, 'src/aobj.c'),
#                      os.path.join(alib_dir, 'src/astr.c'),
#                      os.path.join(alib_dir, 'src/dynastr.c'),
#                      os.path.join(alib_dir, 'src/acom.c'),
#                      os.path.join(alib_dir, 'src/list.c'),
#                      os.path.join(alib_dir, 'src/dlnk.c'),
#                      os.path.join(alib_dir, 'src/dynapool.c'),
#                      os.path.join(alib_dir, 'src/avl.c'),
#                      os.path.join(alib_dir, 'src/stream.c'),
#                      os.path.join(alib_dir, 'src/dynabuf.c'),
#                      os.path.join(alib_dir, 'src/utf8.c')
#                  ],
#                  include_dirs=[
#                      os.path.join(alib_dir, 'include'),
#                  ])

match = Extension('match',
                  sources=[
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
                      os.path.join(actrie_dir, 'actrie/src/wrap.c')
                  ],
                  include_dirs=[
                      os.path.join(alib_dir, 'include'),
                      os.path.join(actrie_dir, 'include'),
                  ])

setup(name="actrie",
      version="2.0",
      description="actrie",
      author="James Yin",
      author_email="ywhjames@hotmail.com",
      ext_modules=[match],
      ext_package='actrie',
      packages=['actrie'])
