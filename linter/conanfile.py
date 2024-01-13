from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("zlib/1.3", override=True)
        self.requires("fmt/10.0.0", override=True)    # string parsing
        self.requires("quill/3.3.1")   # logging
        self.requires("libcurl/8.2.1")
        self.requires("glaze/[^1.3.5]")
        self.requires("pybind11/2.10.4")

        self.requires("argparse/2.9")  # argument parsing
        self.requires("crowcpp-crow/1.0+5")

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")
