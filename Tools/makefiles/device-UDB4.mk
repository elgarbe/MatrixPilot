# this file is included from makefile

TOOLCHAIN ?= XC16
TARGET_TYPE := hex
CPU := 33FJ256GP710A

modules += libUDB libVectorMatrix
incpath += Microchip Microchip/Include libVectorMatrix
#cfgpath := Config

defines += NOFS=1

# mod_list
# inc_list
# def_list