#
# falinux 라이블러리를 사용하는	어플용 Makefile
#

#CROSS_PREFIX	= mipsel-linux
#CROSS_PREFIX	= arm-linux
CROSS_PREFIX   = arm-generic-linux-gnueabi

## 사용자 소스
TARGET			= app_gxlib
SRCS			= sample.c


#---------------------------------------------------------------------
## 소스레벨의 라이브러리
DIR_GXLIB		= ../lib/gxlib/

SRCS_GXLIB		= $(DIR_GXLIB)gx.c $(DIR_GXLIB)gxpng.c $(DIR_GXLIB)gxjpg.c $(DIR_GXLIB)gxbmp.c $(DIR_GXLIB)gxbdf.c $(DIR_GXLIB)fontinfo.c $(DIR_GXLIB)ksc5601.c	$(DIR_GXLIB)gximagelist.c $(DIR_GXLIB)gxmosaic.c $(DIR_GXLIB)gxlayer.c
SRCS			+= $(SRCS_GXLIB) 

## 인클루드	디렉토리
INCLUDEDIRS		+= -I. -I../include/gxlib

OBJS			= $(SRCS:.c=.o)

CFLAGS			= $(INCLUDEDIRS) -W -Wall -O2
LIBS			= -lm -lpng12 -lz -ljpeg

#---------------------------------------------------------------------
CC				=	$(CROSS_PREFIX)-gcc
CXX				=	$(CROSS_PREFIX)-g++
AR				=	$(CROSS_PREFIX)-ar rc
AR2				=	$(CROSS_PREFIX)-ranlib
RANLIB			=	$(CROSS_PREFIX)-ranlib
LD				=	$(CROSS_PREFIX)-ld
NM				=	$(CROSS_PREFIX)-nm
STRIP			=	$(CROSS_PREFIX)-strip
OBJCOPY			=	$(CROSS_PREFIX)-objcopy
CP	= cp
MV	= mv
#--------------------------------------------------------------------

all	: $(TARGET)

$(TARGET) :	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o	$@ $(LIBS)
	$(NM) $(TARGET)	> $(TARGET).map

%.o:%.c
	@echo "Compiling $<	..."
	$(CC) -c $(CFLAGS) -o $@ $<

%.o:%.cc
	@echo "C++ compiling $<	..."
	$(CXX) -c $(CFLAGS)	$(CXXFLAGS)	-o $@ $<

dep	:
	$(CC) -M $(INCLUDEDIRS)	$(SRCS)	> .depend

clean:
	rm -f *.bak
	rm -f *.map
	rm -f *.o
	rm -f $(OBJS)
	rm -f $(TARGET)	core

distclean: clean
	rm -rf .depend


ifeq (.depend,$(wildcard .depend))
include	.depend
endif
