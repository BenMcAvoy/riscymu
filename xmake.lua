add_rules("mode.debug", "mode.release")

add_requires("magic_enum")

set_languages("c++23")

target("riscymu")
    set_kind("binary")
    add_files("src/*.cpp")
    set_rundir("$(projectdir)")
    add_packages("magic_enum")