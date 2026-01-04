@echo off

set "param=%1"
if "%1"=="" (
    set "param=esp32_tft"
)

for /f %%a in ('wmic os get localdatetime ^| find "."') do set datetime=%%a

REM Split into parts
set "YYYY=%datetime:~0,4%"
set "MM=%datetime:~4,2%"
set "DD=%datetime:~6,2%"
set "HH=%datetime:~8,2%"
set "MN=%datetime:~10,2%"
set "SS=%datetime:~12,2%"

set "TIMESTAMP=%YYYY%%MM%%DD%-%HH%%MN%%SS%"

REM Use timestamp in filename
set "FILENAME=log_%TIMESTAMP%.txt"
echo Creating file: %FILENAME%

REM 0x210000 littlefs.bin

echo Parameter is: %param%
copy ..\.pio\build\%param%\*.bin .
copy ..\esp32\boot_app0.bin .
esptool.exe --chip esp32 merge_bin  --flash_mode dio --flash_freq 80m --flash_size 4MB  0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin 0x210000 littlefs.bin -o ../lvgl_mb/firmware/firmware_LVGL_databot2.0_%param%_%TIMESTAMP%.bin
del boot_app0.bin
del bootloader.bin
del partitions.bin
del firmware.bin

