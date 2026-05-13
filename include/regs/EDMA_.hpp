#ifndef __EDMA__HPP
#define __EDMA__HPP

#include <stdint.h>
#include "REGS.hpp"

namespace REGS::EDMA
{

    /* (offset = 0x7F4) [reset state = 0x4516141] */
    typedef union
    {
        struct
        {
                                     /* Register PID */

            uint32_t    usb_pid :16; // bits:0..15  (R) USB Product ID
            uint32_t    usb_vid :16; // bits:16..31 (R) USB Vendor ID
        } b;
        uint32_t reg;
    } PID_reg_t;


    /* (offset = 0x4) [reset state = 0x3224445] */
    typedef union
    {
        struct
        {
                                         /* Register CCCFG */

            uint32_t    NUM_DMACH   : 3; // bits:0..2   (R) Number of DMA channels. 0h = Reserved from 0h to 4h. 4h = Reserved. 5h = 64 DMA
                                         //                channels. 6h = Reserved. 7h = Reserved.
            uint32_t                : 1; // bit:3       (R) Reserved
            uint32_t    NUM_QDMACH  : 3; // bits:4..6   (R) Number of QDMA channels. 0h = Reserved from 0h to 3h. 3h = Reserved. 4h = 8 QDMA
                                         //                channels. 5h = Reserved from 5h to 7. 7h = Reserved.
            uint32_t                : 1; // bit:7       (R) Reserved
            uint32_t    NUM_INTCH   : 3; // bits:8..10  (R) Number of interrupt channels. 0h = Reserved from 0h to 3h. 3h = Reserved. 4h = 64
                                         //                interrupt channels. 5h = Reserved from 5h to 7. 7h = Reserved.
            uint32_t                : 1; // bit:11      (R) Reserved
            uint32_t    NUM_PAENTRY : 3; // bits:12..14 (R) Number of PaRAM sets. 0h = Reserved from 0h to 3h. 3h = Reserved 4h = 256 PaRAM sets. 5h
                                         //                = Reserved from 5h to 7h. 7h = Reserved.
            uint32_t                : 1; // bit:15      (R) Reserved
            uint32_t    NUM_EVQUE   : 3; // bits:16..18 (R) Number of queues/number of TCs. 0h = Reserved. 1h = Reserved. 2h = 3 EDMA3TCs/Event
                                         //                Queues 3h = Reserved from 3h to 7h. 7h = Reserved.
            uint32_t                : 1; // bit:19      (R) Reserved
            uint32_t    NUM_REGN    : 2; // bits:20..21 (R) Number of MP and shadow regions. 0h = Reserved. 1h = Reserved 2h = 4 regions. 3h =
                                         //                Reserved.
            uint32_t                : 2; // bits:22..23 (R) Reserved
            uint32_t    CHMAP_EXIST : 1; // bit:24      (R) Channel mapping existence. 0h = Reserved. 1h = Channel mapping logic included.
            uint32_t    MP_EXIST    : 1; // bit:25      (R) Memory protection existence. 0h = Reserved. 1h = Memory protection logic included.
            uint32_t                : 6; // bits:26..31 (R) Reserved
        } b;
        uint32_t reg;
    } CCCFG_reg_t;


    /* (offset = 0x10) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                       /* Register SYSCONFIG */

