Import("env")

# The strict flags you only want for Core/ code
STRICT_WARNINGS = [
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Werror"
]


def apply_core_warnings(env, node):
    # Replace backslashes to ensure cross-platform compatibility (Windows vs Unix).
    path = node.get_path().replace("\\", "/")

    # Check if the source file is inside the Core/ directory
    if "Core/" in path:
        return env.Object(
            node,
            CCFLAGS=env.get("CCFLAGS", []) + STRICT_WARNINGS
        )

    # Return the unmodified node (no warnings) for Drivers, CMSIS, etc.
    return node


env.AddBuildMiddleware(apply_core_warnings)
