# microarch-security-pocs — build all x86-64 teaching PoCs (Linux / WSL)
CC      ?= gcc
CFLAGS  ?= -O0 -std=c11 -Wall -Wextra
LDFLAGS ?=
THREADS ?= -pthread

BINS := \
	flush-reload/attack \
	spectre-v1/spectre_v1 \
	spectre-v2/spectre_v2 \
	spectre-v4/spectre_v4 \
	prime-probe/prime_probe \
	evict-time/evict_time \
	evict-reload/evict_reload \
	flush-flush/flush_flush \
	aes-timing/aes_timing \
	timing-attack/timing_attack \
	cache-timing-demo/timing \
	meltdown/meltdown_receiver \
	mds/mds_stub \
	rowhammer/rowhammer \
	netspectre/netspectre_local \
	branchscope/branchscope

.PHONY: all clean run-attack run-spectre

all: $(BINS)

flush-reload/attack: flush-reload/attack.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

spectre-v1/spectre_v1: spectre-v1/spectre_v1.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

spectre-v2/spectre_v2: spectre-v2/spectre_v2.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

spectre-v4/spectre_v4: spectre-v4/spectre_v4.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

prime-probe/prime_probe: prime-probe/prime_probe.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

evict-time/evict_time: evict-time/evict_time.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

evict-reload/evict_reload: evict-reload/evict_reload.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

flush-flush/flush_flush: flush-flush/flush_flush.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

aes-timing/aes_timing: aes-timing/aes_timing.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

timing-attack/timing_attack: timing-attack/timing_attack.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

cache-timing-demo/timing: cache-timing-demo/timing.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

meltdown/meltdown_receiver: meltdown/meltdown_receiver.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

mds/mds_stub: mds/mds_stub.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

rowhammer/rowhammer: rowhammer/rowhammer.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

netspectre/netspectre_local: netspectre/netspectre_local.c
	$(CC) $(CFLAGS) $(THREADS) $< $(LDFLAGS) -o $@

branchscope/branchscope: branchscope/branchscope.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(BINS)

run-attack: flush-reload/attack
	./flush-reload/attack

run-spectre: spectre-v1/spectre_v1
	./spectre-v1/spectre_v1
