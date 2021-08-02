CXXFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter -std=c++17
OBJECTS=mantlemap.o format.o LoadShaders.o MapState.o Scene.o TextLabel.o PolyLine.o LightScene.o MapTimeScene.o CmdDebugScene.o DebugTransformScene.o SolarScene.o GLRenderContext.o Astronomy.o NaturalEarth.o
BINARIES=mantlemap

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=led
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a

NOVAS_LIB_DISTRIBUTION=novas
NOVAS_INCDIR=$(NOVAS_LIB_DISTRIBUTION)
NOVAS_LIBDIR=$(NOVAS_LIB_DISTRIBUTION)
NOVAS_LIBRARY_NAME=novas
NOVAS_LIBRARY=$(NOVAS_LIBDIR)/$lib(NOVAS_LIBRARY_NAME).a

ASTRO_LIB_DISTRIBUTION=astro-master
ASTRO_INCDIR=$(ASTRO_LIB_DISTRIBUTION)/ephutil
ASTRO_LIBDIR=$(ASTRO_LIB_DISTRIBUTION)/ephutil
ASTRO_LIBRARY_NAME=ephutil
ASTRO_LIBRARY=$(ASTRO_LIBDIR)/$lib(ASTRO_LIBRARY_NAME).a

EGL_INCDIR=/opt/vc/include
EGL_LIBDIR=/opt/vc/lib
LDFLAGS+= -L$(RGB_LIBDIR) -L$(EGL_LIBDIR) -L$(ASTRO_LIBDIR) -L$(NOVAS_LIBDIR) -l$(RGB_LIBRARY_NAME) -l$(ASTRO_LIBRARY_NAME) -l$(NOVAS_LIBRARY_NAME) 
LDFLAGS+= -lrt -lm -lpthread -lbrcmGLESv2 -lbrcmEGL -lbcm_host -lssl -lcrypto -lstdc++fs

INCDIRS=-I$(RGB_INCDIR) -I$(ASTRO_INCDIR) -I$(NOVAS_INCDIR) -I$(EGL_INCDIR) -Ijson/single_include -Ifmt/include

# Imagemagic flags, only needed if actually compiled.
MAGICK_CXXFLAGS=`GraphicsMagick++-config --cppflags --cxxflags`
MAGICK_LDFLAGS=`GraphicsMagick++-config --ldflags --libs`
AV_CXXFLAGS=`pkg-config --cflags  libavcodec libavformat libswscale libavutil`

all : $(BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

mantlemap: $(OBJECTS) $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(MAGICK_LDFLAGS)

%.o : %.cc
	$(CXX) $(INCDIRS) -c -o $@ $<

mantlemap.o : mantlemap.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

GLRenderContext.o : GLRenderContext.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

Scene.o : Scene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

MapState.o : MapState.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

TextLabel.o : TextLabel.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

PolyLine.o : PolyLine.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

LightScene.o : LightScene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

MapTimeScene.o : MapTimeScene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

CmdDebugScene.o : CmdDebugScene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

DebugTransformScene.o : DebugTransformScene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

SolarScene.o : SolarScene.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) $(MAGICK_CXXFLAGS) -c -o $@ $<

Astronomy.o : Astronomy.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) -c -o $@ $<
	
NaturalEarth.o : NaturalEarth.cpp
	$(CXX) $(INCDIRS) $(CXXFLAGS) -c -o $@ $<

LoadShaders.o : LoadShaders.cpp
	$(CXX) $(INCDIRS) $(MAGICK_CXXFLAGS) $(CXXFLAGS) -c -o $@ $<

format.o : fmt/src/format.cc
	$(CXX) $(INCDIRS) $(MAGICK_CXXFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES) $(OPTIONAL_OBJECTS) $(OPTIONAL_BINARIES)

FORCE:
.PHONY: FORCE
