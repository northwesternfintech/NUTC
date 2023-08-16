from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/10.1.0")
        self.requires("quill/2.9.2")
        self.requires("rabbitmq-c/0.13.0")

    def build_requirements(self):
        self.test_requires("gtest/1.13.0")
