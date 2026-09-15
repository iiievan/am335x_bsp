#ifndef REGS_MMCHS_HPP
#define REGS_MMCHS_HPP

#include <stdint.h>
#include <stddef.h>
#include "REGS.hpp"

namespace REGS::MMCHS
{
    /*********************************************************************************************************************************/

    /**************************************************** AM335x_MMCHS_Type Registers ************************************************/

    /*********************************************************************************************************************************/
    /*  CAUTION (TRM 18.5): the MMC/SD/SDIO registers are limited to 32-bit data accesses.
     *                      16-bit and 8-bit accesses are not allowed and can corrupt the register content.
     *                      Always go through the 'reg' member of the unions below, never through 'b' bitfields,
     *                      if the compiler may split the access.
     */

    /*  @brief      Power management, wake-up and clock gating of the OCP interface.
     *  @details    Controls the idle/standby behaviour of the module, the software reset and the
     *              automatic clock gating strategy.
    (offset = 0x110) [reset = 0x00000000] */
    enum e_SIDLEMODE : uint32_t
    {
        /*  Controls how the module responds to idle requests.
         *  Determines the power management behaviour when the peripheral is idle.
         */
        SIDLE_FORCE    = 0x0,       // Idle request acknowledged unconditionally, module goes to Inactive mode
        SIDLE_NO       = 0x1,       // Idle request is ignored, module keeps on behaving normally
        SIDLE_SMART    = 0x2,       // Smart idle based on internal activity (needs ENAWAKEUP to use wake-up)
        SIDLE_RESERVED = 0x3        // Reserved
    };

    enum e_CLOCKACTIVITY : uint32_t
    {
        /*  Controls which clocks remain active during the wake-up period.
         *  Bit 0 of the field is the interface clock, bit 1 is the functional clock.
         */
        CLKACT_BOTH_OFF = 0x0,      // Interface and functional clock may be switched off
        CLKACT_INTF_ON  = 0x1,      // Interface clock is maintained, functional clock may be switched off
        CLKACT_FUNC_ON  = 0x2,      // Functional clock is maintained, interface clock may be switched off
        CLKACT_BOTH_ON  = 0x3       // Interface and functional clocks are maintained
    };

