#!/usr/bin/env python3
"""Build, run and tabulate the cross-language benchmarks described in SPEC.md.

    python tests/compare/harness.py                 # everything it can build
    python tests/compare/harness.py --lang lobster  # one language
    python tests/compare/harness.py --list          # what it would run, and why not

Run it from the repository root. Every implementation times itself in-process and prints
`bench<TAB>name<TAB>seconds<TAB>checksum`; this script builds each configuration, runs it a
few times, keeps the best time per benchmark, checks that every configuration agreed on the
checksums, and rewrites RESULTS.md.
"""

from __future__ import annotations

import argparse
import glob
import os
import platform
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import Callable

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
BUILD = os.path.join(HERE, "build")
RESULTS = os.path.join(HERE, "RESULTS.md")

BENCHES = ["nbody", "raytrace", "interp", "sort", "astar"]

# How many times each program repeats a benchmark internally and reports the best. Languages
# that are orders of magnitude slower get fewer, which costs nothing: the reported number is a
# minimum either way, only the confidence in it changes.
DEFAULT_REPEATS = 5
REPEATS = {"python": 1, "lua": 2}

# How many times the harness launches each program. Also a minimum, over a different set of
# noise sources: process placement, page cache, JIT warmup that survives inside one process.
PROCESS_RUNS = 3
RUNS = {"python": 1}

IS_WINDOWS = os.name == "nt"
EXE = ".exe" if IS_WINDOWS else ""


# ----------------------------------------------------------------------------- toolchains


def vs_major(install_dir: str) -> int:
    """Visual Studio installs are foldered by year up to 2022 and by major version after it,
    so "18" is newer than "2022" even though it sorts before it."""
    name = os.path.basename(install_dir)
    try:
        n = int(name)
    except ValueError:
        return 0
    return {2017: 15, 2019: 16, 2022: 17}.get(n, n if n < 1000 else 0)


def find_vs_installs() -> list[str]:
    """Every Visual Studio with an x64 developer environment, newest first."""
    pats = [
        r"C:\Program Files\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat",
        r"C:\Program Files (x86)\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat",
    ]
    found: list[str] = []
    for p in pats:
        found += glob.glob(p)
    roots = [os.path.abspath(os.path.join(f, "..", "..", "..", "..")) for f in found]
    return sorted(roots, key=lambda r: vs_major(os.path.dirname(r)), reverse=True)


VS_INSTALLS = find_vs_installs() if IS_WINDOWS else []
VS_ROOT = VS_INSTALLS[0] if VS_INSTALLS else None
VCVARS = os.path.join(VS_ROOT, "VC", "Auxiliary", "Build", "vcvars64.bat") if VS_ROOT else None
MSBUILD = None
if VS_ROOT:
    cand = os.path.join(VS_ROOT, "MSBuild", "Current", "Bin", "MSBuild.exe")
    MSBUILD = cand if os.path.exists(cand) else None
# Clang ships as an optional component, so it may live in a different install than the newest.
CLANG_BIN = None
for _r in VS_INSTALLS:
    _c = os.path.join(_r, "VC", "Tools", "Llvm", "x64", "bin")
    if os.path.exists(os.path.join(_c, "clang-cl.exe")):
        CLANG_BIN = _c
        break


def have(tool: str) -> bool:
    return shutil.which(tool) is not None


def have_clang() -> bool:
    return bool(CLANG_BIN) and os.path.exists(os.path.join(CLANG_BIN, "clang-cl.exe"))


def sh(cmd: list[str], cwd: str = ROOT, env: dict | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=cwd, env=env, capture_output=True, text=True)


def vc_sh(script: str, cwd: str = ROOT) -> subprocess.CompletedProcess:
    """Run a batch script body with the MSVC x64 environment already set up."""
    if not VCVARS:
        raise RuntimeError("no Visual Studio found")
    bat = os.path.join(BUILD, "_vcstep.bat")
    with open(bat, "w", encoding="ascii") as f:
        f.write("@echo off\n")
        f.write(f'call "{VCVARS}" >nul 2>&1\n')
        f.write(f'cd /d "{cwd}"\n')
        f.write(script)
    return subprocess.run(["cmd", "/c", bat], capture_output=True, text=True)


