from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        # Exchange
        self.requires("fmt/[>=10.1.0]")
        self.requires("quill/3.7.0")
        self.requires("rabbitmq-c/0.14.0")
        self.requires("libcurl/8.6.0")
        self.requires("argparse/3.0")
        self.requires("glaze/2.4.0")
        self.requires("ncurses/6.4")

        # Wrapper
        self.requires("pybind11/2.12.0")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")
