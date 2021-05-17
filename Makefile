# From: 'Another Makefile' example from https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html
#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
#CXX = "C:\MinGW\bin\g++.exe"
#CXX = "C:\cygwin64\bin\g++.exe"
#CXX = "C:\msys64\mingw64\bin\g++.exe"
CXX = "C:\MinGW\x86_64-8.1.0-win32-seh-rt_v6-rev0\mingw64\bin\g++.exe"

# define any compile-time flags
CXXFLAGS = -Wall -std=c++17 -xc++ -m64 -D_RELEASE -D_WIN32 -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS

# define any directories containing header files other than /usr/include
INCLUDES = -I"C:/vcpkg/installed/x64-windows/include/"

# define library file paths
LFLAGS = -L"C:/vcpkg/installed/x64-windows/lib/"

# define any libraries to link into executable:
LIBS = -lOpenGL32 -lglfw3dll -lglew32 -lOpenCL -lgdi32 -lkernel32 -luser32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32

# define the C++ source files
SRCS = CLBVHnode.cpp CLEngineBase.cpp CLmathlib.cpp CLOBJloader.cpp CLRaytracer.cpp CLutils.cpp main.cpp

# define the C++ object files 
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = OCLRTX


#===========================================================
#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  Simple compiler named mycc has been compiled

$(MAIN): $(OBJS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .cpp's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .cpp file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it