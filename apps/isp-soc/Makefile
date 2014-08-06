#LD=/soft/android-ndk-r9c/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ld
#GCC=/soft/android-ndk-r9c/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
LD=arm-none-linux-gnueabi-ld
GCC=arm-none-linux-gnueabi-gcc
#OPT=-I/soft/android-ndk-r9c/platforms/android-19/arch-arm/usr/include -march=armv7-a --sysroot=/soft/android-ndk-r9c/platforms/android-19/arch-arm/
OPT=-I/opt/arm-2013.05-linux/lib/gcc/arm-none-linux-gnueabi/4.7.3/include -march=armv7-a
#-mfloat-abi=soft
FW=firmware/object/arm-linux-androideabi-4.8/dbg/DxOISP_Firmware.o

BASE_ROOTFS_DIR :=/home2/tifler/work/ramdisk/vgroup_rootfs
ifndef CROSS_COMPILE
CROSS_COMPILE   :=$(BASE_ROOTFS_DIR)/output/host/opt/ext-toolchain/bin/arm-none-linux-gnueabi-
endif
GCC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld



.PHONY:all clean

all: cameraApp

####################
### link
####################
cameraApp: main.o DxOISP_SensorAPI.o sensorOVT2715.o corrData.o DxOISP_ahblAccess.o $(FW) shell.o dxo.o debug.o ioctl.o isp-io.o event.o
	$(GCC) $(OPT) -lpthread $^ -o $@

####################
### partial link used to generated corrData.o as provided in the delivey
####################
corrData.o: calibDataTabSelection.o ccParamsTabSelection.o data.o ccParams.o
	${LD} -r $^ -o $@

####################
### compilation specific for each c file to show option usage
####################
main.o: exampleCode/main.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

data.o: exampleCode/calibData/data.c
	$(GCC) -c $(OPT) -I. -DDXO_PRINTF=DxOISP_Printf -DDATA_SYMS_H_DIR=firmware/correctionData/include -DDATA_NAMESPACE=OV8820 -DPROC_NAMESPACE=generic -DDATA_ROOT_DIR=firmware/correctionData/include -DDATA_GLOBAL_HEADER=firmware/include/DxOISP.h $^

ccParams.o: exampleCode/calibData/ccParams.c
	$(GCC) -c $(OPT) -IexampleCode/calibData $^

calibDataTabSelection.o: exampleCode/calibData/calibDataTabSelection.c
	$(GCC) -c $(OPT) -I. -DCHIP_VENDOR_PATH_ISP_INC_DIR=firmware/include $^

ccParamsTabSelection.o: exampleCode/calibData/ccParamsTabSelection.c
	$(GCC) -c $(OPT) $^

DxOISP_SensorAPI.o: exampleCode/OV2715/DxOISP_SensorAPI.c
	$(GCC) -c $(OPT) -I. -Ifirmware/include -DCHIP_VENDOR_PATH_ISP_INC_DIR=firmware/include $^

sensorOVT2715.o: exampleCode/OV2715/sensorOVT2715.c
	$(GCC) -c $(OPT) -I. -Ifirmware/include -IexampleCode/OV24715 -DCHIP_VENDOR_PATH_ISP_INC_DIR=firmware/include -IexampleCode/ $^

DxOISP_ahblAccess.o: exampleCode/DxOISP_ahblAccess.c
	$(GCC) -c $(OPT) $^

shell.o: exampleCode/shell.c
	$(GCC) -c $(OPT) $^

dxo.o : exampleCode/dxo.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

debug.o : exampleCode/debug.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

ioctl.o : exampleCode/ioctl.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

isp-io.o : exampleCode/isp-io.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

event.o : exampleCode/event.c
	$(GCC) -c $(OPT) -Ifirmware/include $^

clean:
	@rm -rf cameraApp *.o
