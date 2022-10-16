# NUSofting FlatGUI Demo : shareable raw UX VST/AU library based on VSTGUI 4.3
@ FREE TO USE IN OPEN SOURCE PROJECTS @

 Authors: Luigi Felici and Bernie Maier: (C) 2014-2022

 For Windows build remember to link these files:
 yaml-cppd.lib in Debug 
 and yaml-cpp.lib in Release.

 Note: Currently using MSVS 2019 the Demo_Lib.sln will only build a "demoLibVST.dll" (64-bit VST2 plugin for Windows x64)
 fully functional GUI but no DSP inside.


    By default the project build to  C:\vstplugins\demoLibVST\demoLibVST.dll
    Warning: the folder "C:\vstplugins\demoLibVST"
    must contain the resources subfolders, please copy them manually from 
    their repository location ..\NUSofting-FlatGUI-Demo\Demo_Lib\resources


# VSTGUI 4.3 AND vst 2.x SKD   by Steinberg Media Technologies GmbH, All Rights Reserved