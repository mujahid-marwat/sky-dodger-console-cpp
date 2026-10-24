@echo off
cd /d "%~dp0"
if exist ".venv\Scripts\python.exe" (
    ".venv\Scripts\python.exe" "%~dp0sky_dodger.py"
) else (
    py -3 "%~dp0sky_dodger.py"
)
if errorlevel 1 (
    echo.
    echo The game could not start. Install Python 3 with Tcl/Tk support,
    echo or open the folder in VS Code and run sky_dodger.py.
    pause
)
