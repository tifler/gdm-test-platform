./configure --enable-cross-compile \
             --cross-prefix=arm-linux-androideabi-  \
             --arch=arm --target-os=linux           \
             --cc=arm-linux-androideabi-gcc --host-cc=arm-linux-androideabi-gcc --cpu=armv6 --enable-armv6 --enable-static \
             --extra-ldflags='-L/home/hthwang/develop/diamond/diamond_android4.3/android/out/target/product/diamond/obj/lib -nostdlib  -lc -ldl' \
             --extra-cflags='-D_DECLARE_C99_LDBL_MATH=1 -I/home/hthwang/develop/diamond/diamond_android4.3/android/bionic/libc/include -I/home/hthwang/develop/diamond/diamond_android4.3/android/bionic/libc/arch-arm/include -I/home/hthwang/develop/diamond/diamond_android4.3/android/bionic/libc/kernel/common -I/home/hthwang/develop/diamond/diamond_android4.3/android/bionic/libc/kernel/arch-arm -I/home/hthwang/develop/diamond/diamond_android4.3/android/bionic/libm/include'