# ----------------------------------------------------------------------------- model


@dataclass
class Config:
    lang: str
    name: str
    run: list[str]
    build: Callable[[], None] | None = None
    note: str = ""
    # Filled in as the run proceeds.
    status: str = "pending"
    times: dict[str, float] = field(default_factory=dict)
    checksums: dict[str, float] = field(default_factory=dict)

    @property
    def label(self) -> str:
        return f"{self.lang}/{self.name}"


class Skip(Exception):
    """Raised by a build to say this configuration cannot run here."""


# ----------------------------------------------------------------------------- lobster

LOBSTER = os.path.join(ROOT, "bin", "lobster" + EXE)
LOBSTER_MAIN = os.path.join("tests", "compare", "lobster", "main.lobster")
CL_SLN = os.path.join(ROOT, "dev", "compiled_lobster", "compiled_lobster", "compiled_lobster.sln")
CL_CPP = os.path.join(ROOT, "dev", "compiled_lobster", "src", "compiled_lobster.cpp")
CL_EXE = os.path.join(
    ROOT, "dev", "compiled_lobster", "compiled_lobster", "x64", "Release", "compiled_lobster.exe"
)
# The engine half of the runtime links these, so a `--cpp` build needs them beside it.
RUNTIME_DLLS = ["steam_api64.dll", "openvr_api.dll"]


def lobster_gen_cpp() -> None:
    """`--cpp` insists on being run from the repository root."""
    r = sh([LOBSTER, "--cpp", LOBSTER_MAIN])
    if r.returncode != 0:
        raise Skip(f"lobster --cpp failed: {(r.stderr or r.stdout).strip()[:300]}")


def copy_runtime_dlls() -> None:
    for d in RUNTIME_DLLS:
        src = os.path.join(ROOT, "bin", d)
        if os.path.exists(src):
            shutil.copy2(src, os.path.join(BUILD, d))


def build_lobster_cpp_msvc() -> None:
    if not MSBUILD:
        raise Skip("MSBuild not found")
    lobster_gen_cpp()
    r = sh([MSBUILD, CL_SLN, "-p:Configuration=Release", "-p:Platform=x64", "-m", "-v:m", "-nologo"])
    if r.returncode != 0:
        raise Skip(f"MSBuild failed: {(r.stdout or r.stderr).strip()[-400:]}")
    shutil.copy2(CL_EXE, os.path.join(BUILD, "lobster-cpp-msvc" + EXE))
    copy_runtime_dlls()


