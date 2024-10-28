from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        # Exchange
        self.requires("fmt/10.2.1")
        self.requires("quill/3.7.0")
        self.requires("libcurl/8.6.0")
        self.requires("argparse/3.0")
        self.requires("glaze/3.6.1")
        self.requires("yaml-cpp/0.8.0")
        self.requires("boost/1.83.0")
        self.requires("prometheus-cpp/1.1.0")
        self.requires("crowcpp-crow/1.1.0")
        self.requires("emhash/cci.20240601")
        self.requires("abseil/20240116.2")

        # Wrapper
        self.requires("pybind11/2.12.0")

    def configure(self):
        self.options["boost"].without_test=True

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")
        self.requires("benchmark/1.8.4")
