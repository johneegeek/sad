:: Configuration script.
@echo off
perl configure.pl
if  %ERRORLEVEL% NEQ 0 goto end
:: If we succeeded, we should be able to CD to the build 
:: directory.
cd build
:end