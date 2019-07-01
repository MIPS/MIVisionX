# MIPS port of OpenVX

MIPS port of OpenVX ([mips](amd_openvx/mips)) implements <a href="https://www.khronos.org/openvx/" target="_blank">Khronos OpenVX</a> computer vision specification and is based on AMD OpenVX ([amd_openvx](amd_openvx#amd-openvx-amd_openvx)).
MIPS port is reusing AMD's ago_drama framework for graph optimization and is using MIPS SIMD Architecture ([MSA](https://www.mips.com/products/architectures/ase/simd/)) for optimization of vision functions.


# Build variants

## MSA and C implementations

Source for vision functions found in [mips](amd_openvx/mips) directory contains MSA versions as well as pure C versions of vision functions.
Build-time flag (C_IMPL) determines which implementation is used.

## x86_64 build with disabled OpenCL

For purposes of testing and verification x86_64 build is used without OpenCL support, by using build-time flag CMAKE_DISABLE_FIND_PACKAGE_OpenCL=TRUE.


# Build instructions

Build is performed natively on target platforms.
For building OpenVX for mips64r6, [qemu](http://mips64el.bfsu.edu.cn/debian-new/tarball/) was used.
For building OpenVX for mips32r5, <a href="http://tadviser.com/index.php/Product:ELISE_Semantic_processor_(ELVEES_Image_Semantic_Engine)" target="_blank">ELVEESE</a> platform was used.

## MIPS build
```
mkdir -p build-ovx/mips64/c
cd build-ovx/mips64/c
cmake ../../../ -DBUILD_MIPS=1 -DC_IMPL=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_FIND_PACKAGE_OpenCL=TRUE
make -j8

mkdir -p build-ovx/mips64/msa
cd build-ovx/mips64/msa
cmake ../../../ -DBUILD_MIPS=1 -DC_IMPL=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_FIND_PACKAGE_OpenCL=TRUE
make -j8
```

## referent x86_64 build
```
mkdir -p build-ovx/x86/c
cd build-ovx/x86/c
cmake ../../../ -DBUILD_MIPS=0 -DC_IMPL=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_FIND_PACKAGE_OpenCL=TRUE
make -j8

mkdir -p build-ovx/x86/sse
cd build-ovx/x86/sse
cmake ../../../ -DBUILD_MIPS=0 -DC_IMPL=0 -DCMAKE_BUILD_TYPE=Release -DCMAKE_DISABLE_FIND_PACKAGE_OpenCL=TRUE
make -j8
```

# Running GDF example
```
$ cd build-ovx/mips64/msa/bin
$ ./runvx file ../../../../samples/gdf/canny.gdf

runvx 0.9.9
OK: using AMD OpenVX 0.9.9

csv,HEADER ,STATUS, COUNT,cur-ms,avg-ms,min-ms,clenqueue-ms,clwait-ms,clwrite-ms,clread-ms
OK: capturing 768x512 image(s) into 480x360 RGB image buffer with resize
csv,OVERALL,  PASS,     1,      , 10.41, 10.41,  0.00,  0.00,  0.00,  0.00 (median 10.412)
> total elapsed time:   0.15 sec
Abort: Press any key to exit...
```
