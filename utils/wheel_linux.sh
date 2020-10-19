#!/usr/bin/env bash

set -e

# docker run --rm -it quay.io/pypa/manylinux2014_x86_64 bash


# PLAT=manylinux2010_x86_64
PLAT=manylinux2014_x86_64

PROJECT_NAME=actrie
PROJECT_DIR=~/${PROJECT_NAME}/
PROJECT_DIST=~/${PROJECT_NAME}_dist/
DIST_DIR=~/wheelhouse/

# Compile wheels
for PYBIN in /opt/python/*/bin; do
    "${PYBIN}/pip" wheel $PROJECT_DIR -w $PROJECT_DIST
done

# Bundle external shared libraries into the wheels
for whl in $PROJECT_DIST/*.whl; do
    auditwheel repair "$whl" --plat $PLAT -w $DIST_DIR
done

# Install twine
# /opt/python/cp37-cp37m/bin/pip install --user --upgrade twine

# Upload wheel by twine
/opt/python/cp37-cp37m/bin/python -m twine upload --repository pypi $DIST_DIR/*

# Install packages and test
# for PYBIN in /opt/python/*/bin/; do
#     "${PYBIN}/pip" install $PROJECT_NAME --no-index -f $DIST_DIR
# done

# List installed packages
# for PYBIN in /opt/python/*/bin/; do
#     "${PYBIN}/pip" list
# done

# Uninstall packages
# for PYBIN in /opt/python/*/bin/; do
#     "${PYBIN}/pip" uninstall -y $PROJECT_NAME
# done

# Upload wheel to test pypi by twine
# /opt/python/cp37-cp37m/bin/python -m twine upload --repository testpypi $DIST_DIR/*

# Install packages from pypi test
# for PYBIN in /opt/python/*/bin/; do
#     "${PYBIN}/pip" install --index-url https://test.pypi.org/simple/ --no-deps $PROJECT_NAME
# done
