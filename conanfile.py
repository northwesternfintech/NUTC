from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/10.1.0")    # string parsing
        self.requires("quill/3.3.1")   # logging
        self.requires("libcurl/8.2.1")
        self.requires("glaze/1.3.5")
        self.requires("pybind11/2.10.4")

        self.requires("argparse/2.9")  # argument parsing

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")
