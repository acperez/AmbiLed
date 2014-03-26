TARGET = ambiled
CC = gcc
CFLAGS = -Wall -g -std=c99
#CFLAGS = -Wall -O2 -std=c99
LDFLAGS = -lv4l2 -lSDL2 -lSDL2_image

OBJDIR = obj
BINDIR = bin

vpath %.c src
vpath %.h src

SRC = ambiled.c capture.c frame.c leds.c
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

$(OBJDIR)/%.o: %.c
	$(shell mkdir -p $(OBJDIR))
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(shell mkdir -p bin)
	@$(CC) $(CFLAGS) -o $(BINDIR)/$(TARGET) $(OBJ) $(LDFLAGS)

.PHONY : clean
clean :
	@rm -rf $(BINDIR) $(OBJDIR)
