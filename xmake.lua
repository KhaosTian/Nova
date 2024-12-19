-- root/xmake.lua
set_project("Nova")
set_version("0.1.0")
set_arch("x64")

-- 设置全局配置
set_languages("c++20")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
add_rules("plugin.vsxmake.autoupdate")

-- 添加包管理
add_requires("vulkansdk", "spdlog", "glfw", "glm", "stb")

if is_mode("debug") then
    add_defines("NOVA_DEBUG")
elseif is_mode("release") then 
    add_defines("NOVA_RELEASE")
end

-- 引擎静态库
target("Runtime")
    -- 基础配置
    set_kind("static")
    
    -- 依赖包
    add_packages("vulkansdk", "spdlog", "glfw", "glm", "stb")

    -- 源文件和头文件
    add_files("Source/Runtime/**.cpp")
    add_includedirs("Source/Runtime", {public = true})
    add_headerfiles("Source/Runtime/**.h", "Source/Runtime/**.hpp")
target_end()


target("Editor")
    -- 基础配置
    set_kind("binary")

    -- 依赖包
    add_packages("vulkansdk", "spdlog", "glfw", "glm", "stb")

    -- 依赖关系
    add_deps("Runtime")

    -- 源文件和头文件
    add_files("Source/Editor/**.cpp")
    add_includedirs("Source", {public = true})
    --add_headerfiles("Source/Editor/**.h", "Source/Editor/**.hpp")
target_end()