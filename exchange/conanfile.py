from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/10.1.1", override=True)
        self.requires("quill/2.9.2")
        self.requires("rabbitmq-c/0.13.0")
        self.requires("libcurl/8.2.1")
        self.requires("argparse/2.9")
        self.requires("glaze/1.3.5")
        self.requires("libzip/1.10.1")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")