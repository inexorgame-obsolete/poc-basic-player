# for web

install clang-6.0:

`sudo apt install clang-6.0`

make it the default compiler for now. I do it by doing the following:
```
export CC=/usr/bin/clang-6.0
export CXX=/usr/bin/clang++-6.0
```

you may need to create a symlink at /usr/bin/clang and /usr/bin/clang++ pointing to /usr/bin/clang-6.0 respectively /usr/bin/clang++-6.0:


Alternatively you might use update-alternatives.

## Install conan

If you got dont have python3 installed: `sudo apt install python3-pip`

`pip3 install conan --user`


## Setup an emscripten profile for conan (using clang 6.0)

Create a new conan file ~/.conan/profiles/emscripten.profile *(this is not only for this particular repo, but can be reused for all conan managed repos)*:

```
[settings]
os=Emscripten
arch=wasm
compiler=clang
compiler.version=6.0
compiler.libcxx=libc++
[options]
[build_requires]
emsdk_installer/1.38.29@bincrafters/stable
ninja_installer/1.8.2@bincrafters/stable
[env]

```
*(taken from https://docs.conan.io/en/latest/integrations/cross_platform/emscripten.html#running-the-code-inside-the-browser)*
## Setup a conan
## Install our dependencies via conan and build the app

Create a new build folder (when errors occur: delete the build folder. The reason is that our build system is fucked currently. CMake is shit, but its the standard)

```
cd <REPOSITORY_ROOT_FOLDER>
mkdir build
cd build

export TARGET_WEB=1
cmake ..
cmake --build .
```

Afterwards copy the files EmscriptenApplication.js and WebApplication.css from https://github.com/mosra/magnum/tree/07b3253714fef2af4febbd5a9e3e361d3d6cbc63/src/Magnum/Platform to the build/bin directory.

Then create a file called Inexor.html or sth like that and place the following inside:
```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8" />
  <title>BasicPlayer PoC</title>
  <link rel="stylesheet" href="WebApplication.css" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
</head>
<body>
  <h1>Inexor Basic Player PoC</h1>
  <div id="container">
    <div id="sizer"><div id="expander"><div id="listener">
      <canvas id="canvas"></canvas>
      <div id="status">Initialization...</div>
      <div id="status-description"></div>
      <script src="EmscriptenApplication.js"></script>
      <script async="async" src="inexor-poc-basic-player.js"></script>
    </div></div></div>
  </div>
</body>
</html>  
```

Afterwards open that file in a browser, or serve it via a webserver (e.g. by doing  python -m SimpleHTTPServer inside the bin directory)
