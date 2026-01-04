for /f %%a in ('wmic os get localdatetime ^| find "."') do set datetime=%%a

REM Split into parts
set "YYYY=%datetime:~0,4%"
set "MM=%datetime:~4,2%"
set "DD=%datetime:~6,2%"
set "HH=%datetime:~8,2%"
set "MN=%datetime:~10,2%"
set "SS=%datetime:~12,2%"

set "TIMESTAMP=%YYYY%-%MM%-%DD%_%HH%-%MN%-%SS%"

REM Use timestamp in filename
set "FILENAME=log_%TIMESTAMP%.txt"
echo Creating file: %FILENAME%
echo Log created at %TIMESTAMP% > "%FILENAME%"