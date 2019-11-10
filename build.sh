export CROSS_COMPILE=/home/popravki/1/lineage16/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
export USE_CCACHE=1
export ARCH=arm64 ARCH_MTK_PLATFORM=mt6735
export TARGET=out
make O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE  A610_defconfig
make O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j8
#make mrproper O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE

#make -C k4000plus O=out/target/vz6737m_65_i_m0/ ARCH=arm64 CROSS_COMPILE=toolchains/aarch64-linux-android-4.9/bin/aarch64-linux-android- vz6737m_65_i_m0_defconfig
#make -C k4000plus O=out/target/vz6737m_65_i_m0/ ARCH=arm64 CROSS_COMPILE=toolchains/aarch64-linux-android-4.9/bin/aarch64-linux-android- zImage
#!/bin/bash
#export CROSS_COMPILE=/home/popravki/1/lineage16/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-
#export USE_CCACHE=1
#export ARCH=arm64 ARCH_MTK_PLATFORM=mt6735
#export TARGET=out
#make O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE A610_defconfig
#make O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j8
#make mrproper O=$TARGET ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE

