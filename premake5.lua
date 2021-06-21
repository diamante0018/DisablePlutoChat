dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

newoption {
	trigger = "dev-build",
	description = "Enable development builds of the client."
}

dependencies.load()

workspace "pluto-disable-chat"
location "./build"
objdir "%{wks.location}/obj"
targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"
targetname "%{prj.name}"

configurations {"Debug", "Release"}

architecture "x86"
platforms "x86"

buildoptions "/std:c++latest"
systemversion "latest"
symbols "On"
staticruntime "On"
editandcontinue "Off"
warnings "Extra"
characterset "ASCII"

if _OPTIONS["dev-build"] then
	defines {"DEV_BUILD"}
end

flags {"NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks" }

configuration "windows"
defines {"_WINDOWS", "WIN32"}

configuration "Release"
optimize "Size"
buildoptions {"/GL"}
linkoptions { "/IGNORE:4702", "/LTCG" }

defines {"NDEBUG"}

flags {"FatalCompileWarnings"}

configuration "Debug"
optimize "Debug"

defines {"DEBUG", "_DEBUG"}

configuration {}

project "pluto-disable-chat"
kind "SharedLib"
language "C++"

pchheader "stdinc.hpp"
pchsource "src/stdinc.cpp"

files {"./src/**.hpp", "./src/**.cpp"}

includedirs {"src"}

links {"kernel32", "user32", "Ws2_32"}

dependencies.imports()

group "Dependencies"
dependencies.projects()
