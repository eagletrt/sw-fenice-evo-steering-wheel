import subprocess
Import("env")

repo_dir = env["PROJECT_DIR"]


def git(*args):
    try:
        return subprocess.check_output(
            ["git", *args],
            cwd=repo_dir,
            stderr=subprocess.STDOUT,
        ).decode().strip()

    except subprocess.CalledProcessError as e:
        print(
            f"Git ERROR: 'git {' '.join(args)}' failed in "
            f"{repo_dir}:\n{e.output.decode().strip()}"
        )

    except Exception as e:
        print(f"Git ERROR: cannot run git in {repo_dir}: {e!r}")

    return None


full = git("rev-parse", "HEAD") or ""
commit = full[:7]
commit_num = "0x" + commit if commit else "0x0"

status = git("status", "--porcelain", "--untracked-files=no")
dirty = 1 if status else 0

env.Append(BUILD_FLAGS=[
    f"-DGIT_COMMIT={commit_num}",
    f"-DGIT_DIRTY={dirty}",
])
