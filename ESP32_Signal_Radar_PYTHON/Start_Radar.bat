@echo off
title ESP32 RADAR CONTROL PANEL
C:\Users\Administrator\Documents\RADARWEBSOCKET
echo ====================================
echo   ESP32 RADAR SYSTEM LAUNCHER
echo ====================================
echo.

echo [1/5] Closing Python servers...
taskkill /F /IM python.exe >nul 2>nul

echo [2/5] Closing Arduino IDE (all variants)...

:: Arduino classic
taskkill /F /IM arduino.exe >nul 2>nul

:: Arduino IDE 2.x
taskkill /F /IM arduino-ide.exe >nul 2>nul

:: fallback (sometimes used internally)
taskkill /F /IM javaw.exe >nul 2>nul

echo [3/5] Releasing COM port wait...
timeout /t 3 >nul

echo [4/5] Starting fresh server...
start "Radar Server" cmd /k python server.py

timeout /t 2 >nul

echo [5/5] Opening dashboard...
start http://localhost:5000

echo.
echo SYSTEM ONLINE
pause
