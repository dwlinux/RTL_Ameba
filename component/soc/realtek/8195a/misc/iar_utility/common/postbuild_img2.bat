cd /D %2
set tooldir=%2\..\..\..\component\soc\realtek\8195a\misc\iar_utility\common\tools
set libdir=%2\..\..\..\component\soc\realtek\8195a\misc\bsp

del Debug/Exe/target.map Debug/Exe/application.asm *.bin
cmd /c "%tooldir%\nm Debug/Exe/application.axf | %tooldir%\sort > Debug/Exe/application.map"
cmd /c "%tooldir%\objdump -d Debug/Exe/application.axf > Debug/Exe/application.asm"

for /f "delims=" %%i in ('cmd /c "%tooldir%\grep IMAGE2 Debug/Exe/application.map | %tooldir%\grep Base | %tooldir%\gawk '{print $1}'"') do set ram2_start=0x%%i
for /f "delims=" %%i in ('cmd /c "%tooldir%\grep SDRAM  Debug/Exe/application.map | %tooldir%\grep Base | %tooldir%\gawk '{print $1}'"') do set ram3_start=0x%%i

for /f "delims=" %%i in ('cmd /c "%tooldir%\grep IMAGE2 Debug/Exe/application.map | %tooldir%\grep Limit | %tooldir%\gawk '{print $1}'"') do set ram2_end=0x%%i
for /f "delims=" %%i in ('cmd /c "%tooldir%\grep SDRAM  Debug/Exe/application.map | %tooldir%\grep Limit | %tooldir%\gawk '{print $1}'"') do set ram3_end=0x%%i

::echo %ram1_start% > tmp.txt
::echo %ram2_start% >> tmp.txt
::echo %ram3_start% >> tmp.txt
::echo %ram1_end% >> tmp.txt
::echo %ram2_end% >> tmp.txt
::echo %ram3_end% >> tmp.txt

%tooldir%\objcopy -j "A3 rw" -Obinary Debug/Exe/application.axf Debug/Exe/ram_2.bin
if defined %ram3_start (
	%tooldir%\objcopy -j "A5 rw" -Obinary Debug/Exe/application.axf Debug/Exe/sdram.bin
)

%tooldir%\pick %ram2_start% %ram2_end% Debug\Exe\ram_2.bin Debug\Exe\ram_2.p.bin body+reset_offset+sig
if defined %ram3_start (
%tooldir%\pick %ram3_start% %ram3_end% Debug\Exe\sdram.bin Debug\Exe\ram_3.p.bin body+reset_offset
)

:: check ram_1.p.bin exist, copy default
if not exist Debug\Exe\ram_1.p.bin (
	copy %libdir%\image\ram_1.p.bin Debug\Exe\ram_1.p.bin
)

::if not exist Debug\Exe\data.p.bin (
::	copy %tooldir%\..\image\data.p.bin Debug\Exe\data.p.bin
::)

::padding ram_1.p.bin to 32K+4K+4K+4K, LOADER/RSVD/SYSTEM/CALIBRATION
%tooldir%\padding 44k 0xFF Debug\Exe\ram_1.p.bin

:: SDRAM case
if defined %ram3_start (
copy /b Debug\Exe\ram_1.p.bin+Debug\Exe\ram_2.p.bin+Debug\Exe\ram_3.p.bin Debug\Exe\ram_all.bin
copy /b Debug\Exe\ram_2.p.bin+Debug\Exe\ram_3.p.bin Debug\Exe\ota.bin
)

:: NO SDRAM case
if not defined %ram3_start (
copy /b Debug\Exe\ram_1.p.bin+Debug\Exe\ram_2.p.bin Debug\Exe\ram_all.bin
copy /b Debug\Exe\ram_2.p.bin Debug\Exe\ota.bin
)

:: board generator
%tooldir%\..\gen_board_img2.bat %ram2_start% %ram2_end% %ram3_end%

exit