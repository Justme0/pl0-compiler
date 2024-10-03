set -ex

cd src
clang-format -i pl0.* err.* || true
make
cd ..