def build_lobster_cpp_clang() -> None:
    """clang-cl on the generated code only, linked against the MSVC-built Lobster runtime.

    Rebuilding the whole runtime under clang would take minutes and risks compile errors in
    the vendored engine; what this comparison is about is the code the Lobster backend emits.
    """
    if not have_clang():
        raise Skip("clang-cl not found in the Visual Studio install")
    lang_lib = os.path.join(ROOT, "build", "compiled_lobster", "language", "language64.lib")
    eng_lib = os.path.join(ROOT, "build", "compiled_lobster", "engine", "engine64.lib")
    if not (os.path.exists(lang_lib) and os.path.exists(eng_lib)):
        build_lobster_cpp_msvc()
    if not os.path.exists(CL_CPP):
        lobster_gen_cpp()
    obj = os.path.join(BUILD, "lobster-cpp-clang.obj")
    out = os.path.join(BUILD, "lobster-cpp-clang" + EXE)
    inc = " ".join(
        f'/I"{os.path.join(ROOT, p)}"'
        for p in [
            "dev/src",
            "dev/include",
            "dev/external/freetype/include",
            "dev/external/SDL/include",
            "dev/external/SDL/include/build_config",
        ]
    )
    libs = " ".join(
        f'"{p}"'
        for p in [
            lang_lib,
            eng_lib,
            os.path.join(ROOT, "dev/external/openvr/lib/win64/openvr_api.lib"),
            os.path.join(ROOT, "dev/external/steamworks/lib/win64/steam_api64.lib"),
        ]
    )
    sys_libs = (
        "Setupapi.lib opengl32.lib winmm.lib imm32.lib version.lib gdi32.lib user32.lib "
        "shell32.lib ole32.lib oleaut32.lib advapi32.lib uuid.lib comdlg32.lib kernel32.lib"
    )
    script = (
        f'"{os.path.join(CLANG_BIN, "clang-cl.exe")}" /c /O2 /MT /EHsc /std:c++latest '
        f"/DNDEBUG /D_CONSOLE /DBUILD_CONTEXT_compiled_lobster /D_UNICODE /DUNICODE {inc} "
        # Clang forms FMA in C++ by default, which would move the results away from every
        # other configuration; SPEC.md requires it off.
        f'-Xclang -ffp-contract=off -Wno-everything /Fo"{obj}" "{CL_CPP}"\n'
        f"if errorlevel 1 exit /b 1\n"
        f'link /nologo /LTCG /OUT:"{out}" "{obj}" {libs} {sys_libs}\n'
    )
    r = vc_sh(script)
    if r.returncode != 0 or not os.path.exists(out):
        raise Skip(f"clang-cl build failed: {(r.stdout or r.stderr).strip()[-400:]}")
    copy_runtime_dlls()


def lobster_configs() -> list[Config]:
    if not os.path.exists(LOBSTER):
        return [
            Config("lobster", n, [], note="bin/lobster not built", status="skipped")
            for n in ["libtcc", "mir2", "cpp-msvc", "cpp-clang"]
        ]
    return [
        Config(
            "lobster",
            "libtcc",
            [LOBSTER, LOBSTER_MAIN, "--"],
            note="the default backend: C generated by the compiler, compiled by libtcc",
        ),
        Config(
            "lobster",
            "mir2",
            [LOBSTER, "--mir", "2", LOBSTER_MAIN, "--"],
            note="MIR JIT at its -O2",
        ),
        Config(
            "lobster",
            "cpp-msvc",
            [os.path.join(BUILD, "lobster-cpp-msvc" + EXE)],
            build=build_lobster_cpp_msvc,
            note="--cpp, MSVC /O2 /Oi /Ot /GL (the shipped project settings)",
        ),
        Config(
            "lobster",
            "cpp-clang",
            [os.path.join(BUILD, "lobster-cpp-clang" + EXE)],
            build=build_lobster_cpp_clang,
            note="--cpp, clang-cl /O2 -ffp-contract=off on the generated code, MSVC-built runtime",
        ),
    ]


# ----------------------------------------------------------------------------- c++

CPP_SRC = os.path.join(HERE, "cpp", "main.cpp")


def build_cpp_msvc(name: str, extra_cl: str, extra_link: str) -> Callable[[], None]:
    def go() -> None:
        if not VCVARS:
            raise Skip("no Visual Studio found")
        out = os.path.join(BUILD, name + EXE)
        obj = os.path.join(BUILD, name + ".obj")
        script = (
            f'cl /nologo /O2 /Oi /Ot /std:c++20 /EHsc /MT /fp:precise /DNDEBUG {extra_cl} '
            f'/Fo"{obj}" /Fe"{out}" "{CPP_SRC}"{extra_link}\n'
        )
        r = vc_sh(script)
        if r.returncode != 0 or not os.path.exists(out):
            raise Skip(f"cl failed: {(r.stdout or r.stderr).strip()[-300:]}")

    return go