            uint32_t    Autoidle  : 1; // bit:0      (R/W) Internal OCP clock gating strategy 0h = clkfree : OCP clock is free running 1h =
                                       //                 autoClkGate : Automatic OCP clock gating strategy is applied, bnased on the OCP
                                       //                 interface activity
            uint32_t    SoftReset : 1; // bit:1      (R/W) Software reset. Set this bit to trigger a module reset. The bit is automatically reset
                                       //                 by the hardware. During reads, it always returns 0. 0h(Read) = always_Always returns 0
                                       //                 1h(Read) = never_never happens
            uint32_t              :30; // bits:2..31 (R)   Reserved
        } b;
        uint32_t reg;
    } SYSCONFIG_reg_t;


    /* (offset = 0x260) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QDMAQNUM */

            uint32_t    E0 : 3; // bits:0..2   (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:3       (R)   Reserved
            uint32_t    E1 : 3; // bits:4..6   (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:7       (R)   Reserved
            uint32_t    E2 : 3; // bits:8..10  (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:11      (R)   Reserved
            uint32_t    E3 : 3; // bits:12..14 (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:15      (R)   Reserved
            uint32_t    E4 : 3; // bits:16..18 (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:19      (R)   Reserved
            uint32_t    E5 : 3; // bits:20..22 (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:23      (R)   Reserved
            uint32_t    E6 : 3; // bits:24..26 (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:27      (R)   Reserved
            uint32_t    E7 : 3; // bits:28..30 (R/W) QDMA queue number. Contains the event queue number to be used for the corresponding QDMA
                                //                  channel. 0h = Event n is queued on Q0. 1h = Event n is queued on Q1. 2h = Event n is
                                //                  queued on Q2. 3h = Reserved, from 3h to 7h. Always write 0 to this bit; writes of 1 to
                                //                  this bit are not supported and attempts to do so may result in undefined behavior. 7h =
                                //                  Reserved. Always write 0 to this bit; writes of 1 to this bit are not supported and
                                //                  attempts to do so may result in undefined behavior.
            uint32_t       : 1; // bit:31      (R)   Reserved
        } b;
        uint32_t reg;
    } QDMAQNUM_reg_t;


    /* (offset = 0x284) [reset state = 0x777] */
    typedef union
    {
        struct
        {
                                   /* Register QUEPRI */

            uint32_t    PRIQ0 : 3; // bits:0..2   (R/W) Priority level for queue 0. Dictates the priority level used by TC0 relative to other
                                   //                  masters in the device. A value of 0 means highest priority and a value of 7 means lowest
                                   //                  priority.
            uint32_t          : 1; // bit:3       (R)   Reserved
            uint32_t    PRIQ1 : 3; // bits:4..6   (R/W) Priority level for queue 1. Dictates the priority level used by TC1 relative to other
                                   //                  masters in the device. A value of 0 means highest priority and a value of 7 means lowest
                                   //                  priority.
            uint32_t          : 1; // bit:7       (R)   Reserved
            uint32_t    PRIQ2 : 3; // bits:8..10  (R/W) Priority level for queue 2. Dictates the priority level used by TC2 relative to other
                                   //                  masters in the device. A value of 0 means highest priority and a value of 7 means lowest
                                   //                  priority.
            uint32_t          :21; // bits:11..31 (R)   Reserved
        } b;
        uint32_t reg;
    } QUEPRI_reg_t;


    /* (offset = 0x300) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EMR */

            uint32_t    En :32; // bits:0..31 (R) Channel 0 to 31 event missed. En is cleared by writing a 1 to the corresponding bit in
                                //               the event missed clear register (EMCR). 0h = No missed event. 1h = Missed event
                                //               occurred.
        } b;
        uint32_t reg;
    } EMR_reg_t;


    /* (offset = 0x304) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EMRH */

            uint32_t    En :32; // bits:0..31 (R) Channel 32 to 63 event missed. En is cleared by writing a 1 to the corresponding bit in
                                //               the event missed clear register high (EMCRH). 0h = No missed event. 1h = Missed event
                                //               occurred.
        } b;
        uint32_t reg;
    } EMRH_reg_t;


    /* (offset = 0x308) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EMCR */

            uint32_t    En :32; // bits:0..31 (W) Event missed 0 to 31 clear. All error bits must be cleared before additional error
                                //               interrupts will be asserted by the EDMA3CC. 0h = No effect. 1h = Corresponding missed
                                //               event bit in the event missed register (EMR) is cleared (En = 0).
        } b;
        uint32_t reg;
    } EMCR_reg_t;


    /* (offset = 0x30C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EMCRH */

            uint32_t    En :32; // bits:0..31 (W) Event missed 32 to 63 clear. All error bits must be cleared before additional error
                                //               interrupts will be asserted by the EDMA3CC. 0h = No effect. 1h = Corresponding missed
                                //               event bit in the event missed register high (EMRH) is cleared (En = 0).
        } b;
        uint32_t reg;
    } EMCRH_reg_t;


    /* (offset = 0x310) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QEMR */

            uint32_t    En : 8; // bits:0..7  (R) Channel 0 to 7 QDMA event missed. En is cleared by writing a 1 to the corresponding bit
                                //               in the QDMA event missed clear register (QEMCR). 0h = No missed event. 1h = Missed event
                                //               occurred.
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QEMR_reg_t;


    /* (offset = 0x314) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QEMCR */

            uint32_t    En : 8; // bits:0..7  (W) QDMA event missed clear. All error bits must be cleared before additional error
                                //               interrupts will be asserted by the EDMA3CC. 0h = No effect. 1h = Corresponding missed
                                //               event bit in the QDMA event missed register (QEMR) is cleared (En= 0).
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QEMCR_reg_t;


    /* (offset = 0x318) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                      /* Register CCERR */

            uint32_t    QTHRXCD0 : 1; // bit:0       (R) Queue threshold error for queue 0. QTHRXCD0 is cleared by writing a 1 to the
                                      //                corresponding bit in the EDMA3CC error clear register (CCERRCLR). 0h =
                                      //                Watermark/threshold has not been exceeded. 1h = Watermark/threshold has been exceeded.
            uint32_t    QTHRXCD1 : 1; // bit:1       (R) Queue threshold error for queue 1 . QTHRXCD1 is cleared by writing a 1 to the
                                      //                corresponding bit in the EDMA3CC error clear register (CCERRCLR). 0h =
                                      //                Watermark/threshold has not been exceeded. 1h = Watermark/threshold has been exceeded.
            uint32_t    QTHRXCD2 : 1; // bit:2       (R) Queue threshold error for queue 2. QTHRXCD2 is cleared by writing a 1 to the
                                      //                corresponding bit in the EDMA3CC error clear register (CCERRCLR). 0h =
                                      //                Watermark/threshold has not been exceeded. 1h = Watermark/threshold has been exceeded.
            uint32_t             :13; // bits:3..15  (R) Reserved
            uint32_t    TCCERR   : 1; // bit:16      (R) Transfer completion code error. TCCERR is cleared by writing a 1 to the corresponding
                                      //                bit in the EDMA3CC error clear register (CCERRCLR). 0h = Total number of allowed TCCs
                                      //                outstanding has not been reached. 1h = Total number of allowed TCCs has been reached.
            uint32_t             :15; // bits:17..31 (R) Reserved
        } b;
        uint32_t reg;
    } CCERR_reg_t;


    /* (offset = 0x31C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                      /* Register CCERRCLR */

            uint32_t    QTHRXCD0 : 1; // bit:0       (W) Queue threshold error clear for queue 0. 0h = No effect. 1h = Clears the QTHRXCD0 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 0 (QSTAT0).
            uint32_t    QTHRXCD1 : 1; // bit:1       (W) Queue threshold error clear for queue 1. 0h = No effect. 1h = Clears the QTHRXCD1 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 1 (QSTAT1).
            uint32_t    QTHRXCD2 : 1; // bit:2       (W) Queue threshold error clear for queue 2. 0h = No effect. 1h = Clears the QTHRXCD2 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 2 (QSTAT2).
            uint32_t             :13; // bits:3..15  (R) Reserved
            uint32_t    TCCERR   : 1; // bit:16      (W) Transfer completion code error clear. 0h = No effect. 1h = Clears the TCCERR bit in the
                                      //                EDMA3CC error register (CCERR).
            uint32_t             :15; // bits:17..31 (R) Reserved
        } b;
        uint32_t reg;
    } CCERRCLR_reg_t;


    /* (offset = 0x320) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register EEVAL */

            uint32_t    EVAL : 1; // bit:0      (W) Error interrupt evaluate. 0h = No effect. 1h = Write 1 to clear interrupts when all
                                  //               error registers have been cleared. EDMA3CC error interrupt will remain if any errors
                                  //               have not been cleared in any of the error registers (EMR/EMRH, CCERR, QEMR)
            uint32_t         :31; // bits:1..31 (R) Reserved
        } b;
        uint32_t reg;
    } EEVAL_reg_t;


    /* (offset = 0x340) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE0 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 0. 0h = Accesses via region 0 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 0.
                                //                 1h = Accesses via region 0 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 0.
        } b;
        uint32_t reg;
    } DRAE0_reg_t;


    /* (offset = 0x344) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH0 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 0. 0h = Accesses via region 0
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 0. 1h = Accesses via region 0 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 0.
        } b;
        uint32_t reg;
    } DRAEH0_reg_t;


    /* (offset = 0x348) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE1 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 1. 0h = Accesses via region 1 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 1.
                                //                 1h = Accesses via region 1 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 1.
        } b;
        uint32_t reg;
    } DRAE1_reg_t;


    /* (offset = 0x34C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH1 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 1. 0h = Accesses via region 1
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 1. 1h = Accesses via region 1 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 1.
        } b;
        uint32_t reg;
    } DRAEH1_reg_t;


    /* (offset = 0x350) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE2 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 2. 0h = Accesses via region 2 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 2.
                                //                 1h = Accesses via region 2 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 2.
        } b;
        uint32_t reg;
    } DRAE2_reg_t;


    /* (offset = 0x354) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH2 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 2. 0h = Accesses via region 2
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 2. 1h = Accesses via region 2 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 2.
        } b;
        uint32_t reg;
    } DRAEH2_reg_t;


    /* (offset = 0x358) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE3 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 3. 0h = Accesses via region 3 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 3.
                                //                 1h = Accesses via region 3 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 3.
        } b;
        uint32_t reg;
    } DRAE3_reg_t;


    /* (offset = 0x35C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH3 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 3. 0h = Accesses via region 3
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 3. 1h = Accesses via region 3 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 3.
        } b;
        uint32_t reg;
    } DRAEH3_reg_t;


    /* (offset = 0x360) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE4 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 4. 0h = Accesses via region 4 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 4.
                                //                 1h = Accesses via region 4 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 4.
        } b;
        uint32_t reg;
    } DRAE4_reg_t;


    /* (offset = 0x364) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH4 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 4. 0h = Accesses via region 4
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 4. 1h = Accesses via region 4 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 4.
        } b;
        uint32_t reg;
    } DRAEH4_reg_t;


    /* (offset = 0x368) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE5 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 5. 0h = Accesses via region 5 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 5.
                                //                 1h = Accesses via region 5 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 5.
        } b;
        uint32_t reg;
    } DRAE5_reg_t;


    /* (offset = 0x36C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH5 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 5. 0h = Accesses via region 5
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 5. 1h = Accesses via region 5 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 5.
        } b;
        uint32_t reg;
    } DRAEH5_reg_t;


    /* (offset = 0x370) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE6 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 6. 0h = Accesses via region 6 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 6.
                                //                 1h = Accesses via region 6 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 6.
        } b;
        uint32_t reg;
    } DRAE6_reg_t;


    /* (offset = 0x374) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH6 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 6. 0h = Accesses via region 6
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 6. 1h = Accesses via region 6 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 6.
        } b;
        uint32_t reg;
    } DRAEH6_reg_t;


    /* (offset = 0x378) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAE7 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 31 to 0 in region 7. 0h = Accesses via region 7 address
                                //                 space to bit 31 to 0 in any DMA channel register are not allowed. Reads return 0 on bit
                                //                 n and writes do not modify the state of bit 31 to 0. Enabled interrupt bits for bit n do
                                //                 not contribute to the generation of a transfer completion interrupt for shadow region 7.
                                //                 1h = Accesses via region 7 address space to bit 31 to 0 in any DMA channel register are
                                //                 allowed. Reads return the value from bit n and writes modify the state of bit 31 to 0.
                                //                 Enabled interrupt bits for bit n contribute to the generation of a transfer completion
                                //                 interrupt for shadow region 7.
        } b;
        uint32_t reg;
    } DRAE7_reg_t;


    /* (offset = 0x37C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register DRAEH7 */

            uint32_t    En :32; // bits:0..31 (R/W) DMA region access enable for bit 63 to 32 in region 7. 0h = Accesses via region 7
                                //                 address space to bit 63 to 32 in any DMA channel register are not allowed. Reads return
                                //                 0 on bit n and writes do not modify the state of bit 63 to 32. Enabled interrupt bits
                                //                 for bit 31 to 0 do not contribute to the generation of a transfer completion interrupt
                                //                 for shadow region 7. 1h = Accesses via region 7 address space to bit 63 to 32 in any DMA
                                //                 channel register are allowed. Reads return the value from bit n and writes modify the
                                //                 state of bit 63 to 32. Enabled interrupt bits for bit 31 to 0 contribute to the
                                //                 generation of a transfer completion interrupt for shadow region 7.
        } b;
        uint32_t reg;
    } DRAEH7_reg_t;


    /* (offset = 0x400) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E0 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 0 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 0 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E0_reg_t;


    /* (offset = 0x404) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E1 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 1 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 1 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E1_reg_t;


    /* (offset = 0x408) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E2 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 2 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 2 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E2_reg_t;


    /* (offset = 0x40C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E3 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 3 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 3 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E3_reg_t;


    /* (offset = 0x410) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E4 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 4 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 4 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E4_reg_t;


    /* (offset = 0x414) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E5 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 5 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 5 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E5_reg_t;


    /* (offset = 0x418) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E6 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 6 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 6 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E6_reg_t;


    /* (offset = 0x41C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E7 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 7 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 7 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E7_reg_t;


    /* (offset = 0x420) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E8 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 8 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 8 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E8_reg_t;


    /* (offset = 0x424) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E9 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 9 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 9 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E9_reg_t;


    /* (offset = 0x428) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E10 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 10 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 10 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E10_reg_t;


    /* (offset = 0x42C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E11 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 11 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 11 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E11_reg_t;


    /* (offset = 0x430) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E12 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 12 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 12 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E12_reg_t;


    /* (offset = 0x434) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E13 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 13 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 13 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E13_reg_t;


    /* (offset = 0x438) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E14 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 14 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 14 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E14_reg_t;


    /* (offset = 0x43C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q0E15 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 15 in queue 0. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 15 in queue 0. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q0E15_reg_t;


    /* (offset = 0x440) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E0 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 0 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 0 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E0_reg_t;


    /* (offset = 0x444) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E1 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 1 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 1 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E1_reg_t;


    /* (offset = 0x448) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E2 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 2 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 2 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E2_reg_t;


    /* (offset = 0x44C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E3 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 3 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 3 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E3_reg_t;


    /* (offset = 0x450) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E4 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 4 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 4 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E4_reg_t;


    /* (offset = 0x454) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E5 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 5 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 5 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E5_reg_t;


    /* (offset = 0x458) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E6 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 6 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 6 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E6_reg_t;


    /* (offset = 0x45C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E7 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 7 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 7 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E7_reg_t;


    /* (offset = 0x460) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E8 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 8 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 8 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E8_reg_t;


    /* (offset = 0x464) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E9 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 9 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 9 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E9_reg_t;


    /* (offset = 0x468) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E10 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 10 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 10 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E10_reg_t;


    /* (offset = 0x46C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E11 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 11 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 11 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E11_reg_t;


    /* (offset = 0x470) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E12 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 12 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 12 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E12_reg_t;


    /* (offset = 0x474) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E13 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 13 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 13 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E13_reg_t;


    /* (offset = 0x478) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E14 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 14 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 14 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E14_reg_t;


    /* (offset = 0x47C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q1E15 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 15 in queue 1. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 15 in queue 1. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q1E15_reg_t;


    /* (offset = 0x480) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E0 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 0 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 0 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E0_reg_t;


    /* (offset = 0x484) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E1 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 1 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 1 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E1_reg_t;


    /* (offset = 0x488) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E2 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 2 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 2 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E2_reg_t;


    /* (offset = 0x48C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E3 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 3 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 3 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E3_reg_t;


    /* (offset = 0x490) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E4 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 4 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 4 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E4_reg_t;


    /* (offset = 0x494) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E5 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 5 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 5 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E5_reg_t;


    /* (offset = 0x498) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E6 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 6 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 6 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E6_reg_t;


    /* (offset = 0x49C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E7 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 7 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 7 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E7_reg_t;


    /* (offset = 0x4A0) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E8 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 8 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 8 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E8_reg_t;


    /* (offset = 0x4A4) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E9 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 9 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 9 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E9_reg_t;


    /* (offset = 0x4A8) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E10 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 10 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 10 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E10_reg_t;


    /* (offset = 0x4AC) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E11 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 11 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 11 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E11_reg_t;


    /* (offset = 0x4B0) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E12 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 12 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 12 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E12_reg_t;


    /* (offset = 0x4B4) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E13 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 13 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 13 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E13_reg_t;


    /* (offset = 0x4B8) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E14 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 14 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 14 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E14_reg_t;


    /* (offset = 0x4BC) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register Q2E15 */

            uint32_t    ENUM  : 6; // bits:0..5  (R) Event entry 15 in queue 2. Event number: QDMA channel number (0 to 3). DMA channel/event
                                   //               number (0 to 63).
            uint32_t    ETYPE : 2; // bits:6..7  (R) Event entry 15 in queue 2. Specifies the specific event type for the given entry in the
                                   //               event queue. 0h = Event triggered via ER. 1h = Manual triggered via ESR. 2h = Chain
                                   //               triggered via CER 3h = Autotriggered via QER
            uint32_t          :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } Q2E15_reg_t;


    /* (offset = 0x620) [reset state = 0xA0A0A] */
    typedef union
    {
        struct
        {
                                /* Register QWMTHRA */

            uint32_t    Q0 : 5; // bits:0..4   (R/W) Queue threshold for queue 0 value. The QTHRXCD0 bit in the EDMA3CC error register
                                //                  (CCERR) and the THRXCD bit in the queue status register 0 (QSTAT0) are set when the
                                //                  number of events in queue 0 at an instant in time (visible via the NUMVAL bit in QSTAT0)
                                //                  equals or exceeds the value specified by Q0. 0h = From 0h to 10h, The default is 16
                                //                  (maximum allowed). 11h = Disables the threshold errors. 12h = From 12h to 1Fh, Reserved.
                                //                  Always write 0 to this bit; writes of 1 to this bit are not supported and attempts to do
                                //                  so may result in undefined behavior.
            uint32_t       : 3; // bits:5..7   (R)   Reserved
            uint32_t    Q1 : 5; // bits:8..12  (R/W) Queue threshold for queue 1 value. The QTHRXCD1 bit in the EDMA3CC error register
                                //                  (CCERR) and the THRXCD bit in the queue status register 1 (QSTAT1) are set when the
                                //                  number of events in queue 1 at an instant in time (visible via the NUMVAL bit in QSTAT1)
                                //                  equals or exceeds the value specified by Q1. 0h = From 0h to 10h, The default is 16
                                //                  (maximum allowed). 11h = Disables the threshold errors. 12h = From 12h to 1Fh, Reserved.
                                //                  Always write 0 to this bit; writes of 1 to this bit are not supported and attempts to do
                                //                  so may result in undefined behavior.
            uint32_t       : 3; // bits:13..15 (R)   Reserved
            uint32_t    Q2 : 5; // bits:16..20 (R/W) Queue threshold for queue 2 value. The QTHRXCD2 bit in the EDMA3CC error register
                                //                  (CCERR) and the THRXCD bit in the queue status register 2 (QSTAT2) are set when the
                                //                  number of events in queue 2 at an instant in time (visible via the NUMVAL bit in QSTAT2)
                                //                  equals or exceeds the value specified by Q2. 0h = From 0h to 10h, The default is 16
                                //                  (maximum allowed). 11h = Disables the threshold errors. 12h = From 12h to 1Fh, Reserved.
                                //                  Always write 0 to this bit; writes of 1 to this bit are not supported and attempts to do
                                //                  so may result in undefined behavior.
            uint32_t       :11; // bits:21..31 (R)   Reserved
        } b;
        uint32_t reg;
    } QWMTHRA_reg_t;


    /* (offset = 0x640) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                      /* Register CCSTAT */

            uint32_t    EVTACTV  : 1; // bit:0       (R) DMA event active. 0h = No enabled DMA events are active within the EDMA3CC. 1h = At
                                      //                least one enabled DMA event (ER and EER, ESR, CER) is active within the EDMA3CC.
            uint32_t    QEVTACTV : 1; // bit:1       (R) QDMA event active. 0h = No enabled QDMA events are active within the EDMA3CC. 1h = At
                                      //                least one enabled QDMA event (QER) is active within the EDMA3CC.
            uint32_t    TRACTV   : 1; // bit:2       (R) Transfer request active. 0h = Transfer request processing/submission logic is inactive.
                                      //                1h = Transfer request processing/submission logic is active.
            uint32_t             : 1; // bit:3       (R) Reserved
            uint32_t    ACTV     : 1; // bit:4       (R) Channel controller active. Channel controller active is a logical-OR of each of the
                                      //                *ACTV bits. The ACTV bit remains high through the life of a TR. 0h = Channel is idle..
                                      //                1h = Channel is busy.
            uint32_t             : 3; // bits:5..7   (R) Reserved
            uint32_t    COMPACTV : 6; // bits:8..13  (R) Completion request active. The COMPACTV field reflects the count for the number of
                                      //                completion requests submitted to the transfer controllers. This count increments every
                                      //                time a TR is submitted and is programmed to report completion (the TCINTEN or TCCCHEN
                                      //                bits in OPT in the parameter entry associated with the TR are set). The counter
                                      //                decrements for every valid TCC received back from the transfer controllers. If at any
                                      //                time the count reaches a value of 63, the EDMA3CC will not service any new TRs until the
                                      //                count is less then 63 (or return a transfer completion code from a transfer controller,
                                      //                which would decrement the count). 0h = No completion requests outstanding. 1h = Total of
                                      //                1 completion request to 63 completion requests are outstanding, from 1h to 3Fh.
            uint32_t             : 2; // bits:14..15 (R) Reserved
            uint32_t    QUEACTV0 : 1; // bit:16      (R) Queue 0 active. 0h = No events are queued in queue 0. 1h = At least one TR is queued in
                                      //                queue 0.
            uint32_t    QUEACTV1 : 1; // bit:17      (R) Queue 1 active. 0h = No events are queued in queue 1. 1h = At least one TR is queued in
                                      //                queue 1.
            uint32_t    QUEACTV2 : 1; // bit:18      (R) Queue 2 active. 0h = No events are queued in queue 2. 1h = At least one TR is queued in
                                      //                queue 2.
            uint32_t             :13; // bits:19..31 (R) Reserved
        } b;
        uint32_t reg;
    } CCSTAT_reg_t;


    /* (offset = 0x800) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register MPFAR */

            uint32_t    FADDR :32; // bits:0..31 (R) Fault address. This 32 bit read-only status register contains the fault address when a
                                   //               memory protection violation is detected. This register can only be cleared via the
                                   //               memory protection fault command register (MPFCR). Value 0 to FFFF FFFFh.
        } b;
        uint32_t reg;
    } MPFAR_reg_t;


    /* (offset = 0x804) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                 /* Register MPFSR */

            uint32_t    UXE : 1; // bit:0       (R) User execute error. 0h = No error detected. 1h = User level task attempted to execute
                                 //                from a MP page without UX permissions.
            uint32_t    UWE : 1; // bit:1       (R) User write error. 0h = No error detected. 1h = User level task attempted to write to a
                                 //                MP page without UW permissions.
            uint32_t    URE : 1; // bit:2       (R) User read error. 0h = No error detected. 1h = User level task attempted to read from a
                                 //                MP page without UR permissions.
            uint32_t    SXE : 1; // bit:3       (R) Supervisor execute error. 0h = No error detected. 1h = Supervisor level task attempted
                                 //                to execute from a MP page without SX permissions.
            uint32_t    SWE : 1; // bit:4       (R) Supervisor write error. 0h = No error detected. 1h = Supervisor level task attempted to
                                 //                write to a MP page without SW permissions.
            uint32_t    SRE : 1; // bit:5       (R) Supervisor read error. 0h = No error detected. 1h = Supervisor level task attempted to
                                 //                read from a MP page without SR permissions.
            uint32_t        : 3; // bits:6..8   (R) Reserved
            uint32_t    FID : 4; // bits:9..12  (R) Faulted identification. FID contains valid information if any of the MP error bits (UXE,
                                 //                UWE, URE, SXE, SWE, SRE) are nonzero (that is, if an error has been detected.) The FID
                                 //                field contains the privilege ID for the specific request/requestor that resulted in an
                                 //                MP error. Value 0 to Fh.
            uint32_t        :19; // bits:13..31 (R) Reserved
        } b;
        uint32_t reg;
    } MPFSR_reg_t;


    /* (offset = 0x808) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register MPFCR */

            uint32_t    MPFCLR : 1; // bit:0      (W) Fault clear register. 0h = CPU write of 0 has no effect. 1h = CPU write of 1 to the
                                    //               MPFCLR bit causes any error conditions stored in the memory protection fault address
                                    //               register (MPFAR) and the memory protection fault status register (MPFSR) to be cleared.
            uint32_t           :31; // bits:1..31 (R) Reserved
        } b;
        uint32_t reg;
    } MPFCR_reg_t;


    /* (offset = 0x80C) [reset state = 0x676] */
    typedef union
    {
        struct
        {
                                  /* Register MPPAG */

            uint32_t    UX   : 1; // bit:0       (R/W) User execute permission. 0h = User execute accesses are not allowed from region M. 1h =
                                  //                  User execute accesses are allowed from region M addresses.
            uint32_t    UW   : 1; // bit:1       (R/W) User write permission. 0h = User write accesses are not allowed to region M. 1h = User
                                  //                  write accesses are allowed to region M addresses.
            uint32_t    UR   : 1; // bit:2       (R/W) User read permission. 0h = User read accesses are not allowed from region M. 1h = User
                                  //                  read accesses are allowed from region N addresses.
            uint32_t    SX   : 1; // bit:3       (R/W) Supervisor execute permission. 0h = Supervisor execute accesses are not allowed from
                                  //                  region M. 1h = Supervisor execute accesses are allowed from region M addresses.
            uint32_t    SW   : 1; // bit:4       (R/W) Supervisor write permission. 0h = Supervisor write accesses are not allowed to region M.
                                  //                  1h = Supervisor write accesses are allowed to region N addresses.
            uint32_t    SR   : 1; // bit:5       (R/W) Supervisor read permission. 0h = Supervisor read accesses are not allowed from region M.
                                  //                  1h = Supervisor write accesses are allowed from region M addresses.
            uint32_t         : 3; // bits:6..8   (R)   Reserved
            uint32_t    EXT  : 1; // bit:9       (R/W) External Allowed ID. 0h = Requests with Privilege ID &gt;= 6 are not allowed to region
                                  //                  M, regardless of permission settings (UW, UR, SW, SR). 1h = Requests with Privilege ID
                                  //                  &gt;= 6 are permitted, if access type is allowed as defined by permission settings (UW,
                                  //                  UR, SW, SR).
            uint32_t    AIDm : 6; // bits:10..15 (R/W) Allowed ID &apos;N&apos; 0h = Requests with Privilege ID == N are not allowed to region
                                  //                  M, regardless of permission settings (UW, UR, SW, SR). 1h = Requests with Privilege ID
                                  //                  == N are permitted, if access type is allowed as defined by permission settings (UW, UR,
                                  //                  SW, SR).
            uint32_t         :16; // bits:16..31 (R)   Reserved
        } b;
        uint32_t reg;
    } MPPAG_reg_t;


    /* (offset = 0x10) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                         /* Register ER */

            uint32_t    EN_HINT_ANY : 1; // bit:0      (R/W) The current global enable value when read. Writes set the global enable.
            uint32_t                :31; // bits:1..31 (R)   Reserved
        } b;
        uint32_t reg;
    } ER_reg_t;


    /* (offset = 0x1004) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ERH */

            uint32_t    En :32; // bits:0..31 (R) Event 32 to 63. Events 32 to 63 are captured by the EDMA3CC and are latched into ERH.
                                //               The events are set (En = 1) even when events are disabled (En = 0 in the event enable
                                //               register high, EERH). 0h = EDMA3CC event is not asserted. 1h = EDMA3CC event is
                                //               asserted. Corresponding DMA event is prioritized versus other pending DMA/QDMA events
                                //               for submission to the EDMA3TC.
        } b;
        uint32_t reg;
    } ERH_reg_t;


    /* (offset = 0x1008) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ECR */

            uint32_t    En :32; // bits:0..31 (W) Event clear for event 0 to 31. Any of the event bits in ECR is set to clear the event
                                //               (En) in the event register (ER). A write of 0 has no effect. 0h = No effect. 1h =
                                //               EDMA3CC event is cleared in the event register (ER).
        } b;
        uint32_t reg;
    } ECR_reg_t;


    /* (offset = 0x100C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ECRH */

            uint32_t    En :32; // bits:0..31 (W) Event clear for event 32 to 63. Any of the event bits in ECRH are set to clear the event
                                //               (En) in the event register high (ERH). A write of 0 has no effect. 0h = No effect. 1h =
                                //               EDMA3CC event is cleared in the event register high (ERH).
        } b;
        uint32_t reg;
    } ECRH_reg_t;


    /* (offset = 0x1010) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ESR */

            uint32_t    En :32; // bits:0..31 (R/W) Event set for event 0 to 31. 0h = No effect. 1h = Corresponding DMA event is prioritized
                                //                 versus other pending DMA/QDMA events for submission to the EDMA3TC.
        } b;
        uint32_t reg;
    } ESR_reg_t;


    /* (offset = 0x1014) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ESRH */

            uint32_t    En :32; // bits:0..31 (R/W) Event set for event 32 to 63. 0h = No effect. 1h = Corresponding DMA event is
                                //                 prioritized versus other pending DMA/QDMA events for submission to the EDMA3TC.
        } b;
        uint32_t reg;
    } ESRH_reg_t;


    /* (offset = 0x1018) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register CER */

            uint32_t    En :32; // bits:0..31 (R) Chained event for event 0 to 31. 0h = No effect. 1h = Corresponding DMA event is
                                //               prioritized versus other pending DMA/QDMA events for submission to the EDMA3TC.
        } b;
        uint32_t reg;
    } CER_reg_t;


    /* (offset = 0x101C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register CERH */

            uint32_t    En :32; // bits:0..31 (R) Chained event set for event 32 to 63. 0h = No effect. 1h = Corresponding DMA event is
                                //               prioritized versus other pending DMA/QDMA events for submission to the EDMA3TC.
        } b;
        uint32_t reg;
    } CERH_reg_t;


    /* (offset = 0x1020) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EER */

            uint32_t    En :32; // bits:0..31 (R) Event enable for events 0 to 31. 0h = Event is not enabled. An external event latched in
                                //               the event register (ER) is not evaluated by the EDMA3CC. 1h = Event is enabled. An
                                //               external event latched in the event register (ER) is evaluated by the EDMA3CC.
        } b;
        uint32_t reg;
    } EER_reg_t;


    /* (offset = 0x1024) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EERH */

            uint32_t    En :32; // bits:0..31 (R) Event enable for events 32 to 63. 0h = Event is not enabled. An external event latched
                                //               in the event register high (ERH) is not evaluated by the EDMA3CC. 1h = Event is enabled.
                                //               An external event latched in the event register high (ERH) is evaluated by the EDMA3CC.
        } b;
        uint32_t reg;
    } EERH_reg_t;


    /* (offset = 0x1028) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EECR */

            uint32_t    En :32; // bits:0..31 (W) Event enable clear for events 0 to 31. 0h = No effect. 1h = Event is disabled.
                                //               Corresponding bit in the event enable register (EER) is cleared (En = 0).
        } b;
        uint32_t reg;
    } EECR_reg_t;


    /* (offset = 0x102C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EECRH */

            uint32_t    En :32; // bits:0..31 (W) Event enable clear for events 32 to 63. 0h = No effect. 1h = Event is disabled.
                                //               Corresponding bit in the event enable register high (EERH) is cleared (En = 0).
        } b;
        uint32_t reg;
    } EECRH_reg_t;


    /* (offset = 0x1030) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EESR */

            uint32_t    En :32; // bits:0..31 (W) Event enable set for events 0 to 31. 0h = No effect. 1h = Event is enabled.
                                //               Corresponding bit in the event enable register (EER) is set (En = 1).
        } b;
        uint32_t reg;
    } EESR_reg_t;


    /* (offset = 0x1034) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register EESRH */

            uint32_t    En :32; // bits:0..31 (W) Event enable set for events 32 to 63. 0h = No effect. 1h = Event is enabled.
                                //               Corresponding bit in the event enable register high (EERH) is set (En= 1).
        } b;
        uint32_t reg;
    } EESRH_reg_t;


    /* (offset = 0x1038) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register SER */

            uint32_t    En :32; // bits:0..31 (R) Secondary event register. The secondary event register is used along with the event
                                //               register (ER) to provide information on the state of an event. 0h = Event is not
                                //               currently stored in the event queue. 1h = Event is currently stored in the event queue.
                                //               Event arbiter will not prioritize additional events.
        } b;
        uint32_t reg;
    } SER_reg_t;


    /* (offset = 0x103C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register SERH */

            uint32_t    En :32; // bits:0..31 (R) Secondary event register. The secondary event register is used along with the event
                                //               register high (ERH) to provide information on the state of an event. 0h = Event is not
                                //               currently stored in the event queue. 1h = Event is currently stored in the event queue.
                                //               Event submission/prioritization logic will not prioritize additional events.
        } b;
        uint32_t reg;
    } SERH_reg_t;


    /* (offset = 0x1040) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register SECR */

            uint32_t    En :32; // bits:0..31 (W) Secondary event clear register. 0h = No effect. 1h = Corresponding bit in the secondary
                                //               event register (SER) is cleared (En = 0).
        } b;
        uint32_t reg;
    } SECR_reg_t;


    /* (offset = 0x1044) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register SECRH */

            uint32_t    En :32; // bits:0..31 (W) Secondary event clear register. 0h = No effect. 1h = Corresponding bit in the secondary
                                //               event registers high (SERH) is cleared (En = 0).
        } b;
        uint32_t reg;
    } SECRH_reg_t;

    /*  @brief      Interrupt Enable Register
     *  @details    Interrupt enable registers (IER/IERH) are used to enable/disable the transfer completion interrupt
     *              generation by the EDMA3CC for all DMA/QDMA channels. The IER/IERH cannot be written to directly. To
     *              set any interrupt bit in IER/IERH, a 1 must be written to the corresponding interrupt bit in the interrupt
     *              enable set registers (IESR/IESRH). Similarly, to clear any interrupt bit in IER/IERH, a 1 must be written to
     *              the corresponding interrupt bit in the interrupt enable clear registers (IECR/IECRH). All DMA/QDMA
     *              channels can be set to assert an EDMA3CC completion interrupt to the CPU on transfer completion, by
     *              appropriately configuring the PaRAM entry associated with the channels. This register is used for the
     *              transfer completion interrupt reporting/generating by the EDMA3CC. For more details on EDMA3CC
     *              completion interrupt generation, see EDMA3 Interrupts.
    () [] */
    typedef union
    {
        struct
        {
            uint32_t    In_      :32;        // bit: 0..31        (R) Interrupt enable for channels 0 to 31.
            //                       [0x0 = Interrupt is not enabled.
            //                        0x1 = Interrupt is enabled.]
        } b;                                 // bit access
        uint32_t  reg;                       // raw register
    } IER_reg_t;

    /* (offset = 0x1054) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IERH */

            uint32_t    In :32; // bits:0..31 (R) Interrupt enable for channels 32 to 63. 0h = Interrupt is not enabled. 1h = Interrupt is
                                //               enabled.
        } b;
        uint32_t reg;
    } IERH_reg_t;


    /* (offset = 0x1058) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IECR */

            uint32_t    In :32; // bits:0..31 (W) Interrupt enable clear for channels 0 to 31. 0h = No effect. 1h = Corresponding bit in
                                //               the interrupt enable register (IER) is cleared (In = 0).
        } b;
        uint32_t reg;
    } IECR_reg_t;


    /* (offset = 0x105C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IECRH */

            uint32_t    In :32; // bits:0..31 (W) Interrupt enable clear for channels 32 to 63. 0h = No effect. 1h = Corresponding bit in
                                //               the interrupt enable register high (IERH) is cleared (In = 0).
        } b;
        uint32_t reg;
    } IECRH_reg_t;


    /* (offset = 0x1060) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IESR */

            uint32_t    In :32; // bits:0..31 (W) Interrupt enable set for channels 0 to 31. 0h = No effect. 1h = Corresponding bit in the
                                //               interrupt enable register (IER) is set (In = 1).
        } b;
        uint32_t reg;
    } IESR_reg_t;


    /* (offset = 0x1064) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IESRH */

            uint32_t    In :32; // bits:0..31 (W) Interrupt enable clear for channels 32 to 63. 0h = No effect. 1h = Corresponding bit in
                                //               the interrupt enable register high (IERH) is set (In = 1).
        } b;
        uint32_t reg;
    } IESRH_reg_t;


    /* (offset = 0x1068) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IPR */

            uint32_t    In :32; // bits:0..31 (R) Interrupt pending for TCC = 0 to 31. 0h = Interrupt transfer completion code is not
                                //               detected or was cleared. 1h = Interrupt transfer completion code is detected (In = 1, n
                                //               = EDMA3TC[2:0]).
        } b;
        uint32_t reg;
    } IPR_reg_t;


    /* (offset = 0x106C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register IPRH */

            uint32_t    In :32; // bits:0..31 (R) Interrupt pending for TCC = 32 to 63. 0h = Interrupt transfer completion code is not
                                //               detected or was cleared. 1h = Interrupt transfer completion code is detected (In = 1, n
                                //               = EDMA3TC[2:0]).
        } b;
        uint32_t reg;
    } IPRH_reg_t;


    /* (offset = 0x24) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                         /* Register ICR */

            uint32_t    STS_CLR_IDX :10; // bits:0..9   (W) Writes clear the status of the event given in the index value ( 0-63). Reads return 0.
            uint32_t                :22; // bits:10..31 (R) Reserved
        } b;
        uint32_t reg;
    } ICR_reg_t;


    /* (offset = 0x1074) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register ICRH */

            uint32_t    In :32; // bits:0..31 (W) Interrupt clear register for TCC = 32 to 63. 0h = No effect. 1h = Corresponding bit in
                                //               the interrupt pending register high (IPRH) is cleared (In = 0).
        } b;
        uint32_t reg;
    } ICRH_reg_t;


    /* (offset = 0x1078) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register IEVAL */

            uint32_t    EVAL : 1; // bit:0      (W) Interrupt evaluate. The EDMA3CC completion interrupt that is pulsed depends on which
                                  //               IEVAL is being exercised. For example, writing to the EVAL bit in IEVAL pulses the
                                  //               global completion interrupt, but writing to the EVAL bit in IEVAL0 pulses the region 0
                                  //               completion interrupt. 0h = No effect. 1h = Causes EDMA3CC completion interrupt to be
                                  //               pulsed, if any enabled (IERn/IERHn = 1) interrupts are still pending (IPRn/IPRHn = 1).
            uint32_t         :31; // bits:1..31 (R) Reserved
        } b;
        uint32_t reg;
    } IEVAL_reg_t;


    /* (offset = 0x1080) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QER */

            uint32_t    En : 8; // bits:0..7  (R) QDMA event for channels 0 to 7. 0h = No effect. 1h = Corresponding QDMA event is
                                //               prioritized versus other pending DMA/QDMA events for submission to the EDMA3TC.
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QER_reg_t;


    /* (offset = 0x1084) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QEER */

            uint32_t    En : 8; // bits:0..7  (R) QDMA event enable for channels 0 to 7. 0h = QDMA channel n is not enabled. QDMA event
                                //               will not be recognized and will not latch in the QDMA event register (QER). 1h = QDMA
                                //               channel n is enabled. QDMA events will be recognized and will get latched in the QDMA
                                //               event register (QER).
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QEER_reg_t;


    /* (offset = 0x1088) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QEECR */

            uint32_t    En : 8; // bits:0..7  (W) QDMA event enable clear for channels 0 to 7. 0h = No effect. 1h = QDMA event is
                                //               disabled. Corresponding bit in the QDMA event enable register (QEER) is cleared (En =
                                //               0).
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QEECR_reg_t;


    /* (offset = 0x108C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QEESR */

            uint32_t    En : 8; // bits:0..7  (W) QDMA event enable set for channels 0 to 7. 0h = No effect. 1h = QDMA event is enabled.
                                //               Corresponding bit in the QDMA event enable register (QEER) is set (En = 1).
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QEESR_reg_t;


    /* (offset = 0x1090) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                /* Register QSER */

            uint32_t    En : 8; // bits:0..7  (R) QDMA secondary event register for channels 0 to 7. 0h = QDMA event is not currently
                                //               stored in the event queue. 1h = QDMA event is currently stored in the event queue.
                                //               EDMA3CC will not prioritize additional events.
            uint32_t       :24; // bits:8..31 (R) Reserved
        } b;
        uint32_t reg;
    } QSER_reg_t;

    /* (offset = 0x7F4) [reset state = 0x4516141] */
    typedef union
    {
        struct
        {
                                     /* Register PID */

            uint32_t    usb_pid :16; // bits:0..15  (R) USB Product ID
            uint32_t    usb_vid :16; // bits:16..31 (R) USB Vendor ID
        } b;
        uint32_t reg;
    } PID_reg_t;


    /* (offset = 0x4) [reset state = 0x224] */
    typedef union
    {
        struct
        {
                                       /* Register TCCFG */

            uint32_t    FIFOSIZE  : 3; // bits:0..2   (R) FIFO size 0h = Reserved. 1h = Reserved. 2h = Reserved. 3h = Reserved. 4h = 512 byte FIFO
                                       //                5h = Reserved. 6h = Reserved. 7h = Reserved.
            uint32_t              : 1; // bit:3       (R) Reserved
            uint32_t    BUSWIDTH  : 2; // bits:4..5   (R) Bus width parameterization. 0h = Reserved. 1h = Reserved. 2h = 128-bit. 3h = Reserved.
            uint32_t              : 2; // bits:6..7   (R) Reserved
            uint32_t    DREGDEPTH : 2; // bits:8..9   (R) Destination register FIFO depth parameterization. 0h = Reserved. 1h = Reserved. 2h = 4
                                       //                entry (for TC0, TC1, and TC2) 3h = Reserved.
            uint32_t              :22; // bits:10..31 (R) Reserved
        } b;
        uint32_t reg;
    } TCCFG_reg_t;


    /* (offset = 0x10) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                       /* Register SYSCONFIG */

            uint32_t    Autoidle  : 1; // bit:0      (R/W) Internal OCP clock gating strategy 0h = clkfree : OCP clock is free running 1h =
                                       //                 autoClkGate : Automatic OCP clock gating strategy is applied, bnased on the OCP
                                       //                 interface activity
            uint32_t    SoftReset : 1; // bit:1      (R/W) Software reset. Set this bit to trigger a module reset. The bit is automatically reset
                                       //                 by the hardware. During reads, it always returns 0. 0h(Read) = always_Always returns 0
                                       //                 1h(Read) = never_never happens
            uint32_t              :30; // bits:2..31 (R)   Reserved
        } b;
        uint32_t reg;
    } SYSCONFIG_reg_t;


    /* (offset = 0x100) [reset state = 0x100] */
    typedef union
    {
        struct
        {
                                       /* Register TCSTAT */

            uint32_t    PROGBUSY  : 1; // bit:0       (R) Program register set busy 0h = Program set idle and is available for programming by the
                                       //                EDMA3CC. 1h = Program set busy
            uint32_t    SRCACTV   : 1; // bit:1       (R) Source active state 0h = Source controller is idle. Source active register set contains
                                       //                a previously processed transfer request. 1h = Source controller is busy servicing a
                                       //                transfer request.
            uint32_t    WSACTV    : 1; // bit:2       (R) Write status active 0h = Write status is not pending. Write status has been received for
                                       //                all previously issued write commands. 1h = Write status is pending. Write status has not
                                       //                been received for all previously issued write commands.
            uint32_t              : 1; // bit:3       (R) Reserved
            uint32_t    DSTACTV   : 3; // bits:4..6   (R) Destination active state. Specifies the number of transfer requests (TRs) that are
                                       //                resident in the destination register FIFO at a given instant. This bit field can be
                                       //                primarily used for advanced debugging. Legal values are constrained by the destination
                                       //                register FIFO depth parameterization (DSTREGDEPTH) parameter. 0h = Destination FIFO is
                                       //                empty. 1h = Destination FIFO contains 1 TR. 2h = Destination FIFO contains 2 TRs. 3h =
                                       //                Destination FIFO contains 3 TRs. 4h = Destination FIFO contains 4 TRs. (Full if
                                       //                DSTREGDEPTH==4). If the destination register FIFO is empty, then any TR written to Prog
                                       //                Set immediately transitions to the destination register FIFO. If the destination
                                       //                register FIFO is not empty and not full, then any TR written to Prog Set immediately
                                       //                transitions to the destination register FIFO set if the source active state (SRCACTV)
                                       //                bit is set to idle. If the destination register FIFO is full, then TRs cannot transition
                                       //                to the destination register FIFO. The destination register FIFO becomes not full when
                                       //                the TR at the head of the destination register FIFO is completed. 5h = Reserved. 6h =
                                       //                Reserved. 7h = Reserved.
            uint32_t              : 5; // bits:7..11  (R) Reserved
            uint32_t    DFSTRTPTR : 2; // bits:12..13 (R) Destination FIFO start pointer. Represents the offset to the head entry of the
                                       //                destination register FIFO, in units of entries. Value 0 to 3h.
            uint32_t              :18; // bits:14..31 (R) Reserved
        } b;
        uint32_t reg;
    } TCSTAT_reg_t;


    /* (offset = 0x120) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register ERRSTAT */

            uint32_t    BUSERR  : 1; // bit:0      (R) Bus error event. 0h = Condition is not detected. 1h = EDMA3TC has detected an error at
                                     //               source or destination address. Error information can be read from the error details
                                     //               register (ERRDET).
            uint32_t            : 1; // bit:1      (R) Reserved
            uint32_t    TRERR   : 1; // bit:2      (R) Transfer request (TR) error event. 0h = Condition is not detected. 1h = TR detected that
                                     //               violates constant addressing mode transfer (SAM or DAM is set) alignment rules or has
                                     //               ACNT or BCNT == 0.
            uint32_t    MMRAERR : 1; // bit:3      (R) MMR address error. 0h = Condition is not detected. 1h = User attempted to read or write
                                     //               to an invalid address in configuration memory map.
            uint32_t            :28; // bits:4..31 (R) Reserved
        } b;
        uint32_t reg;
    } ERRSTAT_reg_t;


    /* (offset = 0x124) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register ERREN */

            uint32_t    BUSERR  : 1; // bit:0      (R/W) Interrupt enable for bus error (BUSERR). 0h = BUSERR is disabled. 1h = BUSERR is enabled
                                     //                 and contributes to the state of EDMA3TC error interrupt generation.
            uint32_t            : 1; // bit:1      (R)   Reserved
            uint32_t    TRERR   : 1; // bit:2      (R/W) Interrupt enable for transfer request error (TRERR). 0h = TRERR is disabled. 1h = TRERR
                                     //                 is enabled and contributes to the state of EDMA3TC error interrupt generation.
            uint32_t    MMRAERR : 1; // bit:3      (R/W) Interrupt enable for MMR address error (MMRAERR). 0h = MMRAERR is disabled. 1h = MMRAERR
                                     //                 is enabled and contributes to the state of EDMA3TC error interrupt generation
            uint32_t            :28; // bits:4..31 (R)   Reserved
        } b;
        uint32_t reg;
    } ERREN_reg_t;


    /* (offset = 0x31C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                      /* Register ERRCLR */

            uint32_t    QTHRXCD0 : 1; // bit:0       (W) Queue threshold error clear for queue 0. 0h = No effect. 1h = Clears the QTHRXCD0 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 0 (QSTAT0).
            uint32_t    QTHRXCD1 : 1; // bit:1       (W) Queue threshold error clear for queue 1. 0h = No effect. 1h = Clears the QTHRXCD1 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 1 (QSTAT1).
            uint32_t    QTHRXCD2 : 1; // bit:2       (W) Queue threshold error clear for queue 2. 0h = No effect. 1h = Clears the QTHRXCD2 bit in
                                      //                the EDMA3CC error register (CCERR) and the WM and THRXCD bits in the queue status
                                      //                register 2 (QSTAT2).
            uint32_t             :13; // bits:3..15  (R) Reserved
            uint32_t    TCCERR   : 1; // bit:16      (W) Transfer completion code error clear. 0h = No effect. 1h = Clears the TCCERR bit in the
                                      //                EDMA3CC error register (CCERR).
            uint32_t             :15; // bits:17..31 (R) Reserved
        } b;
        uint32_t reg;
    } ERRCLR_reg_t;


    /* (offset = 0x12C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register ERRDET */

            uint32_t    STAT    : 4; // bits:0..3   (R) Transaction status. Stores the nonzero status/error code that was detected on the read
                                     //                status or write status bus. If read status and write status are returned on the same
                                     //                cycle, then the EDMA3TC chooses nonzero version. If both are nonzero, then the write
                                     //                status is treated as higher priority. 0h = No error. 1h = From 1h to 7h, Read error. 8h
                                     //                = From 8h to Fh, Write error.
            uint32_t            : 4; // bits:4..7   (R) Reserved
            uint32_t    TCC     : 6; // bits:8..13  (R) Transfer complete code. Contains the TCC value in the channel options parameter (OPT)
                                     //                programmed by the channel controller for the read or write transaction that resulted in
                                     //                an error.
            uint32_t            : 2; // bits:14..15 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:16      (R) Transfer completion interrupt enable. Contains the TCINTEN value in the channel options
                                     //                parameter (OPT) programmed by the channel controller for the read or write transaction
                                     //                that resulted in an error.
            uint32_t    TCCHEN  : 1; // bit:17      (R) Transfer completion chaining enable. Contains the TCCHEN value in the channel options
                                     //                parameter (OPT) programmed by the channel controller for the read or write transaction
                                     //                that resulted in an error.
            uint32_t            :14; // bits:18..31 (R) Reserved
        } b;
        uint32_t reg;
    } ERRDET_reg_t;


    /* (offset = 0x130) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register ERRCMD */

            uint32_t    EVAL : 1; // bit:0      (W) Error evaluate 0h = No effect 1h = EDMA3TC error line is pulsed if any of the error
                                  //               status register (ERRSTAT) bits are set.
            uint32_t         :31; // bits:1..31 (R) Reserved
        } b;
        uint32_t reg;
    } ERRCMD_reg_t;


    /* (offset = 0x140) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register RDRATE */

            uint32_t    RDRATE : 3; // bits:0..2  (R/W) Read rate. Controls the number of cycles between read commands. This is a global setting
                                    //                 that applies to all TRs for this EDMA3TC. 0h = Reads issued as fast as possible. 1h = 4
                                    //                 cycles between reads. 2h = 8 cycles between reads. 3h = 16 cycles between reads. 4h = 32
                                    //                 cycles between reads. 5h = Reserved. 6h = Reserved. 7h = Reserved.
            uint32_t           :29; // bits:3..31 (R)   Reserved
        } b;
        uint32_t reg;
    } RDRATE_reg_t;


    /* (offset = 0x240) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register SAOPT */

            uint32_t    SAM     : 1; // bit:0       (R) Source address mode within an array 0h = Increment (INCR) mode. Source addressing within
                                     //                an array increments. 1h = Constant addressing (CONST) mode. Source addressing within an
                                     //                array wraps around upon reaching FIFO width.
            uint32_t    DAM     : 1; // bit:1       (R) Destination address mode within an array 0h = Increment (INCR) mode. Destination
                                     //                addressing within an array increments. 1h = Constant addressing (CONST) mode.
                                     //                Destination addressing within an array wraps around upon reaching FIFO width.
            uint32_t            : 2; // bits:2..3   (R) Reserved
            uint32_t    PRI     : 3; // bits:4..6   (R) Transfer priority. Reflects the values programmed in the QUEPRI register in the EDMACC.
                                     //                0h = Priority 0 - Highest priority 1h = From 1h to 6h, Priority 1 to priority 6 7h =
                                     //                Priority 7 - Lowest priority
            uint32_t            : 1; // bit:7       (R) Reserved
            uint32_t    FWID    : 3; // bits:8..10  (R) FIFO width. Applies if either SAM or DAM is set to constant addressing mode. 0h = FIFO
                                     //                width is 8-bit. 1h = FIFO width is 16-bit. 2h = FIFO width is 32-bit. 3h = FIFO width is
                                     //                64-bit. 4h = FIFO width is 128-bit. 5h = FIFO width is 256-bit. 6h = Reserved. 7h =
                                     //                Reserved.
            uint32_t            : 1; // bit:11      (R) Reserved
            uint32_t    TCC     : 6; // bits:12..17 (R) Transfer complete code. This 6 bit code is used to set the relevant bit in CER or IPR of
                                     //                the EDMA3PCC module.
            uint32_t            : 2; // bits:18..19 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:20      (R) Transfer complete interrupt enable. 0h = Transfer complete interrupt is disabled. 1h =
                                     //                Transfer complete interrupt is enabled.
            uint32_t            : 1; // bit:21      (R) Reserved
            uint32_t    TCCHEN  : 1; // bit:22      (R) Transfer complete chaining enable 0h = Transfer complete chaining is disabled. 1h =
                                     //                Transfer complete chaining is enabled.
            uint32_t            : 9; // bits:23..31 (R) Reserved
        } b;
        uint32_t reg;
    } SAOPT_reg_t;


    /* (offset = 0x244) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register SASRC */

            uint32_t    SADDR :32; // bits:0..31 (R) Source address for Source Active Register Set. EDMA3TC updates value according to source
                                   //               addressing mode (SAM bit in the source active options register, SAOPT). This register
                                   //               does not update during a transfer. Value 0 to FFFFh.
        } b;
        uint32_t reg;
    } SASRC_reg_t;


    /* (offset = 0x248) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register SACNT */

            uint32_t    ACNT :16; // bits:0..15  (R) A dimension count remaining for the Source Active Register Set. Number of bytes to be
                                  //                transferred in first dimension. Represents the amount of data remaining to be read. TC
                                  //                decrements ACNT as necessary after each read command is issued. The final value should
                                  //                be 0 when TR is complete.
            uint32_t    BCNT :16; // bits:16..31 (R) B dimension count remaining for the Source Active Register Set. Number of arrays to be
                                  //                transferred, where each array is ACNT in length. TC decrements BCNT as necessary after
                                  //                each read command is issued. The final value should be 0 when TR is complete. Note that
                                  //                BCNT is always read as 0x0 in the Source Address Register Set.
        } b;
        uint32_t reg;
    } SACNT_reg_t;


    /* (offset = 0x24C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register SADST */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } SADST_reg_t;


    /* (offset = 0x250) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register SABIDX */

            uint32_t    SBIDX :16; // bits:0..15  (R) B-Index offset between source arrays. Represents the offset in bytes between the
                                   //                starting address of each source array (there are BCNT arrays of ACNT elements). SBIDX is
                                   //                always used regardless of whether SAM is in Increment or FIFO mode. Value 0 to FFFFh.
            uint32_t    DBIDX :16; // bits:16..31 (R) B-Index offset between destination arrays. Represents the offset in bytes between the
                                   //                starting address of each destination array (there are BCND arrays of ACNT elements).
                                   //                DBIDX is always used regardless of whether DAM is in Increment or FIFO mode.
        } b;
        uint32_t reg;
    } SABIDX_reg_t;


    /* (offset = 0x254) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register SAMPPRXY */

            uint32_t    PRIVID : 4; // bits:0..3  (R) Privilege ID. This contains the privilege ID of the host that set up the parameter entry
                                    //               in the channel controller. This field is set up when the associated TR is submitted to
                                    //               the EDMA3TC. This PRIVID value is used while issuing read and write commands to the
                                    //               target endpoints so that the target endpoints can perform memory protection checks based
                                    //               on the PRIVID of the host that set up the DMA transaction. Value 0 to Fh.
            uint32_t           : 4; // bits:4..7  (R) Reserved
            uint32_t    PRIV   : 1; // bit:8      (R) Privilege level. The privilege level used by the host to set up the parameter entry in
                                    //               the channel controller. This field is set up when the associated TR is submitted to the
                                    //               EDMA3TC. The privilege ID is used while issuing read and write command to the target
                                    //               endpoints so that the target endpoints can perform memory protection checks based on the
                                    //               PRIV of the host that set up the DMA transaction. 0h = User-level privilege. 1h =
                                    //               Supervisor-level privilege.
            uint32_t           :23; // bits:9..31 (R) Reserved
        } b;
        uint32_t reg;
    } SAMPPRXY_reg_t;


    /* (offset = 0x258) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register SACNTRLD */

            uint32_t    ACNTRLD :16; // bits:0..15  (R) A-count reload value. Represents the originally programmed value of ACNT. The reload
                                     //                value is used to reinitialize ACNT after each array is serviced (that is, ACNT
                                     //                decrements to 0) by the source offset in bytes between the starting address or each
                                     //                source array (there are BCNT arrays of ACNT bytes). Value 0 to FFFFh.
            uint32_t            :16; // bits:16..31 (R) Reserved
        } b;
        uint32_t reg;
    } SACNTRLD_reg_t;


    /* (offset = 0x25C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                       /* Register SASRCBREF */

            uint32_t    SADDRBREF :32; // bits:0..31 (R) Source address B-reference. Represents the starting address for the array currently
                                       //               being read. The next array&apos;s starting address is calculated as the reference
                                       //               address plus the source b-idx value. Value 0 to FFFF FFFFh.
        } b;
        uint32_t reg;
    } SASRCBREF_reg_t;


    /* (offset = 0x260) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register SADSTBREF */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } SADSTBREF_reg_t;


    /* (offset = 0x280) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register DFCNTRLD */

            uint32_t    ACNTRLD :16; // bits:0..15  (R) A-count reload value for the Destination FIFO Register Set. Represents the originally
                                     //                programmed value of ACNT. The reload value is used to reinitialize ACNT after each array
                                     //                is serviced (that is, ACNT decrements to 0) by the source offset in bytes between the
                                     //                starting address of each source array (there are BCNT arrays of ACNT bytes). Value 0 to
                                     //                FFFFh.
            uint32_t            :16; // bits:16..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFCNTRLD_reg_t;


    /* (offset = 0x284) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register DFSRCBREF */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFSRCBREF_reg_t;


    /* (offset = 0x288) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                       /* Register DFDSTBREF */

            uint32_t    DADDRBREF :32; // bits:0..31 (R) Destination address reference for the destination FIFO register set. Represents the
                                       //               starting address for the array currently being written. The next array&apos;s starting
                                       //               address is calculated as the reference address plus the destination B-Index value. Value
                                       //               0 to FFFF FFFFh.
        } b;
        uint32_t reg;
    } DFDSTBREF_reg_t;


    /* (offset = 0x300) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register DFOPT0 */

            uint32_t    SAM     : 1; // bit:0       (R) Source address mode within an array 0h = Increment (INCR) mode. Source addressing within
                                     //                an array increments. 1h = Constant addressing (CONST) mode. Source addressing within an
                                     //                array wraps around upon reaching FIFO width.
            uint32_t    DAM     : 1; // bit:1       (R) Destination address mode within an array 0h = Increment (INCR) mode. Destination
                                     //                addressing within an array increments. 1h = Constant addressing (CONST) mode.
                                     //                Destination addressing within an array wraps around upon reaching FIFO width.
            uint32_t            : 2; // bits:2..3   (R) Reserved
            uint32_t    PRI     : 3; // bits:4..6   (R) Transfer priority 0h = Priority 0 - Highest priority 1h = From 1h to 6h, Priority 1 to
                                     //                priority 6 7h = Priority 7 - Lowest priority
            uint32_t            : 1; // bit:7       (R) Reserved
            uint32_t    FWID    : 3; // bits:8..10  (R) FIFO width. Applies if either SAM or DAM is set to constant addressing mode. 0h = FIFO
                                     //                width is 8-bit. 1h = FIFO width is 16-bit. 2h = FIFO width is 32-bit. 3h = FIFO width is
                                     //                64-bit. 4h = FIFO width is 128-bit. 5h = FIFO width is 256-bit. 6h = Reserved. 7h =
                                     //                Reserved.
            uint32_t            : 1; // bit:11      (R) Reserved
            uint32_t    TCC     : 6; // bits:12..17 (R) Transfer complete code. This 6 bit code is used to set the relevant bit in CER or IPR of
                                     //                the EDMA3PCC module.
            uint32_t            : 2; // bits:18..19 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:20      (R) Transfer complete interrupt enable. 0h = Transfer complete interrupt is disabled. 1h =
                                     //                Transfer complete interrupt is enabled.
            uint32_t            : 1; // bit:21      (R) Reserved
            uint32_t    TCCHEN  : 1; // bit:22      (R) Transfer complete chaining enable 0h = Transfer complete chaining is disabled 1h =
                                     //                Transfer complete chaining is enabled
            uint32_t            : 9; // bits:23..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFOPT0_reg_t;


    /* (offset = 0x304) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register DFSRC0 */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFSRC0_reg_t;


    /* (offset = 0x308) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register DFCNT0 */

            uint32_t    ACNT :16; // bits:0..15  (R) A-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation. Value 0 to FFFFh.
            uint32_t    BCNT :16; // bits:16..31 (R) B-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation.
        } b;
        uint32_t reg;
    } DFCNT0_reg_t;


    /* (offset = 0x30C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFDST0 */

            uint32_t    DADDR :32; // bits:0..31 (R) Destination address for the destination FIFO register set. TC updates the value
                                   //               according to both the destination addressing mode (OPT.SAM) or destination index value
                                   //               (BIDX.DBIDX) after each write command is issued. When a TR is complete, the final value
                                   //               should be the address of the last write command issued. Note: If DAM == CONST, the
                                   //               &apos;active&apos; address will increment internally as if the transfer were an
                                   //               &apos;Increment&apos; transfer. The address issued on the write command interface will
                                   //               correctly issue the same address programmed by the user.
        } b;
        uint32_t reg;
    } DFDST0_reg_t;


    /* (offset = 0x310) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFBIDX0 */

            uint32_t    SBIDX :16; // bits:0..15  (R) B-Index offset between source arrays for the Destination FIFO Register Set. Represents
                                   //                the offset in bytes between the starting address of each source array (there are BCNT
                                   //                arrays of ACNT elements). SBIDX is always used regardless of whether SAM is in Increment
                                   //                or FIFO mode.
            uint32_t    DBIDX :16; // bits:16..31 (R) B-Index offset between destination arrays for the Destination FIFO Register Set.
                                   //                Represents the offset in bytes between the starting address of each destination array
                                   //                (there are BCNT arrays of ACNT elements). DBIDX is always used regardless of whether DAM
                                   //                is in Increment or FIFO mode. Value 0 to FFFFh.
        } b;
        uint32_t reg;
    } DFBIDX0_reg_t;


    /* (offset = 0x314) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register DFMPPRXY0 */

            uint32_t    PRIVID : 4; // bits:0..3  (R) Privilege ID. This contains the Privilege ID of the EDMA3 programmer that set up the
                                    //               parameter entry in the channel controller. This field is set up when the associated TR
                                    //               is submitted to the EDMA3TC. This PRIVID value is used while issuing read and write
                                    //               commands to the target endpoints so that the target endpoints can perform memory
                                    //               protection checks based on the PRIVID of the host that set up the DMA transaction. Value
                                    //               0 to Fh.
            uint32_t           : 4; // bits:4..7  (R) Reserved
            uint32_t    PRIV   : 1; // bit:8      (R) Privilege level. This contains the Privilege level used by the EDMA3 programmer to set
                                    //               up the parameter entry in the channel controller. This field is set up when the
                                    //               associated TR is submitted to the EDMA3TC. The privilege ID is used while issuing read
                                    //               and write command to the target endpoints so that the target endpoints can perform
                                    //               memory protection checks based on the PRIV of the host that set up the DMA transaction.
                                    //               0h = User-level privilege 1h = Supervisor-level privilege
            uint32_t           :23; // bits:9..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFMPPRXY0_reg_t;


    /* (offset = 0x340) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register DFOPT1 */

            uint32_t    SAM     : 1; // bit:0       (R) Source address mode within an array 0h = Increment (INCR) mode. Source addressing within
                                     //                an array increments. 1h = Constant addressing (CONST) mode. Source addressing within an
                                     //                array wraps around upon reaching FIFO width.
            uint32_t    DAM     : 1; // bit:1       (R) Destination address mode within an array 0h = Increment (INCR) mode. Destination
                                     //                addressing within an array increments. 1h = Constant addressing (CONST) mode.
                                     //                Destination addressing within an array wraps around upon reaching FIFO width.
            uint32_t            : 2; // bits:2..3   (R) Reserved
            uint32_t    PRI     : 3; // bits:4..6   (R) Transfer priority 0h = Priority 0 - Highest priority 1h = From 1h to 6h, Priority 1 to
                                     //                priority 6 7h = Priority 7 - Lowest priority
            uint32_t            : 1; // bit:7       (R) Reserved
            uint32_t    FWID    : 3; // bits:8..10  (R) FIFO width. Applies if either SAM or DAM is set to constant addressing mode. 0h = FIFO
                                     //                width is 8-bit. 1h = FIFO width is 16-bit. 2h = FIFO width is 32-bit. 3h = FIFO width is
                                     //                64-bit. 4h = FIFO width is 128-bit. 5h = FIFO width is 256-bit. 6h = Reserved. 7h =
                                     //                Reserved.
            uint32_t            : 1; // bit:11      (R) Reserved
            uint32_t    TCC     : 6; // bits:12..17 (R) Transfer complete code. This 6 bit code is used to set the relevant bit in CER or IPR of
                                     //                the EDMA3PCC module.
            uint32_t            : 2; // bits:18..19 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:20      (R) Transfer complete interrupt enable. 0h = Transfer complete interrupt is disabled. 1h =
                                     //                Transfer complete interrupt is enabled.
            uint32_t            : 1; // bit:21      (R) Reserved
            uint32_t    TCCHEN  : 1; // bit:22      (R) Transfer complete chaining enable 0h = Transfer complete chaining is disabled 1h =
                                     //                Transfer complete chaining is enabled
            uint32_t            : 9; // bits:23..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFOPT1_reg_t;


    /* (offset = 0x344) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register DFSRC1 */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFSRC1_reg_t;


    /* (offset = 0x348) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register DFCNT1 */

            uint32_t    ACNT :16; // bits:0..15  (R) A-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation. Value 0 to FFFFh.
            uint32_t    BCNT :16; // bits:16..31 (R) B-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation.
        } b;
        uint32_t reg;
    } DFCNT1_reg_t;


    /* (offset = 0x34C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFDST1 */

            uint32_t    DADDR :32; // bits:0..31 (R) ARRAY(0x245d830) Note: If DAM == CONST, the &apos;active&apos; address will increment
                                   //               internally as if the transfer were an &apos;Increment&apos; transfer. The address issued
                                   //               on the write command interface will correctly issue the same address programmed by the
                                   //               user.
        } b;
        uint32_t reg;
    } DFDST1_reg_t;


    /* (offset = 0x350) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFBIDX1 */

            uint32_t    SBIDX :16; // bits:0..15  (R) B-Index offset between source arrays for the Destination FIFO Register Set. Represents
                                   //                the offset in bytes between the starting address of each source array (there are BCNT
                                   //                arrays of ACNT elements). SBIDX is always used regardless of whether SAM is in Increment
                                   //                or FIFO mode.
            uint32_t    DBIDX :16; // bits:16..31 (R) B-Index offset between destination arrays for the Destination FIFO Register Set.
                                   //                Represents the offset in bytes between the starting address of each destination array
                                   //                (there are BCNT arrays of ACNT elements). DBIDX is always used regardless of whether DAM
                                   //                is in Increment or FIFO mode. Value 0 to FFFFh.
        } b;
        uint32_t reg;
    } DFBIDX1_reg_t;


    /* (offset = 0x354) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register DFMPPRXY1 */

            uint32_t    PRIVID : 4; // bits:0..3  (R) Privilege ID. This contains the Privilege ID of the EDMA3 programmer that set up the
                                    //               parameter entry in the channel controller. This field is set up when the associated TR
                                    //               is submitted to the EDMA3TC. This PRIVID value is used while issuing read and write
                                    //               commands to the target endpoints so that the target endpoints can perform memory
                                    //               protection checks based on the PRIVID of the host that set up the DMA transaction. Value
                                    //               0 to Fh.
            uint32_t           : 4; // bits:4..7  (R) Reserved
            uint32_t    PRIV   : 1; // bit:8      (R) Privilege level. This contains the Privilege level used by the EDMA3 programmer to set
                                    //               up the parameter entry in the channel controller. This field is set up when the
                                    //               associated TR is submitted to the EDMA3TC. The privilege ID is used while issuing read
                                    //               and write command to the target endpoints so that the target endpoints can perform
                                    //               memory protection checks based on the PRIV of the host that set up the DMA transaction.
                                    //               0h = User-level privilege 1h = Supervisor-level privilege
            uint32_t           :23; // bits:9..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFMPPRXY1_reg_t;


    /* (offset = 0x380) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register DFOPT2 */

            uint32_t    SAM     : 1; // bit:0       (R) Source address mode within an array 0h = Increment (INCR) mode. Source addressing within
                                     //                an array increments. 1h = Constant addressing (CONST) mode. Source addressing within an
                                     //                array wraps around upon reaching FIFO width.
            uint32_t    DAM     : 1; // bit:1       (R) Destination address mode within an array 0h = Increment (INCR) mode. Destination
                                     //                addressing within an array increments. 1h = Constant addressing (CONST) mode.
                                     //                Destination addressing within an array wraps around upon reaching FIFO width.
            uint32_t            : 2; // bits:2..3   (R) Reserved
            uint32_t    PRI     : 3; // bits:4..6   (R) Transfer priority 0h = Priority 0 - Highest priority 1h = From 1h to 6h, Priority 1 to
                                     //                priority 6 7h = Priority 7 - Lowest priority
            uint32_t            : 1; // bit:7       (R) Reserved
            uint32_t    FWID    : 3; // bits:8..10  (R) FIFO width. Applies if either SAM or DAM is set to constant addressing mode. 0h = FIFO
                                     //                width is 8-bit. 1h = FIFO width is 16-bit. 2h = FIFO width is 32-bit. 3h = FIFO width is
                                     //                64-bit. 4h = FIFO width is 128-bit. 5h = FIFO width is 256-bit. 6h = Reserved. 7h =
                                     //                Reserved.
            uint32_t            : 1; // bit:11      (R) Reserved
            uint32_t    TCC     : 6; // bits:12..17 (R) Transfer complete code. This 6 bit code is used to set the relevant bit in CER or IPR of
                                     //                the EDMA3PCC module.
            uint32_t            : 2; // bits:18..19 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:20      (R) Transfer complete interrupt enable. 0h = Transfer complete interrupt is disabled. 1h =
                                     //                Transfer complete interrupt is enabled.
            uint32_t            : 1; // bit:21      (R) Reserved
            uint32_t    TCCHEN  : 1; // bit:22      (R) Transfer complete chaining enable 0h = Transfer complete chaining is disabled 1h =
                                     //                Transfer complete chaining is enabled
            uint32_t            : 9; // bits:23..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFOPT2_reg_t;


    /* (offset = 0x384) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register DFSRC2 */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFSRC2_reg_t;


    /* (offset = 0x388) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register DFCNT2 */

            uint32_t    ACNT :16; // bits:0..15  (R) A-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation. Value 0 to FFFFh.
            uint32_t    BCNT :16; // bits:16..31 (R) B-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation.
        } b;
        uint32_t reg;
    } DFCNT2_reg_t;


    /* (offset = 0x38C) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFDST2 */

            uint32_t    DADDR :32; // bits:0..31 (R) ARRAY(0x248ac60) Note: If DAM == CONST, the &apos;active&apos; address will increment
                                   //               internally as if the transfer were an &apos;Increment&apos; transfer. The address issued
                                   //               on the write command interface will correctly issue the same address programmed by the
                                   //               user.
        } b;
        uint32_t reg;
    } DFDST2_reg_t;


    /* (offset = 0x390) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFBIDX2 */

            uint32_t    SBIDX :16; // bits:0..15  (R) B-Index offset between source arrays for the Destination FIFO Register Set. Represents
                                   //                the offset in bytes between the starting address of each source array (there are BCNT
                                   //                arrays of ACNT elements). SBIDX is always used regardless of whether SAM is in Increment
                                   //                or FIFO mode.
            uint32_t    DBIDX :16; // bits:16..31 (R) B-Index offset between destination arrays for the Destination FIFO Register Set.
                                   //                Represents the offset in bytes between the starting address of each destination array
                                   //                (there are BCNT arrays of ACNT elements). DBIDX is always used regardless of whether DAM
                                   //                is in Increment or FIFO mode. Value 0 to FFFFh.
        } b;
        uint32_t reg;
    } DFBIDX2_reg_t;


    /* (offset = 0x394) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                    /* Register DFMPPRXY2 */

            uint32_t    PRIVID : 4; // bits:0..3  (R) Privilege ID. This contains the Privilege ID of the EDMA3 programmer that set up the
                                    //               parameter entry in the channel controller. This field is set up when the associated TR
                                    //               is submitted to the EDMA3TC. This PRIVID value is used while issuing read and write
                                    //               commands to the target endpoints so that the target endpoints can perform memory
                                    //               protection checks based on the PRIVID of the host that set up the DMA transaction. Value
                                    //               0 to Fh.
            uint32_t           : 4; // bits:4..7  (R) Reserved
            uint32_t    PRIV   : 1; // bit:8      (R) Privilege level. This contains the Privilege level used by the EDMA3 programmer to set
                                    //               up the parameter entry in the channel controller. This field is set up when the
                                    //               associated TR is submitted to the EDMA3TC. The privilege ID is used while issuing read
                                    //               and write command to the target endpoints so that the target endpoints can perform
                                    //               memory protection checks based on the PRIV of the host that set up the DMA transaction.
                                    //               0h = User-level privilege 1h = Supervisor-level privilege
            uint32_t           :23; // bits:9..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFMPPRXY2_reg_t;


    /* (offset = 0x3C0) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                     /* Register DFOPT3 */

            uint32_t    SAM     : 1; // bit:0       (R) Source address mode within an array 0h = Increment (INCR) mode. Source addressing within
                                     //                an array increments. 1h = Constant addressing (CONST) mode. Source addressing within an
                                     //                array wraps around upon reaching FIFO width.
            uint32_t    DAM     : 1; // bit:1       (R) Destination address mode within an array 0h = Increment (INCR) mode. Destination
                                     //                addressing within an array increments. 1h = Constant addressing (CONST) mode.
                                     //                Destination addressing within an array wraps around upon reaching FIFO width.
            uint32_t            : 2; // bits:2..3   (R) Reserved
            uint32_t    PRI     : 3; // bits:4..6   (R) Transfer priority 0h = Priority 0 - Highest priority 1h = From 1h to 6h, Priority 1 to
                                     //                priority 6 7h = Priority 7 - Lowest priority
            uint32_t            : 1; // bit:7       (R) Reserved
            uint32_t    FWID    : 3; // bits:8..10  (R) FIFO width. Applies if either SAM or DAM is set to constant addressing mode. 0h = FIFO
                                     //                width is 8-bit. 1h = FIFO width is 16-bit. 2h = FIFO width is 32-bit. 3h = FIFO width is
                                     //                64-bit. 4h = FIFO width is 128-bit. 5h = FIFO width is 256-bit. 6h = Reserved. 7h =
                                     //                Reserved.
            uint32_t            : 1; // bit:11      (R) Reserved
            uint32_t    TCC     : 6; // bits:12..17 (R) Transfer complete code. This 6 bit code is used to set the relevant bit in CER or IPR of
                                     //                the EDMA3PCC module.
            uint32_t            : 2; // bits:18..19 (R) Reserved
            uint32_t    TCINTEN : 1; // bit:20      (R) Transfer complete interrupt enable. 0h = Transfer complete interrupt is disabled. 1h =
                                     //                Transfer complete interrupt is enabled.
            uint32_t            : 1; // bit:21      (R) Reserved
            uint32_t    TCCHEN  : 1; // bit:22      (R) Transfer complete chaining enable 0h = Transfer complete chaining is disabled 1h =
                                     //                Transfer complete chaining is enabled
            uint32_t            : 9; // bits:23..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFOPT3_reg_t;


    /* (offset = 0x3C4) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                                  /* Register DFSRC3 */

            uint32_t                         :32; // bits:0..31 (R) Reserved
        } b;
        uint32_t reg;
    } DFSRC3_reg_t;


    /* (offset = 0x3C8) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                  /* Register DFCNT3 */

            uint32_t    ACNT :16; // bits:0..15  (R) A-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation. Value 0 to FFFFh.
            uint32_t    BCNT :16; // bits:16..31 (R) B-dimension count remaining for Destination Register Set. Represents the amount of data
                                  //                remaining to be written. For the final TR in the Destination Register FIFO: TC
                                  //                decrements ACNT and BCNT as necessary after each write dataphase is issued. The final
                                  //                value should be 0 when TR is complete. For a non-final TR in the Destination Register
                                  //                FIFO: CNT will hold the originally programmed value or the optimized BCNT value after 2D
                                  //                optimization calculation.
        } b;
        uint32_t reg;
    } DFCNT3_reg_t;


    /* (offset = 0x3CC) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFDST3 */

            uint32_t    DADDR :32; // bits:0..31 (R) ARRAY(0x24b1d10) Note: If DAM == CONST, the &apos;active&apos; address will increment
                                   //               internally as if the transfer were an &apos;Increment&apos; transfer. The address issued
                                   //               on the write command interface will correctly issue the same address programmed by the
                                   //               user.
        } b;
        uint32_t reg;
    } DFDST3_reg_t;


    /* (offset = 0x3D0) [reset state = 0x0] */
    typedef union
    {
        struct
        {
                                   /* Register DFBIDX3 */

            uint32_t    SBIDX :16; // bits:0..15  (R) B-Index offset between source arrays for the Destination FIFO Register Set. Represents
                                   //                the offset in bytes between the starting address of each source array (there are BCNT
                                   //                arrays of ACNT elements). SBIDX is always used regardless of whether SAM is in Increment
                                   //                or FIFO mode.
            uint32_t    DBIDX :16; // bits:16..31 (R) B-Index offset between destination arrays for the Destination FIFO Register Set.
                                   //                Represents the offset in bytes between the starting address of each destination array
                                   //                (there are BCNT arrays of ACNT elements). DBIDX is always used regardless of whether DAM
                                   //                is in Increment or FIFO mode. Value 0 to FFFFh.
        } b;
        uint32_t reg;
    } DFBIDX3_reg_t;

    /*  @brief      Destination FIFO Memory Protection Proxy Register 0..3
     *  @details    The Destination FIFO Memory Protection Proxy Register (DFMPPRXY) is an EDMA3TC channel register.
     *              This EDMA3TC channel register is part of the Destination Register FIFO Register Set. It is read-only and
     *              provided to facilitate debugging by providing a window into how the transfer controller (TC) was
     *              programmed by the channel controller (CC), as well as showing status of the transfer controller (TC)
     *              during a transfer. The number of destination FIFO register sets depends on the destination FIFO depth.
     *              TC0, TC1, and TC2 each have a destination FIFO depth of 4, so there are four sets of destination FIFO
     *              registers for each of these transfer controllers.
     (offset = 0x314,0x354,0x394,0x3D4) [reset = 0x0] */
    typedef union
    {
        struct
        {
            uint32_t    PRIVID      :4;     // bits 0..3  (R) Privilege ID.
                                            //                This contains the Privilege ID of the EDMA3 programmer that set up
                                            //                the parameter entry in the channel controller.
                                            //                This field is set up when the associated TR is submitted to the
                                            //                EDMA3TC.
                                            //                This PRIVID value is used while issuing read and write commands to
                                            //                the target endpoints so that the target endpoints can perform
                                            //                memory protection checks based on the PRIVID of the host that set
                                            //                up the DMA transaction.
                                            //                Value 0x0 to 0xF.
            uint32_t                :4;     // bits 4..7  (R) Reserved.
            uint32_t    PRIV        :1;     // bit  8     (R) Privilege level.
                                            //                This contains the Privilege level used by the EDMA3 programmer to
                                            //                set up the parameter entry in the channel controller.
                                            //                This field is set up when the associated TR is submitted to the
                                            //                EDMA3TC.
                                            //                The privilege ID is used while issuing read and write command to the
                                            //                target endpoints so that the target endpoints can perform memory
                                            //                protection checks based on the PRIV of the host that set up the
                                            //                DMA transaction.
                                            //                [0x0 = User-level privilege
                                            //                 0x1 = Supervisor-level privilege]
            uint32_t                :23;    // bits 9..31 (R) Reserved
        } b;                                // bit access
        uint32_t  reg;                      // raw register
    } DFMPPRXY_reg_t;


    typedef struct
    {
        __R  uint32_t       RESERVED;                      // (0000)
        __RW CCCFG_reg_t     CCCFG;                         // (0004)
        __R  uint32_t       RESERVED_2[2];                 // (0008..000C)
        __RW SYSCONFIG_reg_t SYSCONFIG;                     // (0010)
        __RW ER_reg_t        ER;                            // (0014)
        __R  uint32_t       RESERVED_3[146];               // (0018..025C)
        __RW QDMAQNUM_reg_t  QDMAQNUM;                      // (0260)
        __R  uint32_t       RESERVED_4[8];                 // (0264..0280)
        __RW QUEPRI_reg_t    QUEPRI;                        // (0284)
        __R  uint32_t       RESERVED_5[30];                // (0288..02FC)
        __RW EMR_reg_t       EMR;                           // (0300)
        __RW EMRH_reg_t      EMRH;                          // (0304)
        __RW EMCR_reg_t      EMCR;                          // (0308)
        __RW EMCRH_reg_t     EMCRH;                         // (030C)
        __RW QEMR_reg_t      QEMR;                          // (0310)
        __RW QEMCR_reg_t     QEMCR;                         // (0314)
        __RW CCERR_reg_t     CCERR;                         // (0318)
        __RW CCERRCLR_reg_t  CCERRCLR;                      // (031C)
        __RW EEVAL_reg_t     EEVAL;                         // (0320)
        __R  uint32_t       RESERVED_6[7];                 // (0324..033C)
        __RW DRAE0_reg_t     DRAE0;                         // (0340)
        __RW DRAEH0_reg_t    DRAEH0;                        // (0344)
        __RW DRAE1_reg_t     DRAE1;                         // (0348)
        __RW DRAEH1_reg_t    DRAEH1;                        // (034C)
        __RW DRAE2_reg_t     DRAE2;                         // (0350)
        __RW DRAEH2_reg_t    DRAEH2;                        // (0354)
        __RW DRAE3_reg_t     DRAE3;                         // (0358)
        __RW DRAEH3_reg_t    DRAEH3;                        // (035C)
        __RW DRAE4_reg_t     DRAE4;                         // (0360)
        __RW DRAEH4_reg_t    DRAEH4;                        // (0364)
        __RW DRAE5_reg_t     DRAE5;                         // (0368)
        __RW DRAEH5_reg_t    DRAEH5;                        // (036C)
        __RW DRAE6_reg_t     DRAE6;                         // (0370)
        __RW DRAEH6_reg_t    DRAEH6;                        // (0374)
        __RW DRAE7_reg_t     DRAE7;                         // (0378)
        __RW DRAEH7_reg_t    DRAEH7;                        // (037C)
        __R  uint32_t       RESERVED_7[32];                // (0380..03FC)
        __RW Q0E0_reg_t      Q0E0;                          // (0400)
        __RW Q0E1_reg_t      Q0E1;                          // (0404)
        __RW Q0E2_reg_t      Q0E2;                          // (0408)
        __RW Q0E3_reg_t      Q0E3;                          // (040C)
        __RW Q0E4_reg_t      Q0E4;                          // (0410)
        __RW Q0E5_reg_t      Q0E5;                          // (0414)
        __RW Q0E6_reg_t      Q0E6;                          // (0418)
        __RW Q0E7_reg_t      Q0E7;                          // (041C)
        __RW Q0E8_reg_t      Q0E8;                          // (0420)
        __RW Q0E9_reg_t      Q0E9;                          // (0424)
        __RW Q0E10_reg_t     Q0E10;                         // (0428)
        __RW Q0E11_reg_t     Q0E11;                         // (042C)
        __RW Q0E12_reg_t     Q0E12;                         // (0430)
        __RW Q0E13_reg_t     Q0E13;                         // (0434)
        __RW Q0E14_reg_t     Q0E14;                         // (0438)
        __RW Q0E15_reg_t     Q0E15;                         // (043C)
        __RW Q1E0_reg_t      Q1E0;                          // (0440)
        __RW Q1E1_reg_t      Q1E1;                          // (0444)
        __RW Q1E2_reg_t      Q1E2;                          // (0448)
        __RW Q1E3_reg_t      Q1E3;                          // (044C)
        __RW Q1E4_reg_t      Q1E4;                          // (0450)
        __RW Q1E5_reg_t      Q1E5;                          // (0454)
        __RW Q1E6_reg_t      Q1E6;                          // (0458)
        __RW Q1E7_reg_t      Q1E7;                          // (045C)
        __RW Q1E8_reg_t      Q1E8;                          // (0460)
        __RW Q1E9_reg_t      Q1E9;                          // (0464)
        __RW Q1E10_reg_t     Q1E10;                         // (0468)
        __RW Q1E11_reg_t     Q1E11;                         // (046C)
        __RW Q1E12_reg_t     Q1E12;                         // (0470)
        __RW Q1E13_reg_t     Q1E13;                         // (0474)
        __RW Q1E14_reg_t     Q1E14;                         // (0478)
        __RW Q1E15_reg_t     Q1E15;                         // (047C)
        __RW Q2E0_reg_t      Q2E0;                          // (0480)
        __RW Q2E1_reg_t      Q2E1;                          // (0484)
        __RW Q2E2_reg_t      Q2E2;                          // (0488)
        __RW Q2E3_reg_t      Q2E3;                          // (048C)
        __RW Q2E4_reg_t      Q2E4;                          // (0490)
        __RW Q2E5_reg_t      Q2E5;                          // (0494)
        __RW Q2E6_reg_t      Q2E6;                          // (0498)
        __RW Q2E7_reg_t      Q2E7;                          // (049C)
        __RW Q2E8_reg_t      Q2E8;                          // (04A0)
        __RW Q2E9_reg_t      Q2E9;                          // (04A4)
        __RW Q2E10_reg_t     Q2E10;                         // (04A8)
        __RW Q2E11_reg_t     Q2E11;                         // (04AC)
        __RW Q2E12_reg_t     Q2E12;                         // (04B0)
        __RW Q2E13_reg_t     Q2E13;                         // (04B4)
        __RW Q2E14_reg_t     Q2E14;                         // (04B8)
        __RW Q2E15_reg_t     Q2E15;                         // (04BC)
        __R  uint32_t       RESERVED_8[88];                // (04C0..061C)
        __RW QWMTHRA_reg_t   QWMTHRA;                       // (0620)
        __R  uint32_t       RESERVED_9[7];                 // (0624..063C)
        __RW CCSTAT_reg_t    CCSTAT;                        // (0640)
        __R  uint32_t       RESERVED_10[108];              // (0644..07F0)
        __RW PID_reg_t       PID;                           // (07F4)
        __R  uint32_t       RESERVED_11[2];                // (07F8..07FC)
        __RW MPFAR_reg_t     MPFAR;                         // (0800)
        __RW MPFSR_reg_t     MPFSR;                         // (0804)
        __RW MPFCR_reg_t     MPFCR;                         // (0808)
        __RW MPPAG_reg_t     MPPAG;                         // (080C)
        __R  uint32_t       RESERVED_12[509];              // (0810..1000)
        __RW ERH_reg_t       ERH;                           // (1004)
        __RW ECR_reg_t       ECR;                           // (1008)
        __RW ECRH_reg_t      ECRH;                          // (100C)
        __RW ESR_reg_t       ESR;                           // (1010)
        __RW ESRH_reg_t      ESRH;                          // (1014)
        __RW CER_reg_t       CER;                           // (1018)
        __RW CERH_reg_t      CERH;                          // (101C)
        __RW EER_reg_t       EER;                           // (1020)
        __RW EERH_reg_t      EERH;                          // (1024)
        __RW EECR_reg_t      EECR;                          // (1028)
        __RW EECRH_reg_t     EECRH;                         // (102C)
        __RW EESR_reg_t      EESR;                          // (1030)
        __RW EESRH_reg_t     EESRH;                         // (1034)
        __RW SER_reg_t       SER;                           // (1038)
        __RW SERH_reg_t      SERH;                          // (103C)
        __RW SECR_reg_t      SECR;                          // (1040)
        __RW SECRH_reg_t     SECRH;                         // (1044)


        __R  uint32_t   RESERVED[9];                   // (0000..0020)
        __RW ICR_reg_t   ICR;                           // (0024)
        __R  uint32_t   RESERVED_2[1035];              // (0028..1050)
        __RW IERH_reg_t  IERH;                          // (1054)
        __RW IECR_reg_t  IECR;                          // (1058)
        __RW IECRH_reg_t IECRH;                         // (105C)
        __RW IESR_reg_t  IESR;                          // (1060)
        __RW IESRH_reg_t IESRH;                         // (1064)
        __RW IPR_reg_t   IPR;                           // (1068)
        __RW IPRH_reg_t  IPRH;                          // (106C)
        __R  uint32_t   RESERVED_3;                    // (1070)
        __RW ICRH_reg_t  ICRH;                          // (1074)
        __RW IEVAL_reg_t IEVAL;                         // (1078)
        __R  uint32_t   RESERVED_4;                    // (107C)
        __RW QER_reg_t   QER;                           // (1080)
        __RW QEER_reg_t  QEER;                          // (1084)
        __RW QEECR_reg_t QEECR;                         // (1088)
        __RW QEESR_reg_t QEESR;                         // (108C)
        __RW QSER_reg_t  QSER;                          // (1090)
    } AM335x_EDMA_CC_Type;

    typedef struct
    {
        __RW PID_reg_t       PID;                           // (07F4)
        __RW TCCFG_reg_t     TCCFG;                         // (0004)
        __R  uint32_t       RESERVED_2[2];                 // (0008..000C)
        __RW SYSCONFIG_reg_t SYSCONFIG;                     // (0010)
        __R  uint32_t       RESERVED_3[59];                // (0014..00FC)
        __RW TCSTAT_reg_t    TCSTAT;                        // (0100)
        __R  uint32_t       RESERVED_4[7];                 // (0104..011C)
        __RW ERRSTAT_reg_t   ERRSTAT;                       // (0120)
        __RW ERREN_reg_t     ERREN;                         // (0124)
        __R  uint32_t       RESERVED_5;                    // (0128)
        __RW ERRDET_reg_t    ERRDET;                        // (012C)
        __RW ERRCMD_reg_t    ERRCMD;                        // (0130)
        __R  uint32_t       RESERVED_6[3];                 // (0134..013C)
        __RW RDRATE_reg_t    RDRATE;                        // (0140)
        __R  uint32_t       RESERVED_7[63];                // (0144..023C)
        __RW SAOPT_reg_t     SAOPT;                         // (0240)
        __RW SASRC_reg_t     SASRC;                         // (0244)
        __RW SACNT_reg_t     SACNT;                         // (0248)
        __RW SADST_reg_t     SADST;                         // (024C)
        __RW SABIDX_reg_t    SABIDX;                        // (0250)
        __RW SAMPPRXY_reg_t  SAMPPRXY;                      // (0254)
        __RW SACNTRLD_reg_t  SACNTRLD;                      // (0258)
        __RW SASRCBREF_reg_t SASRCBREF;                     // (025C)
        __RW SADSTBREF_reg_t SADSTBREF;                     // (0260)
        __R  uint32_t       RESERVED_8[7];                 // (0264..027C)
        __RW DFCNTRLD_reg_t  DFCNTRLD;                      // (0280)
        __RW DFSRCBREF_reg_t DFSRCBREF;                     // (0284)
        __RW DFDSTBREF_reg_t DFDSTBREF;                     // (0288)
        __R  uint32_t       RESERVED_9[29];                // (028C..02FC)
        __RW DFOPT0_reg_t    DFOPT0;                        // (0300)
        __RW DFSRC0_reg_t    DFSRC0;                        // (0304)
        __RW DFCNT0_reg_t    DFCNT0;                        // (0308)
        __RW DFDST0_reg_t    DFDST0;                        // (030C)
        __RW DFBIDX0_reg_t   DFBIDX0;                       // (0310)
        __RW DFMPPRXY0_reg_t DFMPPRXY0;                     // (0314)
        __R  uint32_t        RESERVED_10;                   // (0318)
        __RW ERRCLR_reg_t    ERRCLR;                        // (031C)
        __R  uint32_t        RESERVED_11[8];                // (0320..033C)
        __RW DFOPT1_reg_t    DFOPT1;                        // (0340)
        __RW DFSRC1_reg_t    DFSRC1;                        // (0344)
        __RW DFCNT1_reg_t    DFCNT1;                        // (0348)
        __RW DFDST1_reg_t    DFDST1;                        // (034C)
        __RW DFBIDX1_reg_t   DFBIDX1;                       // (0350)
        __RW DFMPPRXY1_reg_t DFMPPRXY1;                     // (0354)
        __R  uint32_t        RESERVED_12[10];               // (0358..037C)
        __RW DFOPT2_reg_t    DFOPT2;                        // (0380)
        __RW DFSRC2_reg_t    DFSRC2;                        // (0384)
        __RW DFCNT2_reg_t    DFCNT2;                        // (0388)
        __RW DFDST2_reg_t    DFDST2;                        // (038C)
        __RW DFBIDX2_reg_t   DFBIDX2;                       // (0390)
        __RW DFMPPRXY2_reg_t DFMPPRXY2;                     // (0394)
        __R  uint32_t        RESERVED_13[10];               // (0398..03BC)
        __RW DFOPT3_reg_t    DFOPT3;                        // (03C0)
        __RW DFSRC3_reg_t    DFSRC3;                        // (03C4)
        __RW DFCNT3_reg_t    DFCNT3;                        // (03C8)
        __RW DFDST3_reg_t    DFDST3;                        // (03CC)
        __RW DFBIDX3_reg_t   DFBIDX3;                       // (03D0)
        __R  uint32_t        RESERVED_14[264];              // (03D4..07F0)
    } AM335x_EDMA_TC_Type;

    constexpr uint32_t EDMA_CC_BASE = 0x49000000;
    constexpr AM335x_EDMA_CC_Type * EDMA_CC = ((AM335x_EDMA_CC_Type *) EDMA_CC_BASE);

}

#endif //__EDMA__HPP