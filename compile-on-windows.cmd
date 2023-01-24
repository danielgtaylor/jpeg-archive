@ECHO OFF

REM make clean by deleting
REM deps\mozjpeg\build
REM src\iqa\build

REM this is currently for x64 build, but should be easily adaptable to 32-bit build

where.exe msbuild.exe
IF ERRORLEVEL 1 (
	echo MSBUILD environment not yet intialized, please run the vcvars##.bat to initialize the environment
	exit /b 1
)

where.exe nasm.exe
IF ERRORLEVEL 1 (
	echo NASM needs to be available on PATH
	exit /b 1
)


REM build mozjpegturbo libs
pushd deps\mozjpeg
mkdir build
cd build
cmake.exe -G"NMake Makefiles" -DPNG_SUPPORTED=0 -DCMAKE_BUILD_TYPE=Release ..
IF ERRORLEVEL 1 (
	echo mozjpeg CMAKE failed
	exit /b 1
)
nmake.exe
IF ERRORLEVEL 1 (
	echo mozjpeg NMAKE failed
	exit /b 1
)
popd



pushd src\iqa
MsBuild.exe iqa.sln /t:Build /p:Configuration=Release /p:Platform=x64 /p:DebugSymbols=false /p:DebugType=None
IF ERRORLEVEL 1 (
	echo iqa msbuild failed
	exit /b 1
)
popd

nmake.exe /NOLOGO -f ..\Makefile.w32
IF ERRORLEVEL 1 (
	echo jpeg-archive nmake failed
	exit /b 1
)

echo Success!  Find built files in %CD%
