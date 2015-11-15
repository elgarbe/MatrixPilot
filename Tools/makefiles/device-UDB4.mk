# this file is included from makefile

TOOLCHAIN ?= XC16
TARGET_TYPE := hex
CPU := 33FJ256GP710A

modules += libUDB libVectorMatrix libFlashFS Microchip
incpath += Microchip Microchip/Include libFlashFS libVectorMatrix
#cfgpath := Config

# mod_list
# inc_list
# def_list
