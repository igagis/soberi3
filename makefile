# Last modified: 2009.11.27
# Author: Ivan Gagis
#         igagis@gmail.com

#==============================================================================
#==============================================================================
#==============================================================================
#                        Project configuration part
#             change these strings to configure project building

#default platform. It may be overriden by specifying platform=xxx in command line when running make
#Known platforms are:
#    linux
#    maemo_linux
#    win32
#    win32cross
platform := maemo_linux

#build configuration:
#    debug
#    release
build_config := debug

zip_res := no

name := out/soberi3

ifeq ($(platform), win32)
    name := $(name).exe
endif
ifeq ($(platform), win32cross)
    name := $(name).exe
endif



#Sources
srcs := src/main.cpp
srcs += src/MyMaemoFW/Shader.cpp
srcs += src/MyMaemoFW/Widget.cpp
srcs += src/MyMaemoFW/Application.cpp
srcs += src/MyMaemoFW/BasicQMainWindow.cpp
srcs += src/MyMaemoFW/GLWindow.cpp
srcs += src/MyMaemoFW/GLTexture.cpp
srcs += src/MyMaemoFW/Updater.cpp
srcs += src/MyMaemoFW/Image.cpp
srcs += src/MyMaemoFW/TexFont.cpp
srcs += src/App.cpp
srcs += src/BasicWindow.cpp
srcs += src/GameWindow.cpp
srcs += src/MainMenuWindow.cpp
srcs += src/SelectLevelWindow.cpp
srcs += src/StatsWindow.cpp
srcs += src/ListWidget.cpp
srcs += src/ProgressBarWidget.cpp
srcs += src/LabelWidget.cpp
srcs += src/ResMan/ResMan.cpp
srcs += src/Resources/ResTexture.cpp
srcs += src/Resources/ResSound.cpp
srcs += src/Resources/ResSprite.cpp
srcs += src/Resources/ResFont.cpp
srcs += src/Resources/ResParticleSystem.cpp
srcs += src/GameField.cpp
srcs += src/Item.cpp
srcs += src/Lightning.cpp
srcs += src/ParticleSystem.cpp
srcs += src/Sprite.cpp
srcs += src/utils.cpp
srcs += src/MusicBox.cpp
srcs += src/Preferences.cpp
srcs += third_party/pugixml/pugixml.cpp



#run Qt moc on the following headers
moc_headers :=
moc_headers += src/MyMaemoFW/GLWindow.hpp
moc_headers += src/MyMaemoFW/Application.hpp
moc_headers += src/MyMaemoFW/BasicQMainWindow.hpp



defines :=

ifeq ($(build_config), debug)
    defines += -DDEBUG
endif

ifeq ($(platform), maemo_linux)
    defines += -DM_MAEMO
endif

ifeq ($(zip_res), yes)
    defines += -DM_ZIP_RES
endif


compiler_flags := -Wall #enable all warnings
compiler_flags += -Wno-comment #no warnings on nested comments
compiler_flags += -funsigned-char #the 'char' is unsigned
compiler_flags += -Wnon-virtual-dtor #warn if base class has non-virtual destructor
compiler_flags += -fno-operator-names #do not treat 'and', 'bitand','bitor','compl','not','or','xor' as keywords
compiler_flags += -Werror #treat warnings as errors
compiler_flags += -Wfatal-errors # stop on first error encountered
                #-P -E #-Wreorder
                #-O3 -funroll-loops -fomit-frame-pointer
#compiler_flags += -std=c++0x

ifeq ($(build_config), debug)
    compiler_flags += -g
endif

ifeq ($(platform), linux)
    compiler_flags += 
endif
ifeq ($(platform), maemo_linux)
    compiler_flags += 
endif



linker_flags :=

ifeq ($(build_config), release)
    linker_flags += -s
endif


#=======================
#  Include directories
#=======================
ifeq ($(platform), maemo_linux)
    include_dirs := -Ithird_party `pkg-config gstreamer-0.10 freetype2 libosso QtGui QtCore QtDBus --cflags`
endif
ifeq ($(platform), linux)
    include_dirs := -Ithird_party -Ithird_party/ogles2emu `pkg-config gstreamer-0.10 freetype2 QtGui QtCore QtDBus --cflags`
