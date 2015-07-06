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
::                      Select Project                                         #
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

echo.
echo target project is %TARGET_PROJ%
echo.

set TARGET_PROJ_DIR=%ROOT_DIR%\project\%TARGET_PROJ%
set TARGET_PROJ_EWP=%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp
set TARGET_RAM_ALL_BIN=%TARGET_PROJ_DIR%\EWARM-RELEASE\Debug\Exe\ram_all.bin
set PERIPHERAL_DIR=%TARGET_PROJ_DIR%\example_sources

::*****************************************************************************#
::                      Build Peripherals                                      #
::*****************************************************************************#

:: list peripherals
if not exist bin (
	mkdir bin 2>&1 >NUL
)

if exist bin\ram_all* (
	del bin\ram_all* 2>&1 >NUL
)

:: build_default
set TARGET_PERIPHERAL=
call :build_code

:: build others
set TARGET_PERIPHERAL=
for /f "delims=" %%D in ('dir /a:d /b %PERIPHERAL_DIR%') do (
	set TARGET_PERIPHERAL=%%~nxD
	call :build_code
)

:build_code

	:: setup login
	for /f "skip=1" %%x in ('wmic os get localdatetime') do if not defined MYDATE set MYDATE=%%x
	if "%TARGET_PERIPHERAL%"=="" (
		set LOG_FILE="log\log_%TARGET_PROJ%_default_%MYDATE:~0,8%_%MYDATE:~8,4%.txt"
	) else (
		set LOG_FILE="log\log_%TARGET_PROJ%_%TARGET_PERIPHERAL%_%MYDATE:~0,8%_%MYDATE:~8,4%.txt"
	)

	set TARGET_PERIPHERAL_DIR=%PERIPHERAL_DIR%\%TARGET_PERIPHERAL%

	if exist "%TARGET_RAM_ALL_BIN%" (
		del %TARGET_RAM_ALL_BIN%
	)

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

	if exist %TARGET_PROJ_DIR%\EWARM-RELEASE\Backup* (
		del %TARGET_PROJ_DIR%\EWARM-RELEASE\Backup* 2>&1 >NUL
	)

	if "%TARGET_PERIPHERAL%"=="" (
		echo start build default
	) else (
		echo start build %TARGET_PERIPHERAL%
		::xcopy "%TARGET_PERIPHERAL_DIR%" "%TARGET_PROJ_DIR%"  /e /y > nul
		xcopy "%TARGET_PERIPHERAL_DIR%" "%TARGET_PROJ_DIR%"  /e /y
	)

	if !LOG_DEBUG! EQU 1 (
		echo ################ MAKE CLEAN ################ >> %LOG_FILE%
		"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -clean Debug -log all 2>&1  >> %LOG_FILE%

		echo ################ BUILD ################ >> %LOG_FILE%
		"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -build Debug -log all 2>&1  >> %LOG_FILE%
	) else (
		"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -clean Debug -log all 2>&1  >NUL
		"%IAR_EXE%"  "%TARGET_PROJ_DIR%\EWARM-RELEASE\Project.ewp" -build Debug -log all 2>&1  >NUL
	)

	::timeout /t 10 /nobreak 2>&1 >NUL
	sleep 20

	if exist "%TARGET_RAM_ALL_BIN%" (
		if "%TARGET_PERIPHERAL%"=="" (
			copy %TARGET_RAM_ALL_BIN% bin\ram_all_default.bin
		) else (
			copy %TARGET_RAM_ALL_BIN% bin\ram_all_%TARGET_PERIPHERAL%.bin
		)
	)
exit /b

echo Build finish

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

:exit
pause
