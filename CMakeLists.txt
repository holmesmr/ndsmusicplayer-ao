cmake_minimum_required(VERSION 3.0)
project(ndsmusicplayer)

SET(CMAKE_MODULE_PATH "${ndsmusicplayer_SOURCE_DIR}/cmake")

Include(TestBigEndian)
Include(FindLibAO)

Test_Big_Endian(IS_BIG_ENDIAN)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    Add_Compile_Definitions(DLONG_IS_64BIT=1)
endif()

if(NOT(IS_BIG_ENDIAN))
    Add_Compile_Definitions(LSB_FIRST=1)
endif()

Set(MATH_LIBRARIES "")
Include(CheckLibraryExists)
Check_Library_Exists(m pow "" LIBM)
if(LIBM)
    List(APPEND MATH_LIBRARIES "m")
endif()


Add_Compile_Definitions(_strnicmp=strncasecmp)

Include_Directories(
    ${ndsmusicplayer_SOURCE_DIR}/vio2sf ${ndsmusicplayer_SOURCE_DIR}/vio2sf/zlib 
    ${ndsmusicplayer_SOURCE_DIR}/vio2sf/desmume
    ${LIBAO_INCLUDE_DIR}
)


Add_Library(ndsmusic_shared STATIC
    libao.c corlett.c xsf_player.c
    vio2sf/vio2sf.c vio2sf/desmume/armcpu.c vio2sf/desmume/arm_instructions.c vio2sf/desmume/bios.c vio2sf/desmume/cp15.c
    vio2sf/desmume/FIFO.c vio2sf/desmume/GPU.c vio2sf/desmume/matrix.c vio2sf/desmume/mc.c vio2sf/desmume/MMU.c
    vio2sf/desmume/NDSSystem.c vio2sf/desmume/SPU.c vio2sf/desmume/thumb_instructions.c
    vio2sf/zlib/adler32.c vio2sf/zlib/crc32.c vio2sf/zlib/infback.c vio2sf/zlib/inffast.c vio2sf/zlib/inflate.c
    vio2sf/zlib/inftrees.c vio2sf/zlib/uncompr.c vio2sf/zlib/zutil.c
)

Add_Executable(ndsmusicplayer main.c)
Add_Executable(ndsmusicdump dump_main.c)

Target_Link_Libraries(ndsmusicplayer PUBLIC ${MATH_LIBRARIES} ${LIBAO_LIBRARIES} PRIVATE ndsmusic_shared)
Target_Link_Libraries(ndsmusicdump PUBLIC ${MATH_LIBRARIES} ${LIBAO_LIBRARIES} PRIVATE ndsmusic_shared)
