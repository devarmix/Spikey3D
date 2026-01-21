workspace "Spikey3D"
    architecture "x64"
    startproject "Spikey3D"

    configurations
    {
        "Game-Debug",
        "Game-Shipping",
        "Editor-Debug",
        "Editor-Shipping"
    }

project "Spikey3D"
    location "Source"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("%{wks.location}/Binaries/%{cfg.system}%{cfg.architecture}%{cfg.buildcfg}")
    objdir ("%{wks.location}/Intermediate")
    pchheader "Source/Engine/Core/Common.h"

    VulkanSDKPath = os.getenv("VULKAN_SDK")
    if VulkanSDKPath == nil then
        error("VulkanSDK is required to build Spikey3D. Please install the latest Vulkan 1.4 SDK.")
    end

    files
    {
        "Source/Engine/**.h",
        "Source/Engine/**.cpp",
        "Source/Editor/**.h",
        "Source/Editor/**.cpp",
        "Source/Platform/**.h",
        "Source/Platform/**.cpp",

        "Source/ThirdParty/VkBootstrap/VkBootstrap.cpp",
        "Source/ThirdParty/ImGui/imgui_demo.cpp",
        "Source/ThirdParty/ImGui/imgui_draw.cpp",
        "Source/ThirdParty/ImGui/imgui_tables.cpp",
        "Source/ThirdParty/ImGui/imgui_widgets.cpp",
        "Source/ThirdParty/ImGui/imgui.cpp",
        "Source/ThirdParty/ImGui/backends/imgui_impl_sdl3.cpp",
        "Source/ThirdParty/SPIRV-Reflect/spirv_reflect.cpp"
    }

    includedirs
    {
        "Source/Engine/",
        "Source/Platform/",
        "Source/Editor/",

        "%{VulkanSDKPath}/Include",
        "Source/ThirdParty/Glm/Include/",
        "Source/ThirdParty/VkBootstrap/Include/",
        "Source/ThirdParty/SDL3/Include/",
        "Source/ThirdParty/ImGui/",
        "Source/ThirdParty/NlohmannJson/",
        "Source/ThirdParty/Tracy/Include/",
        "Source/ThirdParty/Spdlog/Include/",
        "Source/ThirdParty/DirectXShaderCompiler/Include/",
        "Source/ThirdParty/VulkanMemoryAllocator/Include/",
        "Source/ThirdParty/SPIRV-Reflect/Include/"
    }

    links
    {
        "%{VulkanSDKPath}/Lib/vulkan-1.lib",
        "Source/ThirdParty/DirectXShaderCompiler/Lib/x64/dxcompiler.lib",
        "Source/ThirdParty/SDL3/Lib/x64/SDL3.lib",
        "Source/ThirdParty/Tracy/Lib/x64/TracyClient.lib"
    }

    filter "system:windows"
        systemversion "latest"
        buildoptions "/utf-8"

        defines
        {
            "SPIKEY_PLATFORM_WINDOWS"
        }

    filter "configurations:Game-Debug"
        defines "SPIKEY_GAME"
        runtime "Debug"
        symbols "on"

    filter "configurations:Game-Shipping"
        defines "SPIKEY_GAME"
        runtime "Release"
        optimize "on"

    filter "configurations:Editor-Debug"
        defines "SPIKEY_EDITOR"
        runtime "Debug"
        symbols "on"

    filter "configurations:Editor-Shipping"
        defines "SPIKEY_EDITOR"
        runtime "Release"