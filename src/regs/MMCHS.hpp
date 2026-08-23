#ifndef __MMCHS_HPP
#define __MMCHS_HPP

#include <stdint.h>
#include "REGS.hpp"

namespace REGS::MMCHS
{
    /******************************************************************************************************************************************************************************/  
    
    /*************************************************************************** AM335x_MMCHS_Type Registers **********************************************************************/ 
    
    /******************************************************************************************************************************************************************************/      
        
    /* (offset = 0x110) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                                /* Register SD_SYSCONFIG */
            uint32_t    AUTOIDLE      : 1;      // bit:0       (R/W) Internal Clock gating strategy 0h (R) = Clocks are free-running. 1h (W) = Automatic
                                                //                  clock gating strategy is applied, based on the interconnect and MMC interface activity.
                                                //                   [ 0x0 = Clocks free-running;
                                                //                     0x1 = Automatic clock gating ]
            uint32_t    SOFTRESET     : 1;      // bit:1       (R/W) Software reset. The bit is automatically reset by the hardware. During reset, it always
                                                //                  returns 0.
                                                //                   [ 0x0 = No effect;
                                                //                     0x1 = Trigger module reset ]
            uint32_t    ENAWAKEUP     : 1;      // bit:2       (R/W) Wake-up feature control
                                                //                   [ 0x0 = Disabled;
                                                //                     0x1 = Enabled ]
            uint32_t    SIDLEMODE     : 2;      // bits:3..4   (R/W) Power management 0h = If an idle request is detected, the MMC/SD/SDIO host controller
                                                //                  acknowledges it unconditionally and goes in Inactive mode. Interrupt and DMA requests
                                                //                  are unconditionally deasserted. [see e_SIDLEMODE]
            uint32_t                  : 3;      // bits:5..7   (R)   Reserved
            uint32_t    CLOCKACTIVITY : 2;      // bits:8..9   (R/W) Clocks activity during wake up mode period. Bit 8 is the Interface clock. Bit 9 is the
                                                //                  Functional clock. [see e_CLOCKACTIVITY]
            uint32_t                  :22;      // bits:10..31 (R)   Reserved
        } b;
        uint32_t reg;
    } SYSCONFIG_reg_t;

    enum e_SIDLEMODE : uint32_t
    {
        /*  Controls how the module responds to idle requests
         *  Determines power management behavior when peripheral is idle
         */
        SIDLE_FORCE    = 0x0,  // Unconditionally acknowledge idle request
        SIDLE_NO       = 0x1,  // Ignore idle request
        SIDLE_SMART    = 0x2,  // Smart idle based on internal activity
        SIDLE_RESERVED = 0x3   // Reserved
    };

    enum e_CLOCKACTIVITY : uint32_t
    {
        /*  Controls which clocks remain active during wake-up
         *  Determines clock gating behavior during low-power states
         */
        CLKACT_BOTH_OFF   = 0x0,  // Interface and Functional clock may be switched off
        CLKACT_INTF_ON    = 0x1,  // Interface clock maintained
        CLKACT_FUNC_ON    = 0x2,  // Functional clock maintained
        CLKACT_BOTH_ON    = 0x3   // Both clocks maintained
    };

    /* (offset = 0x114) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_SYSSTATUS */
            uint32_t    RESETDONE : 1;  // bit:0      (R) Internal Reset Monitoring. Notethe debounce clock , the interface clock and the
                                        //               functional clock shall be provided to the MMC/SD/SDIO host controller to allow the
                                        //               internal reset monitoring.
                                        //                 [ 0x0 = Reset ongoing;
                                        //                   0x1 = Reset completed ]
            uint32_t              :31;  // bits:1..31 (R) Reserved
        } b;
        uint32_t reg;
    } SYSSTATUS_reg_t;;

    /* (offset = 0x124) [reset state = 0x0] */
    typedef union
    {
        struct
        {
            /* Register SD_CSRE */
            uint32_t    CSRE :32; // bits:0..31 (R/W) Card status response error
        } b;
        uint32_t reg;
    } CSRE_reg_t;

    /* (offset = 0x128) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SD_SYSTEST */
            uint32_t    MCKD : 1;   // bit:0       (R/W) MMC clock output signal data value.
                                    //                  [ 0x0 (W) = The output clock is driven low. 0x0 (R) = Noaction. Returns 0.
                                    //                    0x1 (W) = The output clock is driven high. 0x1 (R) = No action. Returns 1.]
            uint32_t    CDIR : 1;   // bit:1       (R/W) Control of the CMD pin direction
                                    //                  [ 0x0(W) = The CMD line is an output (host to card). 0x0(R) = No action. Returns 0.
                                    //                    0x1 (W) = The CMD line is an input (card to host) . 0x1 (R) =  No action. Returns 1.]
            uint32_t    CDAT : 1;   // bit:2       (R/W) CMD input/output signal data value
                                    //                [0x0 (W) = If SD_SYSTEST[1] CDIR bit = 0 (output mode
                                    //                            direction), the CMD line is driven low. If SD_SYSTEST[1] CDIR bit = 1 (input mode
                                    //                            direction), no effect.
                                    //                 0x0 (R) = If SD_SYSTEST[1] CDIR bit = 1 (input mode direction),
                                    //                            returns the value on the CMD line (low). If SD_SYSTEST[1] CDIR bit = 0 (output mode
                                    //                            direction), returns 0 .
                                    //                 0x1 (W) = If SD_SYSTEST[1] CDIR bit = 0 (output mode direction), the CMD line is driven high.
                                    //                            If SD_SYSTEST[1] CDIR bit = 1 (input mode direction), no effect.
                                    //                 0x1 (R) = If SD_SYSTEST[1] CDIR bit = 1 (input mode direction), returns the value
                                    //                            on the CMD line (high) If SD_SYSTEST[1] CDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    DDIR : 1;   // bit:3       (R/W) Control of the DAT [7:0] pins direction.
                                    //                  [ 0x0 (W) = The DAT lines are outputs (host to card). 0x0 (R) = No action. Returns 0.
                                    //                    0x1 (W) = The DAT lines are inputs (card to host). 0x1 (R) = No action. Returns 1.]
            uint32_t    D0D  : 1;   // bit:4       (R/W) DAT0 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT0 line is driven low.
                                    //                              If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                              returns the value on the DAT0 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                              direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT0 line is driven high.
                                    //                            If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                            on the DAT0 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D1D  : 1;   // bit:5       (R/W) DAT1 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT1 line is driven low. If SD_SYSTEST[3] DDIR bit = 1 (input mode
                                    //                             direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                             returns the value on the DAT1 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                             direction), returns 0.
                                    //                   0x1 (W) =   If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT1 line is driven high.
                                    //                               If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                              returns the value on the DAT1 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D2D  : 1;   // bit:6       (R/W) DAT2 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT2 line is driven low. If SD_SYSTEST[3] DDIR bit = 1 (input mode
                                    //                              direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                              returns the value on the DAT2 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                              direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT2 line is driven high.
                                    //                           If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                           returns the value on the DAT2 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D3D  : 1;   // bit:7       (R/W) DAT3 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT3 line is driven low.
                                    //                              If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                              returns the value on the DAT3 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                              direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction),
                                    //                              the DAT3 line is driven high. If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no
                                    //                              effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                              on the DAT3 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D4D  : 1;   // bit:8       (R/W) DAT4 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT4 line is driven low.
                                    //                              If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                              returns the value on the DAT4 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                              direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction),
                                    //                              the DAT4 line is driven high. If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no
                                    //                              effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                              on the DAT4 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D5D  : 1;   // bit:9       (R/W) DAT5 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                             direction), the DAT5 line is driven low. If SD_SYSTEST[3] DDIR bit = 1 (input mode
                                    //                             direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                             returns the value on the DAT5 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                             direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction),
                                    //                             the DAT5 line is driven high. If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no
                                    //                             effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                             on the DAT5 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D6D  : 1;   // bit:10      (R/W) DAT6 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT6 line is driven low. If SD_SYSTEST[3] DDIR bit = 1 (input mode
                                    //                             direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                             returns the value on the DAT6 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                             direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction),
                                    //                             the DAT6 line is driven high. If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no
                                    //                             effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                             on the DAT6 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    D7D  : 1;   // bit:11      (R/W) DAT7 input/output signal data value.
                                    //                  [0x0 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), the DAT7 line is driven low. If SD_SYSTEST[3] DDIR bit = 1 (input mode
                                    //                             direction), no effect.
                                    //                   0x0 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction),
                                    //                             returns the value on the DAT7 line (low). If SD_SYSTEST[3] DDIR bit = 0 (output mode
                                    //                             direction), returns 0.
                                    //                   0x1 (W) = If SD_SYSTEST[3] DDIR bit = 0 (output mode direction),
                                    //                             the DAT7 line is driven high. If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), no
                                    //                             effect.
                                    //                   0x1 (R) = If SD_SYSTEST[3] DDIR bit = 1 (input mode direction), returns the value
                                    //                             on the DAT7 line (high) If SD_SYSTEST[3] DDIR bit = 0 (output mode direction), returns 1.]
            uint32_t    SSB  : 1;   // bit:12      (R/W) Set status bit. This bit must be cleared prior attempting to clear a status bit of the
                                    //                  interrupt status register (SD_STAT).
                                    //                  [0x0 (W) = Clear this SSB bit field. Writing 0 does
                                    //                             not clear already set status bits.
                                    //                   0x0 (R) = No action. Returns 0.
                                    //                   0x1 (W) = Force to 1 all status bits of the interrupt status register (SD_STAT) only if the corresponding bit
                                    //                             field in the Interrupt signal enable register (SD_ISE) is set.
                                    //                   0x1 (R) = No action. Returns 1.]
            uint32_t    WAKD : 1;   // bit:13      (R/W) Wake request output signal data value.
                                    //                  [0x0 (W) = The pin SWAKEUP is driven low.
                                    //                   0x0 (R) = No action. Returns 0.
                                    //                   0x1 (W) = The pin SWAKEUP is driven high.
                                    //                   0x1 (R) = No action. Returns 1.
            uint32_t    SDWP : 1;   // bit:14      (R/W) Write protect input signal (SDWP) data value
                                    //                   [0x0 = The write protect pin SDWP is driven low.
                                    //                    0x1 = The write protect pin SDWP is driven high.]
            uint32_t    SDCD : 1;   // bit:15      (R/W) Card detect input signal (SDCD) data value
                                    //                  [0x0 = The card detect pin is driven low.
                                    //                   0x1 = The card detect pin is driven high.]
            uint32_t    OBI  : 1;   // bit:16      (R/W) Out-of-band interrupt (OBI) data value.
                                    //                   [0x0 = The out-of-band interrupt pin is driven low.
                                    //                    0x1 = The out-of-band interrupt pin is driven high.]
            uint32_t         :15;   // bits:17..31 (R)   Reserved
        } b;
        uint32_t reg;
    } SYSTEST_reg_t;

    /* (offset = 0x12C) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_CON */
            uint32_t    OD         : 1; // bit:0       (R/W) Card open drain mode (MMC cards only). This bit must be set to 1 for MMC card commands
                                        //                  1, 2, 3 and 40, and if the MMC card bus is operating in open-drain mode during the
                                        //                  response phase to the command sent. Typically, during card identification mode when the
                                        //                  card is either in idle, ready or ident state. It is also necessary to set this bit to 1,
                                        //                  for a broadcast host response (see Broadcast host response register SD_CON[2] HR bit).
                                        //                  [ 0x0 = No open drain;
                                        //                    0x1 = Open drain or broadcast host response]
            uint32_t    INIT       : 1; // bit:1       (R/W) Send initialization stream (all cards). When this bit is set to 1, and the card is idle,
                                        //                  an initialization sequence is sent to the card. An initialization sequence consists of
                                        //                  setting the mmc_cmd line to 1 during 80 clock cycles. The initialization sequence is
                                        //                  mandatory - but it is not required to do it through this bit - this bit makes it easier.
                                        //                  Clock divider (SD_SYSCTL [15:6] CLKD bits) should be set to ensure that 80 clock periods
                                        //                  are greater than 1ms. Ensure that the functional clock frequency of the module and the
                                        //                  clock divider value conform to this requirement before using this bit for
                                        //                  initialization. Note: In this mode, there is no command sent to the card and no response
                                        //                  is expected. A command complete interrupt will be generated once the initialization
                                        //                  sequence is completed. SD_STAT[0] CC bit can be polled.
                                        //                  [0x0 = The host does not send an initialization sequence;
                                        //                   0x1 = The host sends an initialization sequence]
            uint32_t    HR         : 1; // bit:2       (R/W) Broadcast host response (MMC cards only). This register is used to force the host to
                                        //                  generate a 48-bit response for bc command type. It can be used to terminate the
                                        //                  interrupt mode by generating a CMD40 response by the core. In order to have the host
                                        //                  response to be generated in open drain mode, the register SD_CON[OD] must be set to 1.
                                        //                  When SD_CON[12] CEATA bit is set to 1 and SD_ARG cleared to 0, when writing 0000 0000h
                                        //                  into SD_CMD register, the host controller performs a &apos;command completion signal
                                        //                  disable&apos; token (i.e., mmc_cmd line held to 0 during 47 cycles followed by a 1).
                                        //                  [0x0 = The host does not generate a 48-bit response instead of a command;
                                        //                  [0x1 = The host generates a 48-bit response instead of a command or a command
                                        //                         completion signal disabletoken.]
            uint32_t    STR        : 1; // bit:3       (R/W) Stream command (MMC cards only). This bit must be set to 1 only for the stream data
                                        //                  transfers (read or write) of the adtc commands. Stream read is a class 1 command
                                        //                  (CMD11READ_DAT_UNTIL_STOP). Stream write is a class 3 command
                                        //                  (CMD20WRITE_DAT_UNTIL_STOP).
                                        //                  [0x0 = Block oriented data transfer;
                                        //                   0x1 = Stream oriented data transfer]
            uint32_t    MODE       : 1; // bit:4       (R/W) Mode select (all cards). This bit selects the functional mode.
                                        //                   [0x0 = Functional mode. Transfers to the MMC/SD/SDIO cards follow the card protocol. The MMC clock is enabled.
                                        //                          MMC/SD transfers are operated under the control of the SD_CMD register.
                                        //                    0x1 = SYSTEST mode. SYSTEST mode. The signal pins are configured as general-purpose input/output and
                                        //                          the 1024-byte buffer is configured as a stack memory accessible only by the local host
                                        //                          or system DMA. The pins retain their default type (input, output or in- out). SYSTEST
                                        //                          mode is operated under the control of the SYSTEST register.]
            uint32_t    DW8        : 1; // bit:5       (R/W) 8-bit mode MMC select (MMC cards only). For SD/SDIO cards, this bit must be cleared to
                                        //                  0. For MMC card, this bit must be set following a valid SWITCH command (CMD6) with the
                                        //                  correct value and extend CSD index written in the argument. Prior to this command, the
                                        //                  MMC card configuration register (CSD and EXT_CSD) must be verified for compliancy with
                                        //                  MMC standard specification.
                                        //                  [0x0 = 1-bit or 4-bit data width;
                                        //                   0x1 = 8-bit data width]
            uint32_t    MIT        : 1; // bit:6       (R/W) MMC interrupt command (MMC cards only). This bit must be set to 1, when the next write
                                        //                  access to the command register (SD_CMD) is for writing a MMC interrupt command (CMD40)
                                        //                  requiring the command timeout detection to be disabled for the command response.
                                        //                  [0x0 = Command timeout enabled.
                                        //                   0x1 = Command timeout disabled.]
            uint32_t    CDP        : 1; // bit:7       (R/W) Card detect polarity (all cards). This bit selects the active level of the card detect
                                        //                  input signal (SDCD). The usage of the card detect input signal (SDCD) is optional and
                                        //                  depends on the system integration and the type of the connector housing that
                                        //                  accommodates the card.
                                        //                  [0x0 = Active high level;
                                        //                   0x1 = Active low level]
            uint32_t    WPP        : 1; // bit:8       (R/W) Write protect polarity (SD and SDIO cards only). This bit selects the active level of
                                        //                  the write protect input signal (SDWP). The usage of the write protect input signal
                                        //                  (SDWP) is optional and depends on the system integration and the type of the connector
                                        //                  housing that accommodates the card.
                                        //                  [0x0 = Active high level;
                                        //                   0x1 = Active low level]
            uint32_t    DVAL       : 2; // bits:9..10  (R/W) Debounce filter value (all cards). This register is used to define a debounce period to
                                        //                  filter the card detect input signal (SDCD). The usage of the card detect input signal
                                        //                  (SDCD) is optional and depends on the system integration and the type of the connector
                                        //                  housing that accommodates the card. [see e_DVAL]
            uint32_t    CTPL       : 1; // bit:11      (R/W) Control Power for mmc_dat[1] line (SD cards). By default, this bit is cleared to 0 and
                                        //                  the host controller automatically disables all the input buffers outside of a
                                        //                  transaction to minimize the leakage current. SDIO cards. When this bit is set to 1, the
                                        //                  host controller automatically disables all the input buffers except the buffer of
                                        //                  mmc_dat[1] outside of a transaction in order to detect asynchronous card interrupt on
                                        //                  mmc_dat[1] line and minimize the leakage current of the buffers.
                                        //                  [0x0 = Disable all the input buffers outside of a transaction.
                                        //                   0x1 = Disable all the input buffers except the buffer of mmc_dat[1] outside of a transaction.]
            uint32_t    CEATA      : 1; // bit:12      (R/W) CE-ATA control mode (MMC cards compliant with CE-ATA). This bit selects the active level
                                        //                  of the out-of-band interrupt coming from MMC cards. The usage of the Out-of-Band signal
                                        //                  (OBI) is not supported.
                                        //                  [0x0 = Standard MMC/SD/SDIO mode.;
                                        //                   0x1 = CE-ATA mode. Next commands are considered as CE-ATA commands.]
            uint32_t               : 2; // bits:13..14 (R)   Reserved
            uint32_t    PADEN      : 1; // bit:15      (R/W) Control power for MMC lines. This register is only useful when MMC PADs contain power
                                        //                  saving mechanism to minimize its leakage power. It works as a GPIO that directly control
                                        //                  the ACTIVE pin of PADs. Excepted for mmc_dat[1] , the signal is also combine outside the
                                        //                  module with the dedicated power control SD_CON[11] CTPL bit.
                                        //                  [0x0 = ADPIDLE module pin is not forced, it is automatically generated by the MMC fsms.
                                        //                   0x1 = ADPIDLE module pin is forced to active state]
            uint32_t    CLKEXTFREE : 1; // bit:16      (R/W) External clock free running. This register is used to maintain card clock out of
                                        //                  transfer transaction to enable slave module (for example to generate a synchronous
                                        //                  interrupt on mmc_dat[1] ). The Clock will be maintain only if SD_SYSCTL[2] CEN bit is
                                        //                  set.
                                        //                  [0x0 = External card clock is cut off outside active transaction period.;
                                        //                   0x1 = External card clock is maintain even out of active transaction period only if
                                        //                         SD_SYSCTL[2] CEN bit is set.]
            uint32_t    BOOT_ACK   : 1; // bit:17      (R/W) Book acknowledge received. When this bit is set the controller should receive a boot
                                        //                  status on DAT0 line after next command issued. If no status is received a data timeout
                                        //                  will be generated.
                                        //                  [0x0 = No acknowledge to be received.;
                                        //                   0x1 = A boot status will be received on DAT0 line after issuing a command.]
            uint32_t    BOOT_CF0   : 1; // bit:18      (R/W) Boot Status Supported. This register is set when the CMD line needs to be forced to 0
                                        //                  for a boot sequence. CMD line is driven to 0 after writing in SD_CMD. The line is
                                        //                  released when this bit field is de-asserted and aborts data transfer in case of a
                                        //                  pending transaction.
                                        //                  [0x0 (W) = CMD line forced to 0 is enabled.
                                        //                   0x0 (R) = CMD line not forced.
                                        //                   0x1 (W) = CMD line forced to 0 is enabled and will be active after writing into
                                        //                             SD_CMD register.
                                        //                  0x1 (R) = CMD line is released when it was previously forced to 0 by a
                                        //                            boot sequence.]
            uint32_t    DDR        : 1; // bit:19      (R/W) Dual Data Rate mode. When this register is set, the controller uses both clock edge to
                                        //                  emit or receive data. Odd bytes are transmitted on falling edges and even bytes are
                                        //                  transmitted on rise edges. It only applies on Data bytes and CRC, Start, end bits and
                                        //                  CRC status are kept full cycle. This bit field is only meaningful and active for even
                                        //                  clock divider ratio of SD_SYSCTL[CLKD], it is insensitive to SD_HCTL[HSPE] setting.
                                        //                  Note: DDR mode is not supported on AM335x. Always set this bit to 0.
                                        //                  [0x0 = Standard modeData are transmitted on a single edge.
                                        //                   0x1 = Data Bytes and CRC are transmitted on both edges.]
            uint32_t    DMA_MnS    : 1; // bit:20      (R/W) DMA Master or Slave selection. When this bit is set and the controller is configured to
                                        //                  use the DMA, Ocp master interface is used to get datas from system using ADMA2 procedure
                                        //                  (direct access to the memory). This option is only available if generic parameter
                                        //                  MADMA_EN is asserted to 1.
                                        //                  [0x0 = The controller is slave on data transfers with system.
                                        //                   0x1 = Not available on this device.]
            uint32_t    SDMA_LnE   : 1; // bit:21      (R/W) Slave DMA Level/Edge Request. The waveform of the DMA request can be configured either
                                        //                  edge sensitive with early de-assertion on first access to SD_DATA register or late
                                        //                  de-assertion, request remains active until last allowed data written into SD_DATA.
                                        //                  [0x0 = Slave DMA edge sensitive.
                                        //                   0x1 = Slave DMA level sensitive.]
            uint32_t               :10; // bits:22..31 (R)   Reserved
        } b;
        uint32_t reg;
    } CON_reg_t;

    /* Enums for multi-value fields */
    enum e_DVAL : uint32_t
    {
        /*  Selects debounce filter time for card detection
         *  Determines how long signal must be stable before being recognized
         */
        DEBOUNCE_33US   = 0x0,  // 33 μs debounce time
        DEBOUNCE_231US  = 0x1,  // 231 μs debounce time
        DEBOUNCE_1MS    = 0x2,  // 1 ms debounce time
        DEBOUNCE_8_4MS  = 0x3   // 8.4 ms debounce time
    };

    /*(offset = 0x130) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register SD_PWCNT */
            uint32_t    PWRCNT :16; // bits:0..15  (R/W) Power counter register. This register is used to introduce a delay between the PAD
                                    //                  ACTIVE pin assertion and the command issued.
                                    //                  [0x0 = No additional delay added;
                                    //                   0x1 = TCF delay (card clock period)
                                    //                   0x2 = TCF x 2 delay (card clock period)
                                    //                   ...
                                    //                   0xFFFE = TCF x 65534 delay (card clock period)
                                    //                   0xFFFF = TCF x 65535 delay (card clock period)]
            uint32_t           :16; // bits:16..31 (R)   Reserved
        } b;
        uint32_t reg;
    } PWCNT_reg_t;

    /* (offset = 0x200) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                          /* Register SD_SDMASA */

            uint32_t    SDMA_SYSADDR :32; // bits:0..31 (R) This register contains the system memory address for a SDMA transfer. When the Host
                                          //               Controller stops a SDMA transfer, this register shall point to the system address of the
                                          //               next contiguous data position. It can be accessed only if no transaction is executing
                                          //               (i.e., after a transaction has stopped). Read operations during transfers may return an
                                          //               invalid value. The Host Driver shall initialize this register before starting a SDMA
                                          //               transaction. After SDMA has stopped, the next system address of the next contiguous data
                                          //               position can be read from this register. The SDMA transfer waits at the every boundary
                                          //               specified by the Host SDMA Buffer Boundary in the Block Size register. The Host
                                          //               Controller generates DMA Interrupt to request the Host Driver to update this register.
                                          //               The Host Driver sets the next system address of the next data position to this register.
                                          //               When the most upper byte of this register (003h) is written, the Host Controller
                                          //               restarts the SDMA transfer. When restarting SDMA by the Resume command or by setting
                                          //               Continue Request in the Block Gap Control register, the Host Controller shall start at
                                          //               the next contiguous address stored here in the SDMA System Address register. ADMA does
                                          //               not use this register.
        } b;
        uint32_t reg;
    } SDMASA_reg_t;

    /* (offset = 0x204) [reset = 0x0] */
    typedef union
    {                                          /* Block Register
                                                 */
        struct
        {
            uint32_t    BLEN         :12;      // bits 0..11  (R/W) Transfer block size
                                               //                   Specifies block length in bytes for data transfer
            uint32_t                 : 4;      // bits 12..15 (R)   Reserved
            uint32_t    NBLK         :16;      // bits 16..31 (R/W) Number of blocks to transfer
                                               //                   Specifies number of blocks in current transfer
        } b;
        uint32_t reg;
    } BLK_reg_t;

    /* (offset = 0x208) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SD_ARG */
            uint32_t    ARG :32;    // bits:0..31 (R/W) Command argument bits [31:0] .
        } b;
        uint32_t reg;
    } ARG_reg_t;

    /* (offset = 0x20C) [reset = 0x0] */
    typedef union
    {                                          /* Command Register */
        struct
        {
            uint32_t    DE           : 1;       // bit  0      (R/W) DMA Enable.
                                                //                   This bit is used to enable DMA mode for host data access.
                                                //                   [ 0x0 = DMA mode disable;
                                                //                     0x1 = DMA mode enable ]
            uint32_t    BCE          : 1;       // bit  1      (R/W) Block Count Enable (Multiple block transfers only).
                                                //                   This bit is used to enable the block count register (SD_BLK
                                                //                   [31:16] NBLK bits).
                                                //                   When Block Count is disabled (SD_CMD[1] BCE bit is cleared to 0)
                                                //                   in Multiple block transfers (SD_CMD[5] MSBS bits is set to 1), the
                                                //                   module can perform infinite transfer.
                                                //                   [0x0 = Block count disabled for infinite transfer.
                                                //                    0x1 = Block count enabled for multiple block transfer with known number of blocks]
            uint32_t    ACEN         : 1;       // bit  2      (R/W) Auto CMD12 Enable (SD cards only).
                                                //                  When this bit is set to 1, the host controller issues a CMD12
                                                //                  automatically after the transfer completion of the last block.
                                                //                  The Host Driver shall not set this bit to issue commands that do not
                                                //                  require CMD12 to stop data transfer.
                                                //                  In particular, secure commands do not require CMD12.
                                                //                  For CE-ATA commands (SD_CON[12] CEATA bit set to 1), auto
                                                //                  CMD12 is useless
                                                //                  therefore when this bit is set the mechanism to detect command
                                                //                  completion signal, named CCS, interrupt is activated.
                                                //                  [0x0 = Auto CMD12 disable
                                                //                   0x1 = Auto CMD12 enable or CCS detection enabled.]
            uint32_t                 : 1;       // bit  3      (R)   Reserved
            uint32_t    DDIR         : 1;       // bit  4      (R/W) Data transfer Direction.
                                                //                   Select This bit defines either data transfer will be a read or a write.
                                                //                   [0x0 = Data Write (host to card)
                                                //                    0x1 = Data Read (card to host)]
            uint32_t    MSBS         : 1;       // bit  5      (R/W) Multi/Single block select.
                                                //                   This bit must be set to 1 for data transfer in case of multi block
                                                //                   command.
                                                //                   For any others command this bit shall be cleared to 0.
                                                //                   [0x0 = Single block. If this bit is 0, it is not necessary to set the register
                                                //                           SD_BLK[31:16] NBLK bits.
                                                //                    0x1 = Multi block. When Block Count is disabled (SD_CMD[1] BCE bit
                                                //                          is cleared to 0) in Multiple block transfers (SD_CMD[5] MSBS bit is
                                                //                          set to 1), the module can perform infinite transfer.]
            uint32_t                 :10;       // bits 6..15  (R)   Reserved
            uint32_t    RSP_TYPE     : 2;       // bits 16,17  (R/W) Response type.
                                                //                   This bits defines the response type of the command.[see e_RSP_TYPE]
            uint32_t                 : 1;       // bit  18     (R)   Reserved
            uint32_t    CCCE         : 1;       // bit  19     (R/W) Command CRC check enable.
                                                //                   This bit must be set to 1 to enable CRC7 check on command
                                                //                   response to protect the response against transmission errors on the
                                                //                   bus.
                                                //                   If an error is detected, it is reported as a command CRC error
                                                //                   (SD_STAT[17] CCRC bit set to 1).
                                                //                   NoteThe CCCE bit cannot be configured for an Auto CMD12, and
                                                //                   then CRC check is automatically checked when this command is
                                                //                   issued.
                                                //                   [0x0 = CRC7 check disable
                                                //                    0x1 = CRC7 check enable]
            uint32_t    CICE         : 1;       // bit  20     (R/W) Command Index check enable.
                                                //                    This bit must be set to 1 to enable index check on command
                                                //                    response to compare the index field in the response against the
                                                //                    index of the command.
                                                //                    If the index is not the same in the response as in the command, it is
                                                //                    reported as a command index error (SD_STAT[19] CIE bit set to1)
                                                //                    NoteThe CICE bit cannot be configured for an Auto CMD12, then
                                                //                    index check is automatically checked when this command is issued.
                                                //                    [0x0 = Index check disable
                                                //                     0x1 = Index check enable]
            uint32_t    DP           : 1;       // bit  21     (R/W) Data present select.
                                                //                   This register indicates that data is present and mmc_dat line shall be
                                                //                   used.
                                                //                   It must be cleared to 0 in the following conditions: Command using
                                                //                   only mmc_cmd line.
                                                //                   Command with no data transfer but using busy signal on mmc_dat0.
                                                //                   Resume command.
                                                //                   [0x0 = Command with no data transfer
                                                //                    0x1 = Command with data transfer]
            uint32_t    CMD_TYPE     : 2;       // bits 22,23  (R/W) Command type.
                                                //                      This register specifies three types of special commands: Suspend,
                                                //                      Resume and Abort.
                                                //                      These bits shall be cleared to 0b00 for all other commands. [see e_CMD_TYPE]
            uint32_t    INDX         : 6;       // bits 24..29 (R/W) Command index [see e_CMD_INDEX]
            uint32_t                 : 2;       // bits 30,31  (R)   Reserved
        } b;
        uint32_t reg;
    } CMD_reg_t;
        
    enum e_RSP_TYPE : uint32_t
    {
        /*  Defines expected response format from card
         *  Determines how controller interprets response from MMC/SD card
         */
        RSP_NONE       = 0x0,  // No response expected
        RSP_136BIT     = 0x1,  // 136-bit response expected
        RSP_48BIT      = 0x2,  // 48-bit response expected
        RSP_48BIT_BUSY = 0x3   // 48-bit response witbusy signal
    };

    enum e_CMD_TYPE : uint32_t
    {
        /*  Defines command type for current operation
         *  Determines special handling for different command categories
         */
        CMD_NORMAL     = 0x0,  // Normal command (no special handling)
        CMD_SUSPEND    = 0x1,  // Suspend command (pause current operation)
        CMD_RESUME     = 0x2,  // Resume command (continue paused operation)
        CMD_ABORT      = 0x3   // Abort command (terminate current operation)
    };

    enum e_CMD_INDEX : uint32_t
    {
        CMD0_ACMD0   = 0x0,
        CMD1_ACMD1   = 0x1,
        CMD2_ACMD2   = 0x2,
        CMD3_ACMD3   = 0x3,
        CMD4_ACMD4   = 0x4,
        CMD5_ACMD5   = 0x5,
        CMD6_ACMD6   = 0x6,
        CMD7_ACMD7   = 0x7,
        CMD8_ACMD8   = 0x8,
        CMD9_ACMD9   = 0x9,
        CMD10_ACMD10 = 0xA,
        CMD11_ACMD11 = 0xB,
        CMD12_ACMD12 = 0xC,
        CMD13_ACMD13 = 0xD,
        CMD14_ACMD14 = 0xE,
        CMD15_ACMD15 = 0xF,
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
        CMD50_ACMD5  = 0x32,
        CMD51_ACMD5  = 0x33,
        CMD52_ACMD5  = 0x34,
        CMD53_ACMD5  = 0x35
    };

    /* (offset = 0x210) [reset = 0x0] */
    typedef union
    {                                          /* Response Register 0-1 */
        struct
        {
            uint32_t    RSP0         :16;      // bits 0..15  (R) Command Response bits [15:0]
                                               //                 Contains first 16 bits of card response
            uint32_t    RSP1         :16;      // bits 16..31 (R) Command Response bits [31:16]
                                               //                 Contains next 16 bits of card response
        } b;
        uint32_t reg;
    } RSP10_reg_t;

    /* (offset = 0x214) [reset = 0x0] */
    typedef union
    {                                          /* Response Register 2-3 */
        struct
        {
            uint32_t    RSP2         :16;      // bits 0..15  (R) Command Response bits [47:32]
                                               //                 Contains next 16 bits of card response
            uint32_t    RSP3         :16;      // bits 16..31 (R) Command Response bits [63:48]
                                               //                 Contains next 16 bits of card response
        } b;
        uint32_t reg;
    } RSP32_reg_t;

    /* (offset = 0x218) [reset = 0x0] */
    typedef union
    {                                          /* Response Register 4-5 */
        struct
        {
            uint32_t    RSP4         :16;      // bits 0..15  (R) Command Response bits [79:64]
                                               //                 Contains next 16 bits of card response
            uint32_t    RSP5         :16;      // bits 16..31 (R) Command Response bits [95:80]
                                               //                 Contains next 16 bits of card response
        } b;
        uint32_t reg;
    } RSP54_reg_t;

    /* (offset = 0x21C) [reset = 0x0] */
    typedef union
    {                                          /* Response Register 6-7 */
        struct
        {
            uint32_t    RSP6         :16;      // bits 0..15  (R) Command Response bits [111:96]
                                               //                 Contains next 16 bits of card response
            uint32_t    RSP7         :16;      // bits 16..31 (R) Command Response bits [127:112]
                                               //                 Contains final 16 bits of card response
        } b;
        uint32_t reg;
    } RSP76_reg_t;

    /* (offset = 0x220) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SD_DATA */

            uint32_t    DATA :32;   // bits:0..31 (R/W) Data register [31:0]. In functional mode (SD_CON[4] MODE bit set to the default value
                                    //                 0): A read access to this register is allowed only when the buffer read enable status is
                                    //                 set to 1 (SD_PSTATE[11] BRE bit), otherwise a bad access (SD_STAT[29] BADA bit) is
                                    //                 signaled. A write access to this register is allowed only when the buffer write enable
                                    //                 status is set to 1 (SD_PSTATE[10] BWE bit), otherwise a bad access (SD_STAT[29] BADA
                                    //                 bit) is signaled and the data is not written.
        } b;
        uint32_t reg;
    } DATA_reg_t;

    /* (offset = 0x224) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SD_PSTATE */

            uint32_t    CMDI : 1;   // bit:0       (R) Command inhibit(mmc_cmd). This status bit indicates that the mmc_cmd line is in use.
                                    //                This bit is cleared to 0 when the most significant byte is written into the command
                                    //                register. This bit is not set when Auto CMD12 is transmitted. This bit is cleared to 0
                                    //                in either the following cases: After the end bit of the command response, excepted if
                                    //                there is a command conflict error (SD_STAT[17] CCRC bit or SD_STAT[18] CEB bit set to 1)
                                    //                or a Auto CMD12 is not executed (SD_AC12[0] ACNE bit). After the end bit of the command
                                    //                without response (SD_CMD [17:16] RSP_TYPE bits set to '00'). In case of a
                                    //                command data error is detected (SD_STAT[19] CTO bit set to 10, this register is not
                                    //                automatically cleared.
                                    //                [0x0 = Issuing of command using mmc_cmd line is allowed ;
                                    //                 0x1 = Issuing of command using mmc_cmd line is not allowed
            uint32_t    DATI : 1;   // bit:1       (R) Command inhibit (mmc_dat). This status bit is generated if either mmc_dat line is active
                                    //                (SD_PSTATE[2] DLA bit) or Read transfer is active (SD_PSTATE[9] RTA bit) or when a
                                    //                command with busy is issued. This bit prevents the local host to issue a command. A
                                    //                change of this bit from 1 to 0 generates a transfer complete interrupt (SD_STAT[1] TC
                                    //                bit).
                                    //                [0x0 = Issuing of command using the mmc_dat lines is allowed;
                                    //                 0x1 = Issuing of command  using mmc_dat lines is not allowed
            uint32_t    DLA  : 1;   // bit:2       (R) mmc_dat line active. This status bit indicates whether one of the mmc_dat lines is in
                                    //                use. In the case of read transactions (card to host)This bit is set to 1 after the end
                                    //                bit of read command or by activating continue request SD_HCTL[17] CR bit. This bit is
                                    //                cleared to 0 when the host controller received the end bit of the last data block or at
                                    //                the beginning of the read wait mode. In the case of write transactions (host to
                                    //                card)This bit is set to 1 after the end bit of write command or by activating continue
                                    //                request SD_HCTL[17] CR bit. This bit is cleared to 0 on the end of busy event for the
                                    //                last block. The host controller must wait 8 clock cycles with line not busy to really
                                    //                consider not 'busy state' or after the busy block as a result of a stop at gap
                                    //                request.
                                    //                [0x0 = mmc_dat line inactive;
                                    //                 0x1 = mmc_dat line active]
            uint32_t         : 5;   // bits:3..7   (R) Reserved
            uint32_t    WTA  : 1;   // bit:8       (R) Write transfer active. This status indicates a write transfer active. It is set to 1
                                    //                after the end bit of write command or by activating a continue request (SD_HCTL[17] CR
                                    //                bit) following a stop at block gap request. This bit is cleared to 0 when CRC status has
                                    //                been received after last block or after a stop at block gap request.
                                    //                [0x0 = No valid data on the mmc_dat lines.
                                    //                 0x1 = Write data transfer on going.]
            uint32_t    RTA  : 1;   // bit:9       (R) Read transfer active. This status is used for detecting completion of a read transfer.
                                    //                It is set to 1 after the end bit of read command or by activating a continue request
                                    //                (SD_HCTL[17] CR bit) following a stop at block gap request. This bit is cleared to 0
                                    //                when all data have been read by the local host after last block or after a stop at block
                                    //                gap request.
                                    //                [0x0 = No valid data on the mmc_dat lines.
                                    //                 0x1 = Read data transfer on going.]
            uint32_t    BWE  : 1;   // bit:10      (R) Buffer Write enable. This status is used for non-DMA write transfers. It indicates if
                                    //                space is available for write data.
                                    //                [0x0 = There is no room left in the buffer to write BLEN bytes of data.
                                    //                 0x1 = There is enough space in the buffer to write BLEN bytes of data.]
            uint32_t    BRE  : 1;   // bit:11      (R) Buffer read enable. This bit is used for non-DMA read transfers. It indicates that a
                                    //                complete block specified by SD_BLK [10:0] BLEN bits has been written in the buffer and
                                    //                is ready to be read. It is cleared to 0 when the entire block is read from the buffer.
                                    //                It is set to 1 when a block data is ready in the buffer and generates the Buffer read
                                    //                ready status of interrupt (SD_STAT[5] BRR bit).
                                    //                [0x0 = Read BLEN bytes disable;
                                    //                 0x1 = Read BLEN bytes enable. Readable data exists in the buffer.]
            uint32_t         : 4;   // bits:12..15 (R) Reserved
            uint32_t    CINS : 1;   // bit:16      (R) Card inserted. This bit is the debounced value of the card detect input pin (SDCD). An
                                    //                inactive to active transition of the card detect input pin (SDCD) will generate a card
                                    //                insertion interrupt (SD_STAT[CINS]). A active to inactive transition of the card detect
                                    //                input pin (SDCD) will generate a card removal interrupt (SD_STAT[REM]). This bit is not
                                    //                affected by a software reset.
                                    //                [0x0 = If SD_CON[CDP] is cleared to 0 (default), no card is detected.
                                    //                      The card may have been removed from the card slot. If SD_CON[CDP] is set to 1,
                                    //                      the card has been inserted.
                                    //                 0x1 = If SD_CON[CDP] is cleared to 0 (default), the card has
                                    //                      been inserted from the card slot. If SD_CON[CDP] is set to 1, no card is detected. The
                                    //                      card may have been removed from the card slot.]
            uint32_t    CSS  : 1;   // bit:17      (R) Card State Stable. This bit is used for testing. It is set to 1 only when Card Detect
                                    //                Pin Level is stable (SD_PSTATE[18] CPDL). Debouncing is performed on the card detect
                                    //                input pin (SDCD) to detect card stability. This bit is not affected by software reset.
                                    //                [0x0 = Reset or Debouncing.
                                    //                 0x1 = Reset or Debouncing.]
            uint32_t    CDPL : 1;   // bit:18      (R) Card Detect Pin Level. MMC/SD/SDIO1 only. SDIO cards only. This bit reflects the inverse
                                    //                value of the card detect input pin (SDCD). Debouncing is not performed on this bit and
                                    //                is valid only when Card State is stable. (SD_PSTATE[17] is set to 1). This bit must be
                                    //                debounced by software. The value of this register after reset depends on the card detect
                                    //                input pin (SDCD) level at that time.
                                    //                [0x0 = The value of the card detect input pin (SDCD) is 1.
                                    //                 0x1 = The value of the card detect input pin (SDCD) is 0.]
            uint32_t    WP   : 1;   // bit:19      (R) Write Protect. MMC/SD/SDIO1 only. SDIO cards only. This bit reflects the write protect
                                    //                input pin (SDWP) level. The value of this register after reset depends one the protect
                                    //                input pin (SDWP) level at that time.
                                    //                [0x0 = If SD_CON[8] WPP is cleared to 0 (default),
                                    //                        the card is write protected, otherwise the card is not write protected.;
                                    //                 0x1 = If SD_CON[8] WPP is cleared to 0 (default), the card is not write protected, otherwise the
                                    //                       card is write protected.]
            uint32_t    DLEV : 4;   // bits:20..23 (R) mmc_dat [3:0]
                                    //                  line signal level mmc_dat3 equal to or greater than bit 23.
                                    //                  mmc_dat2 equal to or greater than bit 22.
                                    //                  mmc_dat1 equal to or greater than bit 21.
                                    //                  mmc_dat0 equal to or greater than bit 20.
                                    //                This status is used to check mmc_dat line level to recover from
                                    //                errors, and for debugging. This is especially useful in detecting the busy signal level
                                    //                from mmc_dat0 . The value of these registers after reset depends on the mmc_dat lines
                                    //                level at that time.
            uint32_t    CLEV : 1;   // bit:24      (R) mmc_cmd line signal level. This status is used to check the mmc_cmd line level to
                                    //                recover from errors, and for debugging. The value of this register after reset depends
                                    //                on the mmc_cmd line level at that time.
                                    //                [0x0 = The mmc_cmd line level is 0.
                                    //                 0x1 = The mmc_cmd line level is 1.]
            uint32_t         : 7;   // bits:25..31 (R) Reserved
        } b;
        uint32_t reg;
    } PSTATE_reg_t;

    /* (offset = 0x228) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                                     /* Register SD_HCTL */

            uint32_t         : 1; // bit:0       (R)   Reserved
            uint32_t    DTW  : 1; // bit:1       (R/W) Data transfer width. This bit must be set following a valid SET_BUS_WIDTH command
                                  //                  (ACMD6) with the value written in bit 1 of the argument. Prior to this command, the SD
                                  //                  card configuration register (SCR) must be verified for the supported bus width by the SD
                                  //                  card.
                                  //                  [0x0 = 1-bit Data width (mmc_dat0 used);
                                  //                   0x1 = 4-bit Data width (mmc_dat[3:0] used)]
            uint32_t    HSPE : 1; // bit:2       (R/W) High Speed Enable. Before setting this bit, the Host Driver shall check the High Speed
                                  //                  Support in the Capabilities register. If this bit is cleared to 0 (default), the Host
                                  //                  Controller outputs CMD line and DAT lines at the falling edge of the SD Clock. If this
                                  //                  bit is set to 1, the Host Controller outputs CMD line and DAT lines at the rising edge
                                  //                  of the SD Clock. This bit shall not be set when dual data rate mode is activated in
                                  //                  SD_CON[DDR].
                                  //                  [0x0 = Normal speed mode;
                                  //                   0x1 = High speed mode]
            uint32_t    DMAS : 2; // bits:3..4   (R/W) DMA Select. One of the supported DMA modes can be selected. The host driver shall check
                                  //                  support of DMA modes by referencing the Capabilities register. Use of selected DMA is
                                  //                  determined by DMA Enable of the Transfer Mode register. This register is only meaningful
                                  //                  when MADMA_EN is set to 1. When MADMA_EN is cleared to 0 the bit field is read only and
                                  //                  returned value is 0. [see e_DMAS]
            uint32_t         : 1; // bit:5       (R)   Reserved
            uint32_t    CDTL : 1; // bit:6       (R/W) Card Detect Test Level. This bit is enabled while the Card Detect Signal Selection is
                                  //                  set to 1 and it indicates card inserted or not.
                                  //                  [0x0 = No card;
                                  //                   0x1 = Card inserted.]
            uint32_t    CDSS : 1; // bit:7       (R/W) Card Detect Signal Selection. This bit selects source for the card detection. When the
                                  //                  source for the card detection is switched, the interrupt should be disabled during the
                                  //                  switching period by clearing the Interrupt Status/Signal Enable register in order to
                                  //                  mask unexpected interrupt being caused by the glitch. The Interrupt Status/Signal Enable
                                  //                  should be disabled during over the period of debouncing.
                                  //                  [0x0 = SDCD# is selected (for normal use).
                                  //                   0x1 = The Card Detect Test Level is selected (for test purposes)]
            uint32_t    SDBP : 1; // bit:8       (R/W) SD bus power. Before setting this bit, the host driver shall select the SD bus voltage
                                  //                  (SD_HCTL [11:9] SDVS bits). If the host controller detects the No card state, this bit
                                  //                  is automatically cleared to 0. If the module is power off, a write in the command
                                  //                  register (SD_CMD) will not start the transfer. A write to this bit has no effect if the
                                  //                  selected SD bus voltage is not supported according to capability register
                                  //                  (SD_CAPA[VS*]).
                                  //                  [0x0 = Power off;
                                  //                   0x1 = Power on]
            uint32_t    SDVS : 3; // bits:9..11  (R/W) SD bus voltage select (All cards). The host driver should set these bits to select the
                                  //                  voltage level for the card according to the voltage supported by the system (SD_CAPA[26]
                                  //                  VS18 bit, SD_CAPA[25] VS30 bit, SD_CAPA[24] VS33 bit) before starting a transfer. If
                                  //                  MMCHS 2: This field must be set to 5h. If MMCHS 3: This field must be set to 5h. [see e_SDVS]
            uint32_t         : 4; // bits:12..15 (R)   Reserved
            uint32_t    SBGR : 1; // bit:16      (R/W) Stop at block gap request. This bit is used to stop executing a transaction at the next
                                  //                  block gap. The transfer can restart with a continue request (SD_HCTL[17] CR bit) or
                                  //                  during a suspend/resume sequence. In case of read transfer, the card must support read
                                  //                  wait control. In case of write transfer, the host driver shall set this bit after all
                                  //                  block data written. Until the transfer completion (SD_STAT[1] TC bit set to 1), the host
                                  //                  driver shall leave this bit set to 1.If this bit is set, the local host shall not write
                                  //                  to the data register (SD_DATA).
                                  //                  [0x0 = Transfer mode;
                                  //                   0x1 = Stop at block gap]
            uint32_t    CR   : 1; // bit:17      (R/W) Continue request. This bit is used to restart a transaction that was stopped by
                                  //                  requesting a stop at block gap (SD_HCTL[16] SBGR bit). Set this bit to 1 restarts the
                                  //                  transfer. The bit is automatically cleared to 0 by the host controller when transfer has
                                  //                  restarted, that is, mmc_dat line is active (SD_PSTATE[2] DLA bit) or transferring data
                                  //                  (SD_PSTATE[8] WTA bit). The Stop at block gap request must be disabled (SD_HCTL[16] SBGR
                                  //                  bit =0) before setting this bit.
                                  //                  [0x0 = No affect;
                                  //                   0x1 = Transfer restart]
            uint32_t    RWC  : 1; // bit:18      (R/W) Read wait control. The read wait function is optional only for SDIO cards. If the card
                                  //                  supports read wait, this bit must be enabled, then requesting a stop at block gap
                                  //                  (SD_HCTL[16] SBGR bit) generates a read wait period after the current end of block. Be
                                  //                  careful, if read wait is not supported it may cause a conflict on mmc_dat line.
                                  //                  [0x0 = Disable read wait control. Suspend/resume cannot be supported.
                                  //                   0x1 = Enable read wait control]
            uint32_t    IBG  : 1; // bit:19      (R/W) Interrupt block at gap. This bit is valid only in 4-bit mode of SDIO card to enable
                                  //                  interrupt detection in the interrupt cycle at block gap for a multiple block transfer.
                                  //                  For MMC cards and for SD card this bit should be cleared to 0.
                                  //                  [0x0 = Disable interrupt detection at the block gap in 4-bit mode;
                                  //                   0x1 = Enable interrupt detection at the block gap in 4-bit mode]
            uint32_t         : 4; // bits:20..23 (R)   Reserved
            uint32_t    IWE  : 1; // bit:24      (R/W) Wake-up event enable on SD card interrupt. This bit enables wake-up events for card
                                  //                  interrupt assertion. Wake-up is generated if the wake-up feature is enabled
                                  //                  (SD_SYSCONFIG[2] ENAWAKEUP bit) and enable status bit is set (SD_IE[8] CIRQ_ENABLE bit).
                                  //                   [0x0 = Disable wake-up on card interrupt;
                                  //                    0x1 = Enable wake-up on card interrupt]
            uint32_t    INS  : 1; // bit:25      (R/W) Wake-up event enable on SD card insertion This bit enables wake- up events for card
                                  //                  insertion assertion. Wake-up is generated if the wake-up feature is enabled
                                  //                  (SD_SYSCONFIG[2] ENAWAKEUP bit).
                                  //                  [0x0 = Disable wake-up on card insertion;
                                  //                  0x1 = Enable wake-up on card insertion]
            uint32_t    REM  : 1; // bit:26      (R/W) Wake-up event enable on SD card removal. This bit enables wake-up events for card
                                  //                  removal assertion. Wake-up is generated if the wake-up feature is enabled
                                  //                  (SD_SYSCONFIG[2] ENAWAKEUP bit).
                                  //                  [0x0 = Disable wake-up on card removal;
                                  //                  0x1 = Enable wake-up on card removal]
            uint32_t    OBWE : 1; // bit:27      (R/W) Wake-up event enable for 'out-of-band' interrupt. This bit enables wake-up
                                  //                  events for 'out-of-band' assertion. Wake-up is generated if the wake-up
                                  //                  feature is enabled (SD_SYSCONFIG[2] ENAWAKEUP bit). The write to this register is
                                  //                  ignored when SD_CON[14] OBIE bit is not set.
                                  //                  [0x0 = Disable wake-up on 'out-of-band' Interrupt
                                  //                   0x1 = Enable wake-up on 'out-of-band' Interrupt
            uint32_t         : 4; // bits:28..31 (R)   Reserved
        } b;
        uint32_t reg;
    } HCTL_reg_t;

    enum e_DMAS : uint32_t
    {
        /*  Selects DMA mode for data transfers
         *  Determines which DMA engine is used for data movement
         */
        DMA_RESERVED0  = 0x0,  // Reserved value 0
        DMA_RESERVED1  = 0x1,  // Reserved value 1
        DMA_ADMA2_32   = 0x2,  // 32-bit Address ADMA2
        DMA_RESERVED3  = 0x3   // Reserved value 3
    };

    enum e_SDVS : uint32_t
    {
        /*  Selects operating voltage for SD bus
         *  Must match card's supported voltage range
         */
        VS_1_8V        = 0x5,  // 1.8V (Typical)
        VS_3_0V        = 0x6,  // 3.0V (Typical)
        VS_3_3V        = 0x7   // 3.3V (Typical)
    };

    /* (offset = 0x22C) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                                    /* Register SD_SYSCTL */

            uint32_t    ICE  : 1;   // bit:0       (R/W) Internal clock enable. This register controls the internal clock activity. In very low
                                    //                  power state, the internal clock is stopped. NoteThe activity of the debounce clock (used
                                    //                  for wake-up events) and the interface clock (used for reads and writes to the module
                                    //                  register map) are not affected by this register.
                                    //                  [0x0 = The internal clock is stopped (very low power state).
                                    //                   0x1 = The internal clock oscillates and can be automatically
                                    //                         gated when SD_SYSCONFIG[0] AUTOIDLE bit is set to 1 (default value)]
            uint32_t    ICS  : 1;   // bit:1       (R)   Internal clock stable (status)This bit indicates either the internal clock is stable or
                                    //                  not.
                                    //                  [0x0 = The internal clock is not stable;
                                    //                   0x1 = The internal clock is stable after enabling the clock (SD_SYSCTL[0] ICE bit) or after changing the clock ratio
                                    //                  (SD_SYSCTL[15:6] CLKD bits)]
            uint32_t    CEN  : 1;   // bit:2       (R/W) Clock enable. This bit controls if the clock is provided to the card or not.
                                    //                  [0x0 = The clock is not provided to the card . Clock frequency can be changed .
                                    //                   0x1 = The clock is provided to the card and can be automatically gated when SD_SYSCONFIG[0] AUTOIDLE bit is
                                    //                  set to 1 (default value). The host driver shall wait to set this bit to 1 until the
                                    //                  Internal clock is stable (SD_SYSCTL[1] ICS bit)]
            uint32_t         : 3;   // bits:3..5   (R)   Reserved
            uint32_t    CLKD :10;   // bits:6..15  (R/W) Clock frequency select. These bits define the ratio between a reference clock frequency
                                    //                  (system dependant) and the output clock frequency on the mmc_clk pin of either the
                                    //                  memory card (MMC, SD, or SDIO).
                                    //                  [0x0 = Clock Ref bypass;
                                    //                   0x1 = Clock Ref bypass
                                    //                   0x2 = Clock Ref/2;
                                    //                   0x3 = Clock Ref/3;
                                    //                   ....
                                    //                   0x3FF = Clock Ref / 1023]
            uint32_t    DTO  : 4;   // bits:16..19 (R/W) Data timeout counter value and busy timeout. This value determines the interval by which
                                    //                  mmc_dat lines timeouts are detected. The host driver needs to set this bit field based
                                    //                  on: The maximum read access time (NAC) (Refer to the SD Specification Part1 Physical
                                    //                  Layer). The data read access time values (TAAC and NSAC) in the card specific data
                                    //                  register (CSD) of the card. The timeout clock base frequency (SD_CAPA [5:0] TCF bits).
                                    //                  If the card does not respond within the specified number of cycles, a data timeout error
                                    //                  occurs (SD_STAT[20] DTO bit). The SD_SYSCTL[19,16] DTO bit field is also used to check
                                    //                  busy duration, to generate busy timeout for commands with busy response or for busy
                                    //                  programming during a write command. Timeout on CRC status is generated if no CRC token
                                    //                  is present after a block write. [see e_DTO]
            uint32_t         : 4;   // bits:20..23 (R)   Reserved
            uint32_t    SRA  : 1;   // bit:24      (R/W) Software reset for all. This bit is set to 1 for reset , and released to 0 when
                                    //                  completed. This reset affects the entire host controller except for the card detection
                                    //                  circuit and capabilities registers.
                                    //                  [0x0 = Reset completed;
                                    //                   0x1 = Software reset for all the design;
            uint32_t    SRC  : 1;   // bit:25      (R/W) Software reset for mmc_cmd line. This bit is set to 1 for reset and released to 0 when
                                    //                  completed. Due to additional implementation logic, the reset does not immediately start
                                    //                  when asserted. The proper procedure is: (a) Set to 1 to start reset, (b) Poll for 1 to
                                    //                  identify start of reset, and (c) Poll for 0 to identify reset is complete. mmc_cmd
                                    //                  finite state machine in both clock domain are also reset. These registers are cleared by
                                    //                  the SD_SYSCTL[25] SRC bit: SD_PSTATECMDI. SD_STATCC Interconnect and MMC command status
                                    //                  management is reinitialized. Note: If a soft reset is issued when an interrupt is
                                    //                  asserted, data may be lost.
                                    //                  [0x0 = Reset completed;
                                    //                   0x1 = Software reset for mmc_cmd line]
            uint32_t    SRD  : 1;   // bit:26      (R/W) Software reset for mmc_dat line. This bit is set to 1 for reset and released to 0 when
                                    //                  completed. Due to additional implementation logic, the reset does not immediately start
                                    //                  when asserted. The proper procedure is: (a) Set to 1 to start reset, (b) Poll for 1 to
                                    //                  identify start of reset, and (c) Poll for 0 to identify reset is complete. mmc_dat
                                    //                  finite state machine in both clock domain are also reset. These registers are cleared by
                                    //                  the SD_SYSCTL[26] SRD bit: SD_DATA. SD_PSTATEBRE, BWE, RTA, WTA, DLA and DATI.
                                    //                  SD_HCTLSBGR and CR. SD_STATBRR, BWR, BGE and TC Interconnect and MMC buffer data
                                    //                  management is reinitialized. Note: If a soft reset is issued when an interrupt is
                                    //                  asserted, data may be lost.
                                    //                  [0x0 = Reset completed;
                                    //                   0x1 = Software reset for mmc_dat line]
            uint32_t         : 5;   // bits:27..31 (R)   Reserved
        } b;
        uint32_t reg;
    } SYSCTL_reg_t;

    enum e_DTO : uint32_t
    {
        /*  Selects data timeout period
         *  Determines how long controller waits for response before timing out
         */
        DTO_2_13       = 0x0,  // TCF x 2^13 (8.192 ms at 100 kHz)
        DTO_2_14       = 0x1,  // TCF x 2^14 (16.384 ms at 100 kHz)
        DTO_2_15       = 0x2,  // TCF x 2^15 (32.768 ms at 100 kHz)
        DTO_2_16       = 0x3,  // TCF x 2^16 (65.536 ms at 100 kHz)
        DTO_2_17       = 0x4,  // TCF x 2^17 (131.072 ms at 100 kHz)
        DTO_2_18       = 0x5,  // TCF x 2^18 (262.144 ms at 100 kHz)
        DTO_2_19       = 0x6,  // TCF x 2^19 (524.288 ms at 100 kHz)
        DTO_2_20       = 0x7,  // TCF x 2^20 (1.048576 s at 100 kHz)
        DTO_2_21       = 0x8,  // TCF x 2^21 (2.097152 s at 100 kHz)
        DTO_2_22       = 0x9,  // TCF x 2^22 (4.194304 s at 100 kHz)
        DTO_2_23       = 0xA,  // TCF x 2^23 (8.388608 s at 100 kHz)
        DTO_2_24       = 0xB,  // TCF x 2^24 (16.777216 s at 100 kHz)
        DTO_2_25       = 0xC,  // TCF x 2^25 (33.554432 s at 100 kHz)
        DTO_2_26       = 0xD,  // TCF x 2^26 (67.108864 s at 100 kHz)
        DTO_2_27       = 0xE   // TCF x 2^27 (134.217728 s at 100 kHz)
    };

    /* (offset = 0x230) [reset = 0x0] */
    typedef union
    {
        struct
        {
            uint32_t    CC    :1;   // bit  0      (R/W) Command complete.
                                    //                   This bit is set when a
                                    //                   1-to-0 transition occurs in the register command inhibit
                                    //                   (SD_PSTATE[0] CMDI bit)
                                    //                   [0x0 (W) = Status bit unchanged;
                                    //                    0x0 (R) = No command complete;
                                    //                    0x1 (W) = Status is cleared;
                                    //                    0x1 (R) = Command complete;]
            uint32_t    TC    :1;   // bit  1      (R/W) Transfer completed.
                                    //                    This bit is always set when a read/write transfer is completed or
                                    //                    between two blocks when the transfer is stopped due to a stop at
                                    //                    block gap request (SD_HCTL[16] SBGR bit).
                                    //                    [0x0 (W) = Status bit unchanged
                                    //                     0x0 (R) = No transfer complete
                                    //                     0x1 (W) = Status is cleared
                                    //                     0x1 (R) = Data transfer complete]
            uint32_t    BGE   :1;   // bit  2      (R/W) Block gap event.
                                    //                   When a stop at block gap is requested (SD_HCTL[16] SBGR bit),
                                    //                   this bit is automatically set when transaction is stopped at the block
                                    //                   gap during a read or write operation.
                                    //                   [0x0 (W) = Status bit unchanged;
                                    //                    0x0 (R) = No block gap event;
                                    //                    0x1 (W) = Status is cleared;
                                    //                    0x1 (R) = Transaction stopped at block gap]
            uint32_t    DMA   :1;   // bit  3      (R/W) DMA Interrupt.
                                    //                   This status is set when an interrupt is required in the ADMA
                                    //                   instruction and after the data transfer completion.
                                    //                   [0x0 (W) = Status bit unchanged;
                                    //                    0x0 (R) = DMA Interrupt detected;
                                    //                    0x1 (W) = Status is cleared;
                                    //                    0x1 (R) = No DMA Interrupt]
            uint32_t    BWR   :1;   // bit  4      (R/W) Buffer write ready.
                                    //                   This bit is set automatically during a write operation to the card (see
                                    //                   class
                                    //                   4 - block oriented write command) when the host can write a
                                    //                   complete block as specified by SD_BLK
                                    //                   [10:0] BLEN.
                                    //                   It indicates that the memory card has emptied one block from the
                                    //                   buffer and that the local host is able to write one block of data into
                                    //                   the buffer.
                                    //                   Note: If the DMA transmit mode is enabled, this bit is never set
                                    //                   instead, a DMA transmit request to the main DMA controller of the
                                    //                   system is generated.
                                    //                   [0x0 (W) = Status bit unchanged;
                                    //                    0x0 (R) = Not ready to write buffer;
                                    //                    0x1 (W) = Status is cleared;
                                    //                    0x1 (R) = Ready to write buffer]
            uint32_t    BRR   : 1;  // bit:5       (R/W) Buffer read ready. This bit is set automatically during a read operation to the card
                                    //                  (see class 2 - block oriented read commands) when one block specified by the SD_BLK
                                    //                  [10:0] BLEN bit field is completely written in the buffer. It indicates that the memory
                                    //                  card has filled out the buffer and that the local host needs to empty the buffer by
                                    //                  reading it. Note: If the DMA receive-mode is enabled, this bit is never set instead a
                                    //                  DMA receive request to the main DMA controller of the system is generated.
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = Not ready to read buffer
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Ready to read buffer]
            uint32_t    CINS  : 1;  // bit:6       (R/W) Card Insertion. This bit is set automatically when SD_PSTATE[CINS] changes from 0 to 1.
                                    //                  A clear of this bit doesn&apos;t affect Card inserted present state (SD_PSTATE[CINS]).
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = Card State stable or debouncing
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Card inserted]
            uint32_t    CREM  : 1;  // bit:7       (R/W) Card Removal. This bit is set automatically when SD_PSTATE[CINS] changes from 1 to 0. A
                                    //                  clear of this bit doesn&apos;t affect Card inserted present state (SD_PSTATE[CINS]).
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = Card State stable or debouncing
                                    //                   0x1 (W) = Status is cleared
                                    //                   0x1 (R) = Card Removed]
            uint32_t    CIRQ  : 1;  // bit:8       (R)   Card interrupt. This bit is only used for SD and SDIO cards. In 1-bit mode, interrupt
                                    //                  source is asynchronous (can be a source of asynchronous wake-up). In 4-bit mode,
                                    //                  interrupt source is sampled during the interrupt cycle. In CE-ATA mode, interrupt source
                                    //                  is detected when the card drives mmc_cmd line to zero during one cycle after data
                                    //                  transmission end. All modes above are fully exclusive. The controller interrupt must be
                                    //                  clear by setting SD_IE[8] CIRQ_ENABLE to 0, then the host driver must start the
                                    //                  interrupt service with card (clearing card interrupt status) to remove card interrupt
                                    //                  source. Otherwise the Controller interrupt will be reasserted as soon as SD_IE[8]
                                    //                  CIRQ_ENABLE is set to 1. Writes to this bit are ignored.
                                    //                  [0x0 (R) = No card interrupt;
                                    //                   0x1 (R) = Generate card interrupt]
            uint32_t    OBI   : 1;  // bit:9       (R)   Out-of-band interrupt (This interrupt is only useful for MMC card). This bit is set
                                    //                  automatically when SD_CON[14] OBIE bit is set and an out-of-band interrupt occurs on OBI
                                    //                  pin. The interrupt detection depends on polarity controlled by SD_CON[13] OBIP bit. The
                                    //                  out-of-band interrupt signal is a system specific feature for future use, this signal is
                                    //                  not required for existing specification implementation.
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = No out-of-band interrupt
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Interrupt out-of-band occurs]
            uint32_t    BSR   : 1;  // bit:10      (R/W) Boot Status Received Interrupt. This bit is set automatically when SD_CON[BOOT] is set 1
                                    //                  or 2 and a boot status is received on DAT[0] line. This interrupt is only useful for MMC
                                    //                  card.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No interrupt;
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Boot Status Received Interrupt occurred.]
            uint32_t          : 4;  // bits:11..14 (R)   Reserved
            uint32_t    ERRI  : 1;  // bit:15      (R)   Error interrupt. If any of the bits in the Error Interrupt Status register (SD_STAT
                                    //                  [31:16]) are set, then this bit is set to 1. Therefore the host driver can efficiently
                                    //                  test for an error by checking this bit first. Writes to this bit are ignored.
                                    //                  [0x0 (R) = No interrupt;
                                    //                   0x1 (R) = Error interrupt event(s) occurred]
            uint32_t    CTO   : 1;  // bit:16      (R/W) Command timeout error. This bit is set automatically when no response is received within
                                    //                  64 clock cycles from the end bit of the command. For commands that reply within 5 clock
                                    //                  cycles - the timeout is still detected at 64 clock cycles.
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = No error
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Time Out]
            uint32_t    CCRC  : 1;  // bit:17      (R/W) Command CRC error. This bit is set automatically when there is a CRC7 error in the
                                    //                  command response depending on the enable bit (SD_CMD[19] CCCE).
                                    //                  [0x0 (W) = Status bit unchanged
                                    //                   0x0 (R) = No error
                                    //                   0x1 (W) = Status is cleared.
                                    //                   0x1 (R) = Command CRC error]
            uint32_t    CEB   : 1;  // bit:18      (R/W) Command end bit error. This bit is set automatically when detecting a 0 at the end bit
                                    //                  position of a command response.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Command end bit error]
            uint32_t    CIE   : 1;  // bit:19      (R/W) Command index error. This bit is set automatically when response index differs from
                                    //                  corresponding command index previously emitted. It depends on the enable bit (SD_CMD[20]
                                    //                  CICE).
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Command index error]
            uint32_t    DTO   : 1;  // bit:20      (R/W) Data timeout error. This bit is set automatically according to the following conditions:
                                    //                  Busy timeout for R1b, R5b response type. Busy timeout after write CRC status. Write CRC
                                    //                  status timeout. Read data timeout.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Time out]
            uint32_t    DCRC  : 1;  // bit:21      (R/W) Data CRC Error. This bit is set automatically when there is a CRC16 error in the data
                                    //                  phase response following a block read command or if there is a 3-bit CRC status
                                    //                  different of a position '010' token during a block write command.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Data CRC error]
            uint32_t    DEB   : 1;  // bit:22      (R/W) Data End Bit error. This bit is set automatically when detecting a 0 at the end bit
                                    //                  position of read data on mmc_dat line or at the end position of the CRC status in write
                                    //                  mode.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Data end bit error]
            uint32_t          : 1;  // bit:23      (R)   Reserved
            uint32_t    ACE   : 1;  // bit:24      (R/W) Auto CMD12 error. This bit is set automatically when one of the bits in Auto CMD12 Error
                                    //                  status register has changed from 0 to 1.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = AutoCMD12 error]
            uint32_t    ADMAE : 1;  // bit:25      (R/W) ADMA Error. This bit is set when the Host Controller detects errors during ADMA based
                                    //                  data transfer. The state of the ADMA at an error occurrence is saved in the ADMA Error
                                    //                  Status Register. In addition, the Host Controller generates this interrupt when it
                                    //                  detects invalid descriptor data (Valid=0) at the ST_FDS state. ADMA Error State in the
                                    //                  ADMA Error Status indicates that an error occurs in ST_FDS state. The Host Driver may
                                    //                  find that Valid bit is not set at the error descriptor.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No interrupt;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = ADMA error]
            uint32_t          : 2;  // bits:26..27 (R)   Reserved
            uint32_t    CERR  : 1;  // bit:28      (R/W) Card error. This bit is set automatically when there is at least one error in a response
                                    //                  of type R1, R1b, R6, R5 or R5b. Only bits referenced as type E (error) in status field
                                    //                  in the response can set a card status error. An error bit in the response is flagged
                                    //                  only if corresponding bit in card status response error SD_CSRE in set. There is no card
                                    //                  error detection for autoCMD12 command. The host driver shall read SD_RSP76 register to
                                    //                  detect error bits in the command response.
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No error;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Card error]
            uint32_t    BADA  : 1;  // bit:29      (R/W) Bad access to data space. This bit is set automatically to indicate a bad access to
                                    //                  buffer when not allowed: During a read access to the data register (SD_DATA) while
                                    //                  buffer reads are not allowed (SD_PSTATE[11] BRE bit =0). During a write access to the
                                    //                  data register (SD_DATA) while buffer writes are not allowed (SD_PSTATE[10] BWE bit=0).
                                    //                  [0x0 (W) = Status bit unchanged;
                                    //                   0x0 (R) = No interrupt;
                                    //                   0x1 (W) = Status is cleared;
                                    //                   0x1 (R) = Bad access]
            uint32_t          : 2;  // bits:30..31 (R)   Reserved
        } b;
        uint32_t reg;
    } STAT_reg_t;
        
    /* (offset = 0x234) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                            /* Register SD_IE */

            uint32_t    CC_ENABLE   : 1;    // bit:0       (R/W) Command completed interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    TC_ENABLE   : 1;    // bit:1       (R/W) Transfer completed interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BGE_ENABLE  : 1;    // bit:2       (R/W) Block gap event interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DMA_ENABLE  : 1;    // bit:3       (R/W) DMA interrupt enable [0x0 = Masked; 0x1 = Enable]
            uint32_t    BWR_ENABLE  : 1;    // bit:4       (R/W) Buffer write ready interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BRR_ENABLE  : 1;    // bit:5       (R/W) Buffer read ready interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CINS_ENABLE : 1;    // bit:6       (R/W) Card Insertion interrupt Enable This bit must be set to 1 when entering in smart idle
                                            //                  mode to enable system to identity wake-up event and to allow controller to clear
                                            //                  internal wake-up source. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CREM_ENABLE : 1;    // bit:7       (R/W) Card Removal interrupt Enable This bit must be set to 1 when entering in smart idle mode
                                            //                  to enable system to identity wake-up event and to allow controller to clear internal
                                            //                  wake-up source.[0x0 = Masked; 0x1 = Enabled]
            uint32_t    CIRQ_ENABLE : 1;    // bit:8       (R/W) Card interrupt enable. A clear of this bit also clears the corresponding status bit.
                                            //                  During 1-bit mode, if the interrupt routine does not remove the source of a card
                                            //                  interrupt in the SDIO card, the status bit is reasserted when this bit is set to 1. This
                                            //                  bit must be set to 1 when entering in smart idle mode to enable system to identity
                                            //                  wake-up event and to allow controller to clear internal wake-up source.
                                            //                  [0x0 = Masked 0x1 = Enabled]
            uint32_t    OBI_ENABLE  : 1;    // bit:9       (R/W) Out-of-band interrupt enable A write to this register when SD_CON[14] OBIE is cleared to
                                            //                  0 is ignored. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BSR_ENABLE  : 1;    // bit:10      (R/W) Boot Status Interrupt Enable A write to this register when SD_CON[BOOT] is cleared to 0
                                            //                  is ignored. [0x0 = Masked; 0x1 = Enabled]
            uint32_t                : 4;    // bits:11..14 (R)   Reserved
            uint32_t    NULL_        : 1;    // bit:15      (R)   Fixed to 0. The host driver shall control error interrupts using the Error Interrupt
                                            //                  Signal Enable register. Writes to this bit are ignored.
            uint32_t    CTO_ENABLE  : 1;    // bit:16      (R/W) Command timeout error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CCRC_ENABLE : 1;    // bit:17      (R/W) Command CRC error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CEB_ENABLE  : 1;    // bit:18      (R/W) Command end bit error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CIE_ENABLE  : 1;    // bit:19      (R/W) Command index error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DTO_ENABLE  : 1;    // bit:20      (R/W) Data timeout error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DCRC_ENABLE : 1;    // bit:21      (R/W) Data CRC error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DEB_ENABLE  : 1;    // bit:22      (R/W) Data end bit error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t                : 1;    // bit:23      (R)   Reserved
            uint32_t    ACE_ENABLE  : 1;    // bit:24      (R/W) Auto CMD12 error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    ADMA_ENABLE : 1;    // bit:25      (R/W) ADMA error Interrupt Enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t                : 2;    // bits:26..27 (R)   Reserved
            uint32_t    CERR_ENABLE : 1;    // bit:28      (R/W) Card error interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BADA_ENABLE : 1;    // bit:29      (R/W) Bad access to data space interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t                : 2;    // bits:30..31 (R)   Reserved
        } b;
        uint32_t reg;
    } IE_reg_t;

    /* (offset = 0x238) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                            /* Register SD_ISE */

            uint32_t    CC_SIGEN   : 1;     // bit:0       (R/W) Command completed signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    TC_SIGEN   : 1;     // bit:1       (R/W) Transfer completed signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BGE_SIGEN  : 1;     // bit:2       (R/W) Block gap event signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DMA_SIGEN  : 1;     // bit:3       (R/W) DMA signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BWR_SIGEN  : 1;     // bit:4       (R/W) Buffer write ready signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BRR_SIGEN  : 1;     // bit:5       (R/W) Buffer read ready signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CINS_SIGEN : 1;     // bit:6       (R/W) Card Insertion signal status enable. This bit must be set to 1 when entering in smart
                                            //                  idle mode to enable system to identity wake-up event and to allow controller to clear
                                            //                  internal wake-up source. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CREM_SIGEN : 1;     // bit:7       (R/W) Card Removal signal status enable This bit must be set to 1 when entering in smart idle
                                            //                  mode to enable system to identity wake-up event and to allow controller to clear
                                            //                  internal wake-up source. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CIRQ_SIGEN : 1;     // bit:8       (R/W) Card interrupt signal status enable. A clear of this bit also clears the corresponding
                                            //                  status bit. During 1-bit mode, if the interrupt routine does not remove the source of a
                                            //                  card interrupt in the SDIO card, the status bit is reasserted when this bit is set to 1.
                                            //                  This bit must be set to 1 when entering in smart idle mode to enable system to identity
                                            //                  wake-up event and to allow controller to clear internal wake-up source. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    OBI_SIGEN  : 1;     // bit:9       (R/W) Out-of-band interrupt signal status enable. A write to this register when SD_CON[14]
                                            //                  OBIE is cleared to 0 is ignored. [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BSR_SIGEN  : 1;     // bit:10      (R/W) Boot Status signal status enable. A write to this register when SD_CON[BOOT] is cleared
                                            //                  to 0 is ignored [0x0 = Masked; 0x1 = Enabled]
            uint32_t               : 4;     // bits:11..14 (R)   Reserved
            uint32_t    NULL_      : 1;     // bit:15      (R)   Fixed to 0. The host driver shall control error interrupts using the error interrupt
                                            //                  signal enable register. Writes to this bit are ignored.
            uint32_t    CTO_SIGEN  : 1;     // bit:16      (R/W) Command timeout error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CCRC_SIGEN : 1;     // bit:17      (R/W) Command CRC error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CEB_SIGEN  : 1;     // bit:18      (R/W) Command end bit error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    CIE_SIGEN  : 1;     // bit:19      (R/W) Command index error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DTO_SIGEN  : 1;     // bit:20      (R/W) Data timeout error signal status enable
                                            //                   [0x0 = Masked. The host controller provides the clock to the card until the card sends the data or the transfer is aborted.
                                            //                    0x1 = Enabled]
            uint32_t    DCRC_SIGEN : 1;     // bit:21      (R/W) Data CRC error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    DEB_SIGEN  : 1;     // bit:22      (R/W) Data end bit error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t               : 1;     // bit:23      (R)   Reserved
            uint32_t    ACE_SIGEN  : 1;     // bit:24      (R/W) Auto CMD12 error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    ADMA_SIGEN : 1;     // bit:25      (R/W) ADMA error signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t               : 2;     // bits:26..27 (R)   Reserved
            uint32_t    CERR_SIGEN : 1;     // bit:28      (R/W) Card error interrupt signal status enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t    BADA_SIGEN : 1;     // bit:29      (R/W) Bad access to data space interrupt enable [0x0 = Masked; 0x1 = Enabled]
            uint32_t               : 2;     // bits:30..31 (R)   Reserved
        } b;
        uint32_t reg;
    } ISE_reg_t;

    /* (offset = 0x23C) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register SD_AC12 */

            uint32_t    ACNE : 1; // bit:0      (R) Auto CMD12 not executed. This bit is set to 1 if multiple block data transfer command
                                  //               has started and if an error occurs in command before auto CMD12 starts.
                                  //               [0x0 = Auto CMD12 executed; 0x1 = Auto CMD12 not executed]
            uint32_t    ACTO : 1; // bit:1      (R) Auto CMD12 timeout error. This bit is set to 1 if no response is received within 64
                                  //               clock cycles from the end bit of the auto CMD12 command. [0x0 = No error; 0x1 = Auto CMD12 time out]
            uint32_t    ACCE : 1; // bit:2      (R) Auto CMD12 CRC error. This bit is automatically set to 1 when a CRC7 error is detected
                                  //               in the auto CMD12 command response depending on the enable in the SD_CMD[19] CCCE bit.
                                  //               [0x0 = No error; 0x1 = Auto CMD12 CRC error]
            uint32_t    ACEB : 1; // bit:3      (R) Auto CMD12 end bit error. This bit is set to 1 when detecting a 0 at the end bit
                                  //               position of auto CMD12 command response. [0x0 = No error; 0x1 = AutoCMD12 end bit error]
            uint32_t    ACIE : 1; // bit:4      (R) Auto CMD12 index error. This bit is a set to 1 when response index differs from
                                  //               corresponding command auto CMD12 index previously emitted. This bit depends on the
                                  //               command index check enable (SD_CMD[20] CICE bit). [0x0 = No error; 0x1 = Auto CMD12 index error]
            uint32_t         : 2; // bits:5..6  (R) Reserved
            uint32_t    CNI  : 1; // bit:7      (R) Command not issue by auto CMD12 error. If this bit is set to 1, it means that pending
                                  //               command is not executed due to auto CMD12 error ACEB, ACCE, ACTO, or ACNE.
                                  //               [0x0 = Not error; 0x1 = Command not issued]
            uint32_t         :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } AC12_reg_t;

    /* (offset = 0x240) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                       /* Register SD_CAPA */

            uint32_t    TCF       : 6; // bits:0..5   (R)   Timeout clock frequency. The timeout clock frequency is used to detect Data Timeout
                                       //                  Error (SD_STAT[20] DTO bit). [0x0 = The timeout clock frequency depends on the frequency
                                       //                  of the clock provided to the card. The value of the timeout clock frequency is not
                                       //                  available in this register.
            uint32_t              : 1; // bit:6       (R)   Reserved
            uint32_t    TCU       : 1; // bit:7       (R)   Timeout clock unit. This bit shows the unit of base clock frequency used to detect Data
                                       //                  Timeout Error (SD_STAT[20] DTO bit). [0x0 = kHz; 0x1 = MHz]
            uint32_t    BCF       : 6; // bits:8..13  (R)   Base clock frequency for clock provided to the card. ARRAY(0x1bfe1b0)
            uint32_t              : 2; // bits:14..15 (R)   Reserved
            uint32_t    MBL       : 2; // bits:16..17 (R)   Maximum block length. This value indicates the maximum block size that the host driver
                                       //                  can read and write to the buffer in the host controller. The host controller supports
                                       //                  512 bytes and 1024 bytes block transfers. [see e_MBL]
            uint32_t              : 1; // bit:18      (R)   Reserved
            uint32_t    AD2S      : 1; // bit:19      (R)   This bit indicates whether the Host Controller is capable of using ADMA2. It depends on
                                       //                  setting of generic parameter MADMA_EN. [0x0 = ADMA2 supported; 0x1 = ADMA2 not supported]
            uint32_t              : 1; // bit:20      (R)   Reserved
            uint32_t    HSS       : 1; // bit:21      (R)   High-speed support. This bit indicates that the host controller supports high speed
                                       //                  operations and can supply an up-to-52 MHz clock to the card.
                                       //                  [0x0 = DMA not supported; 0x1 = DMA supported]
            uint32_t    DS        : 1; // bit:22      (R)   DMA support. This bit indicates that the Host controller is able to use DMA to transfer
                                       //                  data between system memory and the Host controller directly.
                                       //                  [0x0 = DMA not supported;
                                       //                   0x1 = DMA supported]
            uint32_t    SRS       : 1; // bit:23      (R)   Suspend/resume support (SDIO cards only). This bit indicates whether the host controller
                                       //                  supports Suspend/Resume functionality.
                                       //                  [0x0 = The Host controller does not suspend/resume functionality.
                                       //                   0x1 = The Host controller supports suspend/resume functionality.]
            uint32_t    VS33      : 1; // bit:24      (R/W) Voltage support 3.3V. Initialization of this register (via a write access to this
                                       //                  register) depends on the system capabilities. The host driver shall not modify this
                                       //                  register after the initialization. This register is only reinitialized by a hard reset
                                       //                  (via mmc_RESET signal).
                                       //                  [0x0 (W) = 3.3 V not supported;
                                       //                   0x0 (R) = 3.3 V not supported;
                                       //                   0x1 (W) = 3.3 V supported;
                                       //                   0x1 (R) = 3.3 V supported]
            uint32_t    VS30      : 1; // bit:25      (R/W) Voltage support 3.0V. Initialization of this register (via a write access to this
                                       //                  register) depends on the system capabilities. The host driver shall not modify this
                                       //                  register after the initialization. This register is only reinitialized by a hard reset
                                       //                  (via mmc_RESET signal).
                                       //                  [0x0 (W) = 3.0 V not supported;
                                       //                   0x0 (R) = 3.0 V not supported;
                                       //                   0x1 (W) = 3.0 V supported;
                                       //                   0x1 (R) = 3.0 V supported]
            uint32_t    VS18      : 1; // bit:26      (R/W) Voltage support 1.8 V. Initialization of this register (via a write access to this
                                       //                  register) depends on the system capabilities. The host driver shall not modify this
                                       //                  register after the initialization. This register is only reinitialized by a hard reset
                                       //                  (via mmc_RESET signal).
                                       //                  [0x0 (W) = 1.8 V not supported;
                                       //                   0x0 (R) = 1.8 V not supported;
                                       //                   0x1 (W) = 1.8 V supported;
                                       //                   0x1 (R) = 1.8 V supported]
            uint32_t              : 1; // bit:27      (R)   Reserved
            uint32_t    BUS_64BIT : 1; // bit:28      (R/W) 64 Bit System Bus Support. Setting 1 to this bit indicates that the Host Controller
                                       //                  supports 64-bit address descriptor mode and is connected to 64-bit address system bus.
                                       //                  [0x0 (R) = 32-bit System bus address;
                                       //                   0x1 (R) = 64-bit System bus address]
            uint32_t              : 3; // bits:29..31 (R)   Reserved
        } b;
        uint32_t reg;
    } CAPA_reg_t;

    enum e_MBL : uint32_t
    {
        /*  Maximum block length supported by controller
         *  Determines largest single block that can be transferred
         */
        MBL_512        = 0x0,  // 512 bytes maximum block length
        MBL_1024       = 0x1,  // 1024 bytes maximum block length
        MBL_2048       = 0x2   // 2048 bytes maximum block length
    };

    /* (offset = 0x248) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_CUR_CAPA */

            uint32_t    CUR_3V3 : 8;    // bits:0..7   (R/W) Maximum current for 3.3 V
                                        //                   0x0 (R) = The maximum current capability for this voltage is not available. Feature not implemented.
            uint32_t    CUR_3V0 : 8;    // bits:8..15  (R/W) Maximum current for 3.0 V
                                        //                   0x0 (R) = The maximum current capability for this voltage is not available. Feature not implemented.
            uint32_t    CUR_1V8 : 8;    // bits:16..23 (R/W) Maximum current for 1.8 V
                                        //                   0x0 (R) = The maximum current capability for this voltage is not available. Feature not implemented.
            uint32_t            : 8;    // bits:24..31 (R)   Reserved
        } b;
        uint32_t reg;
    } CUR_CAPA_reg_t;

    /* (offset = 0x250) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_FE */

            uint32_t    FE_ACNE  : 1;   // bit:0       (W) Force Event Auto CMD12 not executed.
                                        //                 [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_ACTO  : 1;   // bit:1       (W) Force Event Auto CMD12 timeout error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_ACCE  : 1;   // bit:2       (W) Force Event Auto CMD12 CRC error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_ACEB  : 1;   // bit:3       (W) Force Event Auto CMD12 end bit error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_ACIE  : 1;   // bit:4       (W) Force Event Auto CMD12 index error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t             : 2;   // bits:5..6   (R) Reserved
            uint32_t    FE_CNI   : 1;   // bit:7       (W) Force Event Command not issue by Auto CMD12 error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t             : 8;   // bits:8..15  (R) Reserved
            uint32_t    FE_CTO   : 1;   // bit:16      (W) Force Event Command Timeout error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_CCRC  : 1;   // bit:17      (W) Force Event Comemand CRC error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_CEB   : 1;   // bit:18      (W) Force Event Command end bit error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_CIE   : 1;   // bit:19      (W) Force Event Command index error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_DTO   : 1;   // bit:20      (W) Force Event Data timeout error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_DCRC  : 1;   // bit:21      (W) Force Event Data CRC error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_DEB   : 1;   // bit:22      (W) Force Event Data End Bit error. [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t             : 1;   // bit:23      (R) Reserved
            uint32_t    FE_ACE   : 1;   // bit:24      (W) Force Event Auto CMD12 error. [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_ADMAE : 1;   // bit:25      (W) Force Event ADMA error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t             : 2;   // bits:26..27 (R) Reserved
            uint32_t    FE_CERR  : 1;   // bit:28      (W) Force Event Card error [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t    FE_BADA  : 1;   // bit:29      (W) Force Event Bad access to data space. [0x0 = No effect, no interrupt; 0x1 = Interrupt forced]
            uint32_t             : 2;   // bits:30..31 (R) Reserved
        } b;
        uint32_t reg;
    } FE_reg_t;

    /* (offset = 0x254) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SD_ADMAES */

            uint32_t    AES : 2;    // bits:0..1  (R/W) ADMA Error State. This field indicates the state of ADMA when an error occurred during
                                    //                 an ADMA data transfer. This field never indicates '10' because ADMA never
                                    //                 stops in this state. [see e_AES]
            uint32_t    LME : 1;    // bit:2      (W)   ADMA Length Mismatch Error: While Block Count Enable is being set, the total data length
                                    //                 specified by the Descriptor table is different from that specified by the Block Count
                                    //                 and Block Length. Total data length cannot be divided by the block length.
                                    //                 [0x0 = No error; 0x1 = Error]
            uint32_t        :29;    // bits:3..31 (R)   Reserved
        } b;
        uint32_t reg;
    } ADMAES_reg_t;

    enum e_AES : uint32_t
    {
        /*  ADMA Error State indicates current state when error occurred
         *  Helps diagnose ADMA transfer failures
         */
        AES_ST_STOP    = 0x0,  // Stop DMA (error occurred in idle state)
        AES_ST_FDS     = 0x1,  // Fetch Descriptor (error during descriptor fetch)
        AES_RESERVED   = 0x2,  // Never set (reserved value)
        AES_ST_TFR     = 0x3   // Transfer Data (error during data transfer)
    };

    /* (offset = 0x258) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_ADMASAL */

            uint32_t    ADMA_A32B :32;  // bits:0..31 (R/W) The ADMA increments this register address, which points to the next line, whenever
                                        //                 fetching a Descriptor line. When the ADMA Error Interrupt is generated, this register
                                        //                 holds the valid Descriptor address depending on the ADMA state. The Host Driver shall
                                        //                 program the Descriptor Table on a 32-bit boundary and set the 32-bit boundary address to
                                        //                 this register. ADMA2 ignores the lower 2 bits of this register and assumes it to be 00b.
        } b;
        uint32_t reg;
    } ADMASAL_reg_t;

    /* (offset = 0x25C) [reset = 0x0] */
    typedef union
    {
        struct
        {
                                        /* Register SD_ADMASAH */

            uint32_t    ADMA_A32B :32;  // bits:0..31 (R/W) ADMA_A32B.
        } b;
        uint32_t reg;
    } ADMASAH_reg_t;

    /* (offset = 0x2FC) [reset = 0x31010000] */
    typedef union
    {                                          /* Revision Register
                                                 */
        struct
        {
            uint32_t    SIS          : 1;       // bit  0      (R) Slot Interrupt Status.
                                                //                 This status bit indicates the inverted state of interrupt signal for the
                                                //                 module. By a power on reset or by setting a software reset for all
                                                //                 (SD_SYSCTL[24] SRA), the interrupt signal shall be deasserted and
                                                //                 this status shall read 0.
            uint32_t                 :15;       // bits 1..15  (R) Reserved
            uint32_t    SREV         : 8;       // bits 16..23 (R) SSpecification Version Number.
                                                //                  This status indicates the Standard SD Host Controller Specification Version.
                                                //                  The upper and lower 4 bits indicate the version.
                                                //                  Reset value is 0x01. 0x0h = SD Host Specification Version 1.0
            uint32_t    VREV         : 8;       // bits 24..31 (R) Vendor Version Number.
                                                //                Bits 7 to 4 are the major revision, bits 3 to 0 are the minor revision.
                                                //                Examples: 0x10 for 1.0 and 0x21 for 2.1.
                                                //                Reset value is 0x31.
        } b;
        uint32_t reg;
    } REV_reg_t;

    struct AM335x_MMCHS_Type
    {
        __R    uint32_t         RESERVED[68];
        __RW   SYSCONFIG_reg_t  SYSCONFIG;        // (0x110)
        __R    SYSSTATUS_reg_t  SYSSTATUS;        // (0x114)
        __R    uint32_t         RESERVED1[3];
        __RW   CSRE_reg_t       CSRE;             // (0x124)
        __RW   SYSTEST_reg_t    SYSTEST;          // (0x128)
        __RW   CON_reg_t        CON;              // (0x12C)
        __RW   PWCNT_reg_t      PWCNT;            // (0x130)
        __R    uint32_t         RESERVED2[51];
        __R    SDMASA_reg_t     SDMASA;           // (0x200)
        __RW   BLK_reg_t        BLK;              // (0x204)
        __RW   ARG_reg_t        ARG;              // (0x208)
        __RW   CMD_reg_t        CMD;              // (0x20C)
        __R    RSP10_reg_t      RSP10;            // (0x210)
        __R    RSP32_reg_t      RSP32;            // (0x214)
        __R    RSP54_reg_t      RSP54;            // (0x218)
        __R    RSP76_reg_t      RSP76;            // (0x21C)
        __RW   DATA_reg_t       DATA;             // (0x220)
        __RW    PSTATE_reg_t    PSTATE;           // (0x224)
        __RW   HCTL_reg_t       HCTL;             // (0x228)
        __RW   SYSCTL_reg_t     SYSCTL;           // (0x22C)
        __RW   STAT_reg_t       STAT;             // (0x230)
        __RW   IE_reg_t         IE;               // (0x234)
        __RW   ISE_reg_t        ISE;              // (0x238)
        __R    AC12_reg_t       AC12;             // (0x23C)
        __RW   CAPA_reg_t       CAPA;             // (0x240)
        __R    uint32_t         RESERVED3[1];
        __RW   CUR_CAPA_reg_t   CUR_CAPA;         // (0x248)
        __R    uint32_t         RESERVED4[1];
        __W    FE_reg_t         FE;               // (0x250)
        __RW   ADMAES_reg_t     ADMAES;           // (0x254)
        __RW   ADMASAL_reg_t    ADMASAL;          // (0x258)
        __RW   ADMASAH_reg_t    ADMASAH;          // (0x25C)
        __R    uint32_t         RESERVED5[39];
        __R    REV_reg_t        REV;              // (0x2FC)

        uint32_t RSP(const uint8_t n)  { return (0x210 + (n * 4)); }
    };

    constexpr uint32_t AM335x_MMCHS_0_BASE = 0x48060000;
    constexpr uint32_t AM335x_MMCHS_1_BASE = 0x481D8000;
    constexpr uint32_t AM335x_MMCHS_2_BASE = 0x47810000;

    inline AM335x_MMCHS_Type * AM335x_MMCHS_0 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_0_BASE);
    inline AM335x_MMCHS_Type * AM335x_MMCHS_1 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_1_BASE);
    inline AM335x_MMCHS_Type * AM335x_MMCHS_2 = reinterpret_cast<AM335x_MMCHS_Type *>(AM335x_MMCHS_2_BASE);

    enum e_BUS_WIDTH : uint32_t
    {
        WIDTH_1BIT  = 0x1,
        WIDTH_4BIT  = 0x4,
        WIDTH_8BIT  = 0x8
    };

} // namespace MMCHS

#endif //__MMCHS_HPP