    typedef union
    {
        struct
        {
            uint32_t    AUTOIDLE      : 1;      // bit  0       (R/W) Internal clock gating strategy
                                                //                    [ 0x0 (R) = Clocks are free-running;
                                                //                      0x1 (W) = Automatic clock gating applied, based on the
                                                //                                interconnect and MMC interface activity ]
            uint32_t    SOFTRESET     : 1;      // bit  1       (R/W) Software reset. The bit is automatically reset by the hardware.
                                                //                    During reset it always returns 0.
                                                //                    [ 0x0 (W) = No effect; 0x0 (R) = Normal mode;
                                                //                      0x1 (W) = Trigger a module reset; 0x1 (R) = The module is reset ]
            uint32_t    ENAWAKEUP     : 1;      // bit  2       (R/W) Wake-up feature control
                                                //                    [ 0x0 = Wake-up capability is disabled;
                                                //                      0x1 = Wake-up capability is enabled ]
            uint32_t    SIDLEMODE     : 2;      // bits 3,4     (R/W) Power management [see e_SIDLEMODE]
            uint32_t                  : 3;      // bits 5..7    (R)   Reserved
            uint32_t    CLOCKACTIVITY : 2;      // bits 8,9     (R/W) Clocks activity during the wake-up mode period [see e_CLOCKACTIVITY]
            uint32_t                  :22;      // bits 10..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } SYSCONFIG_reg_t;

    /*  @brief      System Status Register
     *  @details    Provides status information about the module, excluding the interrupt status information.
    (offset = 0x114) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    RESETDONE : 1;          // bit  0       (R)   Internal reset monitoring. Note: the debounce clock, the interface
                                                //                    clock and the functional clock shall be provided to the MMC/SD/SDIO
                                                //                    host controller to allow the internal reset monitoring.
                                                //                    [ 0x0 = Internal module reset is on-going;
                                                //                      0x1 = Reset completed ]
            uint32_t              :31;          // bits 1..31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } SYSSTATUS_reg_t;

    /*  @brief      Card Status Response Error Register
     *  @details    Enables the host controller to detect card status errors of response type R1, R1b for all cards
     *              and of R5, R5b and R6 for SD or SDIO cards. When SD_CSRE[i] is set to 1 and the corresponding bit
     *              of SD_RSP10 is set to 1, the host controller raises a card error (SD_STAT[28] CERR).
    (offset = 0x124) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CSRE :32;               // bits 0..31   (R/W) Card status response error mask
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } CSRE_reg_t;

    /*  @brief      System Test Register
     *  @details    Controls the signals connected to the I/O pins when the module is configured in system test
     *              (SYSTEST) mode for boundary connectivity verification. In SYSTEST mode a write into SD_CMD does
     *              not start a transfer and the buffer behaves as a stack accessible only by the local host.
    (offset = 0x128) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    MCKD : 1;               // bit  0       (R/W) MMC clock output signal data value
                                                //                    [ 0x0 (W) = The output clock is driven low;  0x0 (R) = No action, returns 0;
                                                //                      0x1 (W) = The output clock is driven high; 0x1 (R) = No action, returns 1 ]
            uint32_t    CDIR : 1;               // bit  1       (R/W) Control of the CMD pin direction
                                                //                    [ 0x0 (W) = The CMD line is an output (host to card);
                                                //                      0x1 (W) = The CMD line is an input (card to host) ]
            uint32_t    CDAT : 1;               // bit  2       (R/W) CMD input/output signal data value.
                                                //                    In output mode (CDIR = 0) a write drives the CMD line;
                                                //                    in input mode (CDIR = 1) a read returns the CMD line level.
            uint32_t    DDIR : 1;               // bit  3       (R/W) Control of the DAT[7:0] pins direction
                                                //                    [ 0x0 (W) = The DAT lines are outputs (host to card);
                                                //                      0x1 (W) = The DAT lines are inputs (card to host) ]
            uint32_t    D0D  : 1;               // bit  4       (R/W) DAT0 input/output signal data value.
                                                //                    In output mode (DDIR = 0) a write drives the line;
                                                //                    in input mode (DDIR = 1) a read returns the line level.
            uint32_t    D1D  : 1;               // bit  5       (R/W) DAT1 input/output signal data value (see D0D)
            uint32_t    D2D  : 1;               // bit  6       (R/W) DAT2 input/output signal data value (see D0D)
            uint32_t    D3D  : 1;               // bit  7       (R/W) DAT3 input/output signal data value (see D0D)
            uint32_t    D4D  : 1;               // bit  8       (R/W) DAT4 input/output signal data value (see D0D)
            uint32_t    D5D  : 1;               // bit  9       (R/W) DAT5 input/output signal data value (see D0D)
            uint32_t    D6D  : 1;               // bit  10      (R/W) DAT6 input/output signal data value (see D0D)
            uint32_t    D7D  : 1;               // bit  11      (R/W) DAT7 input/output signal data value (see D0D)
            uint32_t    SSB  : 1;               // bit  12      (R/W) Set status bit. This bit must be cleared before attempting to clear a
                                                //                    status bit of the interrupt status register (SD_STAT).
                                                //                    [ 0x0 (W) = Clear this SSB bit field. Writing 0 does not clear already
                                                //                                set status bits;
                                                //                      0x1 (W) = Force to 1 all status bits of SD_STAT, only if the
                                                //                                corresponding bit field in SD_ISE is set ]
            uint32_t    WAKD : 1;               // bit  13      (R/W) Wake request output signal data value
                                                //                    [ 0x0 (W) = The pin SWAKEUP is driven low;
                                                //                      0x1 (W) = The pin SWAKEUP is driven high ]
            uint32_t    SDWP : 1;               // bit  14      (R/W) Write protect input signal (SDWP) data value
                                                //                    [ 0x0 = The write protect pin SDWP is driven low;
                                                //                      0x1 = The write protect pin SDWP is driven high ]
            uint32_t    SDCD : 1;               // bit  15      (R/W) Card detect input signal (SDCD) data value
                                                //                    [ 0x0 = The card detect pin is driven low;
                                                //                      0x1 = The card detect pin is driven high ]
            uint32_t    OBI  : 1;               // bit  16      (R/W) Out-of-band interrupt (OBI) data value
                                                //                    [ 0x0 = The out-of-band interrupt pin is driven low;
                                                //                      0x1 = The out-of-band interrupt pin is driven high ]
            uint32_t         :15;               // bits 17..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } SYSTEST_reg_t;

    /*  @brief      Configuration Register
     *  @details    Selects the functional mode for any card, sends an initialization sequence to any card and
     *              enables the detection on the mmc_dat[1] signal of a card interrupt for SDIO cards only.
     *              It also configures the parameters related to the card detect and write protect input signals.
    (offset = 0x12C) [reset = 0x00000000] */
    enum e_DVAL : uint32_t
    {
        /*  Selects the debounce filter time for the card detect input signal (SDCD).
         *  Determines how long the signal must be stable before being recognized.
         */
        DEBOUNCE_33US  = 0x0,       // 33 us debounce period
        DEBOUNCE_231US = 0x1,       // 231 us debounce period
        DEBOUNCE_1MS   = 0x2,       // 1 ms debounce period
        DEBOUNCE_8_4MS = 0x3        // 8.4 ms debounce period
    };

    typedef union
    {
        struct
        {
            uint32_t    OD         : 1;         // bit  0       (R/W) Card open drain mode (MMC cards only). Must be set to 1 for MMC card
                                                //                    commands 1, 2, 3 and 40 and if the MMC card bus is operating in
                                                //                    open-drain mode during the response phase of the command sent.
                                                //                    Typically during card identification mode when the card is in idle,
                                                //                    ready or ident state. It is also necessary for a broadcast host
                                                //                    response (see SD_CON[2] HR bit).
                                                //                    [ 0x0 = No open drain;
                                                //                      0x1 = Open drain or broadcast host response ]
            uint32_t    INIT       : 1;         // bit  1       (R/W) Send initialization stream (all cards). When set to 1 and the card is
                                                //                    idle, an initialization sequence (mmc_cmd held to 1 during 80 clock
                                                //                    cycles) is sent to the card. The clock divider (SD_SYSCTL[15:6] CLKD)
                                                //                    must be set so that 80 clock periods are greater than 1 ms.
                                                //                    Note: no command is sent and no response is expected; a command
                                                //                    complete interrupt is generated once the sequence is completed and
                                                //                    SD_STAT[0] CC can be polled.
                                                //                    [ 0x0 = The host does not send an initialization sequence;
                                                //                      0x1 = The host sends an initialization sequence ]
            uint32_t    HR         : 1;         // bit  2       (R/W) Broadcast host response (MMC cards only). Forces the host to generate
                                                //                    a 48-bit response for the bc command type. To have the response
                                                //                    generated in open drain mode, SD_CON[0] OD must be set to 1.
                                                //                    [ 0x0 = The host does not generate a 48-bit response instead of a command;
                                                //                      0x1 = The host generates a 48-bit response instead of a command or
                                                //                            a command completion signal disable token ]
            uint32_t    STR        : 1;         // bit  3       (R/W) Stream command (MMC cards only). Must be set to 1 only for the stream
                                                //                    data transfers (read or write) of the adtc commands.
                                                //                    [ 0x0 = Block oriented data transfer;
                                                //                      0x1 = Stream oriented data transfer ]
            uint32_t    MODE       : 1;         // bit  4       (R/W) Mode select (all cards)
                                                //                    [ 0x0 = Functional mode. Transfers follow the card protocol, the MMC
                                                //                            clock is enabled, transfers are operated under the control of
                                                //                            the SD_CMD register;
                                                //                      0x1 = SYSTEST mode. The signal pins are configured as GPIO and the
                                                //                            1024-byte buffer is configured as a stack memory ]
            uint32_t    DW8        : 1;         // bit  5       (R/W) 8-bit mode MMC select (MMC cards only). For SD/SDIO cards this bit must
                                                //                    be cleared to 0. For MMC cards it must be set following a valid SWITCH
                                                //                    command (CMD6).
                                                //                    [ 0x0 = 1-bit or 4-bit data width;
                                                //                      0x1 = 8-bit data width ]
            uint32_t    MIT        : 1;         // bit  6       (R/W) MMC interrupt command (MMC cards only). Must be set to 1 when the next
                                                //                    write access to SD_CMD is an MMC interrupt command (CMD40) requiring
                                                //                    the command timeout detection to be disabled.
                                                //                    [ 0x0 = Command timeout enabled;
                                                //                      0x1 = Command timeout disabled ]
            uint32_t    CDP        : 1;         // bit  7       (R/W) Card detect polarity (all cards). Selects the active level of the card
                                                //                    detect input signal (SDCD).
                                                //                    [ 0x0 = Active high level;
                                                //                      0x1 = Active low level ]
            uint32_t    WPP        : 1;         // bit  8       (R/W) Write protect polarity (SD and SDIO cards only). Selects the active
                                                //                    level of the write protect input signal (SDWP).
                                                //                    [ 0x0 = Active high level;
                                                //                      0x1 = Active low level ]
            uint32_t    DVAL       : 2;         // bits 9,10    (R/W) Debounce filter value (all cards) [see e_DVAL]
            uint32_t    CTPL       : 1;         // bit  11      (R/W) Control power for the mmc_dat[1] line. By default the host controller
                                                //                    disables all the input buffers outside of a transaction to minimize the
                                                //                    leakage current.
                                                //                    [ 0x0 = Disable all the input buffers outside of a transaction;
                                                //                      0x1 = Disable all the input buffers except the buffer of mmc_dat[1]
                                                //                            outside of a transaction (asynchronous SDIO card interrupt) ]
            uint32_t    CEATA      : 1;         // bit  12      (R/W) CE-ATA control mode (MMC cards compliant with CE-ATA). The usage of the
                                                //                    Out-of-Band signal (OBI) is not supported.
                                                //                    [ 0x0 = Standard MMC/SD/SDIO mode;
                                                //                      0x1 = CE-ATA mode, next commands are considered as CE-ATA commands ]
            uint32_t               : 2;         // bits 13,14   (R)   Reserved
            uint32_t    PADEN      : 1;         // bit  15      (R/W) Control power for the MMC lines. Only useful when the MMC PADs contain a
                                                //                    power saving mechanism. Works as a GPIO that directly controls the
                                                //                    ACTIVE pin of the PADs.
                                                //                    [ 0x0 = ADPIDLE module pin is not forced, it is automatically generated
                                                //                            by the MMC FSMs;
                                                //                      0x1 = ADPIDLE module pin is forced to the active state ]
            uint32_t    CLKEXTFREE : 1;         // bit  16      (R/W) External clock free running. Maintains the card clock outside of a
                                                //                    transfer transaction. The clock is maintained only if SD_SYSCTL[2] CEN
                                                //                    is set.
                                                //                    [ 0x0 = External card clock is cut off outside the active transaction period;
                                                //                      0x1 = External card clock is maintained even outside the active
                                                //                            transaction period, only if SD_SYSCTL[2] CEN is set ]
            uint32_t    BOOT_ACK   : 1;         // bit  17      (R/W) Boot acknowledge received. When set, the controller expects a boot status
                                                //                    on the DAT0 line after the next command issued; if no status is
                                                //                    received a data timeout is generated.
                                                //                    [ 0x0 = No acknowledge to be received;
                                                //                      0x1 = A boot status will be received on the DAT0 line ]
            uint32_t    BOOT_CF0   : 1;         // bit  18      (R/W) Boot status supported. Set when the CMD line needs to be forced to 0 for
                                                //                    a boot sequence. The CMD line is driven to 0 after writing in SD_CMD and
                                                //                    released when this bit field is de-asserted.
                                                //                    [ 0x0 (R) = CMD line not forced;
                                                //                      0x1 (W) = CMD line forced to 0, active after writing into SD_CMD ]
            uint32_t    DDR        : 1;         // bit  19      (R/W) Dual data rate mode. Only meaningful for an even clock divider ratio of
                                                //                    SD_SYSCTL[CLKD]; insensitive to SD_HCTL[HSPE].
                                                //                    Note: DDR mode is NOT supported on AM335x - always set this bit to 0.
                                                //                    [ 0x0 = Standard mode, data are transmitted on a single edge;
                                                //                      0x1 = Data bytes and CRC are transmitted on both edges ]
            uint32_t    DMA_MnS    : 1;         // bit  20      (R/W) DMA master or slave selection. Only available if the generic parameter
                                                //                    MADMA_EN is asserted to 1.
                                                //                    [ 0x0 = The controller is slave on data transfers with the system;
                                                //                      0x1 = Not available on this device ]
            uint32_t    SDMA_LnE   : 1;         // bit  21      (R/W) Slave DMA level/edge request
                                                //                    [ 0x0 = Slave DMA edge sensitive (early de-assertion on the first
                                                //                            access to SD_DATA);
                                                //                      0x1 = Slave DMA level sensitive (late de-assertion, the request
                                                //                            remains active until the last allowed data) ]
            uint32_t               :10;         // bits 22..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } CON_reg_t;

    /*  @brief      Power Counter Register
     *  @details    Programs an MMC counter to delay the command transfers after activating the PAD power.
     *              This value depends on the PAD characteristics and voltage.
    (offset = 0x130) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    PWRCNT :16;             // bits 0..15   (R/W) Power counter register. Introduces a delay between the PAD ACTIVE pin
                                                //                    assertion and the command issued.
                                                //                    [ 0x0    = No additional delay added;
                                                //                      0x1    = TCF delay (card clock period);
                                                //                      0x2    = TCF x 2 delay (card clock period);
                                                //                      ...
                                                //                      0xFFFF = TCF x 65535 delay (card clock period) ]
            uint32_t           :16;             // bits 16..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } PWCNT_reg_t;

    /*  @brief      SDMA System Address Register
     *  @details    Contains the system memory address for an SDMA transfer. When the host controller stops an SDMA
     *              transfer this register points to the system address of the next contiguous data position.
     *              It can be accessed only if no transaction is executing; reads during a transfer may return an
     *              invalid value. ADMA does not use this register.
     *  @note       The TRM 'Type' column marks this register as R, but the very same section states that the host
     *              driver shall initialize it before starting an SDMA transaction and that writing its most upper
     *              byte restarts the transfer. It is therefore mapped as read/write below.
    (offset = 0x200) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    SDMA_SYSADDR :32;       // bits 0..31   (R/W) System memory address of the next contiguous data position
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } SDMASA_reg_t;

    /*  @brief      Transfer Length Configuration Register
     *  @details    SD_BLK[BLEN] is the block size register, SD_BLK[NBLK] is the block count register.
     *              This register can be accessed only if no transaction is executing; reads during a transfer may
     *              return an invalid value and writes are ignored.
    (offset = 0x204) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    BLEN :12;               // bits 0..11   (R/W) Transfer block size in bytes
                                                //                    [ 0x000 = No data transfer;
                                                //                      0x001 = 1 byte block length;
                                                //                      ...
                                                //                      0x200 = 512 bytes block length;
                                                //                      0x800 = 2048 bytes block length ]
            uint32_t         : 4;               // bits 12..15  (R)   Reserved
            uint32_t    NBLK :16;               // bits 16..31  (R/W) Blocks count for the current transfer. Enabled when SD_CMD[1] BCE is
                                                //                    set to 1 and valid only for multiple block transfers. The host
                                                //                    controller decrements the block count after each block transfer and
                                                //                    stops when the count reaches zero.
                                                //                    [ 0x0000 = Stop count;
                                                //                      0x0001 = 1 block;
                                                //                      ...
                                                //                      0xFFFF = 65535 blocks ]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } BLK_reg_t;

    /*  @brief      Command Argument Register
     *  @details    Contains the command argument specified as bits 39-8 of the command format. This register must be
     *              initialized prior to sending the command itself to the card (write into SD_CMD).
    (offset = 0x208) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    ARG :32;                // bits 0..31   (R/W) Command argument bits [31:0]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } ARG_reg_t;

    /*  @brief      Command and Transfer Mode Register
     *  @details    SD_CMD[31:16] is the command register, SD_CMD[15:0] is the transfer mode. A write into the most
     *              significant byte sends the command. A write into SD_CMD[15:0] during a data transfer has no
     *              effect. In SYSTEST mode a write into SD_CMD does not start a transfer.
    (offset = 0x20C) [reset = 0x00000000] */
    enum e_RSP_TYPE : uint32_t
    {
        /*  Defines the expected response format from the card.
         *  Determines how the controller interprets the response of the MMC/SD card.
         */
        RSP_NONE       = 0x0,       // No response
        RSP_136BIT     = 0x1,       // Response length 136 bits
        RSP_48BIT      = 0x2,       // Response length 48 bits
        RSP_48BIT_BUSY = 0x3        // Response length 48 bits with busy after response
    };

    enum e_CMD_TYPE : uint32_t
    {
        /*  Defines the command type for the current operation.
         *  Determines the special handling for the Suspend / Resume / Abort command categories.
         */
        CMD_NORMAL  = 0x0,          // Other commands (no special handling)
        CMD_SUSPEND = 0x1,          // Upon CMD52 "Bus Suspend" operation
        CMD_RESUME  = 0x2,          // Upon CMD52 "Function Select" operation
        CMD_ABORT   = 0x3           // Upon CMD12 or CMD52 "I/O Abort" command
    };

    enum e_CMD_INDEX : uint32_t
    {
        /*  Command index, binary encoded value from 0 to 63 specifying the command number sent to the card.
         */
        CMD0_ACMD0   = 0x00,
        CMD1_ACMD1   = 0x01,
        CMD2_ACMD2   = 0x02,
        CMD3_ACMD3   = 0x03,
        CMD4_ACMD4   = 0x04,
        CMD5_ACMD5   = 0x05,
        CMD6_ACMD6   = 0x06,
        CMD7_ACMD7   = 0x07,
        CMD8_ACMD8   = 0x08,
        CMD9_ACMD9   = 0x09,
        CMD10_ACMD10 = 0x0A,
        CMD11_ACMD11 = 0x0B,
        CMD12_ACMD12 = 0x0C,
        CMD13_ACMD13 = 0x0D,
        CMD14_ACMD14 = 0x0E,
        CMD15_ACMD15 = 0x0F,
        CMD16_ACMD16 = 0x10,
        CMD17_ACMD17 = 0x11,
        CMD18_ACMD18 = 0x12,
        CMD19_ACMD19 = 0x13,
        CMD20_ACMD20 = 0x14,
        CMD21_ACMD21 = 0x15,
        CMD22_ACMD22 = 0x16,
        CMD23_ACMD23 = 0x17,
        CMD24_ACMD24 = 0x18,
        CMD25_ACMD25 = 0x19,
        CMD26_ACMD26 = 0x1A,
        CMD27_ACMD27 = 0x1B,
        CMD28_ACMD28 = 0x1C,
        CMD29_ACMD29 = 0x1D,
        CMD30_ACMD30 = 0x1E,
        CMD31_ACMD31 = 0x1F,
        CMD32_ACMD32 = 0x20,
        CMD33_ACMD33 = 0x21,
        CMD34_ACMD34 = 0x22,
        CMD35_ACMD35 = 0x23,
        CMD36_ACMD36 = 0x24,
        CMD37_ACMD37 = 0x25,
        CMD38_ACMD38 = 0x26,
        CMD39_ACMD39 = 0x27,
        CMD40_ACMD40 = 0x28,
        CMD41_ACMD41 = 0x29,
        CMD42_ACMD42 = 0x2A,
        CMD43_ACMD43 = 0x2B,
        CMD44_ACMD44 = 0x2C,
        CMD45_ACMD45 = 0x2D,
        CMD46_ACMD46 = 0x2E,
        CMD47_ACMD47 = 0x2F,
        CMD48_ACMD48 = 0x30,
        CMD49_ACMD49 = 0x31,
        CMD50_ACMD50 = 0x32,
        CMD51_ACMD51 = 0x33,
        CMD52_ACMD52 = 0x34,
        CMD53_ACMD53 = 0x35,
        CMD54_ACMD54 = 0x36,
        CMD55_ACMD55 = 0x37,
        CMD56_ACMD56 = 0x38,
        CMD57_ACMD57 = 0x39,
        CMD58_ACMD58 = 0x3A,
        CMD59_ACMD59 = 0x3B,
        CMD60_ACMD60 = 0x3C,
        CMD61_ACMD61 = 0x3D,
        CMD62_ACMD62 = 0x3E,
        CMD63_ACMD63 = 0x3F
    };

    typedef union
    {
        struct
        {
            uint32_t    DE       : 1;           // bit  0       (R/W) DMA enable, enables the DMA mode for host data access
                                                //                    [ 0x0 = DMA mode disable;
                                                //                      0x1 = DMA mode enable ]
            uint32_t    BCE      : 1;           // bit  1       (R/W) Block count enable (multiple block transfers only). Enables the block
                                                //                    count register SD_BLK[31:16] NBLK. When block count is disabled in a
                                                //                    multiple block transfer (MSBS = 1) the module performs an infinite
                                                //                    transfer.
                                                //                    [ 0x0 = Block count disabled for infinite transfer;
                                                //                      0x1 = Block count enabled for multiple block transfer with a known
                                                //                            number of blocks ]
            uint32_t    ACEN     : 1;           // bit  2       (R/W) Auto CMD12 enable (SD cards only). The host controller issues a CMD12
                                                //                    automatically after the transfer completion of the last block. For
                                                //                    CE-ATA commands (SD_CON[12] CEATA = 1) auto CMD12 is useless and this
                                                //                    bit instead activates the command completion signal (CCS) detection.
                                                //                    [ 0x0 = Auto CMD12 disable;
                                                //                      0x1 = Auto CMD12 enable or CCS detection enabled ]
            uint32_t             : 1;           // bit  3       (R)   Reserved
            uint32_t    DDIR     : 1;           // bit  4       (R/W) Data transfer direction
                                                //                    [ 0x0 = Data write (host to card);
                                                //                      0x1 = Data read (card to host) ]
            uint32_t    MSBS     : 1;           // bit  5       (R/W) Multi/single block select. Must be set to 1 for a data transfer of a
                                                //                    multi block command, cleared to 0 for any other command.
                                                //                    [ 0x0 = Single block, SD_BLK[31:16] NBLK need not be set;
                                                //                      0x1 = Multi block ]
            uint32_t             :10;           // bits 6..15   (R)   Reserved
            uint32_t    RSP_TYPE : 2;           // bits 16,17   (R/W) Response type of the command [see e_RSP_TYPE]
            uint32_t             : 1;           // bit  18      (R)   Reserved
            uint32_t    CCCE     : 1;           // bit  19      (R/W) Command CRC check enable. Enables the CRC7 check on the command
                                                //                    response. On error SD_STAT[17] CCRC is set. The bit cannot be
                                                //                    configured for an Auto CMD12, where the check is automatic.
                                                //                    [ 0x0 = CRC7 check disable;
                                                //                      0x1 = CRC7 check enable ]
            uint32_t    CICE     : 1;           // bit  20      (R/W) Command index check enable. Compares the index field in the response
                                                //                    against the index of the command. On mismatch SD_STAT[19] CIE is set.
                                                //                    The bit cannot be configured for an Auto CMD12, where the check is
                                                //                    automatic.
                                                //                    [ 0x0 = Index check disable;
                                                //                      0x1 = Index check enable ]
            uint32_t    DP       : 1;           // bit  21      (R/W) Data present select. Must be cleared to 0 for a command using only the
                                                //                    mmc_cmd line, a command with no data transfer but using the busy signal
                                                //                    on mmc_dat0, and for a Resume command.
                                                //                    [ 0x0 = Command with no data transfer;
                                                //                      0x1 = Command with data transfer ]
            uint32_t    CMD_TYPE : 2;           // bits 22,23   (R/W) Command type [see e_CMD_TYPE]
            uint32_t    INDX     : 6;           // bits 24..29  (R/W) Command index [see e_CMD_INDEX]
            uint32_t             : 2;           // bits 30,31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } CMD_reg_t;

    /*  @brief      Command Response 0 and 1
     *  @details    Holds bit positions [31:0] of the command response type R1, R1b, R2, R3, R4, R5, R5b or R6.
    (offset = 0x210) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    RSP0 :16;               // bits 0..15   (R)   Command response [15:0]
            uint32_t    RSP1 :16;               // bits 16..31  (R)   Command response [31:16]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } RSP10_reg_t;

    /*  @brief      Command Response 2 and 3
     *  @details    Holds bit positions [63:32] of the command response type R2.
    (offset = 0x214) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    RSP2 :16;               // bits 0..15   (R)   Command response [47:32]
            uint32_t    RSP3 :16;               // bits 16..31  (R)   Command response [63:48]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } RSP32_reg_t;

    /*  @brief      Command Response 4 and 5
     *  @details    Holds bit positions [95:64] of the command response type R2.
    (offset = 0x218) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    RSP4 :16;               // bits 0..15   (R)   Command response [79:64]
            uint32_t    RSP5 :16;               // bits 16..31  (R)   Command response [95:80]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } RSP54_reg_t;

    /*  @brief      Command Response 6 and 7
     *  @details    Holds bit positions [127:96] of the command response type R2.
    (offset = 0x21C) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    RSP6 :16;               // bits 0..15   (R)   Command response [111:96]
            uint32_t    RSP7 :16;               // bits 16..31  (R)   Command response [127:112]
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } RSP76_reg_t;

    /*  @brief      Data Register
     *  @details    32-bit entry point of the buffer for read or write data transfers. The buffer size is
     *              32 bits x 256 (1024 bytes) and can be used as two 512-byte buffers. Bytes within a word are
     *              stored and read in little endian format. Sequential and contiguous access is necessary to
     *              increment the pointer correctly; random or skipped access is not allowed.
    (offset = 0x220) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    DATA :32;               // bits 0..31   (R/W) Data register [31:0]. In functional mode (SD_CON[4] MODE = 0) a read
                                                //                    is allowed only when SD_PSTATE[11] BRE is set to 1 and a write only
                                                //                    when SD_PSTATE[10] BWE is set to 1, otherwise a bad access
                                                //                    (SD_STAT[29] BADA) is signalled.
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } DATA_reg_t;

    /*  @brief      Present State Register
     *  @details    The host can get the status of the host controller from this 32-bit read only register.
    (offset = 0x224) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CMDI : 1;               // bit  0       (R)   Command inhibit (mmc_cmd). Indicates that the mmc_cmd line is in use.
                                                //                    Cleared when the most significant byte is written into SD_CMD; not set
                                                //                    when Auto CMD12 is transmitted.
                                                //                    [ 0x0 = Issuing of a command using the mmc_cmd line is allowed;
                                                //                      0x1 = Issuing of a command using the mmc_cmd line is not allowed ]
            uint32_t    DATI : 1;               // bit  1       (R)   Command inhibit (mmc_dat). Generated if the mmc_dat line is active
                                                //                    (DLA), or a read transfer is active (RTA), or a command with busy was
                                                //                    issued. A change of this bit from 1 to 0 generates a transfer complete
                                                //                    interrupt (SD_STAT[1] TC).
                                                //                    [ 0x0 = Issuing of a command using the mmc_dat lines is allowed;
                                                //                      0x1 = Issuing of a command using the mmc_dat lines is not allowed ]
            uint32_t    DLA  : 1;               // bit  2       (R)   mmc_dat line active. Indicates whether one of the mmc_dat lines is in
                                                //                    use. For reads it is set after the end bit of the read command and
                                                //                    cleared on the end bit of the last data block; for writes it is set
                                                //                    after the end bit of the write command and cleared on the end of the
                                                //                    busy event for the last block.
                                                //                    [ 0x0 = mmc_dat line inactive;
                                                //                      0x1 = mmc_dat line active ]
            uint32_t         : 5;               // bits 3..7    (R)   Reserved
            uint32_t    WTA  : 1;               // bit  8       (R)   Write transfer active. Set after the end bit of the write command,
                                                //                    cleared when the CRC status has been received after the last block.
                                                //                    [ 0x0 = No valid data on the mmc_dat lines;
                                                //                      0x1 = Write data transfer ongoing ]
            uint32_t    RTA  : 1;               // bit  9       (R)   Read transfer active. Set after the end bit of the read command,
                                                //                    cleared when all data have been read by the local host.
                                                //                    [ 0x0 = No valid data on the mmc_dat lines;
                                                //                      0x1 = Read data transfer ongoing ]
            uint32_t    BWE  : 1;               // bit  10      (R)   Buffer write enable, used for non-DMA write transfers
                                                //                    [ 0x0 = There is no room left in the buffer to write BLEN bytes of data;
                                                //                      0x1 = There is enough space in the buffer to write BLEN bytes of data ]
            uint32_t    BRE  : 1;               // bit  11      (R)   Buffer read enable, used for non-DMA read transfers. Indicates that a
                                                //                    complete block specified by SD_BLK[11:0] BLEN has been written in the
                                                //                    buffer and is ready to be read.
                                                //                    [ 0x0 = Read BLEN bytes disable;
                                                //                      0x1 = Read BLEN bytes enable, readable data exists in the buffer ]
            uint32_t         : 4;               // bits 12..15  (R)   Reserved
            uint32_t    CINS : 1;               // bit  16      (R)   Card inserted, the debounced value of the card detect input pin (SDCD).
                                                //                    This bit is not affected by a software reset.
                                                //                    [ 0x0 = If SD_CON[7] CDP is cleared to 0 (default) no card is detected,
                                                //                            otherwise the card has been inserted;
                                                //                      0x1 = If SD_CON[7] CDP is cleared to 0 (default) the card has been
                                                //                            inserted, otherwise no card is detected ]
            uint32_t    CSS  : 1;               // bit  17      (R)   Card state stable, used for testing. Set to 1 only when the card detect
                                                //                    pin level (CDPL) is stable. Not affected by a software reset.
                                                //                    [ 0x0 = Reset or debouncing;
                                                //                      0x1 = Card detect pin level is stable ]
            uint32_t    CDPL : 1;               // bit  18      (R)   Card detect pin level, reflects the inverse value of the card detect
                                                //                    input pin (SDCD). Debouncing is not performed on this bit and it is
                                                //                    valid only when CSS is set to 1; it must be debounced by software.
                                                //                    [ 0x0 = The value of the card detect input pin (SDCD) is 1;
                                                //                      0x1 = The value of the card detect input pin (SDCD) is 0 ]
            uint32_t    WP   : 1;               // bit  19      (R)   Write protect, reflects the write protect input pin (SDWP) level
                                                //                    [ 0x0 = If SD_CON[8] WPP is cleared to 0 (default) the card is write
                                                //                            protected, otherwise it is not;
                                                //                      0x1 = If SD_CON[8] WPP is cleared to 0 (default) the card is not write
                                                //                            protected, otherwise it is ]
            uint32_t    DLEV : 4;               // bits 20..23  (R)   mmc_dat[3:0] line signal level: bit 20 is mmc_dat0 ... bit 23 is
                                                //                    mmc_dat3. Used to check the mmc_dat line level to recover from errors
                                                //                    and for debugging, especially to detect the busy signal on mmc_dat0.
            uint32_t    CLEV : 1;               // bit  24      (R)   mmc_cmd line signal level, used to recover from errors and for debugging
                                                //                    [ 0x0 = The mmc_cmd line level is 0;
                                                //                      0x1 = The mmc_cmd line level is 1 ]
            uint32_t         : 7;               // bits 25..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } PSTATE_reg_t;

    /*  @brief      Host Control Register
     *  @details    Defines the host controls to set power, wake-up and transfer parameters.
     *              SD_HCTL[31:24] = wake-up control, SD_HCTL[23:16] = block gap control,
     *              SD_HCTL[15:8]  = power control,   SD_HCTL[7:0]   = host control.
    (offset = 0x228) [reset = 0x00000000] */
    enum e_DMAS : uint32_t
    {
        /*  Selects the DMA mode used for the data transfers.
         *  Only meaningful when the generic parameter MADMA_EN is set to 1.
         */
        DMA_RESERVED0 = 0x0,        // Reserved
        DMA_RESERVED1 = 0x1,        // Reserved
        DMA_ADMA2_32  = 0x2,        // 32-bit address ADMA2 is selected
        DMA_RESERVED3 = 0x3         // Reserved
    };

    enum e_SDVS : uint32_t
    {
        /*  Selects the operating voltage of the SD bus.
         *  Must match the voltage supported by the system (see SD_CAPA[26:24] VS18/VS30/VS33).
         */
        VS_1_8V = 0x5,              // 1.8 V (typical)
        VS_3_0V = 0x6,              // 3.0 V (typical)
        VS_3_3V = 0x7               // 3.3 V (typical)
    };

    typedef union
    {
        struct
        {
            uint32_t         : 1;               // bit  0       (R)   Reserved
            uint32_t    DTW  : 1;               // bit  1       (R/W) Data transfer width. Must be set following a valid SET_BUS_WIDTH
                                                //                    command (ACMD6) with the value written in bit 1 of the argument. Prior
                                                //                    to this command the SD card configuration register (SCR) must be
                                                //                    verified for the bus width supported by the card.
                                                //                    [ 0x0 = 1-bit data width (mmc_dat0 used);
                                                //                      0x1 = 4-bit data width (mmc_dat[3:0] used) ]
            uint32_t    HSPE : 1;               // bit  2       (R/W) High speed enable. Before setting this bit the host driver shall check
                                                //                    the high speed support in SD_CAPA. Shall not be set when the dual data
                                                //                    rate mode is activated in SD_CON[19] DDR.
                                                //                    [ 0x0 = Normal speed mode, CMD/DAT lines are output at the falling edge
                                                //                            of the SD clock;
                                                //                      0x1 = High speed mode, CMD/DAT lines are output at the rising edge of
                                                //                            the SD clock ]
            uint32_t    DMAS : 2;               // bits 3,4     (R/W) DMA select [see e_DMAS]
            uint32_t         : 1;               // bit  5       (R)   Reserved
            uint32_t    CDTL : 1;               // bit  6       (R/W) Card detect test level. Enabled while CDSS is set to 1.
                                                //                    [ 0x0 = No card;
                                                //                      0x1 = Card inserted ]
            uint32_t    CDSS : 1;               // bit  7       (R/W) Card detect signal selection. When the source for the card detection is
                                                //                    switched, the interrupt should be disabled during the switching period
                                                //                    by clearing SD_IE / SD_ISE to mask a glitch-induced interrupt.
                                                //                    [ 0x0 = SDCD# is selected (for normal use);
                                                //                      0x1 = The card detect test level (CDTL) is selected (for test purposes) ]
            uint32_t    SDBP : 1;               // bit  8       (R/W) SD bus power. Before setting this bit the host driver shall select the
                                                //                    SD bus voltage (SDVS). If the host controller detects the No card state
                                                //                    this bit is automatically cleared to 0. A write has no effect if the
                                                //                    selected SD bus voltage is not supported according to SD_CAPA[VS*].
                                                //                    [ 0x0 = Power off;
                                                //                      0x1 = Power on ]
            uint32_t    SDVS : 3;               // bits 9..11   (R/W) SD bus voltage select (all cards) [see e_SDVS]
            uint32_t         : 4;               // bits 12..15  (R)   Reserved
            uint32_t    SBGR : 1;               // bit  16      (R/W) Stop at block gap request. Stops executing a transaction at the next
                                                //                    block gap; the transfer can restart with a continue request (CR). For a
                                                //                    read transfer the card must support read wait control; for a write
                                                //                    transfer the host driver shall set this bit after all block data is
                                                //                    written. If this bit is set the local host shall not write to SD_DATA.
                                                //                    [ 0x0 = Transfer mode;
                                                //                      0x1 = Stop at block gap ]
            uint32_t    CR   : 1;               // bit  17      (R/W) Continue request. Restarts a transaction that was stopped by a stop at
                                                //                    block gap request. Automatically cleared by the host controller when
                                                //                    the transfer has restarted. SBGR must be cleared before setting it.
                                                //                    [ 0x0 = No effect;
                                                //                      0x1 = Transfer restart ]
            uint32_t    RWC  : 1;               // bit  18      (R/W) Read wait control, optional and only for SDIO cards. If the card does
                                                //                    not support read wait, enabling it may cause a conflict on the mmc_dat
                                                //                    line.
                                                //                    [ 0x0 = Disable read wait control, suspend/resume cannot be supported;
                                                //                      0x1 = Enable read wait control ]
            uint32_t    IBG  : 1;               // bit  19      (R/W) Interrupt block at gap. Valid only in 4-bit mode of an SDIO card. For
                                                //                    MMC and SD cards this bit should be cleared to 0.
                                                //                    [ 0x0 = Disable interrupt detection at the block gap in 4-bit mode;
                                                //                      0x1 = Enable interrupt detection at the block gap in 4-bit mode ]
            uint32_t         : 4;               // bits 20..23  (R)   Reserved
            uint32_t    IWE  : 1;               // bit  24      (R/W) Wake-up event enable on SD card interrupt. Wake-up is generated if the
                                                //                    wake-up feature is enabled (SD_SYSCONFIG[2] ENAWAKEUP) and the enable
                                                //                    status bit is set (SD_IE[8] CIRQ_ENABLE).
                                                //                    [ 0x0 = Disable wake-up on card interrupt;
                                                //                      0x1 = Enable wake-up on card interrupt ]
            uint32_t    INS  : 1;               // bit  25      (R/W) Wake-up event enable on SD card insertion. Wake-up is generated if the
                                                //                    wake-up feature is enabled (SD_SYSCONFIG[2] ENAWAKEUP).
                                                //                    [ 0x0 = Disable wake-up on card insertion;
                                                //                      0x1 = Enable wake-up on card insertion ]
            uint32_t    REM  : 1;               // bit  26      (R/W) Wake-up event enable on SD card removal. Wake-up is generated if the
                                                //                    wake-up feature is enabled (SD_SYSCONFIG[2] ENAWAKEUP).
                                                //                    [ 0x0 = Disable wake-up on card removal;
                                                //                      0x1 = Enable wake-up on card removal ]
            uint32_t    OBWE : 1;               // bit  27      (R/W) Wake-up event enable for the 'out-of-band' interrupt. The write to this
                                                //                    register is ignored when SD_CON[14] OBIE is not set.
                                                //                    [ 0x0 = Disable wake-up on 'out-of-band' interrupt;
                                                //                      0x1 = Enable wake-up on 'out-of-band' interrupt ]
            uint32_t         : 4;               // bits 28..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } HCTL_reg_t;

    /*  @brief      SD System Control Register
     *  @details    Defines the system controls to set the software resets, the clock frequency management and the
     *              data timeout. SD_SYSCTL[31:24] = software resets, SD_SYSCTL[23:16] = timeout control,
     *              SD_SYSCTL[15:0] = clock control.
    (offset = 0x22C) [reset = 0x00000000] */
    enum e_DTO : uint32_t
    {
        /*  Selects the data timeout period as a multiple of the timeout clock frequency (TCF).
         *  Determines how long the controller waits for the card before raising SD_STAT[20] DTO.
         */
        DTO_TCF_2_13 = 0x0,         // TCF x 2^13
        DTO_TCF_2_14 = 0x1,         // TCF x 2^14
        DTO_TCF_2_15 = 0x2,         // TCF x 2^15
        DTO_TCF_2_16 = 0x3,         // TCF x 2^16
        DTO_TCF_2_17 = 0x4,         // TCF x 2^17
        DTO_TCF_2_18 = 0x5,         // TCF x 2^18
        DTO_TCF_2_19 = 0x6,         // TCF x 2^19
        DTO_TCF_2_20 = 0x7,         // TCF x 2^20
        DTO_TCF_2_21 = 0x8,         // TCF x 2^21
        DTO_TCF_2_22 = 0x9,         // TCF x 2^22
        DTO_TCF_2_23 = 0xA,         // TCF x 2^23
        DTO_TCF_2_24 = 0xB,         // TCF x 2^24
        DTO_TCF_2_25 = 0xC,         // TCF x 2^25
        DTO_TCF_2_26 = 0xD,         // TCF x 2^26
        DTO_TCF_2_27 = 0xE,         // TCF x 2^27
        DTO_RESERVED = 0xF          // Reserved
    };

    typedef union
    {
        struct
        {
            uint32_t    ICE  : 1;               // bit  0       (R/W) Internal clock enable. The activity of the debounce clock (used for
                                                //                    wake-up events) and of the interface clock (used for reads and writes
                                                //                    to the register map) is not affected by this register.
                                                //                    [ 0x0 = The internal clock is stopped (very low power state);
                                                //                      0x1 = The internal clock oscillates and can be automatically gated
                                                //                            when SD_SYSCONFIG[0] AUTOIDLE is set to 1 ]
            uint32_t    ICS  : 1;               // bit  1       (R)   Internal clock stable (status)
                                                //                    [ 0x0 = The internal clock is not stable;
                                                //                      0x1 = The internal clock is stable after enabling the clock (ICE) or
                                                //                            after changing the clock ratio (CLKD) ]
            uint32_t    CEN  : 1;               // bit  2       (R/W) Clock enable, controls whether the clock is provided to the card
                                                //                    [ 0x0 = The clock is not provided to the card, the clock frequency can
                                                //                            be changed;
                                                //                      0x1 = The clock is provided to the card. The host driver shall wait
                                                //                            until the internal clock is stable (ICS) before setting it ]
            uint32_t         : 3;               // bits 3..5    (R)   Reserved
            uint32_t    CLKD :10;               // bits 6..15   (R/W) Clock frequency select, ratio between the reference clock frequency and
                                                //                    the output clock frequency on the mmc_clk pin
                                                //                    [ 0x000 = Clock Ref bypass;
                                                //                      0x001 = Clock Ref bypass;
                                                //                      0x002 = Clock Ref / 2;
                                                //                      ...
                                                //                      0x3FF = Clock Ref / 1023 ]
            uint32_t    DTO  : 4;               // bits 16..19  (R/W) Data timeout counter value and busy timeout [see e_DTO]
            uint32_t         : 4;               // bits 20..23  (R)   Reserved
            uint32_t    SRA  : 1;               // bit  24      (R/W) Software reset for all. Set to 1 to reset, released to 0 when completed.
                                                //                    Affects the entire host controller except the card detection circuit
                                                //                    and the capabilities registers.
                                                //                    [ 0x0 = Reset completed;
                                                //                      0x1 = Software reset for all the design ]
            uint32_t    SRC  : 1;               // bit  25      (R/W) Software reset for the mmc_cmd line. Set to 1 to reset, released to 0
                                                //                    when completed. Due to additional implementation logic the reset does
                                                //                    not start immediately: the proper procedure is (a) set to 1 to start
                                                //                    the reset, (b) poll for 1 to identify the start of the reset,
                                                //                    (c) poll for 0 to identify that the reset is complete.
                                                //                    Clears SD_PSTATE[CMDI] and SD_STAT[CC].
                                                //                    [ 0x0 = Reset completed;
                                                //                      0x1 = Software reset for the mmc_cmd line ]
            uint32_t    SRD  : 1;               // bit  26      (R/W) Software reset for the mmc_dat line. Same (a)(b)(c) procedure as SRC.
                                                //                    Clears SD_DATA, SD_PSTATE[BRE, BWE, RTA, WTA, DLA, DATI],
                                                //                    SD_HCTL[SBGR, CR] and SD_STAT[BRR, BWR, BGE, TC].
                                                //                    [ 0x0 = Reset completed;
                                                //                      0x1 = Software reset for the mmc_dat line ]
            uint32_t         : 5;               // bits 27..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } SYSCTL_reg_t;

    /*  @brief      SD Interrupt Status Register
     *  @details    Regroups all the status of the module internal events that can generate an interrupt.
     *              SD_STAT[31:16] = error interrupt status, SD_STAT[15:0] = normal interrupt status.
     *              All bits are cleared by writing a 1 to them, except bits 15 and 8: ERRI is automatically cleared
     *              when the error causing it is handled (that is, when bits 31:16 are cleared), CIRQ is cleared by
     *              writing a 0 to SD_IE[8] and servicing the interrupt.
    (offset = 0x230) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CC    : 1;              // bit  0       (R/W) Command complete, set when a 1-to-0 transition occurs in SD_PSTATE[0] CMDI
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Command complete ]
            uint32_t    TC    : 1;              // bit  1       (R/W) Transfer completed, always set when a read/write transfer is completed or
                                                //                    between two blocks when the transfer is stopped by a stop at block gap
                                                //                    request (SD_HCTL[16] SBGR)
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Data transfer complete ]
            uint32_t    BGE   : 1;              // bit  2       (R/W) Block gap event, set when the transaction is stopped at the block gap
                                                //                    during a read or write operation after a stop at block gap request
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Transaction stopped at block gap ]
            uint32_t    DMA   : 1;              // bit  3       (R/W) DMA interrupt, set when an interrupt is required in the ADMA instruction
                                                //                    and after the data transfer completion
                                                //                    [ 0x1 (W) = Status is cleared ]
            uint32_t    BWR   : 1;              // bit  4       (R/W) Buffer write ready, set during a write operation when the host can write
                                                //                    a complete block as specified by SD_BLK[11:0] BLEN.
                                                //                    Note: if the DMA transmit mode is enabled this bit is never set, a DMA
                                                //                    transmit request is generated instead.
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Ready to write buffer ]
            uint32_t    BRR   : 1;              // bit  5       (R/W) Buffer read ready, set during a read operation when one block specified
                                                //                    by SD_BLK[11:0] BLEN is completely written in the buffer.
                                                //                    Note: if the DMA receive mode is enabled this bit is never set, a DMA
                                                //                    receive request is generated instead.
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Ready to read buffer ]
            uint32_t    CINS  : 1;              // bit  6       (R/W) Card insertion, set when SD_PSTATE[CINS] changes from 0 to 1. Clearing
                                                //                    this bit does not affect SD_PSTATE[CINS].
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Card inserted ]
            uint32_t    CREM  : 1;              // bit  7       (R/W) Card removal, set when SD_PSTATE[CINS] changes from 1 to 0. Clearing
                                                //                    this bit does not affect SD_PSTATE[CINS].
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Card removed ]
            uint32_t    CIRQ  : 1;              // bit  8       (R)   Card interrupt, only used for SD and SDIO cards. Must be cleared by
                                                //                    setting SD_IE[8] CIRQ_ENABLE to 0, then the host driver must service
                                                //                    the card interrupt to remove the source. Writes to this bit are ignored.
                                                //                    [ 0x0 (R) = No card interrupt; 0x1 (R) = Generate card interrupt ]
            uint32_t    OBI   : 1;              // bit  9       (R)   Out-of-band interrupt (only useful for MMC cards), set when SD_CON[14]
                                                //                    OBIE is set and an out-of-band interrupt occurs on the OBI pin
                                                //                    [ 0x1 (R) = Interrupt out-of-band occurs ]
            uint32_t    BSR   : 1;              // bit  10      (R/W) Boot status received interrupt (only useful for MMC cards), set when
                                                //                    SD_CON[BOOT] is set to 1 or 2 and a boot status is received on DAT[0]
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Boot status received ]
            uint32_t          : 4;              // bits 11..14  (R)   Reserved
            uint32_t    ERRI  : 1;              // bit  15      (R)   Error interrupt, set to 1 if any of the bits in SD_STAT[31:16] is set,
                                                //                    so that the host driver can efficiently test for an error by checking
                                                //                    this bit first. Writes to this bit are ignored.
                                                //                    [ 0x0 (R) = No interrupt; 0x1 (R) = Error interrupt event(s) occurred ]
            uint32_t    CTO   : 1;              // bit  16      (R/W) Command timeout error, set when no response is received within 64 clock
                                                //                    cycles from the end bit of the command
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Time out ]
            uint32_t    CCRC  : 1;              // bit  17      (R/W) Command CRC error, set when there is a CRC7 error in the command
                                                //                    response, depending on the enable bit SD_CMD[19] CCCE
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Command CRC error ]
            uint32_t    CEB   : 1;              // bit  18      (R/W) Command end bit error, set when detecting a 0 at the end bit position of
                                                //                    a command response
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Command end bit error ]
            uint32_t    CIE   : 1;              // bit  19      (R/W) Command index error, set when the response index differs from the
                                                //                    corresponding command index, depending on SD_CMD[20] CICE
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Command index error ]
            uint32_t    DTO   : 1;              // bit  20      (R/W) Data timeout error: busy timeout for the R1b/R5b response types, busy
                                                //                    timeout after the write CRC status, write CRC status timeout or read
                                                //                    data timeout
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Time out ]
            uint32_t    DCRC  : 1;              // bit  21      (R/W) Data CRC error, set when there is a CRC16 error in the data phase
                                                //                    response following a block read command, or when the 3-bit CRC status
                                                //                    differs from the "010" token during a block write command
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Data CRC error ]
            uint32_t    DEB   : 1;              // bit  22      (R/W) Data end bit error, set when detecting a 0 at the end bit position of the
                                                //                    read data on the mmc_dat line, or at the end position of the CRC status
                                                //                    in write mode
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Data end bit error ]
            uint32_t          : 1;              // bit  23      (R)   Reserved
            uint32_t    ACE   : 1;              // bit  24      (R/W) Auto CMD12 error, set when one of the bits of SD_AC12 changed from 0 to 1
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Auto CMD12 error ]
            uint32_t    ADMAE : 1;              // bit  25      (R/W) ADMA error, set when the host controller detects errors during an ADMA
                                                //                    based data transfer. The state of the ADMA at the error occurrence is
                                                //                    saved in SD_ADMAES.
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = ADMA error ]
            uint32_t          : 2;              // bits 26,27   (R)   Reserved
            uint32_t    CERR  : 1;              // bit  28      (R/W) Card error, set when there is at least one error in a response of type
                                                //                    R1, R1b, R6, R5 or R5b. An error bit in the response is flagged only if
                                                //                    the corresponding bit in SD_CSRE is set.
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Card error ]
            uint32_t    BADA  : 1;              // bit  29      (R/W) Bad access to data space: read access to SD_DATA while SD_PSTATE[11] BRE
                                                //                    is 0, or write access to SD_DATA while SD_PSTATE[10] BWE is 0
                                                //                    [ 0x1 (W) = Status is cleared; 0x1 (R) = Bad access ]
            uint32_t          : 2;              // bits 30,31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } STAT_reg_t;

    /*  @brief      SD Interrupt Enable Register
     *  @details    Enables/disables the module to set the status bits of SD_STAT, on an event-by-event basis.
     *              SD_IE[31:16] = error interrupt status enable, SD_IE[15:0] = normal interrupt status enable.
     *  @note       This register only enables the reflection of the events in SD_STAT. To let an event generate a
     *              hardware interrupt request, the matching bit of SD_ISE must be set as well.
    (offset = 0x234) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CC_ENABLE   : 1;        // bit  0       (R/W) Command completed interrupt enable      [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    TC_ENABLE   : 1;        // bit  1       (R/W) Transfer completed interrupt enable     [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BGE_ENABLE  : 1;        // bit  2       (R/W) Block gap event interrupt enable        [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DMA_ENABLE  : 1;        // bit  3       (R/W) DMA interrupt enable                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BWR_ENABLE  : 1;        // bit  4       (R/W) Buffer write ready interrupt enable     [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BRR_ENABLE  : 1;        // bit  5       (R/W) Buffer read ready interrupt enable      [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CINS_ENABLE : 1;        // bit  6       (R/W) Card insertion interrupt enable. Must be set to 1 when entering smart
                                                //                    idle mode, to let the system identify the wake-up event and to allow
                                                //                    the controller to clear the internal wake-up source.
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CREM_ENABLE : 1;        // bit  7       (R/W) Card removal interrupt enable. Must be set to 1 when entering smart idle
                                                //                    mode (see CINS_ENABLE).
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CIRQ_ENABLE : 1;        // bit  8       (R/W) Card interrupt enable. Clearing this bit also clears the corresponding
                                                //                    status bit. In 1-bit mode, if the interrupt routine does not remove the
                                                //                    source of a card interrupt in the SDIO card, the status bit is
                                                //                    reasserted when this bit is set to 1. Must be set to 1 when entering
                                                //                    smart idle mode.
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    OBI_ENABLE  : 1;        // bit  9       (R/W) Out-of-band interrupt enable. A write to this bit when SD_CON[14] OBIE
                                                //                    is cleared to 0 is ignored.
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BSR_ENABLE  : 1;        // bit  10      (R/W) Boot status interrupt enable. A write to this bit when SD_CON[BOOT] is
                                                //                    cleared to 0 is ignored.
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t                : 4;        // bits 11..14  (R)   Reserved
            uint32_t    NULL_       : 1;        // bit  15      (R)   Fixed to 0. The host driver shall control the error interrupts using the
                                                //                    error interrupt signal enable register. Writes to this bit are ignored.
            uint32_t    CTO_ENABLE  : 1;        // bit  16      (R/W) Command timeout error interrupt enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CCRC_ENABLE : 1;        // bit  17      (R/W) Command CRC error interrupt enable      [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CEB_ENABLE  : 1;        // bit  18      (R/W) Command end bit error interrupt enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CIE_ENABLE  : 1;        // bit  19      (R/W) Command index error interrupt enable    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DTO_ENABLE  : 1;        // bit  20      (R/W) Data timeout error interrupt enable     [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DCRC_ENABLE : 1;        // bit  21      (R/W) Data CRC error interrupt enable         [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DEB_ENABLE  : 1;        // bit  22      (R/W) Data end bit error interrupt enable     [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t                : 1;        // bit  23      (R)   Reserved
            uint32_t    ACE_ENABLE  : 1;        // bit  24      (R/W) Auto CMD12 error interrupt enable       [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    ADMA_ENABLE : 1;        // bit  25      (R/W) ADMA error interrupt enable             [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t                : 2;        // bits 26,27   (R)   Reserved
            uint32_t    CERR_ENABLE : 1;        // bit  28      (R/W) Card error interrupt enable             [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BADA_ENABLE : 1;        // bit  29      (R/W) Bad access to data space enable         [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t                : 2;        // bits 30,31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } IE_reg_t;

    /*  @brief      SD Interrupt Signal Enable Register
     *  @details    Enables/disables the routing of a status bit of SD_STAT to the module interrupt line, on an
     *              event-by-event basis. SD_ISE[31:16] = error interrupt signal enable,
     *              SD_ISE[15:0] = normal interrupt signal enable.
    (offset = 0x238) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CC_SIGEN   : 1;         // bit  0       (R/W) Command completed signal status enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    TC_SIGEN   : 1;         // bit  1       (R/W) Transfer completed signal status enable [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BGE_SIGEN  : 1;         // bit  2       (R/W) Block gap event signal status enable    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DMA_SIGEN  : 1;         // bit  3       (R/W) DMA signal status enable                [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BWR_SIGEN  : 1;         // bit  4       (R/W) Buffer write ready signal status enable [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BRR_SIGEN  : 1;         // bit  5       (R/W) Buffer read ready signal status enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CINS_SIGEN : 1;         // bit  6       (R/W) Card insertion signal status enable. Must be set to 1 when entering
                                                //                    smart idle mode. [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CREM_SIGEN : 1;         // bit  7       (R/W) Card removal signal status enable. Must be set to 1 when entering smart
                                                //                    idle mode. [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CIRQ_SIGEN : 1;         // bit  8       (R/W) Card interrupt signal status enable. Clearing this bit also clears the
                                                //                    corresponding status bit. Must be set to 1 when entering smart idle
                                                //                    mode. [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    OBI_SIGEN  : 1;         // bit  9       (R/W) Out-of-band interrupt signal status enable. A write to this bit when
                                                //                    SD_CON[14] OBIE is cleared to 0 is ignored.
                                                //                    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BSR_SIGEN  : 1;         // bit  10      (R/W) Boot status signal status enable. A write to this bit when SD_CON[BOOT]
                                                //                    is cleared to 0 is ignored. [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t               : 4;         // bits 11..14  (R)   Reserved
            uint32_t    NULL_      : 1;         // bit  15      (R)   Fixed to 0. Writes to this bit are ignored.
            uint32_t    CTO_SIGEN  : 1;         // bit  16      (R/W) Command timeout error signal status enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CCRC_SIGEN : 1;         // bit  17      (R/W) Command CRC error signal status enable      [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CEB_SIGEN  : 1;         // bit  18      (R/W) Command end bit error signal status enable  [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    CIE_SIGEN  : 1;         // bit  19      (R/W) Command index error signal status enable    [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DTO_SIGEN  : 1;         // bit  20      (R/W) Data timeout error signal status enable
                                                //                    [ 0x0 = Masked. The host controller provides the clock to the card until
                                                //                            the card sends the data or the transfer is aborted;
                                                //                      0x1 = Enabled ]
            uint32_t    DCRC_SIGEN : 1;         // bit  21      (R/W) Data CRC error signal status enable         [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    DEB_SIGEN  : 1;         // bit  22      (R/W) Data end bit error signal status enable     [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t               : 1;         // bit  23      (R)   Reserved
            uint32_t    ACE_SIGEN  : 1;         // bit  24      (R/W) Auto CMD12 error signal status enable       [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    ADMA_SIGEN : 1;         // bit  25      (R/W) ADMA error signal status enable             [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t               : 2;         // bits 26,27   (R)   Reserved
            uint32_t    CERR_SIGEN : 1;         // bit  28      (R/W) Card error signal status enable             [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t    BADA_SIGEN : 1;         // bit  29      (R/W) Bad access to data space signal enable      [ 0x0 = Masked; 0x1 = Enabled ]
            uint32_t               : 2;         // bits 30,31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } ISE_reg_t;

    /*  @brief      Auto CMD12 Error Status Register
     *  @details    Lets the host driver determine which of the error cases related to Auto CMD12 has occurred.
     *              Valid only when Auto CMD12 is enabled (SD_CMD[2] ACEN) and the Auto CMD12 error (SD_STAT[24] ACE)
     *              is set to 1. These bits are automatically reset when starting a new adtc command with data.
    (offset = 0x23C) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    ACNE : 1;               // bit  0       (R)   Auto CMD12 not executed, set if a multiple block data transfer command
                                                //                    has started and an error occurs in the command before Auto CMD12 starts
                                                //                    [ 0x0 = Auto CMD12 executed; 0x1 = Auto CMD12 not executed ]
            uint32_t    ACTO : 1;               // bit  1       (R)   Auto CMD12 timeout error, set if no response is received within 64 clock
                                                //                    cycles from the end bit of the Auto CMD12 command
                                                //                    [ 0x0 = No error; 0x1 = Auto CMD12 time out ]
            uint32_t    ACCE : 1;               // bit  2       (R)   Auto CMD12 CRC error, set when a CRC7 error is detected in the Auto
                                                //                    CMD12 command response, depending on SD_CMD[19] CCCE
                                                //                    [ 0x0 = No error; 0x1 = Auto CMD12 CRC error ]
            uint32_t    ACEB : 1;               // bit  3       (R)   Auto CMD12 end bit error, set when detecting a 0 at the end bit position
                                                //                    of the Auto CMD12 command response
                                                //                    [ 0x0 = No error; 0x1 = Auto CMD12 end bit error ]
            uint32_t    ACIE : 1;               // bit  4       (R)   Auto CMD12 index error, set when the response index differs from the
                                                //                    corresponding Auto CMD12 index, depending on SD_CMD[20] CICE
                                                //                    [ 0x0 = No error; 0x1 = Auto CMD12 index error ]
            uint32_t         : 2;               // bits 5,6     (R)   Reserved
            uint32_t    CNI  : 1;               // bit  7       (R)   Command not issued by Auto CMD12 error: the pending command is not
                                                //                    executed due to an Auto CMD12 error (ACEB, ACCE, ACTO or ACNE)
                                                //                    [ 0x0 = Not error; 0x1 = Command not issued ]
            uint32_t         :24;               // bits 8..31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } AC12_reg_t;

    /*  @brief      Capabilities Register
     *  @details    Lists the capabilities of the MMC/SD/SDIO host controller. The voltage support bits are written
     *              once during the initialization, according to the system capabilities, and shall not be modified
     *              afterwards; they are only reinitialized by a hard reset (mmc_RESET signal).
    (offset = 0x240) [reset = 0x00000000] */
    enum e_MBL : uint32_t
    {
        /*  Maximum block length supported by the controller.
         *  Determines the largest single block that can be transferred through the host buffer.
         */
        MBL_512  = 0x0,             // 512 bytes maximum block length
        MBL_1024 = 0x1,             // 1024 bytes maximum block length
        MBL_2048 = 0x2              // 2048 bytes maximum block length
    };

    typedef union
    {
        struct
        {
            uint32_t    TCF       : 6;          // bits 0..5    (R)   Timeout clock frequency, used to detect the data timeout error
                                                //                    (SD_STAT[20] DTO). The unit is given by TCU.
                                                //                    [ 0x0 = The timeout clock frequency depends on the frequency of the
                                                //                            clock provided to the card and is not available here ]
            uint32_t              : 1;          // bit  6       (R)   Reserved
            uint32_t    TCU       : 1;          // bit  7       (R)   Timeout clock unit, the unit of the base clock frequency used to detect
                                                //                    the data timeout error (SD_STAT[20] DTO)
                                                //                    [ 0x0 = kHz; 0x1 = MHz ]
            uint32_t    BCF       : 6;          // bits 8..13   (R)   Base clock frequency of the clock provided to the card, encoded in the
                                                //                    unit given by TCU (0 means "not available in this register")
            uint32_t              : 2;          // bits 14,15   (R)   Reserved
            uint32_t    MBL       : 2;          // bits 16,17   (R)   Maximum block length [see e_MBL]
            uint32_t              : 1;          // bit  18      (R)   Reserved
            uint32_t    AD2S      : 1;          // bit  19      (R)   ADMA2 capability, depends on the setting of the generic parameter
                                                //                    MADMA_EN
                                                //                    [ 0x0 = ADMA2 supported; 0x1 = ADMA2 not supported ]
            uint32_t              : 1;          // bit  20      (R)   Reserved
            uint32_t    HSS       : 1;          // bit  21      (R)   High-speed support: the host controller supports high speed operations
                                                //                    and can supply an up-to-52 MHz clock to the card
                                                //                    [ 0x0 = High speed not supported; 0x1 = High speed supported ]
            uint32_t    DS        : 1;          // bit  22      (R)   DMA support: the host controller is able to use DMA to transfer data
                                                //                    between the system memory and the host controller directly
                                                //                    [ 0x0 = DMA not supported; 0x1 = DMA supported ]
            uint32_t    SRS       : 1;          // bit  23      (R)   Suspend/resume support (SDIO cards only)
                                                //                    [ 0x0 = Suspend/resume not supported;
                                                //                      0x1 = Suspend/resume supported ]
            uint32_t    VS33      : 1;          // bit  24      (R/W) Voltage support 3.3 V
                                                //                    [ 0x0 = 3.3 V not supported; 0x1 = 3.3 V supported ]
            uint32_t    VS30      : 1;          // bit  25      (R/W) Voltage support 3.0 V
                                                //                    [ 0x0 = 3.0 V not supported; 0x1 = 3.0 V supported ]
            uint32_t    VS18      : 1;          // bit  26      (R/W) Voltage support 1.8 V
                                                //                    [ 0x0 = 1.8 V not supported; 0x1 = 1.8 V supported ]
            uint32_t              : 1;          // bit  27      (R)   Reserved
            uint32_t    BUS_64BIT : 1;          // bit  28      (R/W) 64-bit system bus support: the host controller supports the 64-bit
                                                //                    address descriptor mode and is connected to a 64-bit address system bus
                                                //                    [ 0x0 (R) = 32-bit system bus address;
                                                //                      0x1 (R) = 64-bit system bus address ]
            uint32_t              : 3;          // bits 29..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } CAPA_reg_t;

    /*  @brief      Maximum Current Capabilities Register
     *  @details    Indicates the maximum current capability for each voltage. The value is meaningful only if the
     *              matching voltage support is set in SD_CAPA. Written once during the initialization, according to
     *              the system capabilities, and only reinitialized by a hard reset (mmc_RESET signal).
    (offset = 0x248) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    CUR_3V3 : 8;            // bits 0..7    (R/W) Maximum current for 3.3 V
                                                //                    [ 0x0 (R) = The maximum current capability for this voltage is not
                                                //                                available, feature not implemented ]
            uint32_t    CUR_3V0 : 8;            // bits 8..15   (R/W) Maximum current for 3.0 V (see CUR_3V3)
            uint32_t    CUR_1V8 : 8;            // bits 16..23  (R/W) Maximum current for 1.8 V (see CUR_3V3)
            uint32_t            : 8;            // bits 24..31  (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } CUR_CAPA_reg_t;

    /*  @brief      Force Event Register
     *  @details    Not a physically implemented register: it is an address at which the error interrupt status
     *              register can be written. The effect of a write is reflected in SD_STAT[31:16], if the
     *              corresponding bit of SD_IE is set.
    (offset = 0x250) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    FE_ACNE  : 1;           // bit  0       (W)   Force event Auto CMD12 not executed
                                                //                    [ 0x0 = No effect, no interrupt; 0x1 = Interrupt forced ]
            uint32_t    FE_ACTO  : 1;           // bit  1       (W)   Force event Auto CMD12 timeout error      [ 0x1 = Interrupt forced ]
            uint32_t    FE_ACCE  : 1;           // bit  2       (W)   Force event Auto CMD12 CRC error          [ 0x1 = Interrupt forced ]
            uint32_t    FE_ACEB  : 1;           // bit  3       (W)   Force event Auto CMD12 end bit error      [ 0x1 = Interrupt forced ]
            uint32_t    FE_ACIE  : 1;           // bit  4       (W)   Force event Auto CMD12 index error        [ 0x1 = Interrupt forced ]
            uint32_t             : 2;           // bits 5,6     (R)   Reserved
            uint32_t    FE_CNI   : 1;           // bit  7       (W)   Force event command not issued by Auto CMD12 error
                                                //                    [ 0x1 = Interrupt forced ]
            uint32_t             : 8;           // bits 8..15   (R)   Reserved
            uint32_t    FE_CTO   : 1;           // bit  16      (W)   Force event command timeout error         [ 0x1 = Interrupt forced ]
            uint32_t    FE_CCRC  : 1;           // bit  17      (W)   Force event command CRC error             [ 0x1 = Interrupt forced ]
            uint32_t    FE_CEB   : 1;           // bit  18      (W)   Force event command end bit error         [ 0x1 = Interrupt forced ]
            uint32_t    FE_CIE   : 1;           // bit  19      (W)   Force event command index error           [ 0x1 = Interrupt forced ]
            uint32_t    FE_DTO   : 1;           // bit  20      (W)   Force event data timeout error            [ 0x1 = Interrupt forced ]
            uint32_t    FE_DCRC  : 1;           // bit  21      (W)   Force event data CRC error                [ 0x1 = Interrupt forced ]
            uint32_t    FE_DEB   : 1;           // bit  22      (W)   Force event data end bit error            [ 0x1 = Interrupt forced ]
            uint32_t             : 1;           // bit  23      (R)   Reserved
            uint32_t    FE_ACE   : 1;           // bit  24      (W)   Force event Auto CMD12 error              [ 0x1 = Interrupt forced ]
            uint32_t    FE_ADMAE : 1;           // bit  25      (W)   Force event ADMA error                    [ 0x1 = Interrupt forced ]
            uint32_t             : 2;           // bits 26,27   (R)   Reserved
            uint32_t    FE_CERR  : 1;           // bit  28      (W)   Force event card error                    [ 0x1 = Interrupt forced ]
            uint32_t    FE_BADA  : 1;           // bit  29      (W)   Force event bad access to data space      [ 0x1 = Interrupt forced ]
            uint32_t             : 2;           // bits 30,31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } FE_reg_t;

    /*  @brief      ADMA Error Status Register
     *  @details    When an ADMA error interrupt has occurred, this register holds the ADMA state and SD_ADMASAL
     *              holds the address around the error descriptor. The host controller generates the ADMA error
     *              interrupt when it detects invalid descriptor data (Valid = 0) at the ST_FDS state.
    (offset = 0x254) [reset = 0x00000000] */
    enum e_AES : uint32_t
    {
        /*  ADMA error state: indicates the state of the ADMA when the error occurred.
         *  Helps to locate the error descriptor through SD_ADMASAL.
         */
        AES_ST_STOP  = 0x0,         // ST_STOP (stop DMA), SD_ADMASAL points to the error descriptor
        AES_ST_FDS   = 0x1,         // ST_FDS (fetch descriptor), SD_ADMASAL points to the error descriptor
        AES_RESERVED = 0x2,         // Never set, the ADMA never stops in this state
        AES_ST_TFR   = 0x3          // ST_TFR (transfer data), SD_ADMASAL points to the 'next' of the error descriptor
    };

    typedef union
    {
        struct
        {
            uint32_t    AES : 2;                // bits 0,1     (R/W) ADMA error state [see e_AES]
            uint32_t    LME : 1;                // bit  2       (W)   ADMA length mismatch error: while block count enable is set, the total
                                                //                    data length specified by the descriptor table differs from the one
                                                //                    specified by the block count and block length, or the total data length
                                                //                    cannot be divided by the block length
                                                //                    [ 0x0 = No error; 0x1 = Error ]
            uint32_t        :29;                // bits 3..31   (R)   Reserved
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } ADMAES_reg_t;

    /*  @brief      ADMA System Address Low Bits Register
     *  @details    Holds the byte address of the executing command of the descriptor table. The 32-bit address
     *              descriptor uses the lower 32 bits of this register. At the start of the ADMA the host driver
     *              shall set the start address of the descriptor table.
    (offset = 0x258) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    ADMA_A32B :32;          // bits 0..31   (R/W) The ADMA increments this address, which points to the next line,
                                                //                    whenever a descriptor line is fetched. When the ADMA error interrupt is
                                                //                    generated it holds the valid descriptor address depending on the ADMA
                                                //                    state. The descriptor table shall be programmed on a 32-bit boundary;
                                                //                    ADMA2 ignores the lower 2 bits and assumes them to be 00b.
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } ADMASAL_reg_t;

    /*  @brief      ADMA System Address High Bits Register
     *  @details    Upper half of the ADMA system address, used only with the 64-bit address descriptor mode.
    (offset = 0x25C) [reset = 0x00000000] */
    typedef union
    {
        struct
        {
            uint32_t    ADMA_A32B :32;          // bits 0..31   (R/W) ADMA system address, high 32 bits
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } ADMASAH_reg_t;

    /*  @brief      Versions Register
     *  @details    Contains the hard coded RTL vendor revision number, the version number of the SD specification
     *              compliancy and a slot status bit. SD_REV[31:16] = host controller version,
     *              SD_REV[15:0] = slot interrupt status.
    (offset = 0x2FC) [reset = 0x31010000] */
    typedef union
    {
        struct
        {
            uint32_t    SIS  : 1;               // bit  0       (R)   Slot interrupt status: the inverted state of the interrupt signal of the
                                                //                    module. After a power-on reset, or after a software reset for all
                                                //                    (SD_SYSCTL[24] SRA), the interrupt signal is deasserted and this status
                                                //                    reads 0.
            uint32_t         :15;               // bits 1..15   (R)   Reserved
            uint32_t    SREV : 8;               // bits 16..23  (R)   Specification version number: the standard SD host controller
                                                //                    specification version. The upper and lower 4 bits indicate the version.
                                                //                    Reset value is 0x01 [ 0x00 = SD host specification version 1.0 ]
            uint32_t    VREV : 8;               // bits 24..31  (R)   Vendor version number. Bits 7 to 4 are the major revision, bits 3 to 0
                                                //                    are the minor revision, e.g. 0x10 for 1.0 and 0x21 for 2.1.
                                                //                    Reset value is 0x31.
        } b;                                    // Structure used for bit access
        uint32_t  reg;                          // Type used for register access
    } REV_reg_t;

    /*  @brief      MMC/SD/SDIO host controller register map (MULTIMEDIA_CARD, TRM table 18-19)
     *  @details    The module register map starts at the instance base address and is 0x300 bytes long. Every
     *              offset that is not listed in the TRM is reserved and must not be modified, so the gaps are
     *              covered by the RESERVEDn arrays below.
     */
    struct AM335x_MMCHS_Type
    {
        __R    uint32_t         RESERVED0[68];   // (0x000..0x10C) Reserved
        __RW   SYSCONFIG_reg_t  SYSCONFIG;       // (0x110) System Configuration Register
        __R    SYSSTATUS_reg_t  SYSSTATUS;       // (0x114) System Status Register
        __R    uint32_t         RESERVED1[3];    // (0x118..0x120) Reserved
        __RW   CSRE_reg_t       CSRE;            // (0x124) Card Status Response Error Register
        __RW   SYSTEST_reg_t    SYSTEST;         // (0x128) System Test Register
        __RW   CON_reg_t        CON;             // (0x12C) Configuration Register
        __RW   PWCNT_reg_t      PWCNT;           // (0x130) Power Counter Register
        __R    uint32_t         RESERVED2[51];   // (0x134..0x1FC) Reserved
        __RW   SDMASA_reg_t     SDMASA;          // (0x200) SDMA System Address Register
        __RW   BLK_reg_t        BLK;             // (0x204) Transfer Length Configuration Register
        __RW   ARG_reg_t        ARG;             // (0x208) Command Argument Register
        __RW   CMD_reg_t        CMD;             // (0x20C) Command and Transfer Mode Register
        __R    RSP10_reg_t      RSP10;           // (0x210) Command Response 0 and 1
        __R    RSP32_reg_t      RSP32;           // (0x214) Command Response 2 and 3
        __R    RSP54_reg_t      RSP54;           // (0x218) Command Response 4 and 5
        __R    RSP76_reg_t      RSP76;           // (0x21C) Command Response 6 and 7
        __RW   DATA_reg_t       DATA;            // (0x220) Data Register
        __R    PSTATE_reg_t     PSTATE;          // (0x224) Present State Register (read only)
        __RW   HCTL_reg_t       HCTL;            // (0x228) Host Control Register
        __RW   SYSCTL_reg_t     SYSCTL;          // (0x22C) SD System Control Register
        __RW   STAT_reg_t       STAT;            // (0x230) SD Interrupt Status Register (write 1 to clear)
        __RW   IE_reg_t         IE;              // (0x234) SD Interrupt Enable Register
        __RW   ISE_reg_t        ISE;             // (0x238) SD Interrupt Signal Enable Register
        __R    AC12_reg_t       AC12;            // (0x23C) Auto CMD12 Error Status Register
        __RW   CAPA_reg_t       CAPA;            // (0x240) Capabilities Register
        __R    uint32_t         RESERVED3[1];    // (0x244) Reserved
        __RW   CUR_CAPA_reg_t   CUR_CAPA;        // (0x248) Maximum Current Capabilities Register
        __R    uint32_t         RESERVED4[1];    // (0x24C) Reserved
        __W    FE_reg_t         FE;              // (0x250) Force Event Register (write only)
        __RW   ADMAES_reg_t     ADMAES;          // (0x254) ADMA Error Status Register
        __RW   ADMASAL_reg_t    ADMASAL;         // (0x258) ADMA System Address Low Bits Register
        __RW   ADMASAH_reg_t    ADMASAH;         // (0x25C) ADMA System Address High Bits Register
        __R    uint32_t         RESERVED5[39];   // (0x260..0x2F8) Reserved
        __R    REV_reg_t        REV;             // (0x2FC) Versions Register

        /*  @brief      Read one 32-bit word of the card response.
         *  @param  n   Response word index: 0 -> SD_RSP10 [31:0], 1 -> SD_RSP32 [63:32],
         *                                   2 -> SD_RSP54 [95:64], 3 -> SD_RSP76 [127:96].
         *  @return     The value of the selected response register (not its offset).
         */
        uint32_t RSP(const uint8_t n) const volatile
        {
            switch (n & 0x3u)
            {
                case 0:  return RSP10.reg;
                case 1:  return RSP32.reg;
                case 2:  return RSP54.reg;
                default: return RSP76.reg;
            }
        }
    };

    /*  The register map must cover exactly the TRM range 0x000..0x2FC inclusive. */
    static_assert(sizeof(AM335x_MMCHS_Type) == (0x2FCu + 4u), "AM335x_MMCHS_Type must be exactly 0x300 bytes");

    static_assert(offsetof(AM335x_MMCHS_Type, SYSCONFIG) == 0x110, "SD_SYSCONFIG offset");
    static_assert(offsetof(AM335x_MMCHS_Type, SYSSTATUS) == 0x114, "SD_SYSSTATUS offset");
    static_assert(offsetof(AM335x_MMCHS_Type, CSRE)      == 0x124, "SD_CSRE offset");
    static_assert(offsetof(AM335x_MMCHS_Type, SYSTEST)   == 0x128, "SD_SYSTEST offset");
    static_assert(offsetof(AM335x_MMCHS_Type, CON)       == 0x12C, "SD_CON offset");
    static_assert(offsetof(AM335x_MMCHS_Type, PWCNT)     == 0x130, "SD_PWCNT offset");
    static_assert(offsetof(AM335x_MMCHS_Type, SDMASA)    == 0x200, "SD_SDMASA offset");
    static_assert(offsetof(AM335x_MMCHS_Type, BLK)       == 0x204, "SD_BLK offset");
    static_assert(offsetof(AM335x_MMCHS_Type, ARG)       == 0x208, "SD_ARG offset");
    static_assert(offsetof(AM335x_MMCHS_Type, CMD)       == 0x20C, "SD_CMD offset");
    static_assert(offsetof(AM335x_MMCHS_Type, RSP10)     == 0x210, "SD_RSP10 offset");
    static_assert(offsetof(AM335x_MMCHS_Type, RSP32)     == 0x214, "SD_RSP32 offset");
    static_assert(offsetof(AM335x_MMCHS_Type, RSP54)     == 0x218, "SD_RSP54 offset");
    static_assert(offsetof(AM335x_MMCHS_Type, RSP76)     == 0x21C, "SD_RSP76 offset");
    static_assert(offsetof(AM335x_MMCHS_Type, DATA)      == 0x220, "SD_DATA offset");
    static_assert(offsetof(AM335x_MMCHS_Type, PSTATE)    == 0x224, "SD_PSTATE offset");
    static_assert(offsetof(AM335x_MMCHS_Type, HCTL)      == 0x228, "SD_HCTL offset");
    static_assert(offsetof(AM335x_MMCHS_Type, SYSCTL)    == 0x22C, "SD_SYSCTL offset");
    static_assert(offsetof(AM335x_MMCHS_Type, STAT)      == 0x230, "SD_STAT offset");
    static_assert(offsetof(AM335x_MMCHS_Type, IE)        == 0x234, "SD_IE offset");
    static_assert(offsetof(AM335x_MMCHS_Type, ISE)       == 0x238, "SD_ISE offset");
    static_assert(offsetof(AM335x_MMCHS_Type, AC12)      == 0x23C, "SD_AC12 offset");
    static_assert(offsetof(AM335x_MMCHS_Type, CAPA)      == 0x240, "SD_CAPA offset");
    static_assert(offsetof(AM335x_MMCHS_Type, CUR_CAPA)  == 0x248, "SD_CUR_CAPA offset");
    static_assert(offsetof(AM335x_MMCHS_Type, FE)        == 0x250, "SD_FE offset");
    static_assert(offsetof(AM335x_MMCHS_Type, ADMAES)    == 0x254, "SD_ADMAES offset");
    static_assert(offsetof(AM335x_MMCHS_Type, ADMASAL)   == 0x258, "SD_ADMASAL offset");
    static_assert(offsetof(AM335x_MMCHS_Type, ADMASAH)   == 0x25C, "SD_ADMASAH offset");
    static_assert(offsetof(AM335x_MMCHS_Type, REV)       == 0x2FC, "SD_REV offset");

    constexpr uint32_t AM335x_MMCHS_0_BASE = 0x48060000;    // MMC0, connected to the SD slot on the BeagleBone Black
    constexpr uint32_t AM335x_MMCHS_1_BASE = 0x481D8000;    // MMC1, connected to the on-board eMMC on the BeagleBone Black
    constexpr uint32_t AM335x_MMCHS_2_BASE = 0x47810000;    // MMC2

    inline AM335x_MMCHS_Type * AM335x_MMCHS_0 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_0_BASE);
    inline AM335x_MMCHS_Type * AM335x_MMCHS_1 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_1_BASE);
    inline AM335x_MMCHS_Type * AM335x_MMCHS_2 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_2_BASE);

    //-> MMC/SD controller instance number. <-//
    enum e_MMCHS_INSTANCE_NUM : int
    {
        MMCHS_INSTANCE_NA = -1,
        MMCHS_INSTANCE_0  = 0x0,
        MMCHS_INSTANCE_1  = 0x1,
        MMCHS_INSTANCE_2  = 0x2
    };

    //-> Values used to select the bus/data width (SD_CON[5] DW8 and SD_HCTL[1] DTW). <-//
    enum e_BUS_WIDTH : uint32_t
    {
        WIDTH_1BIT = 0x1,
        WIDTH_4BIT = 0x4,
        WIDTH_8BIT = 0x8
    };

    //-> Software reset requests, SD_SYSCTL[26:24]. <-//
    enum e_LINE_RESET : uint32_t
    {
        RESET_ALL      = BIT(24),      // SRA - software reset for all the design
        RESET_CMD_LINE = BIT(25),      // SRC - software reset for the mmc_cmd line
        RESET_DAT_LINE = BIT(26)       // SRD - software reset for the mmc_dat line
    };

    /*  Event masks shared by SD_STAT, SD_IE and SD_ISE: the three registers have the very same bit layout,
     *  so one set of masks serves the status, the status-enable and the signal-enable registers.
     */
    enum e_EVENT : uint32_t
    {
        EVENT_CMD_COMPLETE  = BIT(0),  // CC    - command complete
        EVENT_XFER_COMPLETE = BIT(1),  // TC    - transfer completed
        EVENT_BLOCK_GAP     = BIT(2),  // BGE   - block gap event
        EVENT_DMA           = BIT(3),  // DMA   - ADMA interrupt
        EVENT_BUF_WR_READY  = BIT(4),  // BWR   - buffer write ready
        EVENT_BUF_RD_READY  = BIT(5),  // BRR   - buffer read ready
        EVENT_CARD_INS      = BIT(6),  // CINS  - card insertion
        EVENT_CARD_REM      = BIT(7),  // CREM  - card removal
        EVENT_CARD_IRQ      = BIT(8),  // CIRQ  - card interrupt
        EVENT_OOB_IRQ       = BIT(9),  // OBI   - out-of-band interrupt
        EVENT_BOOT_STATUS   = BIT(10), // BSR   - boot status received
        EVENT_ERROR         = BIT(15), // ERRI  - any error of the 31..16 group (SD_STAT only, read only)
        EVENT_ERR_CMD_TOUT  = BIT(16), // CTO   - command timeout error
        EVENT_ERR_CMD_CRC   = BIT(17), // CCRC  - command CRC error
        EVENT_ERR_CMD_EB    = BIT(18), // CEB   - command end bit error
        EVENT_ERR_CMD_INDX  = BIT(19), // CIE   - command index error
        EVENT_ERR_DATA_TOUT = BIT(20), // DTO   - data timeout error
        EVENT_ERR_DATA_CRC  = BIT(21), // DCRC  - data CRC error
        EVENT_ERR_DATA_EB   = BIT(22), // DEB   - data end bit error
        EVENT_ERR_ACMD12    = BIT(24), // ACE   - Auto CMD12 error
        EVENT_ERR_ADMA      = BIT(25), // ADMAE - ADMA error
        EVENT_ERR_CARD      = BIT(28), // CERR  - card error
        EVENT_ERR_BAD_ACCESS= BIT(29)  // BADA  - bad access to data space
    };

    //-> Mask of every error bit of SD_STAT / SD_IE / SD_ISE (bits 31..16). <-//
    constexpr uint32_t EVENT_ERR_MASK = 0xFFFF0000u;

    //-> Mask that clears every writable status bit of SD_STAT. <-//
    constexpr uint32_t EVENT_ALL_MASK = 0xFFFFFFFFu;

    constexpr uint32_t BLOCK_LEN_MAX   = 2048U;    // widest block SD_BLK[BLEN] can encode
    constexpr uint32_t BLOCK_LEN_SD    = 512U;     // block length used by standard capacity SD/SDHC cards
    constexpr uint32_t BUFFER_SIZE     = 1024U;    // size of the embedded data buffer of the AM335x MMCHS
    constexpr uint32_t RESPONSE_WORDS  = 4U;       // SD_RSP10 .. SD_RSP76

}   // namespace REGS::MMCHS

#endif //REGS_MMCHS_HPP
