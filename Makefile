
TARGET = memcpymark.elf

OBJS = bench.o memcpy.o memmove.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS)

rm-elf:
	-rm -f $(TARGET)

$(TARGET): $(OBJS)
	kos-cc -O3 -o  $(TARGET) $(OBJS) -lfastmem

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

runip: $(TARGET)
	$(KOS_IP_LOADER) $(TARGET)


