@echo off
setlocal

gcc -std=c17 -Wall -Wextra -Wpedantic src\Main.c src\Command.c src\Database.c src\storage.c src\PageManager.c src\BufferPool.c src\RecordManager.c src\SQLParser.c src\TableManager.c src\TableRecordManager.c src\SQL6.c -o Main.exe
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
echo Build completed: Main.exe
endlocal
