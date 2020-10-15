
@echo off

WHERE cl >nul 2>nul
IF %ERRORLEVEL% NEQ 0 call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 

set mainDir=..\..\..\..
set fileName=driver_AzureTracker.dll
Rem Need to add .h library files here, and .cpp library files on the cl line
set IncludeFlags=-EHsc -I %mainDir%\library\openvr\headers -I %mainDir%\library\enumser

pushd build\AzureTracker\bin\win64

cl %IncludeFlags% /D_USRDLL /D_WINDLL %mainDir%\driver.cpp %mainDir%\driverlog.cpp %mainDir%\library\enumser\enumser.cpp %mainDir%\library\enumser\stdafx.cpp /link /DLL /OUT:%fileName%

popd
