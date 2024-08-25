import os

content = """
[settings]
arch=armv8
build_type=Release
compiler=clang
compiler.cppstd=gnu23
compiler.libcxx=libc++
compiler.version=17
os=Macos
"""

filepath = os.path.expanduser("~/.conan2/profiles/default")

f = open(filepath, "w")
f.write(content)
f.close()
