apt install bazel
bazel --version
echo "building app"
mkdir -p ./build
echo "Now moving src to build"
mv ./src/web ./build/bin