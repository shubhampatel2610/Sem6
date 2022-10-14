workspace "SDLSandbox"
    startproject "test"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

tdir = "bin/%{cfg.buildcfg}/%{prj.name}"
odir = "bin-obj/%{cfg.buildcfg}/%{prj.name}"
-- External Dependencies
externals = {}
externals["sdl2"] = "external/sdl2"



Runstatic = "on"

project "test"
    location "test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime(Runstatic)
    
    links 
    {
        "SDL2.lib",
        "SDL2_image.lib"
    }

    flags
    {
        "FatalWarnings"
    }

    targetdir(tdir)
    objdir(odir)
    
    sysincludedirs
    {
        "%{prj.name}/include",
        "%{externals.sdl2}/include"
    }

    libdirs
    {
        "%{externals.sdl2}/lib"
    }

    files
    {
        "%{prj.name}/include/**.h",     
        "%{prj.name}/include/**.cpp",   
        "%{prj.name}/include/**.hpp",   
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/**.natvis"
    }

    filter {"configurations:Debug"}
        runtime "Debug"
        symbols "on"
    
    filter {"configurations:Release"}
        runtime "Release"
        symbols "off"
        optimize "on"
