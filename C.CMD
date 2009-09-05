call compile rc settings.rc
call compile ms emul.cpp /link settings.res
@if errorlevel 1 goto fail
fs -rn emul.exe >nul
fs -c  emul.exe  >nul
copy emul.exe unreal.exe
@goto buildok
:fail
@echo 
:buildok
@echo 
::net send CRAY "compile done"
