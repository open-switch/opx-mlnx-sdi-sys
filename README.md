#opx-mlnx-sdi-sys
This repository contains the implementation of SDI entity abstraction layer for Mellanox platforms. The PAS component uses the SDI API to access hardware devices - in a platform independent manner.  
  
##Packages
libopx-sdi-sys-dev\_*version*\_*arch*.deb — Development package (libraries and headers) for SDI for hardware implementation  

libopx-sdi-sys1\_*version*\_*arch*.deb — Libraries package for SDI for hardware implementation  

opx-sdi-sys\_*version*.dsc — Source package  
  
##Build
To build this repo please refer to [instructions in opx-build repository](https://github.com/open-switch/opx-build) and use [opx\_build\_mlnx](https://github.com/volodymyrsamotiy/opx-build/raw/master/scripts/opx_build_mlnx) script.

To build opx-mlnx-sdi-sys repo with all dependencies use below command:
console\# opx\_build\_mlnx opx-logging opx-common-utils opx-cps opx-base-model opx-platform-config opx-sdi-api opx-mlnx-sdi-sys

To build all packages for the Mellanox platforms use the following command:
console\# opx\_build\_mlnx all

##Install
Before installing built packages some additional packages should be installed on the platform. Copy all Debian packages from the following location: https://github.com/Mellanox/SAI-Implementation/raw/sonic/sdk/*.deb. Then install all of them:
console\# dpkg -i *.deb

After that just copy to the system all built packages and install them.
console\# dpkg - i *.deb

(c) 2017 Mellanox

