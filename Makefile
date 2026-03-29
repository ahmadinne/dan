CC = gcc
CFLAGS = -Wall -g
PREFIX = /usr/local
TARGET = dan
SOURCE = dan.c operations.c

all: $(TARGET)

dan: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $@

clean: 
	rm -f $(TARGET)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	cp -f $(TARGET) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
