#include "init.h"
#include <cstdio>
#include "startup/cp15.h"
#include "regs/REGS.hpp"
#include "rtt/rtt_log.h"
#include "hal/INTC.hpp"
#include "hal/sysTimer.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "hal/MMU.hpp"
#include "hal/EDMA/EDMA.hpp"

#define TAG "brd_ini"

extern "C"
{
    void Entry(void);
    void UndefInstHandler(void);
    void SVC_Handler(void);
    void PrefetchAbortHandler(void);
    void DataAbortHandler(void);
    void IRQHandler(void);
    void FIQHandler(void);
}

static uint32_t const vec_tbl[15] =
{
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE24FF008,
    0xE59FF014,
    0xE59FF014,

    reinterpret_cast<uint32_t>(Entry),
    reinterpret_cast<uint32_t>(UndefInstHandler),
    reinterpret_cast<uint32_t>(SVC_Handler),
    reinterpret_cast<uint32_t>(PrefetchAbortHandler),
    reinterpret_cast<uint32_t>(DataAbortHandler),
    reinterpret_cast<uint32_t>(IRQHandler),
    reinterpret_cast<uint32_t>(FIQHandler)
};

extern HAL::TIMERS::sysTimer<SYST_t> sys_time;

static void mpu_pll_init();
static void core_pll_init();
static void per_pll_init();
static void interface_clocks_init();

struct alignas(8) FaultContext
{
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
    uint32_t fault_pc;
    uint32_t spsr;
    uint32_t dfsr_ifsr;
    uint32_t dfar_ifar;
};

extern "C" __attribute__((noinline)) void c_data_abort_handler(const FaultContext* ctx)
{
    uint32_t dfsr = ctx->dfsr_ifsr;
    uint32_t status_code = (dfsr & 0x0F) | ((dfsr >> 6) & 0x10);

    RTT_LOG_E("ABORT",
        "\n=== DATA ABORT DETECTED ===\n"
        "Faulting PC : 0x%08x\n"
        "DFAR (Addr) : 0x%08x\n"
        "DFSR        : 0x%08x (Status: 0x%02x)\n"
        "SPSR        : 0x%08x",
        (unsigned)ctx->fault_pc,
        (unsigned)ctx->dfar_ifar,
        (unsigned)dfsr,
        (unsigned)status_code,
        (unsigned)ctx->spsr
    );

    while (true) {
        __asm volatile("nop");
    }
}

extern "C" __attribute__((noinline)) void c_prefetch_abort_handler(const FaultContext* ctx)
{
    uint32_t ifsr = ctx->dfsr_ifsr;
    uint32_t status_code = (ifsr & 0x0F) | ((ifsr >> 6) & 0x10);
    bool is_thumb = (ctx->spsr & (1u << 5)) != 0;

    char opcode_str[32];
    if (status_code != 0b00101 && status_code != 0b00111) // Не Translation Fault
    {
        if (is_thumb)
        {
            uint16_t opcode = *reinterpret_cast<volatile uint16_t*>(ctx->fault_pc);
            snprintf(opcode_str, sizeof(opcode_str), "0x%04X (16-bit)", (unsigned)opcode);
        }
        else
        {
            uint32_t opcode = *reinterpret_cast<volatile uint32_t*>(ctx->fault_pc);
            snprintf(opcode_str, sizeof(opcode_str), "0x%08X (32-bit)", (unsigned)opcode);
        }
    }
    else
    {
        snprintf(opcode_str, sizeof(opcode_str), "[UNMAPPED MEMORY]");
    }

    RTT_LOG_E("ABORT",
        "\n=== PREFETCH ABORT DETECTED ===\n"
        "Faulting PC : 0x%08x\n"
        "IFAR        : 0x%08x\n"
        "IFSR        : 0x%08x (Status: 0x%02x)\n"
        "SPSR        : 0x%08x (Mode: %s)\n"
        "Opcode @ PC : %s",
        (unsigned)ctx->fault_pc,
        (unsigned)ctx->dfar_ifar,
        (unsigned)ifsr,
        (unsigned)status_code,
        (unsigned)ctx->spsr,
        is_thumb ? "Thumb" : "ARM",
        opcode_str
    );

    while (true) {
        __asm volatile("nop");
    }
}

static void copy_vector_table()
{
    auto *dest = reinterpret_cast<uint32_t*>(AM335X_VECTOR_BASE);
    auto *src  = const_cast<uint32_t*>(vec_tbl);

    cp15_vector_base_addr_set(AM335X_VECTOR_BASE);

    for(uint32_t count = 0; count < sizeof(vec_tbl)/sizeof(vec_tbl[0]); count++)
    {
        dest[count] = src[count];
    }
}

static void rtt_cache_clean()
{
    // Очищаем и инвалидируем кэш для RTT области
    // RTT область: 0x40300000 - 0x40310000 (64KB)
    cp15_D_cache_clean_flush_buff(0x40300000, 0x10000);
    cp15_I_cache_flush_buff(0x40300000, 0x10000);
    cp15_DSB_ISB_sync_barrier();
}

bool init_board()
{
    copy_vector_table();

    rtt_log_init();
    RTT_LOG_I(TAG, "=== AM335x EDMA test starting ===");
    rtt_cache_clean();

    init_memory();

    mpu_pll_init();
    core_pll_init();
    per_pll_init();
    interface_clocks_init();

    HAL::INTC::init();

    Board::init_user_leds();

    Board::get_uart0().init_polling();

    HAL::EDMA::module_clock_config();
    HAL::EDMA::init(REGS::EDMA::EVENT_Q0);

    HAL::INTC::master_IRQ_enable();

    RTT_LOG_I(TAG, "UART0 initialized in DMA mode");
    RTT_LOG_I(TAG, "EDMA initialized");

    return true;
}

