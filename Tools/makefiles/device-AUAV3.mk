# this file is included from makefile

#TOOLCHAIN ?= C30
TOOLCHAIN ?= XC16
TARGET_TYPE := hex
CPU := 33EP512MU810

modules += libUDB libVectorMatrix libFlashFS Microchip
incpath += Microchip Microchip/Include libFlashFS libVectorMatrix
#cfgpath := Config
