wget https://github.com/bazelbuild/bazel/releases/download/0.29.1/bazel-0.29.1-installer-linux-x86_64.sh
chomd +x bazel-0.29.1-installer-linux-x86_64.sh
./bazel-0.29.1-installer-linux-x86_64.sh --user
export PATH="$PATH:$HOME/bin"
bazel --version
echo "building app"
mkdir -p ./build
echo "Now moving src to build"
mv ./src/web ./build/bin