def build_cpp_clang(name: str, extra: str) -> Callable[[], None]:
    def go() -> None:
        if not have_clang():
            raise Skip("clang-cl not found in the Visual Studio install")
        out = os.path.join(BUILD, name + EXE)
        obj = os.path.join(BUILD, name + ".obj")
        # SPEC.md requires contraction off: clang forms FMA in C++ by default, which would move
        # its floating point results away from every other configuration.
        script = (
            f'"{os.path.join(CLANG_BIN, "clang-cl.exe")}" /O2 /std:c++20 /EHsc /MT /DNDEBUG '
            f"-Xclang -ffp-contract=off {extra} "
            f'/Fo"{obj}" /Fe"{out}" "{CPP_SRC}"\n'
        )
        r = vc_sh(script)
        if r.returncode != 0 or not os.path.exists(out):
            raise Skip(f"clang-cl failed: {(r.stdout or r.stderr).strip()[-300:]}")

    return go


def cpp_configs() -> list[Config]:
    return [
        Config("cpp", "msvc-o2", [os.path.join(BUILD, "cpp-msvc-o2" + EXE)],
               build=build_cpp_msvc("cpp-msvc-o2", "", ""), note="MSVC /O2 /Oi /Ot /fp:precise"),
        Config("cpp", "msvc-ltcg", [os.path.join(BUILD, "cpp-msvc-ltcg" + EXE)],
               build=build_cpp_msvc("cpp-msvc-ltcg", "/GL", " /link /LTCG"),
               note="MSVC /O2 plus whole-program optimisation"),
        Config("cpp", "clang-o2", [os.path.join(BUILD, "cpp-clang-o2" + EXE)],
               build=build_cpp_clang("cpp-clang-o2", ""),
               note="clang-cl /O2 -ffp-contract=off"),
        Config("cpp", "clang-lto", [os.path.join(BUILD, "cpp-clang-lto" + EXE)],
               build=build_cpp_clang("cpp-clang-lto", "-flto -fuse-ld=lld"),
               note="clang-cl /O2 -flto -ffp-contract=off"),
    ]


# ----------------------------------------------------------------------------- rust

RUST_SRC = os.path.join(HERE, "rust", "main.rs")


def build_rust(name: str, flags: list[str]) -> Callable[[], None]:
    def go() -> None:
        if not have("rustc"):
            raise Skip("rustc not on PATH")
        out = os.path.join(BUILD, name + EXE)
        r = sh(["rustc", "-C", "debuginfo=0", *flags, "-o", out, RUST_SRC])
        if r.returncode != 0 or not os.path.exists(out):
            raise Skip(f"rustc failed: {(r.stderr or r.stdout).strip()[-300:]}")

    return go


def rust_configs() -> list[Config]:
    return [
        Config("rust", "o3", [os.path.join(BUILD, "rust-o3" + EXE)],
               build=build_rust("rust-o3", ["-C", "opt-level=3"]),
               note="rustc -C opt-level=3"),
        Config("rust", "o3-lto", [os.path.join(BUILD, "rust-o3-lto" + EXE)],
               build=build_rust("rust-o3-lto",
                                ["-C", "opt-level=3", "-C", "lto=fat", "-C", "codegen-units=1"]),
               note="rustc -C opt-level=3 -C lto=fat -C codegen-units=1"),
    ]


# ----------------------------------------------------------------------------- c#

CS_DIR = os.path.join(HERE, "csharp")
DOTNET = shutil.which("dotnet") or r"C:\Program Files\dotnet\dotnet.exe"


def build_csharp() -> None:
    if not os.path.exists(DOTNET):
        raise Skip("dotnet not found")
    r = sh([DOTNET, "--list-sdks"])
    if r.returncode != 0 or not r.stdout.strip():
        raise Skip("no .NET SDK installed, only a runtime")
    out = os.path.join(BUILD, "csharp")
    r = sh([DOTNET, "build", "-c", "Release", "-o", out], cwd=CS_DIR)
    if r.returncode != 0:
        raise Skip(f"dotnet build failed: {(r.stdout or r.stderr).strip()[-300:]}")


