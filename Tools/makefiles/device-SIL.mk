# this file is included from makefile

TOOLCHAIN := GCC
ifeq ($(OS),Windows_NT)
TARGET_TYPE := exe
else
TARGET_TYPE := out
endif
CPU :=

modules += Tools/MatrixPilot-SIL FreeRTOS
incpath += FreeRTOS/include FreeRTOS/portable FreeRTOS/portable/MSVC-MingW
#cfgpath := Config
