# multi-lookup

Multithreaded **producer–consumer** DNS resolver in C using **POSIX threads**, a thread-safe bounded buffer, and mutex-protected I/O. Requester threads read hostnames from files; resolver threads dequeue them and resolve via `getaddrinfo` (IPv4 then IPv6).

**Good for:** systems programming, concurrency, and performance discussions on a résumé or GitHub.

---

## Build

```bash
make              # default: debug symbols, warnings
make release      # optimized (-O2); use before serious timing
make clean
```

Requires **GCC** (or compatible) and **pthreads**. Tested on macOS and Linux.

---

## How to run

**Syntax:**

```text
./multi-lookup <requester_threads> <resolver_threads> <serviced_log> <results_csv> <input_files...>
```

- **Requester threads** (producers): 1–10  
- **Resolver threads** (consumers): 1–10  
- **serviced_log**: hostnames read from inputs (order reflects scheduling)  
- **results_csv**: `hostname, ip` or `hostname, NOT_RESOLVED`  

**Examples:**

```bash
# Small run (few files)
./multi-lookup 2 2 serviced.txt results.txt input/names1.txt input/names2.txt

# All bundled inputs (~618 hostnames across 30 files)
./multi-lookup 5 5 serviced.txt results.txt input/*.txt

# Makefile shortcuts (need network — live DNS)
make run          # 2×2 threads, three small input files
make run-full     # 10×10 threads, all input/*.txt
```

DNS queries need **network access**. If everything shows `NOT_RESOLVED`, check connectivity and resolver settings.

---

## Performance: 1×1 vs 10×10 threads

Workload is **DNS-bound**: wall-clock time is dominated by `getaddrinfo` latency, not CPU. More resolver threads let many lookups overlap, so total runtime usually drops sharply until you hit diminishing returns (network, OS, and remote DNS limits).

### Quick comparison (same machine, same inputs)

Use the repo’s **three smallest files** (~63 hostnames total) so you can reproduce numbers in about a minute:

```bash
make bench-quick
```

Example output shape (your numbers will vary by machine and network):

| Configuration | Program-reported total time (illustrative) |
|---------------|---------------------------------------------|
| 1 requester / 1 resolver | ~4 s |
| 10 requesters / 10 resolvers | ~0.08 s |

That’s a large speedup on this **small** run because ten resolvers finish the backlog of lookups almost in parallel; the single-threaded case serializes DNS.

### Full bundled dataset (618 hostnames, 30 files)

```bash
make bench          # several minutes for the 1×1 case
# or, manually:
./multi-lookup 1 1 serviced.txt results_1.txt input/*.txt
./multi-lookup 10 10 serviced.txt results_10.txt input/*.txt
```

On typical class/lab hardware, **10×10** versus **1×1** is often on the order of **~5–7× faster end-to-end** for this full set, because most of the work is overlapping DNS waits rather than contending on your mutexes.

**Tip:** Run `make release` first if you want timings closer to what you’d quote in an interview (debug `-g` builds are slightly slower).

---

## Project layout

| File | Role |
|------|------|
| `multi-lookup.c` | Main, producer/consumer threads, CLI |
| `array.c` / `array.h` | Bounded buffer + sync |
| `newDNSlookup.c` | `dnslookup()` via `getaddrinfo` (IPv4, then IPv6) |
| `util.h` | `dnslookup` declaration, return codes |
| `input/*.txt` | Sample hostnames |

---
