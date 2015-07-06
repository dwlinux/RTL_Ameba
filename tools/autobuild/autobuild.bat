@echo off
::*****************************************************************************#
::                      Set Parameters	                                       #
::*****************************************************************************#
cls
setlocal enableextensions enabledelayedexpansion

:: change to en-US Locale for parsing formatting some command results
chcp 437 >NUL

set RTK_AUTOBUILD_VER=1.0
echo Realtek autobuild batch ver %RTK_AUTOBUILD_VER%
echo.

set LOG_DEBUG=1

set ROOT_DIR=..\..
set PROJ_DIR=%ROOT_DIR%\project

:: detect IAR default folder in either 32-bit or 64-bit system
if "%ProgramFiles(x86)%"=="" (
	set DEFAULT_IAR_FOLDER="%ProgramFiles%\IAR Systems"
) else (
	::set DEFAULT_IAR_FOLDER="%ProgramFiles(x86)%\IAR Systems"
	set DEFAULT_IAR_FOLDER="%ProgramFiles(x86)%\IAR Systems"
)
set DEFAULT_IAR_FOLDER=%DEFAULT_IAR_FOLDER:"=%


echo Welcome, %USERNAME%

::*****************************************************************************#
::                      Check IAR environment                                  #
::*****************************************************************************#

set IAR_DIR=

if exist "%DEFAULT_IAR_FOLDER%" (

	set counter=0
	for /f "delims=" %%D in ('dir /a:d /b "%DEFAULT_IAR_FOLDER%"') do (
		set /a counter+=1
	)

	if !counter! EQU 1 (
		for /f "delims=" %%D in ('dir /a:d /b "%DEFAULT_IAR_FOLDER%"') do (
			set IAR_DIR="%DEFAULT_IAR_FOLDER%\%%~nxD"
		)
	) else (
		if !counter! EQU 0 (
			echo There is no IAR system in default directory
			set /p IAR_DIR="Please input the IAR directory: "
			if not exist "%IAR_DIR%" (
				goto invalid_iar_dir
			)
		) else (
			echo There are several IAR system found in default directory
			echo Select which IAR system to build:
			echo.
			set counter=1
			for /f "delims=" %%D in ('dir /a:d /b "%DEFAULT_IAR_FOLDER%"') do (
				echo !counter!. %%~nxD
				set /a counter+=1
			)
			echo. 
			echo 0. Quit
			echo.

			set /p choice="Enter your choice: "
			if !choice! EQU 0 goto exit
			if !choice! GEQ counter goto invalid_iar_dir

			set counter=1
			for /f "delims=" %%D in ('dir /a:d /b "%DEFAULT_IAR_FOLDER%"') do (
				if !choice!==!counter! set IAR_DIR="%DEFAULT_IAR_FOLDER%\%%~nxD"
				set /a counter+=1
			)
		)
	)
) else (
	echo There is no IAR system in default directory
	set /p IAR_DIR="Please input the IAR directory: "
)

set IAR_DIR=%IAR_DIR:"=%
if not exist "%IAR_DIR%" (
	goto invalid_iar_dir
)

set IAR_EXE=%IAR_DIR%\common\bin\IarBuild.exe
if not exist "%IAR_EXE%" (
	goto invalid_iar_dir
)

::*****************************************************************************#
::                      Operation	                                           #
::*****************************************************************************#


:: list projects

set counter=0
for /f "delims=" %%D in ('dir /a:d /b %PROJ_DIR%') do (
	set /a counter+=1
)
if !counter! EQU 0 (
	goto no_project_found
)
if !counter! GEQ 2 (
	echo.
	echo Select which project to build:
	echo.

	set counter=1
	for /f "delims=" %%D in ('dir /a:d /b %PROJ_DIR%') do (
		echo !counter!. %%~nxD
		set /a counter+=1
	)
	echo. 
	echo 0. Quit
	echo.

	set /p choice="Enter your choice: "
	if "%choice%"=="0" goto exit
	if !choice! GEQ !counter! goto invalid_option

	set counter=1
	set TARGET_PROJ=
	for /f "delims=" %%D in ('dir /a:d /b %PROJ_DIR%') do (
		if !choice!==!counter! set TARGET_PROJ=%%~nxD
		set /a counter+=1
	)	
) else (
	set TARGET_PROJ=
	for /f "delims=" %%D in ('dir /a:d /b %PROJ_DIR%') do (
		set TARGET_PROJ=%%~nxD
	)	
)

