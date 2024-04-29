from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("zlib/1.3", override=True)
        self.requires("fmt/10.2.1")
        self.requires("quill/3.7.0")   # logging
        self.requires("libcurl/8.6.0")
        self.requires("glaze/2.4.0")
        self.requires("pybind11/2.12.0")

        self.requires("argparse/3.0")
        self.requires("crowcpp-crow/1.1.0")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")
