@set msg=
call compile rc /fo"settings.res" src/settings.rc
call compile p1 src/emul.cpp /Fe"release/p1/unreal.exe" /link settings.res
fs -rn release/p1/unreal.exe >nul
fs -c  release/p1/unreal.exe >nul
@echo %MSG%
::net send CRAY "compile done"