def csharp_configs() -> list[Config]:
    return [
        Config("csharp", "jit", [os.path.join(BUILD, "csharp", "csharp" + EXE)],
               build=build_csharp, note=".NET 9, default tiered JIT"),
    ]


# ----------------------------------------------------------------------------- go

GO_DIR = os.path.join(HERE, "go")


def find_go() -> str | None:
    p = shutil.which("go")
    if p:
        return p
    for c in [r"C:\Program Files\Go\bin\go.exe", r"C:\Go\bin\go.exe"]:
        if os.path.exists(c):
            return c
    return None


def build_go(name: str, flags: list[str]) -> Callable[[], None]:
    def go_build() -> None:
        gob = find_go()
        if not gob:
            raise Skip("go not found")
        out = os.path.join(BUILD, name + EXE)
        env = dict(os.environ)
        env["PATH"] = os.path.dirname(gob) + os.pathsep + env.get("PATH", "")
        r = sh([gob, "build", *flags, "-o", out, "."], cwd=GO_DIR, env=env)
        if r.returncode != 0 or not os.path.exists(out):
            raise Skip(f"go build failed: {(r.stderr or r.stdout).strip()[-300:]}")

    return go_build


def go_configs() -> list[Config]:
    return [
        Config("go", "default", [os.path.join(BUILD, "go-default" + EXE)],
               build=build_go("go-default", []), note="go build, default flags"),
        Config("go", "nobounds", [os.path.join(BUILD, "go-nobounds" + EXE)],
               build=build_go("go-nobounds", ["-gcflags=all=-B"]),
               note="go build -gcflags=all=-B, bounds checks disabled"),
    ]


# ----------------------------------------------------------------------------- java

JAVA_SRC = os.path.join(HERE, "java", "Main.java")
JAVA_OUT = os.path.join(BUILD, "java")


def build_java() -> None:
    if not have("javac"):
        raise Skip("javac not on PATH")
    os.makedirs(JAVA_OUT, exist_ok=True)
    r = sh(["javac", "-d", JAVA_OUT, JAVA_SRC])
    if r.returncode != 0:
        raise Skip(f"javac failed: {(r.stderr or r.stdout).strip()[-300:]}")


def java_configs() -> list[Config]:
    return [
        Config("java", "default", ["java", "-cp", JAVA_OUT, "Main"],
               build=build_java, note="HotSpot, default tiered compilation"),
    ]


# ----------------------------------------------------------------------------- scripting


def scripting_configs() -> list[Config]:
    node_src = os.path.join(HERE, "node", "main.js")
    py_src = os.path.join(HERE, "python", "main.py")
    lua_src = os.path.join(HERE, "lua", "main.lua")
    cfgs = [
        Config("node", "default", ["node", node_src], note="V8, default"),
        Config("python", "cpython", [sys.executable, py_src],
               note=f"CPython {platform.python_version()}"),
    ]
    for name, exe in [("lua55", r"C:\stuff\lua\lua55.exe"), ("luajit", r"C:\stuff\lua\luajit.exe")]:
        found = shutil.which(name) or (exe if os.path.exists(exe) else None)
        if found:
            cfgs.append(Config("lua", name, [found, lua_src],
                               note=f"{name}, interpreted" if name == "lua55" else "LuaJIT 2.1"))
        else:
            cfgs.append(Config("lua", name, [], note=f"{name} not found", status="skipped"))
    return cfgs


# ----------------------------------------------------------------------------- registry


def all_configs() -> list[Config]:
    return (
        lobster_configs()
        + cpp_configs()
        + rust_configs()
        + csharp_configs()
        + go_configs()
        + java_configs()
        + scripting_configs()
    )


# ----------------------------------------------------------------------------- running


