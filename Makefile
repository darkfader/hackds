all: arm7.bin header.bin blockram
	blockram

header.bin: ndstool
	ndstool -p Metroid.Prime.Hunters.First.Hunt-DF.nds header.bin

blockram: blockram.cpp
#	cl /O2 blockram.cpp /IC:\NitroSDK\include /IC:\NitroSDK\include\nitro\hw\ARM7
	g++ -O2 -o blockram blockram.cpp -IC:\NitroSDK\include -lioperm

ndstool: ndstool.cpp
	g++ -O2 -o ndstool ndstool.cpp

%.elf: %.o hack.x Makefile
	arm-elf-ld.exe -o $@ $(filter %.o,$<) -T hack.x -Map map.txt
#-Tbss 0x08000000 -Tdata 0x08000000 -Ttext 0x08000000 
#--section-start text2=0x02200000

%.bin: %.elf
	arm-elf-objcopy.exe $< -O binary $@

%.o: %.S
	arm-elf-gcc.exe -c -o $@ $< -I C:/NitroSDK/include/
