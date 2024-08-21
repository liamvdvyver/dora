CC = clang
CFLAGS = `pkg-config --cflags --libs libnotify`
BUILD = out
SRC = src

vpath %.h $(SRC)
vpath %.c $(SRC)

default: all

all: checkdirs $(BUILD)/dora $(BUILD)/doractl

$(BUILD)/dora: dora.c $(BUILD)/printing.o $(BUILD)/ipc.o $(BUILD)/notify.o $(BUILD)/strategies.o $(BUILD)/listener.o $(BUILD)/timer.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/doractl: doractl.c $(BUILD)/ipc.o $(BUILD)/printing.o
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/notify.o: notify.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/%.o: %.c
	$(CC) -c -o $@ $^

checkdirs: $(BUILD)

$(BUILD):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD)

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: $(BUILD)/dora $(BUILD)/doractl
	install -d $(PREFIX)/bin/
	install -m 755 $(BUILD)/dora $(PREFIX)/bin/
	install -d $(PREFIX)/bin/
	install -m 755 $(BUILD)/doractl $(PREFIX)/bin/

uninstall:
	rm $(PREFIX)/bin/dora $(PREFIX)/bin/doractl