def parse_output(text: str) -> tuple[dict[str, float], dict[str, float], list[str]]:
    times: dict[str, float] = {}
    sums: dict[str, float] = {}
    notes: list[str] = []
    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue
        if line.startswith("#"):
            notes.append(line)
            continue
        parts = line.split("\t")
        if len(parts) == 4 and parts[0] == "bench":
            times[parts[1]] = float(parts[2])
            sums[parts[1]] = float(parts[3])
        else:
            notes.append(line)
    return times, sums, notes


def run_config(cfg: Config, runs: int) -> None:
    repeats = REPEATS.get(cfg.lang, DEFAULT_REPEATS)
    runs = min(runs, RUNS.get(cfg.lang, runs))
    best: dict[str, float] = {}
    for _ in range(runs):
        r = subprocess.run(
            cfg.run + [str(repeats)], cwd=ROOT, capture_output=True, text=True, timeout=3600
        )
        if r.returncode != 0:
            cfg.status = f"run failed (exit {r.returncode}): {(r.stderr or r.stdout).strip()[:200]}"
            return
        times, sums, notes = parse_output(r.stdout)
        if not times:
            cfg.status = f"no benchmark output: {(r.stdout or r.stderr).strip()[:200]}"
            return
        for k, v in times.items():
            best[k] = min(best.get(k, v), v)
        cfg.checksums = sums
        for n in notes:
            print(f"    {cfg.label}: {n}")
    cfg.times = best
    cfg.status = "ok"


# ----------------------------------------------------------------------------- reporting


def check_checksums(configs: list[Config]) -> list[str]:
    """Every configuration must agree with the first one that produced a value."""
    problems = []
    for b in BENCHES:
        ref_cfg = None
        ref = None
        for c in configs:
            if c.status == "ok" and b in c.checksums:
                ref_cfg, ref = c, c.checksums[b]
                break
        if ref is None:
            continue
        for c in configs:
            if c.status != "ok" or b not in c.checksums:
                continue
            got = c.checksums[b]
            scale = max(abs(ref), 1e-12)
            if abs(got - ref) / scale > 1e-9:
                problems.append(
                    f"{b}: {c.label} gave {got!r}, {ref_cfg.label} gave {ref!r}"
                )
    return problems


