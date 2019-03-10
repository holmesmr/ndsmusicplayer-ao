ndsmusicplayer
==============

modified fork of ndsmusicplayer/vio2play that uses libao so you don't have to mess
around with `aoss` or `padsp` (and use them on platforms where OSS doesn't
exist at all). Additionally uses CMake because I decided to.

Only external dependency is `libao` (plus whatever additional dependencies are
required on your platform for the default driver). Build in the usual CMake way 
(`mkdir build && cd build && cmake .. && make`).

By way of being a derivative work of DeSmuME this program is licensed under GPLv2.
In addition, the FindLibAO CMake module was cribbed from 
[the iola non-linear video editor](https://github.com/dahnielson/iola) which is
also GPLv2 licensed.