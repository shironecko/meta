@echo off

if not exist ..\build mkdir ..\build

cl test.c ^
	/Fe..\build\ /Fd..\build\ /Fo..\build\ ^
	/nologo /Od /Zi /MTd ^
	/link /subsystem:console
