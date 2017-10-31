#!/usr/bin/env bash

if [ -z "${ROOT_DIR}" ]; then
	ROOT_DIR="$(pwd)"
fi

if [ -z "${BUILD_DIR}" ]; then
	BUILD_DIR="${ROOT_DIR}/cmake_build"
fi

flags=""
if [ -n "${ALIB_DIR}" ]; then
	flags="-DALIB_DIR=${ALIB_DIR}"
fi

mkdir "${BUILD_DIR}"
pushd "${BUILD_DIR}"
cmake "${flags}" "${ROOT_DIR}"
make "actrie"
popd
