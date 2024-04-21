# Make file for XYPLOT-32
#
# Copyright (c) 1998--2023 Krishna Myneni, <krishna.myneni@ccreweb.org>
#
# Provided under the terms of the GNU Affero General Public License
# (AGPL), v3.0 or later.
#
# Possible invocations:
#
#	make		creates dynamically linked release executable
#	make clean	remove all object files belonging to this project
#	make static	creates statically linked release executable
#	make debug	create statically linked debug executable
#
# Notes:
#
#   1. If a debug version is being built, always invoke "make clean" 
#      before "make debug".
#
COPYRIGHT=1999--2024
VERSION=2.8.0
KFORTH_VER=1.8.0
MODULE_DIR_VAR=XYPLOT_DIR
BUILD_DATE=`date +%F`
DEBUG = 
TARGET_CPU = x86
TARGET_OS  = linux

# The macro INCLUDEDIRS specifies the path for the 
# Lesstif includes. Edit this line if your include files are in
# a different directory.

INCLUDEDIRS = /usr/X11R6/include/


# The macro LIBDIRS specifies the path for the X11 and the
# Lesstif libraries. Edit this line if your libraries are in
# a different directory.

LIBDIRS = /usr/lib:/usr/lib32

# The macro LIBS lists the libraries needed

LIBS = -lXm -lXt -lXext -lX11 -lSM -lICE -lm -ldl

# Specify dynamic libraries by default to make a compact
# executable. If you wish to create a static executable,
# invoke "make static"

LDFLAGS = -Bdynamic

# Default debug option creates a release version of the executable.
# Invoke "make debug" if you want to create an executable
# that contains debugging information for the GNU debugger (gdb).

# location of gcc and g++
GCCDIR = /usr/bin

CPP = ${GCCDIR}/g++
CC  = ${GCCDIR}/gcc
CPPFLAGS = -c -m32
CFLAGS = -c -m32

OBJS = xyplot.o CDataset.o CDatabase.o CDeviceContext.o CGrid.o\
  CPlotView.o CPlotDisplay.o CPlotList.o CPlotObject.o CPlotWindow.o\
  CPolarGrid.o CTransform.o CWorkspace41.o CXyFile.o CXyPlot.o\
  CxDC.o CpsDC.o AlgebraCompiler.o ForthCompiler.o ForthVM.o vm32.o\
  vmc.o

xyplot32: ${OBJS}
	${CPP} -m32 -o xyplot32 ${DEBUG} -L${LIBDIRS} -Xlinker ${LDFLAGS} ${OBJS} ${LIBS} 

static:
	- rm xyplot32
	make xyplot32 "LDFLAGS = -Bstatic"

clean:
	- rm -f ${OBJS} xyplot32

debug:
	make xyplot32 "DEBUG = -g" "LDFLAGS = -Bstatic"

xyplot.o: xyplot.cpp xyplot.h CPlotMessage.h CPlotWindow.h resource.h 
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} xyplot.cpp

CDataset.o: CDataset.cpp CDataset.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS}  ${DEBUG} CDataset.cpp

CDatabase.o: CDatabase.cpp CDatabase.h CDataset.h CXyFile.h CWorkspace41.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CDatabase.cpp

CDeviceContext.o: CDeviceContext.cpp CDeviceContext.h CPoint.h CRect.h CPlotMessage.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CDeviceContext.cpp

CGrid.o: CGrid.cpp CGrid.h CPlotObject.h CPoint.h CRect.h CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CGrid.cpp

CPlotView.o: CPlotView.cpp CPlotView.h CTransform.h CGrid.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CPlotView.cpp

CPlotDisplay.o: CPlotDisplay.cpp CPlotDisplay.h CPlotList.h \
	CTransform.h CGrid.h CPolarGrid.h CPlotView.h \
	CXyPlot.h CWorkspace41.h CPoint.h CRect.h CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CPlotDisplay.cpp

CPlotList.o: CPlotList.cpp CPlotList.h CPlot.h CPoint.h CRect.h\
	CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CPlotList.cpp

CPlotObject.o: CPlotObject.cpp CPlotObject.h CTransform.h CPoint.h\
	CRect.h CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CPlotObject.cpp

CPlotWindow.o: CPlotWindow.cpp CPlotWindow.h CPlotDisplay.h CXyPlot.h\
	CXyFile.h CPoint.h CRect.h CPlotMessage.h CDeviceContext.h\
	CxDC.h CpsDC.h ColorTable.h resource.h fbc.h
	${CPP} ${CPPFLAGS} -DVERSION=\"${VERSION}\" \
	                   -DKFORTH_VER=\"${KFORTH_VER}\" \
			   -DBUILD_DATE=\"${BUILD_DATE}\" \
			   -DCOPYRIGHT=\"${COPYRIGHT}\" \
	-I${INCLUDEDIRS}  ${DEBUG} CPlotWindow.cpp

CPolarGrid.o: CPolarGrid.cpp CPolarGrid.h CGrid.h CPoint.h CRect.h\
	CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CPolarGrid.cpp

CTransform.o: CTransform.cpp CTransform.h CPoint.h CRect.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CTransform.cpp

CWorkspace41.o: CWorkspace41.cpp CWorkspace41.h xyp41.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS}  ${DEBUG} CWorkspace41.cpp

CXyFile.o: CXyFile.cpp CXyFile.h CDataset.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CXyFile.cpp

CXyPlot.o: CXyPlot.cpp CXyPlot.h CPlot.h CPoint.h CRect.h\
	CDeviceContext.h CxDC.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CXyPlot.cpp

CxDC.o: CxDC.cpp CxDC.h CPoint.h CRect.h CPlotMessage.h\
	CDeviceContext.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS}  ${DEBUG} CxDC.cpp

CpsDC.o: CpsDC.cpp CpsDC.h CPoint.h CRect.h CDeviceContext.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} CpsDC.cpp

AlgebraCompiler.o: fbc.h AlgebraCompiler.cpp
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} AlgebraCompiler.cpp

ForthCompiler.o: fbc.h ForthCompiler.h ForthCompiler.cpp ForthWords.h \
	VMerrors.h kfmacros.h
	${CPP} ${CPPFLAGS} -I${INCLUDEDIRS} ${DEBUG} ForthCompiler.cpp

ForthVM.o: fbc.h ForthCompiler.h ForthVM.h ForthVM.cpp VMerrors.h \
	kfmacros.h 
	${CPP} ${CPPFLAGS} -DDIR_ENV_VAR=\"${MODULE_DIR_VAR}\" \
        -I${INCLUDEDIRS} ${DEBUG} ForthVM.cpp

vm32.o: vm32.s vm32-common.s
	as --32 -o vm32.o vm32.s

vmc.o: vmc.c kfmacros.h
	${CC} ${CFLAGS} ${DEBUG} vmc.c

# end of makefile


