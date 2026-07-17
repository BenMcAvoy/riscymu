add_rules("mode.debug", "mode.release")
set_policy("build.optimization.lto", true)

add_requires("magic_enum")

set_languages("c++23")

target("riscymu")
    set_kind("binary")
    add_files("src/*.cpp")
    set_rundir("$(projectdir)")
    add_packages("magic_enum")

set_symbols("debug")

-- check for windows
if is_plat("windows") then
    add_defines("RISCYMU_WINDOWS")
end