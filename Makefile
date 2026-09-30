TARGET		:= meritous
TITLE	    := MERITOUS1
SOURCES		:= src
			
INCLUDES	:= src

PKG_CONFIG = arm-vita-eabi-pkg-config
SDL_LIBS := $(shell $(PKG_CONFIG) --static --libs SDL_mixer SDL_image)

LIBS = -Wl,--start-group $(SDL_LIBS) -lmikmod -lvorbisfile -lvorbis -logg -lsndfile -lSceLibKernel_stub \
	-lScePvf_stub -lSceTouch_stub -lm -lSceAppUtil_stub -lScePgf_stub -ljpeg -lfreetype -lc -lScePower_stub \
	-lpng16 -lz -lSceSysmodule_stub -lSDL -lSceAudio_stub -lSceHid_stub -limgui -lvitaGL -lSceGxm_stub \
	-lSceTouch_stub -lmathneon -lSceAppMgr_stub -lSceDisplay_stub -lSceCtrl_stub -lSceCommonDialog_stub \
	-lvitashark -lSceShaccCgExt -ltaihen_stub -lSceShaccCg_stub -lSceKernelDmacMgr_stub -Wl,--end-group

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = $(INCLUDE) -g -Wl,-q -O2 -ftree-vectorize
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -c -s $< build/eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) "$(TARGET)" build/sce_sys/param.sfo
	cp -f build/sce_sys/param.sfo param.sfo
	vita-pack-vpk -s build/sce_sys/param.sfo -b build/eboot.bin \
		--add build/sce_sys/icon0.png=sce_sys/icon0.png \
		--add build/sce_sys/livearea=sce_sys/livearea \
		--add build/shaders=shaders $@

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)

# Preserve the original argument evaluation and pre-C23 call semantics.
src/boss.o src/levelblit.o: CFLAGS += -std=gnu17
