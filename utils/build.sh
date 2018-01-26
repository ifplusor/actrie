#!/usr/bin/env bash

__FILE__="${BASH_SOURCE-$0}"
__DIRECTORY__="$(dirname "${__FILE__}")"
__DIRECTORY__="$(cd "${__DIRECTORY__}"; pwd)"

echo "actrie configuration summary:"

if [ -z "${ROOT_DIR}" ]; then
	ROOT_DIR="$(cd "${__DIRECTORY__}/.."; pwd)"
fi
echo "  ROOT_DIR = ${ROOT_DIR}"

if [ -z "${BUILD_DIR}" ]; then
	BUILD_DIR="${ROOT_DIR}/cmake_build"
fi
echo "  BUILD_DIR = ${BUILD_DIR}"

flags=""
if [ -n "${ALIB_DIR}" ]; then
	flags="-Dalib_DIR=${ALIB_DIR}"
fi
echo "  CMake flags = ${flags}"
echo ""

mkdir "${BUILD_DIR}"
pushd "${BUILD_DIR}"
cmake "${flags}" "${ROOT_DIR}"
make "actrie"
popd
