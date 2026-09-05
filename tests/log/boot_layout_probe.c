void Entry(void) { }
#if defined(PROBE_RTT)
__attribute__((section(".rtt"), used)) char rtt_probe[6400];
#endif
#if defined(PROBE_LARGE)
char large_probe[70 * 1024];
#endif
