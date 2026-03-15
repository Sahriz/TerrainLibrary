project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    files{
        "src/glad.c",
        "include/**.h"
    }

    includedirs{
        "include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    filter "configurations:Release"
        runtime "Release"
        optimize "on"