@echo off
if %1 == ALL goto _Build_ALL
if %1 == AMD64 goto _Build_AMD64
if %1 == X86 goto _Build_X86

echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Build type is not set, Use AMD64 As Default
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
goto _Build_AMD64

:_Build_AMD64
call builda_AMD64.cmd
goto END

:_Build_X86
call builda_X86.cmd
goto END

:_Build_ALL
call builda_AMD64.cmd
call builda_X86.cmd
:END