def write_results(configs: list[Config], problems: list[str], runs: int) -> None:
    ok = [c for c in configs if c.status == "ok"]
    lines = []
    lines.append("# Cross-language benchmark results")
    lines.append("")
    lines.append(
        "Generated by `tests/compare/harness.py`. The benchmarks and the rules they follow are "
        "in [SPEC.md](SPEC.md). Do not edit this file by hand."
    )
    lines.append("")
    stamp = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    lines.append(f"- Generated: {stamp}")
    lines.append(f"- Machine: {platform.processor() or platform.machine()}, {platform.platform()}")
    lines.append(f"- Python driving the harness: {platform.python_version()}")
    lines.append(
        f"- Each program repeats every benchmark internally and reports its best; the harness "
        f"launched each configuration {runs} time(s) and kept the best of those."
    )
    lines.append("")

    if problems:
        lines.append("## Checksum disagreement")
        lines.append("")
        lines.append(
            "These configurations did not compute the same answer, so their times are not "
            "comparable until this is fixed."
        )
        lines.append("")
        for p in problems:
            lines.append(f"- {p}")
        lines.append("")

    if ok:
        # Lobster is what the suite exists to place, so its rows and its totals are marked.
        def label_of(c: Config) -> str:
            return f"**{c.label}**" if c.lang == "lobster" else c.label

        def total_of(c: Config, text: str) -> str:
            return f"**{text}**" if c.lang == "lobster" else text

        rows = sorted((sum(c.times.get(b, 0.0) for b in BENCHES), c.label, c) for c in ok)

        lines.append("## Seconds, lower is better")
        lines.append("")
        lines.append("| configuration | " + " | ".join(BENCHES) + " | total |")
        lines.append("| --- |" + " ---: |" * (len(BENCHES) + 1))
        for total, _, c in rows:
            cells = [f"{c.times[b]:.4f}" if b in c.times else "-" for b in BENCHES]
            lines.append(
                f"| {label_of(c)} | " + " | ".join(cells) + f" | {total_of(c, f'{total:.4f}')} |"
            )
        lines.append("")

        lines.append("## Relative to the best configuration, per benchmark and overall")
        lines.append("")
        lines.append("| configuration | " + " | ".join(BENCHES) + " | total |")
        lines.append("| --- |" + " ---: |" * (len(BENCHES) + 1))
        bestof = {
            b: min((c.times[b] for c in ok if b in c.times), default=None) for b in BENCHES
        }
        best_total = rows[0][0] if rows else None
        for total, _, c in rows:
            cells = []
            for b in BENCHES:
                if b in c.times and bestof[b]:
                    cells.append(f"{c.times[b] / bestof[b]:.2f}x")
                else:
                    cells.append("-")
            rel = f"{total / best_total:.2f}x" if best_total else "-"
            lines.append(
                f"| {label_of(c)} | " + " | ".join(cells) + f" | {total_of(c, rel)} |"
            )
        lines.append("")

        lines.append("## Checksums")
        lines.append("")
        lines.append("| benchmark | value |")
        lines.append("| --- | --- |")
        for b in BENCHES:
            for c in ok:
                if b in c.checksums:
                    lines.append(f"| {b} | {c.checksums[b]!r} |")
                    break
        lines.append("")

    lines.append("## Configurations")
    lines.append("")
    lines.append("| configuration | status | notes |")
    lines.append("| --- | --- | --- |")
    for c in configs:
        status = "ok" if c.status == "ok" else c.status
        label = f"**{c.label}**" if c.lang == "lobster" else c.label
        lines.append(f"| {label} | {status} | {c.note} |")
    lines.append("")

    with open(RESULTS, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


# ----------------------------------------------------------------------------- main


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--lang", action="append", help="only these languages")
    ap.add_argument("--config", action="append", help="only these lang/name configurations")
    ap.add_argument("--runs", type=int, default=PROCESS_RUNS, help="process launches per config")
    ap.add_argument("--list", action="store_true", help="list configurations and exit")
    ap.add_argument("--no-build", action="store_true", help="use whatever is already built")
    args = ap.parse_args()

    os.makedirs(BUILD, exist_ok=True)
    configs = all_configs()
    if args.lang:
        configs = [c for c in configs if c.lang in args.lang]
    if args.config:
        configs = [c for c in configs if c.label in args.config]

    if args.list:
        for c in configs:
            print(f"{c.label:24} {c.status:10} {c.note}")
        return 0

    for c in configs:
        if c.status == "skipped":
            print(f"[skip]  {c.label}: {c.note}")
            continue
        if c.build and not args.no_build:
            print(f"[build] {c.label}")
            t0 = time.time()
            try:
                c.build()
            except Skip as e:
                c.status = "skipped"
                c.note = str(e)
                print(f"[skip]  {c.label}: {e}")
                continue
            print(f"        built in {time.time() - t0:.1f}s")
        if not c.run or not (os.path.exists(c.run[0]) or shutil.which(c.run[0])):
            c.status = "skipped"
            c.note = c.note or f"{c.run[0] if c.run else '?'} missing"
            print(f"[skip]  {c.label}: {c.note}")
            continue
        print(f"[run]   {c.label}")
        run_config(c, args.runs)
        if c.status == "ok":
            total = sum(c.times.get(b, 0.0) for b in BENCHES)
            per = "  ".join(f"{b}={c.times[b]:.4f}" for b in BENCHES if b in c.times)
            print(f"        {per}  total={total:.4f}")
        else:
            print(f"[fail]  {c.label}: {c.status}")

    problems = check_checksums(configs)
    for p in problems:
        print(f"[CHECKSUM] {p}")
    write_results(configs, problems, args.runs)
    print(f"\nwrote {os.path.relpath(RESULTS, ROOT)}")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
