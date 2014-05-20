TARGET = ambiled
CC = g++
CXXFLAGS = -Wall -g -D__STDC_CONSTANT_MACROS #-std=c99
#LDFLAGS = -lv4l2 -lSDL2 -lSDL2_image -lm -lavformat -lavcodec -lavutil -lswscale
LDFLAGS = -lv4l2 -lm -lavformat -lavcodec -lavutil -lswscale
#LOCAL_CPPFLAGS += -DDEBUG

OBJDIR = obj
BINDIR = bin

vpath %.cpp src
vpath %.h src

SRC = ambiled.cpp usb_grabber.cpp engine.cpp leds.cpp arduino.cpp gpio.cpp video.cpp
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

$(OBJDIR)/%.o: %.cpp
	$(shell mkdir -p $(OBJDIR))
	@$(CC) $(CXXFLAGS) $(LOCAL_CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(shell mkdir -p bin)
	@$(CC) $(CXXFLAGS) -o $(BINDIR)/$(TARGET) $(OBJ) $(LDFLAGS)

.PHONY : clean
clean :
	@rm -rf $(BINDIR) $(OBJDIR)
