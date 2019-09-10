#!/usr/bin/env python
# -*- coding: utf-8 -*-

from conans import ConanFile, CMake, tools
from conans.errors import ConanException
import os
import shutil


def sort_libs(correct_order, libs, lib_suffix='', reverse_result=False):
    # Add suffix for correct string matching
    correct_order[:] = [s.__add__(lib_suffix) for s in correct_order]

    result = []
    for expectedLib in correct_order:
        for lib in libs:
            if expectedLib == lib:
                result.append(lib)

    if reverse_result:
        # Linking happens in reversed order
        result.reverse()

    return result


class PocConan(ConanFile):
    name = "inexor-poc"
    version = "2019.07"
    description = ""
    # topics can get used for searches, GitHub topics, Bintray tags etc. Add here keywords about the library
    topics = ("conan", "corrad", "magnum", "filesystem", "console", "environment", "os")
    url = "https://github.com/inexorgame"
    homepage = "https://inexor.org"
    author = "a_teammate"
    license = "MIT"  # Indicates license type of the packaged library; please use SPDX Identifiers https://spdx.org/licenses/
    generators = "cmake"

    # Options may need to change depending on the packaged library.
    settings = "os", "arch", "compiler", "build_type"
    options = { }
    default_options = {"magnum:with_sdl2application":False}

    requires = (
        "doctest/2.3.1@bincrafters/stable"
    )

    _build_subfolder = "build_subfolder"

    def requirements(self):
        if self.settings.os == "Emscripten":
            self.requires("magnum/2019.07_inofficial@inexorgame/testing")
        else:
            self.requires("magnum/2019.01@inexorgame/testing")

    def config_options(self):
        if self.settings.os == "Emscripten":
            self.options["magnum"].with_emscriptenapplication = True
            self.options["magnum"].with_glfwapplication = False
        else:
            self.options["magnum"].with_emscriptenapplication = False
            self.options["magnum"].with_glfwapplication = True

    def _configure_cmake(self):
        cmake = CMake(self)

        def add_cmake_option(option, value):
            var_name = "{}".format(option).upper()
            value_str = "{}".format(value)
            var_value = "ON" if value_str == 'True' else "OFF" if value_str == 'False' else value_str
            cmake.definitions[var_name] = var_value

        for option, value in self.options.items():
            add_cmake_option(option, value)

        # Corrade uses suffix on the resulting 'lib'-folder when running cmake.install()
        # Set it explicitly to empty, else Corrade might set it implicitly (eg. to "64")
        add_cmake_option("LIB_SUFFIX", "")

        # add_cmake_option("BUILD_STATIC", not self.options.shared)

        if self.settings.compiler == 'Visual Studio':
            add_cmake_option("MSVC2015_COMPATIBILITY", int(self.settings.compiler.version.value) == 14)
            add_cmake_option("MSVC2017_COMPATIBILITY", int(self.settings.compiler.version.value) == 17)

        
        add_cmake_option("CMAKE_TOOLCHAIN_FILE", "toolchains/generic/Emscripten-wasm.cmake")
        cmake.configure()

        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


