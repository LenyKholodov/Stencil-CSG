SOURCES     := $(wildcard *.cpp)
EXTERN_DIRS := glut glew opencsg lua glui
OBJS        := $(SOURCES:%.cpp=%.obj)
TARGET      := test.exe
LIBS        := freeglut_static.lib glew32s.lib OpenCSG.lib lua.lib lualib.lib glui32.lib
CFLAGS      := /nologo /O2 /c $(EXTERN_DIRS:%=/I"extern/%/h") /D_CRT_SECURE_NO_DEPRECATE /EHsc \
               /DFREEGLUT_STATIC /DGLEW_STATIC /DGLUI_FREEGLUT
LINK_FLAGS  := /nologo $(EXTERN_DIRS:%=/libpath:"extern/%/lib") $(LIBS)

all: $(TARGET)

%.obj: %.cpp
	@cl $(CFLAGS) $< /Fo$@
	
$(TARGET): $(OBJS)
	@link $(LINK_FLAGS) $^ /out:$@
	
rebuild: clean $(TARGET)

clean:
	@rm -f *.obj

run: $(TARGET)
	@./$(TARGET)
