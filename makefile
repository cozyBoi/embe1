CC=arm-none-linux-gnueabi-gcc

.SUFFIXES: .c .h .o

DEPS= main.h #input_process.h output_process.h
CFLAGS= #-W

TARGET = 20171677_hw1
OBJDIR = build/

_OBJS = input.o output.o main.o device_all.o mode_all.o
OBJS = $(addprefix $(OBJDIR),$(_OBJS))

$(OBJDIR)$(TARGET): $(OBJS)
   $(CC) -static -o $(OBJDIR)$(TARGET) $(OBJS)

$(OBJDIR)%.o: %.c $(OBJDIR)
   $(CC) -static -c -o $@ $<

$(OBJDIR):
   mkdir -p $(OBJDIR)

.PHONY: clean
clean:
   rm -rf $(OBJDIR)

.PHONY: push
push:
   make
   adb push $(OBJDIR)$(TARGET) /data/local/tmp
