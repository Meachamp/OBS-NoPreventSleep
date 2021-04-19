workspace "OBS-Plugin"
    location "build"
    configurations { "Release" }
    platforms { "x64" }

project "OBS-NoPreventSleep"
    architecture "x64"
    kind "SharedLib"
    includedirs { "../obs-studio/libobs"}
    language "C++"
    targetdir "bin"
    targetname "enable_sleep"
    files { "**.h", "**.cpp" }
    optimize "On"
    characterset "MBCS"