static void mpu_pll_init()
{
    using namespace REGS::PRCM;
    auto& wkup = *AM335x_CM_WKUP;

    // Switch dpll mpu to bypass mode and  wait for bypass status
    wkup.CLKMODE_DPLL_MPU.reg = DPLL_MNBYPASS;
    while (wkup.IDLEST_DPLL_MPU.b.ST_MN_BYPASS == 0){}

    // configure divider and multipler
    // DPLL_MULT = 1000, DPLL_DIV = 23 (actual division factor is N+1)
    // 24MHz*1000/24 = 1GHz
    wkup.CLKSEL_DPLL_MPU.reg = (1000 << 8) | (23);

    wkup.DIV_M2_DPLL_MPU.b.DPLL_CLKOUT_DIV = 0x0;
    wkup.DIV_M2_DPLL_MPU.b.DPLL_CLKOUT_DIV |= 0x1;

    // Lock dpll mpu and  wait locking status
    wkup.CLKMODE_DPLL_MPU.reg = DPLL_LOCKMODE;
    while (wkup.IDLEST_DPLL_MPU.b.DPLL == 0){}
}

// Core PLL Configuration based on AM335x TRM 8.1.6.7.1
// All values based on AM335x TRM Table 8-22 Core PLL Typical Frequencies OPP100
// clock source is 24MHz crystal on OSC0-IN (BBB schematic page 3)
static void core_pll_init()
{
    using namespace REGS::PRCM;
    auto& wkup = *AM335x_CM_WKUP;

    // Switch dpll core to bypass mode and wait to baypass status
    wkup.CLKMODE_DPLL_CORE.b.DPLL_EN = DPLL_MNBYPASS;
    while (wkup.IDLEST_DPLL_CORE.b.ST_MN_BYPASS == 0){}

    // configure divider and multiplier
    // DPLL_MULT = 500, DPLL_DIV = 23 (actual division factor is N+1)
    // 24MHz*500/24 = 500 MHz
    wkup.CLKSEL_DPLL_CORE.reg = (500 << 8) | (23);

    // Set M4,M5,M6 dividers
    // Set M4,M5,M6 diveders
    wkup.DIV_M4_DPLL_CORE.b.HSDIVIDER_CLKOUT1_DIV = 0x0;
    wkup.DIV_M4_DPLL_CORE.b.HSDIVIDER_CLKOUT1_DIV |= 0x10;
    wkup.DIV_M5_DPLL_CORE.b.HSDIVIDER_CLKOUT2_DIV = 0x0;
    wkup.DIV_M5_DPLL_CORE.b.HSDIVIDER_CLKOUT2_DIV |= 0x8;
    wkup.DIV_M6_DPLL_CORE.b.HSDIVIDER_CLKOUT3_DIV = 0x0;
    wkup.DIV_M6_DPLL_CORE.b.HSDIVIDER_CLKOUT3_DIV |= 0x4;

    // Lock dpll core and wait locking status
    wkup.CLKMODE_DPLL_CORE.b.DPLL_EN = DPLL_LOCKMODE;
    while (wkup.IDLEST_DPLL_CORE.b.ST_DPLL_CLK == 0){}
}

// PER PLL Configuration based on AM335x TRM 8.1.6.8.1
// All values based on AM335x TRM Table 8-24 PER PLL Typical Frequencies OPP100
// clock source is 24MHz crystal on OSC0-IN (BBB schematic page 3)
static void per_pll_init()
{
    using namespace REGS::PRCM;
    auto& wkup = *AM335x_CM_WKUP;

    // Switch dpll per to bypas mode and wait bypass status
    wkup.CLKMODE_DPLL_PER.b.DPLL_EN = PER_MNBYPASS;
    while (wkup.IDLEST_DPLL_PER.b.ST_MN_BYPASS == 0){}

    // configure divider and multipler
    // DPLL_MULT = 960, DPLL_DIV = 23 (actual division factor is N+1)
    // 24MHz*960/24 = 960MHz
    wkup.CLKSEL_DPLL_PERIPH.reg = (960 << 8) | (23);

    wkup.DIV_M2_DPLL_PER.b.DPLL_CLKOUT_DIV = 0x0;
    wkup.DIV_M2_DPLL_PER.b.DPLL_CLKOUT_DIV |= 0x5;

    // Lock dpll per and wait locking status
    wkup.CLKMODE_DPLL_PER.b.DPLL_EN = PER_LOCKMODE;
    while (wkup.IDLEST_DPLL_PER.b.ST_DPLL_CLK == 0){}
}

static void interface_clocks_init()
{
    using namespace REGS::PRCM;
    auto& per = *AM335x_CM_PER;
    auto& wkup = *AM335x_CM_WKUP;

    wkup.CONTROL_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
    per.L4LS_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
    per.L3_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
    wkup.CLKSTCTRL.b.CLKTRCTRL = SW_WKUP;
    per.L4LS_CLKSTCTRL.b.CLKTRCTRL = SW_WKUP;
    per.L3S_CLKSTCTRL.b.CLKTRCTRL = SW_WKUP;
}

