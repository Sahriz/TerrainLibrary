project "ImGui"
    kind "StaticLib"
    language "C++"
    staticruntime "on"

    targetdir ("../../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    files {
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui_demo.cpp",  -- optional, useful for testing

        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_opengl3.cpp",
        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_opengl3.h"
    }

    includedirs {
        ".",
        "backends",
        "../../Vendor/glfw/include",
        "../../Vendor/Glad/include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"