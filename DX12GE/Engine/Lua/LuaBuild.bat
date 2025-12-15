@echo off
setlocal enabledelayedexpansion

:: ====== CONFIGURATION SECTION ======
:: Set the number of directory levels to go up from the script location
:: 1 = one level up, 2 = two levels up, etc.
set "LEVELS_UP=2"
:: ====== END CONFIGURATION ======

:: Get the current directory where the .bat file is located
set "CURRENT_DIR=%~dp0"
:: Remove trailing backslash if exists
if "%CURRENT_DIR:~-1%"=="\" set "CURRENT_DIR=%CURRENT_DIR:~0,-1%"

echo Current directory: "%CURRENT_DIR%"
echo Configured to go up %LEVELS_UP% directory levels

:: Check for DX12GE.exe in the current folder
if not exist "%CURRENT_DIR%\DX12GE.exe" (
    echo ERROR: DX12GE.exe file not found in directory "%CURRENT_DIR%"
    echo Please place this script in the folder containing DX12GE.exe
    pause
    exit /b 1
)

echo DX12GE.exe file found.

:: Navigate up the specified number of levels using pushd/popd method
pushd "%CURRENT_DIR%"
echo Starting directory navigation...
echo Current location: %cd%

:: Go up the specified number of levels
for /l %%i in (1,1,%LEVELS_UP%) do (
    cd ..
    echo Moved up to level %%i: %cd%
)

set "PROJECT_ROOT=%cd%"
popd

echo Project root folder: "%PROJECT_ROOT%"

:: Check if the root folder exists
if not exist "%PROJECT_ROOT%" (
    echo ERROR: Project root folder does not exist: "%PROJECT_ROOT%"
    pause
    exit /b 1
)

:: Create a temporary file for the list of .lua files
set "LUA_LIST=%TEMP%\lua_files_%RANDOM%.txt"
del "%LUA_LIST%" >nul 2>&1

:: Search for all .lua files recursively in the root folder
echo Searching for all .lua files in "%PROJECT_ROOT%"...
dir /s /b "%PROJECT_ROOT%\*.lua" > "%LUA_LIST%"

:: Check if the search was successful
if errorlevel 1 (
    echo ERROR: Failed to search for .lua files
    pause
    exit /b 1
)

:: Check if files were found
if not exist "%LUA_LIST%" (
    echo ERROR: Failed to create file list
    pause
    exit /b 1
)

:: Count the number of found files
set "FILE_COUNT=0"
for /f %%a in ('type "%LUA_LIST%" ^| find /c /v ""') do set "FILE_COUNT=%%a"

if %FILE_COUNT% equ 0 (
    echo ERROR: No .lua files found in "%PROJECT_ROOT%"
    del "%LUA_LIST%" >nul 2>&1
    pause
    exit /b 1
)

echo Found %FILE_COUNT% .lua files:
echo Listing files...
for /f "usebackq delims=" %%f in ("%LUA_LIST%") do (
    set "FILENAME=%%~nxf"
    echo   !FILENAME!
)

:: ====== CORRECTED PATH HANDLING ======
:: Build the copy command properly without escaped quotes

:: Start with the base executable
set "COPY_COMMAND=copy /b "%CURRENT_DIR%\DX12GE.exe""

:: Add all .lua files to the command
set "FILE_COUNTER=0"
for /f "usebackq delims=" %%f in ("%LUA_LIST%") do (
    set /a FILE_COUNTER+=1
    set "COPY_COMMAND=!COPY_COMMAND! + "%%f""
    :: Show progress for large numbers of files
    if !FILE_COUNTER! lss 10 (
        echo Adding file !FILE_COUNTER!: %%~nxf
    ) else if !FILE_COUNTER! equ 10 (
        echo ... (showing first 10 files, %FILE_COUNT% total)
    )
)

:: Add the output file
set "COPY_COMMAND=!COPY_COMMAND! "%CURRENT_DIR%\Game.exe""

:: ====== END CORRECTED PATH HANDLING ======

:: Execute the merge
echo.
echo Merging files...
echo Command: !COPY_COMMAND!
!COPY_COMMAND! >nul

if errorlevel 1 (
    echo ERROR: Failed to merge files - command failed
    echo Please check the paths and try again
    del "%LUA_LIST%" >nul 2>&1
    pause
    exit /b 1
)

:: Check the result
if exist "%CURRENT_DIR%\Game.exe" (
    echo.
    echo SUCCESS! Game.exe has been successfully created.
    
    :: Get and display file sizes
    for %%F in ("%CURRENT_DIR%\DX12GE.exe") do set "ORIGINAL_SIZE=%%~zF"
    for %%F in ("%CURRENT_DIR%\Game.exe") do set "FINAL_SIZE=%%~zF"
    
    echo Original DX12GE.exe size: !ORIGINAL_SIZE! bytes
    echo Final Game.exe size: !FINAL_SIZE! bytes
    
    :: Calculate and show size increase
    set /a SIZE_INCREASE=!FINAL_SIZE! - !ORIGINAL_SIZE!
    echo Size increase: !SIZE_INCREASE! bytes
    
    echo.
    echo Build completed successfully!
) else (
    echo ERROR: Failed to create Game.exe
    del "%LUA_LIST%" >nul 2>&1
    pause
    exit /b 1
)

:: Clean up temporary files
del "%LUA_LIST%" >nul 2>&1
echo Temporary files cleaned up.

:: Show summary information
echo.
echo =================================================
echo                   BUILD SUMMARY
echo =================================================
echo - Source executable: "%CURRENT_DIR%\DX12GE.exe"
echo - Project root folder: "%PROJECT_ROOT%"
echo - Directory levels navigated up: %LEVELS_UP%
echo - Found .lua files: %FILE_COUNT%
echo - Output file: "%CURRENT_DIR%\Game.exe"
echo - Final file size: !FINAL_SIZE! bytes
echo =================================================

echo.
echo Process completed successfully!
endlocal
pause
