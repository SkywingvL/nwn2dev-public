@echo off
set _NTTREE=%~dp0\
set NO_BINPLACE=
doskey BLD=build -ge $*
doskey BCZ=build -gecZP $*
