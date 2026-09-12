#include "log/log.h"
#include <cassert>
#include <mutex>
#include <string>
#include <thread>
#include <cstdio>

static std::mutex mutex;
static bool reject;
static unsigned acquired, released, clock_reads;
extern "C" uint64_t log_get_system_time_ms(void) { return ++clock_reads; }
static int acquire(void*)
{
    if (reject) return 0;
    mutex.lock();
    ++acquired;
    return 42;
}
static void release(void*, int token)
{
    assert(token == 42);
    ++released;
    mutex.unlock();
}
static void capture(void* ctx, log_level_t, const char* data, size_t size)
{
    static_cast<std::string*>(ctx)->append(data, size);
    std::this_thread::yield();
}
int main()
{
    std::string a, b;
    log_sink_t sa{&a, capture}, sb{&b, capture};
    assert(!log_set_lock(nullptr, acquire, nullptr));
    assert(log_set_lock(nullptr, acquire, release));
    assert(log_register_sink(&sa) && log_register_sink(&sb));
    auto writer = [](const char* tag) {
        for (unsigned i = 0; i < 200; ++i) LOG_I(tag, "record=%u", i);
    };
    std::thread one(writer, "ONE"), two(writer, "TWO");
    one.join(); two.join();
    assert(a == b && clock_reads == 400 && acquired == 400 && released == 400);
    reject = true;
    LOG_I("DROP", "no formatter or transport");
    assert(clock_reads == 400 && released == 400);
    reject = false;
    log_set_level(LOG_LEVEL_NONE);
    LOG_I("DROP", "filtered level");
    assert(acquired == 401 && released == 401);
    log_set_level(LOG_LEVEL_DEBUG);
    assert(log_exclude_tag("DROP"));
    LOG_I("DROP", "filtered tag");
    assert(acquired == 402 && released == 402);
    log_unregister_sink(&sa); log_unregister_sink(&sb);
    LOG_I("NONE", "no sink");
    assert(acquired == 403 && released == 403 && clock_reads == 400);
    assert(log_set_lock(nullptr, nullptr, nullptr));
    std::puts("PASS: 400 serialized dual-sink records, rejection, unlock on filtered/no-sink paths");
}
