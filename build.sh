set -ex

clang-format -i src/pl0.{c,h} || true
make -C src
