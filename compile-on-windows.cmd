@ECHO OFF
CD src\iqa
MsBuild.exe iqa.sln /t:Build /p:Configuration=Release /p:Platform=x64 /p:DebugSymbols=false /p:DebugType=None
CD ..\..
nmake /NOLOGO -f Makefile.w32
