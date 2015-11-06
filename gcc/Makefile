#******************************************************************************
#
# Makefile - Rules for building the blinky example.
#
#  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
#
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#*****************************************************************************

#
# The base directory for TivaWare.
#
ROOT=../../..

#
# Include the common make definitions.
#
include ${ROOT}/tools/gcc_scripts/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=../
VPATH+=../../drivers
VPATH+=../../common

#
# Additional Compiler Flags
#
#CFLAGS+=-Dgcc
CFLAGS+=-DUSE_FREERTOS
CFLAGS+=-DTIMER1
#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../..
IPATH+=../../../inc
IPATH+=../../common
IPATH+=../../../oslib
IPATH+=../../../driverlib
IPATH+=../../../third_party/FreeRTOS
IPATH+=../../../third_party/FreeRTOS/source
IPATH+=../../../third_party/FreeRTOS/source/portable/GCC/ARM_CM4
IPATH+=../../../third_party/FreeRTOS/source/include


#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${BINDIR}/amaze-cc3200.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${OBJDIR} ${wildcard *~}
	@rm -rf ${BINDIR} ${wildcard *~}


#
# The rule to create the target directories.
#
${OBJDIR}:
	@mkdir -p ${OBJDIR}

${BINDIR}:
	@mkdir -p ${BINDIR}

#
# Rules for building the get_time example.
#
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/main.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/pinmux.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/gpio_if.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/filter.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/i2c_if.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/i2cdev.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/imu.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/mpu6050.o
#${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/stabilizer.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/timer_if.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/uart_if.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/led.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/ledseq.o
${BINDIR}/amaze-cc3200.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/amaze-cc3200.axf: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
${BINDIR}/amaze-cc3200.axf: ${ROOT}/oslib/${COMPILER}/${BINDIR}/FreeRTOS.a
SCATTERgcc_amaze-cc3200=amaze-cc3200.ld
ENTRY_amaze-cc3200=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${OBJDIR}/*.d} __dummy__
endif
