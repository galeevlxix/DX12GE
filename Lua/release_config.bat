@echo off
set SCRIPT="%TEMP%\CreateShortcut.vbs"

:: --- НАСТРОЙКИ (АВТОМАТИЧЕСКИЕ) ---
:: %CD% - это текущая папка, откуда запущен скрипт
:: %USERPROFILE% - это папка пользователя (C:\Users\Name)

set EXEPATH="%CD%\DX12GE.exe"
set LNKPATH="%CD%\DX12GE (Release).lnk"
set ARGS="-release"
set WORKDIR="%CD%"

:: --- ГЕНЕРАЦИЯ VBS ---
echo Set oWS = WScript.CreateObject("WScript.Shell") > %SCRIPT%
echo sLinkFile = %LNKPATH% >> %SCRIPT%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %SCRIPT%
echo oLink.TargetPath = %EXEPATH% >> %SCRIPT%
echo oLink.Arguments = %ARGS% >> %SCRIPT%
echo oLink.WorkingDirectory = %WORKDIR% >> %SCRIPT%
echo oLink.Save >> %SCRIPT%

:: --- ЗАПУСК ---
cscript /nologo %SCRIPT%
del %SCRIPT%

echo Universal shortcut created on Desktop!
pause