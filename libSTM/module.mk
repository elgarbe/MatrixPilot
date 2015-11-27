# this file is included from makefile

# NOTE: this test is no longer required - since we now include module.mk files explicitly
ifeq ($(DEVICE),PX4)

extra_dirs := Src Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates Drivers/STM32F4xx_HAL_Driver/Src Middlewares/Third_Party/FreeRTOS/Source Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang Middlewares/Third_Party/FatFs/src Middlewares/Third_Party/FatFs/src/drivers Middlewares/Third_Party/FatFs/src/option

$(call mkoutdir, $(addprefix $(subdirectory)/,$(extra_dirs)))

local_src := $(wildcard $(SOURCE_DIR)/$(subdirectory)/*.c)
local_src += $(foreach i,$(extra_dirs),$(wildcard $(SOURCE_DIR)/$(subdirectory)/$(i)/*.c))

#local_inc := Config FreeRTOS/include libUDB libSTM libSTM/Inc libSTM/Src
#local_inc += libSTM/Drivers libSTM/Drivers/CMSIS/Device/ST/STM32F4xx/Include libSTM/Drivers/CMSIS/Include libSTM/Drivers/STM32F4xx_HAL_Driver/Inc
#local_inc += libSTM/Middlewares libSTM/Middlewares/Third_Party/FreeRTOS/Source/include libSTM/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS libSTM/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

local_inc := Inc Src Drivers Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include Drivers/STM32F4xx_HAL_Driver/Inc Middlewares Middlewares/Third_Party/FreeRTOS/Source/include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F Middlewares\Third_Party\FatFs\src Middlewares/Third_Party/FatFs/src/drivers ../libUDB

# NOTE: include the header files directory for the libUDB interface we are fullfilling (temporarily removed as the below breaks the project generator, which cannot yet handle +=
#local_inc := ../libUDB

#local_inc += Inc Src
#local_inc += Drivers
#local_inc += Drivers/CMSIS/Device/ST/STM32F4xx/Include
#local_inc += Drivers/CMSIS/Include
#local_inc += Drivers/STM32F4xx_HAL_Driver/Inc
#local_inc += Middlewares
#local_inc += Middlewares/Third_Party/FreeRTOS/Source/include
#local_inc += Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
#local_inc += Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
#local_inc += Middlewares/Third_Party/FatFs/src
#local_inc += Middlewares/Third_Party/FatFs/src/drivers
#local_inc += ../libUDB

#incpath += $(addprefix $(SOURCE_DIR)/$(subdirectory)/,$(local_inc))
incpath += $(addprefix $(subdirectory)/,$(local_inc))
#$(warning incpath = $(incpath))

$(eval $(call make-library,$(subdirectory)/$(subdirectory).a,$(local_src)))
#$(eval $(call make-target,$(subdirectory)/$(subdirectory).a,$(local_src)))

endif
