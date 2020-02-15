@echo off

mkdir ..\build
pushd ..\build
cl -Zi ..\code\main.cpp hid.lib setupapi.lib winusb.lib winmm.lib ntdll.lib
popd