:: list peripherals
echo.
echo target project is %TARGET_PROJ%
echo.
echo Select which peripheral to build:
echo.

set TARGET_PROJ_DIR=%ROOT_DIR%\project\%TARGET_PROJ%
set TARGET_PROJ_EWP=%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp
set TARGET_RAM_ALL_BIN=%TARGET_PROJ_DIR%\EWARM-RELEASE\Debug\Exe\ram_all.bin
set PERIPHERAL_DIR=%TARGET_PROJ_DIR%\example_sources

echo 1. default ^(without copy files^)
set counter=2
for /f "delims=" %%D in ('dir /a:d /b %PERIPHERAL_DIR%') do (
	echo !counter!. %%~nxD
	set /a counter+=1
)
echo. 
echo 0. Quit
echo.

set /p choice="Enter your choice: "
if "%choice%"=="0" goto exit
if !choice! GEQ !counter! goto invalid_option

if !choice! NEQ 1 (
	set counter=2
	set TARGET_PERIPHERAL=
	for /f "delims=" %%D in ('dir /a:d /b %PERIPHERAL_DIR%') do (
		if !choice!==!counter! set TARGET_PERIPHERAL=%%~nxD
		set /a counter+=1
	)
)

echo.
if "%TARGET_PERIPHERAL%"=="" (
	echo target peripheral is default
) else (
	echo target peripheral is %TARGET_PERIPHERAL%
)
set TARGET_PERIPHERAL_DIR=%PERIPHERAL_DIR%\%TARGET_PERIPHERAL%

if exist %TARGET_PROJ_DIR%\EWARM-RELEASE\Backup* (
	del %TARGET_PROJ_DIR%\EWARM-RELEASE\Backup* 2>&1 >NUL
)

::*****************************************************************************#
::                      Copy Peripheral files and build                        #
::*****************************************************************************#

:: setup login
for /f "skip=1" %%x in ('wmic os get localdatetime') do if not defined MYDATE set MYDATE=%%x
if "%TARGET_PERIPHERAL%"=="" (
	set LOG_FILE="log\log_%TARGET_PROJ%_default_%MYDATE:~0,8%_%MYDATE:~8,4%.txt"
) else (
	set LOG_FILE="log\log_%TARGET_PROJ%_%TARGET_PERIPHERAL%_%MYDATE:~0,8%_%MYDATE:~8,4%.txt"
)

echo.
echo Build Environment:
echo IAR_DIR                  "%IAR_DIR%"
echo IAR_EXE                  "%IAR_EXE%"
echo ROOT_DIR                 "%ROOT_DIR%"
echo PROJ_DIR                 "%PROJ_DIR%"
echo TARGET_PROJ              "%TARGET_PROJ%"
echo TARGET_PROJ_DIR          "%TARGET_PROJ_DIR%"
echo TARGET_PROJ_EWP          "%TARGET_PROJ_EWP%"
echo TARGET_RAM_ALL_BIN       "%TARGET_RAM_ALL_BIN%"
echo PERIPHERAL_DIR           "%PERIPHERAL_DIR%"
echo TARGET_PERIPHERAL        "%TARGET_PERIPHERAL%"
echo TARGET_PERIPHERAL_DIR    "%TARGET_PERIPHERAL_DIR%"
echo.

if !LOG_DEBUG! EQU 1 (
	if not exist log (
		mkdir log 2>&1 >NUL
	)
	echo Realtek autobuild batch ver %RTK_AUTOBUILD_VER%    > %LOG_FILE%
	echo %DATE% %TIME%                                      >> %LOG_FILE%
	echo.                                                   >> %LOG_FILE%
	echo ################ ENVIRONMENT ################      >> %LOG_FILE%
	echo.                                                   >> %LOG_FILE%
	echo IAR_DIR                  "%IAR_DIR%"               >> %LOG_FILE%
	echo IAR_EXE                  "%IAR_EXE%"               >> %LOG_FILE%
	echo ROOT_DIR                 "%ROOT_DIR%"              >> %LOG_FILE%
	echo PROJ_DIR                 "%PROJ_DIR%"              >> %LOG_FILE%
	echo TARGET_PROJ              "%TARGET_PROJ%"           >> %LOG_FILE%
	echo TARGET_PROJ_DIR          "%TARGET_PROJ_DIR%"       >> %LOG_FILE%
    echo TARGET_PROJ_EWP          "%TARGET_PROJ_EWP%"       >> %LOG_FILE%
	echo TARGET_RAM_ALL_BIN       "%TARGET_RAM_ALL_BIN%"    >> %LOG_FILE%
	echo PERIPHERAL_DIR           "%PERIPHERAL_DIR%"        >> %LOG_FILE%
	echo TARGET_PERIPHERAL        "%TARGET_PERIPHERAL%"     >> %LOG_FILE%
	echo TARGET_PERIPHERAL_DIR    "%TARGET_PERIPHERAL_DIR%" >> %LOG_FILE%
	echo.                                                   >> %LOG_FILE%
)

