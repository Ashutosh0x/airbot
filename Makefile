# ═══════════════════════════════════════════════════════════════
# Airbot — Executable Information System
# Makefile — Pure C99, Zero External Dependencies
# ═══════════════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -O2
LDFLAGS =
LDLIBS  = -lws2_32
DEBUG_CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG

# Source directory
SRCDIR  = src
TESTDIR = tests
BUILDDIR = build

# Source files (ordered by dependency)
SRCS = \
	$(SRCDIR)/bitstream.c \
	$(SRCDIR)/blake3.c \
	$(SRCDIR)/chacha20.c \
	$(SRCDIR)/tlv.c \
	$(SRCDIR)/capability.c \
	$(SRCDIR)/eiu.c \
	$(SRCDIR)/eia.c \
	$(SRCDIR)/vm.c \
	$(SRCDIR)/verifier.c \
	$(SRCDIR)/environment.c \
	$(SRCDIR)/state.c \
	$(SRCDIR)/replicator.c \
	$(SRCDIR)/assembler.c \
	$(SRCDIR)/disassembler.c \
	$(SRCDIR)/metrics.c \
	$(SRCDIR)/experiments.c \
	$(SRCDIR)/matrix.c \
	$(SRCDIR)/visibility.c \
	$(SRCDIR)/privacy.c \
	$(SRCDIR)/onion.c \
	$(SRCDIR)/benchmark.c \
	$(SRCDIR)/crypto_test.c \
	$(SRCDIR)/main.c

# Object files
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

# Library objects (everything except main.o)
LIB_OBJS = $(filter-out $(BUILDDIR)/main.o,$(OBJS))

# Output binary
TARGET = $(BUILDDIR)/airbot

# Test files
TEST_SRCS = $(wildcard $(TESTDIR)/*.c)
TEST_TARGET = $(BUILDDIR)/test_runner

# ─── Targets ────────────────────────────────────────────────

.PHONY: all clean test debug dirs privacy-check

all: dirs $(TARGET)
	@echo "=== Airbot built successfully ==="
	@echo "    Binary: $(TARGET)"

debug: CFLAGS = $(DEBUG_CFLAGS)
debug: dirs $(TARGET)
	@echo "=== Airbot built (debug mode) ==="

dirs:
	@if not exist $(BUILDDIR) mkdir $(BUILDDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | dirs
	$(CC) $(CFLAGS) -c -o $@ $<

# Test build
test: dirs $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) $(LIB_OBJS)
	$(CC) $(CFLAGS) -I$(SRCDIR) -o $@ $^ $(LDLIBS)

# Privacy invariants + official crypto vectors. Fails the build on violation.
# Full security gate. Any failure fails the build.
privacy-check: all
	@echo "--- static security audit ---"
	sh tools/static-audit.sh
	@echo "--- cryptographic vectors (BLAKE3 / RFC 7539 / RFC 8439 / X25519) ---"
	$(TARGET) crypto-test
	@echo "--- per-hop onion, key separation, forward secrecy, replay ---"
	$(TARGET) onion-test
	@echo "--- privacy invariants (fail-closed, DNS, IPv4/IPv6, Tor) ---"
	$(TARGET) privacy-test
	@echo "--- adversarial / red-team suite ---"
	$(TARGET) adversarial-test
	@echo "--- relay key authentication (substitution / expiry / rollback) ---"
	$(TARGET) relaykey-test
	@echo "--- production channel codec ---"
	$(TARGET) channel-test
	@echo "--- relay batching (bounds, backpressure, shuffle) ---"
	$(TARGET) batch-test
	@echo "--- LIVE socket integration: real 3-relay onion chain ---"
	$(TARGET) live-test
	@echo "=== ALL SECURITY GATES PASSED ==="

clean:
	@if exist $(BUILDDIR) rmdir /s /q $(BUILDDIR)
	@echo "=== Clean ==="

# ─── Individual module compilation (for development) ─────

$(BUILDDIR)/bitstream.o:    $(SRCDIR)/bitstream.c $(SRCDIR)/bitstream.h
$(BUILDDIR)/blake3.o:       $(SRCDIR)/blake3.c $(SRCDIR)/blake3.h
$(BUILDDIR)/tlv.o:          $(SRCDIR)/tlv.c $(SRCDIR)/tlv.h $(SRCDIR)/bitstream.h
$(BUILDDIR)/capability.o:   $(SRCDIR)/capability.c $(SRCDIR)/capability.h $(SRCDIR)/blake3.h
$(BUILDDIR)/eiu.o:          $(SRCDIR)/eiu.c $(SRCDIR)/eiu.h $(SRCDIR)/bitstream.h $(SRCDIR)/tlv.h
$(BUILDDIR)/eia.o:          $(SRCDIR)/eia.c $(SRCDIR)/eia.h $(SRCDIR)/blake3.h $(SRCDIR)/capability.h
$(BUILDDIR)/vm.o:           $(SRCDIR)/vm.c $(SRCDIR)/vm.h $(SRCDIR)/eiu.h $(SRCDIR)/bitstream.h
$(BUILDDIR)/verifier.o:     $(SRCDIR)/verifier.c $(SRCDIR)/verifier.h $(SRCDIR)/vm.h $(SRCDIR)/eiu.h
$(BUILDDIR)/environment.o:  $(SRCDIR)/environment.c $(SRCDIR)/environment.h $(SRCDIR)/vm.h $(SRCDIR)/eiu.h
$(BUILDDIR)/state.o:        $(SRCDIR)/state.c $(SRCDIR)/state.h $(SRCDIR)/eiu.h $(SRCDIR)/environment.h
$(BUILDDIR)/replicator.o:   $(SRCDIR)/replicator.c $(SRCDIR)/replicator.h $(SRCDIR)/eiu.h $(SRCDIR)/capability.h
$(BUILDDIR)/assembler.o:    $(SRCDIR)/assembler.c $(SRCDIR)/assembler.h $(SRCDIR)/eiu.h $(SRCDIR)/vm.h
$(BUILDDIR)/disassembler.o: $(SRCDIR)/disassembler.c $(SRCDIR)/disassembler.h $(SRCDIR)/eiu.h $(SRCDIR)/vm.h
$(BUILDDIR)/metrics.o:      $(SRCDIR)/metrics.c $(SRCDIR)/metrics.h $(SRCDIR)/eiu.h
$(BUILDDIR)/experiments.o:  $(SRCDIR)/experiments.c $(SRCDIR)/experiments.h $(SRCDIR)/eiu.h $(SRCDIR)/vm.h
$(BUILDDIR)/matrix.o:       $(SRCDIR)/matrix.c $(SRCDIR)/matrix.h $(SRCDIR)/eiu.h $(SRCDIR)/environment.h
$(BUILDDIR)/transport.o:    $(SRCDIR)/transport.c $(SRCDIR)/transport.h $(SRCDIR)/blake3.h
$(BUILDDIR)/netcmd.o:       $(SRCDIR)/netcmd.c $(SRCDIR)/netcmd.h $(SRCDIR)/transport.h $(SRCDIR)/eiu.h
$(BUILDDIR)/main.o:         $(SRCDIR)/main.c