endif
ifeq ($(platform), win32)
    include_dirs :=
endif
ifeq ($(platform), win32cross)
    include_dirs :=
endif



#============================
#  Libs and lib directories
#============================
ifeq ($(platform), maemo_linux)
    libs:= -lGLESv2 `pkg-config gstreamer-0.10 freetype2 libosso libpulse libpulse-simple QtGui QtDBus QtOpenGL --libs` -ljpeg
    libs += third_party/unzip/armel/*
    libs += -lting
    libs += -laumiks
endif
ifeq ($(platform), linux)
    libs:= `pkg-config gstreamer-0.10 libpulse libpulse-simple freetype2 QtGui QtDBus QtOpenGL --libs`
    libs += -ljpeg -Lthird_party/ogles2emu -lEGL -lGLESv2
    libs += third_party/unzip/i386/*
    libs += -lting
    libs += -laumiks
endif
ifeq ($(platform), win32)
    libs:=
endif
ifeq ($(platform), win32cross)
    libs:=
endif



remove_on_clean:= debian/tmp



#                     __
#                    /  \__/  end of configuration part
#==============================================================================

#remove program
ifeq ($(platform),win32)
    remove:=del /F /Q
else
    remove:=rm -f
endif

obj_dir := obj/1/2
compiler := g++




#==============================================================================
#=============================Qt related stuff=================================
#==============================================================================
moc_cpp_files := $(addprefix $(obj_dir)/,$(patsubst %.hpp,%_moc.cpp,$(moc_headers)))

srcs += $(moc_cpp_files)

#list *_moc.cpp files as .SECONDARY to prevent their deletion when make exits, because
#otherwise they are considered as ordinary intermediate files (because they are generated as 
#a result of chained implicit template rules, e.g. %.o:%.cpp) and are deleted when they are
#no longer needed.
.SECONDARY: $(moc_cpp_files)



#==============================================================================
#=============================TARGETS==========================================
#==============================================================================

#==========================
#=project (default) target=
proj: create-obj-dir $(name)

create-obj-dir:
ifeq ($(platform), win32)
	@if not exist $(obj_dir) mkdir $(obj_dir)
else
	@mkdir -p $(obj_dir)
endif

#find all .cpp files and get an .o file name for it to get dependancies for this target
$(name): $(addprefix $(obj_dir)/,$(patsubst %.cpp,%.o,$(srcs)))
	@echo Linking $@...
	@$(compiler) $^ -o "$@" $(libs) $(linker_flags)



#Qt related target to run moc on hpp files
$(obj_dir)/%_moc.cpp: %.hpp
	@echo Running moc on $<...
	@moc $(defines) $< -o $@



#======================
#=compile .cpp sources=
$(obj_dir)/%.o:%.cpp
	@echo Compiling $<...
	@mkdir -p $(dir $@)
# -MF option specifies dependency output file name
	@$(compiler) -c -MF "$(patsubst %.o,%.d,$@)" -MD -o "$@" $(compiler_flags) $(defines) $(include_dirs) $<
#workaround for gcc 4.2 compiler (it behaves differently than gcc 4.1 for some reason)
#ifeq ($(platform), linux)
#	@echo -n $(obj_dir)/ > $(patsubst %.o,%.d_,$@)
#	@cat $(patsubst %.o,%.d,$@) >> $(patsubst %.o,%.d_,$@)
#	@mv $(patsubst %.o,%.d_,$@) $(patsubst %.o,%.d,$@)
#endif

include $(wildcard $(addsuffix /*.d,$(dir $(addprefix $(obj_dir)/,$(srcs)))))



#==================
#=build all target=
all: clean proj



#==============
#=clean target=
#delete all objects and executables

#it is an abstract target (not a file), declare as .PHONY
.PHONY: clean
clean:
ifeq ($(platform),win32)
	@$(remove) $(name)
	@$(remove) $(obj_dir)\*
else
	@$(remove) $(name)
	@$(remove) -rf $(dir $(obj_dir))
	@$(remove) -rf $(remove_on_clean)
endif
