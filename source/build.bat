@echo off

if not exist ..\build mkdir ..\build

cl parser.c ^
	/Fe..\build\ /Fd..\build\ /Fo..\build\ ^
	/nologo /Od /Zi /MTd ^
	/link /subsystem:console
