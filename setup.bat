@echo off
echo ================================================
echo   Sistema de Monitoreo Acuatico - BLUME x IMARPE
echo   Setup
echo ================================================
echo.

echo [1/3] Creando entorno virtual...
python -m venv .venv
if errorlevel 1 ( echo ERROR: Python no encontrado. & pause & exit /b 1 )

echo [2/3] Instalando dependencias...
.venv\Scripts\pip install -q -r requirements.txt
if errorlevel 1 ( echo ERROR: Fallo al instalar dependencias. & pause & exit /b 1 )

echo [3/3] Inicializando base de datos...
.venv\Scripts\python init.py
if errorlevel 1 ( echo ERROR: Fallo al inicializar la base de datos. & pause & exit /b 1 )

echo.
echo ================================================
echo   Setup completo.
echo.
echo   Para iniciar el sistema:
echo     1. .venv\Scripts\python server.py
echo     2. .venv\Scripts\python data\scripts\simulador.py
echo.
echo   Luego abre: http://localhost:5000
echo ================================================
pause
