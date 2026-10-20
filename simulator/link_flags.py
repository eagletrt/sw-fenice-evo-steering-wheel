Import("env")

from sys import platform

if platform == "darwin":
    env.Append(LINKFLAGS=[
        "-framework", "Cocoa",
        "-framework", "OpenGL",
        "-framework", "CoreGraphics",
    ])
elif platform == "linux":
    env.Append(LIBS=["GL", "X11"])
