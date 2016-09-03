# Project name, C flags, and compiler
PROJECT_NAME = ictjam
CFLAGS = -Os -Wall -D DEBUG -std=c11
CC = gcc

# Object file names
OBJECT_NAMES = main.o Defaults.o Circle.o Player.o Enemy.o xorshiftstar.o Font.o Component.o
BMP_NAMES = Background.bmp Player.bmp HullStrength_txt.bmp HelpScreen.bmp

# SDL2 paths
SDL2_LDIR = SDL/lib
SDL2_LIB = SDL2
SDL2_IDIR = SDL/include

# Our project paths
ODIR = obj
BDIR = bin
SDIR = src
IDIR = inc
RDIR = rsc

# Extend object names into full object paths
OBJECTS = $(OBJECT_NAMES:%.o=$(ODIR)/%.o)
BMPS = $(BMP_NAMES:%.bmp=$(RDIR)/%.bmp)

REF_FRAMEWORKS =
UNAME = $(shell uname)
# If on Mac OS X, reference required frameworks
ifeq ($(UNAME),Darwin)
	REQ_FRAMEWORKS = AudioToolbox AudioUnit Carbon Cocoa CoreAudio CoreFoundation ForceFeedback IOKit OpenGL
	REF_FRAMEWORKS = $(REQ_FRAMEWORKS:%=-framework %)
endif

# Make the object directory
$(ODIR) :
	mkdir $(ODIR)

# Make the bin directory
$(BDIR) :
	mkdir $(BDIR)

$(RDIR) : $(BDIR)
	mkdir $(BDIR)/$(RDIR)

# Make the object files
$(ODIR)/%.o : $(SDIR)/%.c $(ODIR)
	$(CC) -c -I$(IDIR) -I$(SDL2_IDIR) $(CFLAGS) -o $@ $<

# Copy resources
$(RDIR)/%.bmp : $(RDIR) FORCE
	cp $@ $(subst $(RDIR),$(BDIR)/$(RDIR),$@)
FORCE:

# Make the project in the bin directory
$(PROJECT_NAME): $(OBJECTS) $(BDIR) $(BMPS)
	$(CC) -L$(SDL2_LDIR) -l$(SDL2_LIB) $(REF_FRAMEWORKS) -o $(BDIR)/$@ $(OBJECTS)

# Clean the project
.PHONY : clean
clean :
	rm -f $(OBJECTS) $(BDIR)/$(PROJECT_NAME)
	rm -f -r $(ODIR) $(BDIR)

#Rebuild the project
rebuild :
	make clean
	make $(PROJECT_NAME)
