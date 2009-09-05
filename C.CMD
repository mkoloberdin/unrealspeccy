@set msg=
call compile rc /fo"settings.res" src/settings.rc
::call compile p4 src/emul.cpp /Fe"unreal.exe" /YX"stdheads.pch" /link settings.res
call compile ms src/emul.cpp /Fe"unreal.exe" /YX"stdheads.pch" /link settings.res
@if errorlevel 1 goto fail
fs -rn unreal.exe >nul
fs -c  unreal.exe >nul
@goto buildok
:fail
@echo %MSG%
:buildok
@echo %MSG%
::net send CRAY "compile done"
