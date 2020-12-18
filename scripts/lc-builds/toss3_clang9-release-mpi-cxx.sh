#!/usr/bin/env bash

SCRIPT_PATH=${0%/*}
. "$SCRIPT_PATH/utils/parse-args.sh"

BUILD_SUFFIX=lc_toss3-clang-9.0.0-rel-mpi-cxx

rm -rf ${BUILD_SUFFIX} 2>/dev/null
mkdir -p ${BUILD_SUFFIX}/install
mkdir -p ${BUILD_SUFFIX}/build && cd ${BUILD_SUFFIX}/build

module load cmake/3.14.5
module load clang/9.0.0

cmake \
  ${SRC_DIR} \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=/usr/tce/packages/clang/clang-9.0.0/bin/clang++ \
  -DCMAKE_C_COMPILER=/usr/tce/packages/clang/clang-9.0.0/bin/clang \
  -C ${SRC_DIR}/host-configs/lc-builds/toss3/clangX_tpl.cmake \
  -DENABLE_OPENMP=On \
  -DENABLE_MPI=On \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  -DENABLE_STATIC_CXXONLY=On \
  $CMAKE_ARGS \