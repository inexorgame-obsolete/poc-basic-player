# for web

install clang-6.0 and do


export CC=/usr/bin/clang-6.0
export CXX=/usr/bin/clang++-6.0

you may need to create a symlink at /usr/bin/clang and /usr/bin/clang++ pointing to /usr/bin/clang-6.0 respectively /usr/bin/clang++-6.0

Download emscripten.profile for conan and put it to .conan/profiles/:

https://docs.conan.io/en/latest/integrations/cross_platform/emscripten.html#running-the-code-inside-the-browser

Do
`conan install .. -pr emscripten.profile --build missing`

inside the build directory

then

`conan build ..` (you need to edit the CMakeLists.txt before: add `set(TARGET_WEB 1)` somewhere at the very beginning of the file.

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
