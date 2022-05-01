-- premake5.lua
workspace "RayTracingTut"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "RayTracingTut"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "RayTracingTut"