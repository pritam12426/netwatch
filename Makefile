UNAME_S := $(shell uname -s)

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man

STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install

CFLAGS_OPTIMIZATION ?= -O3

BUILD := build

BIN_SERVER = netwatch
BIN_CLI    = $(BIN_SERVER)-cli
# BIN_CLI    = $(BUILD)/netwatch-cli

CFLAGS  ?= ""
LDFLAGS ?= ""

# ──────────────────────────────────────────────────────────────
# Debug / Release
# ──────────────────────────────────────────────────────────────

O_DEBUG := 0

ifneq ($(filter debug,$(MAKECMDGOALS)),)
	O_DEBUG := 1
endif

ifeq ($(strip $(O_DEBUG)),1)
	CFLAGS += -g3 -DDEBUG -DLOG_SHOW_SOURCE_LOCATION
    ifneq (,$(findstring clang,$(CC)))
        CFLAGS += -ffreestanding
    endif
else
	CFLAGS += $(CFLAGS_OPTIMIZATION)
endif

# ──────────────────────────────────────────────────────────────
# Platform
# ──────────────────────────────────────────────────────────────

ifeq ($(UNAME_S),Darwin)
	# LDLIBS +=
else
	CFLAGS += -D_GNU_SOURCE
endif

# ──────────────────────────────────────────────────────────────
# Sources
# ──────────────────────────────────────────────────────────────

SHARED_SRC := $(wildcard shared/*.c)
DAEMON_SRC := $(wildcard daemon/*.c)
CLIENT_SR  := $(wildcard client/*.c)

SHARED_HDR := $(wildcard shared/*.h)
DAEMON_HDR := $(wildcard daemon/*.h)
CLIENT_HDR := $(wildcard client/*.h)

# ──────────────────────────────────────────────────────────────
# Compiler Flags
# ──────────────────────────────────────────────────────────────

CFLAGS += -std=c17 -Ishared

# ──────────────────────────────────────────────────────────────
# Objects
# ──────────────────────────────────────────────────────────────

SHARED_OBJ := $(SHARED_SRC:%.c=$(BUILD)/%.o)
SHARED_CFLAGS := $(CFLAGS)

DAEMON_OBJ := $(DAEMON_SRC:%.c=$(BUILD)/%.o)
DAEMON_CFLAGS := $(CFLAGS)
# DAEMON_LIBS   := $(LDFLAGS) $(LDLIBS)
DAEMON_LIBS   :=

CLIENT_OBJ    := $(CLIENT_SR:%.c=$(BUILD)/%.o)
# CLIENT_CFLAGS :=
# CLIENT_LIBS   :=
# CLIENT_LIBS   := $(LDFLAGS) $(LDLIBS)

# ──────────────────────────────────────────────────────────────
# Adding third-party libraries
# ──────────────────────────────────────────────────────────────

ifeq ($(shell $(PKG_CONFIG) --exists libpcap && echo 1),1)
	DAEMON_CFLAGS += $(shell $(PKG_CONFIG) --cflags libpcap)
	DAEMON_LIBS   += $(shell $(PKG_CONFIG) --libs   libpcap)
else
    $(error libpcap not found. Install it via your package manager.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists argtable3 && echo 1),1)
	CLIENT_CFLAGS += $(shell $(PKG_CONFIG) --cflags argtable3)
	CLIENT_LIBS   += $(shell $(PKG_CONFIG) --libs   argtable3)
else
    $(error argtable3 not found. Install it via your package manager.)
endif

# ──────────────────────────────────────────────────────────────
# Targets
# ──────────────────────────────────────────────────────────────

all: $(BIN_SERVER) $(BIN_CLI)

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
	awk 'BEGIN {FS=":.*?## "}; {printf "\033[33m%-20s\033[0m %s\n", $$1, $$2}'

# Create build directories automatically
$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: %.c $(SHARED_HDR) $(DAEMON_HDR) ## CLIENT_CFLAGS find there you have to add this
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_SERVER): $(SHARED_OBJ) $(DAEMON_OBJ)   ## Build the netwatch binary
	$(CC) -o $@ $^ $(DAEMON_LIBS) $(LDFLAGS)

$(BIN_CLI): $(SHARED_OBJ) $(CLIENT_OBJ)  ## Build the netwatch-cli binary
	$(CC) -o $@ $^ $(CLIENT_LIBS) $(LDFLAGS)

debug: all  ## Build the debug binary run `make debug -B O_DEBUG=1`

install: all  ## Install the netwatch & netwatch-cli binary
	$(INSTALL) -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 $(BIN_SERVER) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 $(BIN_CLI) $(DESTDIR)$(PREFIX)/bin

uninstall:  ## Uninstall the netwatch & netwatch-cli binary
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN_SERVER)
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN_CLI)

strip: $(BIN) ## Strip the netwatch & netwatch-cli binary
	$(STRIP) $(BIN_SERVER)
	$(STRIP) $(BIN_CLI)

clean: ## Clean up build artifacts
	$(RM) -f $(BIN_SERVER) $(BIN_CLI) $(SHARED_OBJ) $(DAEMON_OBJ) $(CLI_OBJ)

.PHONY: all help debug install uninstall strip clean
