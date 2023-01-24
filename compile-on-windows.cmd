@ECHO OFF

REM make clean by deleting
REM deps\mozjpeg\build
REM src\iqa\build

REM this is currently for x64 build, but should be easily adaptable to 32-bit build

echo == Checking Environment ==

where.exe msbuild.exe
IF ERRORLEVEL 1 (
	echo MSBUILD environment not yet intialized, please run the vcvars##.bat to initialize the environment
	exit /b 1
)

where.exe cl.exe
IF ERRORLEVEL 1 (
	echo CL is required to build, please run the vcvars##.bat to initialize the environment
	exit /b 1
)

where.exe nasm.exe
IF ERRORLEVEL 1 (
	echo NASM needs to be available on PATH
	exit /b 1
)



REM build mozjpegturbo libs
echo == Build deps\mozjpeg ==
pushd deps\mozjpeg
mkdir build
cd build
IF EXIST "jpeg.lib" (
	REM use any cached version if exists
	echo JPEG.LIB already exists, not rebuilding mozjpeg
	GOTO SKIP_MOZJPEG
)
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
:SKIP_MOZJPEG
REM copy the required linked DLL file from mozjpeg\build
echo Copying jpeg62.dll to base directory...
copy jpeg62.dll ..\..\..\
popd



echo == Build src\iqa ==
pushd src\iqa
MsBuild.exe iqa.sln /t:Build /p:Configuration=Release /p:Platform=x64 /p:DebugSymbols=false /p:DebugType=None
IF ERRORLEVEL 1 (
	echo iqa msbuild failed
	exit /b 1
)
popd


echo == Build jpeg-archive ==
nmake.exe /NOLOGO -f Makefile.w32
IF ERRORLEVEL 1 (
	echo jpeg-archive nmake failed
	exit /b 1
)

echo Success!  Find built files in %CD%