if exist "%TARGET_RAM_ALL_BIN%" (
	del "%TARGET_RAM_ALL_BIN%"
)

if "%TARGET_PERIPHERAL%"=="" (
	echo use default and skip copy files
) else (
	xcopy "%TARGET_PERIPHERAL_DIR%" "%TARGET_PROJ_DIR%"  /e /y > nul
)

:: start to build
echo Building...

if !LOG_DEBUG! EQU 1 (
	echo ################ MAKE CLEAN ################ >> %LOG_FILE%
	"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -clean Debug -log all 2>&1  >> %LOG_FILE%

	echo ################ BUILD ################ >> %LOG_FILE%
	"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -build Debug -log all 2>&1  >> %LOG_FILE%
) else (
	"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -clean Debug -log all 2>&1  >NUL
	"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -build Debug -log all 2>&1  >NUL
)

:: wait for IAR to generate ram_all.bin
timeout /t 10 /nobreak 2>&1 >NUL

:: check if ram_all.bin is produced
if not exist "%TARGET_RAM_ALL_BIN%" (
	goto build_fail
)

echo Build finish

::*****************************************************************************#
::                      Search mbed device and download bin file               #
::*****************************************************************************#

set MBED_LABEL="MBED"

set mbeddisk=
for /f "tokens=*" %%d in ('"wmic logicaldisk get caption,volumename"') do (
	call :parse_disk_line %%d
)

if "%mbeddisk%"=="" (
	echo.
	echo Warning, please plug in device
	pause
)

set mbeddisk=
for /f "tokens=*" %%d in ('"wmic logicaldisk get caption,volumename"') do (
	call :parse_disk_line %%d
)

if "%mbeddisk%"=="" (
	goto no_mbed_device_found
)

echo.
echo Download Environment:
echo MBED_LABEL               %MBED_LABEL%
echo mbeddisk                 "%mbeddisk%"

if !LOG_DEBUG! EQU 1 (
	echo ################ DOWNLOAD ################ >> %LOG_FILE%
	echo.                                           >> %LOG_FILE%
	echo Download Environment:                      >> %LOG_FILE%
	echo MBED_LABEL        %MBED_LABEL%             >> %LOG_FILE%
	echo mbeddisk          "%mbeddisk%"             >> %LOG_FILE%
	echo.                                           >> %LOG_FILE%
)

echo.
echo Downloading...

if !LOG_DEBUG! EQU 1 (
	echo copy "%TARGET_RAM_ALL_BIN%" "%mbeddisk%"   >> %LOG_FILE%
	copy /y "%TARGET_RAM_ALL_BIN%" "%mbeddisk%"     >> %LOG_FILE%
) else (
	copy /y "%TARGET_RAM_ALL_BIN%" "%mbeddisk%"     >NUL
)
echo Download finish
echo.
echo Program Success, please reboot your device

goto endofdownload %mbeddisk%

:: sub routine for parsing disk label to find out mbed disk driver
:parse_disk_line
	if "%2"==%MBED_LABEL% (
		set mbeddisk=%1
	)
exit /b

:endofdownload

::*****************************************************************************#
::                      Error Handling                                         #
::*****************************************************************************#
endlocal
goto exit

:invalid_iar_dir
echo.
echo Program fail, the IAR directory is invalid
echo.
goto exit

:no_project_found
echo.
echo Program fail, No project in project folder
echo.
goto exit

:invalid_option
echo.
echo Program fail, you select invalid option
echo.
goto exit

:build_fail
echo.
echo Program fail, build Fail
echo.
goto exit

:no_mbed_device_found
echo.
echo Program fail, no device found
echo.
goto exit

:exit
pause
