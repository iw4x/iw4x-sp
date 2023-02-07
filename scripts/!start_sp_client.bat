@echo off
:: IW4x-SP Client Configuration start-up file

:: Either put the batch in the IW4x-SP install dir, or change the value below to the IW4x-SP install dir
set XLABS_MW2_INSTALL=%~dp0

:: Remove this line, if automatic updates on start should be disabled
start /W xlabs.exe -update

:: You're done!! WARNING!!! Don't mess with anything below this line
set GAME_EXE=%localappdata%\xlabs\data\iw4x\iw4x-sp.exe

start %GAME_EXE%
