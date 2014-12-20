//
// uspios.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
//
// External dependencies made by Marco Maccaferri <macca@maccasoft.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <uspi/synchronize.h>
#include <uspi/types.h>
#include <uspi/assert.h>
#include <uspi/bcm2835.h>
#include <uspios.h>

//
// Mailbox
//
#define MAIL_BASE   (ARM_IO_BASE +   0xB880) // Mailbox Base Address

#define MAIL_READ      0x00 // Mailbox Read Register
#define MAIL_CONFIG    0x1C // Mailbox Config Register
#define MAIL_STATUS    0x18 // Mailbox Status Register
#define MAIL_WRITE     0x20 // Mailbox Write Register

#define MAIL_EMPTY  0x40000000 // Mailbox Status Register: Mailbox Empty (There is nothing to read from the Mailbox)
#define MAIL_FULL   0x80000000 // Mailbox Status Register: Mailbox Full  (There is no space to write into the Mailbox)

#define MAIL_POWER    0x0 // Mailbox Channel 0: Power Management Interface
#define MAIL_FB       0x1 // Mailbox Channel 1: Frame Buffer
#define MAIL_VUART    0x2 // Mailbox Channel 2: Virtual UART
#define MAIL_VCHIQ    0x3 // Mailbox Channel 3: VCHIQ Interface
#define MAIL_LEDS     0x4 // Mailbox Channel 4: LEDs Interface
#define MAIL_BUTTONS  0x5 // Mailbox Channel 5: Buttons Interface
#define MAIL_TOUCH    0x6 // Mailbox Channel 6: Touchscreen Interface
#define MAIL_COUNT    0x7 // Mailbox Channel 7: Counter
#define MAIL_TAGS     0x8 // Mailbox Channel 8: Tags (ARM to VC)

#define TAG_SET_POWER_STATE     0x00028001 // Power: Set Power State (Response: Device ID, State)

#define POWER_STATE_OFF     (0 << 0)
#define POWER_STATE_ON      (1 << 0)

//
// Power: Unique Device ID's
//
#define PWR_USB_HCD_ID  0x3 // USB HCD

//
// System Timers
//
#define ARM_SYSTIMER_BASE   (ARM_IO_BASE + 0x3000)

#define ARM_SYSTIMER_CS     (ARM_SYSTIMER_BASE + 0x00)
#define ARM_SYSTIMER_CLO    (ARM_SYSTIMER_BASE + 0x04)
#define ARM_SYSTIMER_CHI    (ARM_SYSTIMER_BASE + 0x08)
#define ARM_SYSTIMER_C0     (ARM_SYSTIMER_BASE + 0x0C)
#define ARM_SYSTIMER_C1     (ARM_SYSTIMER_BASE + 0x10)
#define ARM_SYSTIMER_C2     (ARM_SYSTIMER_BASE + 0x14)
#define ARM_SYSTIMER_C3     (ARM_SYSTIMER_BASE + 0x18)

//
// Interrupt Controller
//
#define ARM_IC_BASE     (ARM_IO_BASE + 0xB000)

#define ARM_IC_IRQ_BASIC_PENDING  (ARM_IC_BASE + 0x200)
#define ARM_IC_IRQ_PENDING_1      (ARM_IC_BASE + 0x204)
#define ARM_IC_IRQ_PENDING_2      (ARM_IC_BASE + 0x208)
#define ARM_IC_FIQ_CONTROL    (ARM_IC_BASE + 0x20C)
#define ARM_IC_ENABLE_IRQS_1      (ARM_IC_BASE + 0x210)
#define ARM_IC_ENABLE_IRQS_2      (ARM_IC_BASE + 0x214)
#define ARM_IC_ENABLE_BASIC_IRQS  (ARM_IC_BASE + 0x218)
#define ARM_IC_DISABLE_IRQS_1     (ARM_IC_BASE + 0x21C)
#define ARM_IC_DISABLE_IRQS_2     (ARM_IC_BASE + 0x220)
#define ARM_IC_DISABLE_BASIC_IRQS (ARM_IC_BASE + 0x224)

#define ARM_IRQS_PER_REG    32

#define ARM_IRQ1_BASE       0
#define ARM_IRQ2_BASE       (ARM_IRQ1_BASE + ARM_IRQS_PER_REG)
#define ARM_IRQBASIC_BASE   (ARM_IRQ2_BASE + ARM_IRQS_PER_REG)

#define ARM_IRQ_TIMER3      (ARM_IRQ1_BASE + 3)
#define ARM_IRQ_USB         (ARM_IRQ1_BASE + 9)

#define IRQ_LINES       (ARM_IRQS_PER_REG * 2 + 8)

#define ARM_IC_IRQ_PENDING(irq) (  (irq) < ARM_IRQ2_BASE    \
                 ? ARM_IC_IRQ_PENDING_1     \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_IRQ_PENDING_2   \
                   : ARM_IC_IRQ_BASIC_PENDING))
#define ARM_IC_IRQS_ENABLE(irq) (  (irq) < ARM_IRQ2_BASE    \
                 ? ARM_IC_ENABLE_IRQS_1     \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_ENABLE_IRQS_2   \
                   : ARM_IC_ENABLE_BASIC_IRQS))
#define ARM_IC_IRQS_DISABLE(irq) (  (irq) < ARM_IRQ2_BASE   \
                 ? ARM_IC_DISABLE_IRQS_1    \
                 : ((irq) < ARM_IRQBASIC_BASE   \
                   ? ARM_IC_DISABLE_IRQS_2  \
                   : ARM_IC_DISABLE_BASIC_IRQS))
#define ARM_IRQ_MASK(irq)   (1 << ((irq) & (ARM_IRQS_PER_REG-1)))

static volatile unsigned s_nCriticalLevel = 0;
static volatile boolean s_bWereEnabled;

static unsigned int mmio_read(unsigned int reg) {
    DataMemBarrier();
    return *(volatile unsigned int *) (reg);
}

static void mmio_write(unsigned int reg, unsigned int data) {
    DataMemBarrier();
    *(volatile unsigned int *) (reg) = data;
    DataMemBarrier();
}

static unsigned int read32(unsigned int reg) {
    return *(volatile unsigned int *) (reg);
}

static void write32(unsigned int reg, unsigned int data) {
    *(volatile unsigned int *) (reg) = data;
}

static void mbox_write(unsigned char channel, unsigned int data) {
    while (mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_FULL)
        ;
    mmio_write(MAIL_BASE + MAIL_WRITE, (data & 0xfffffff0) | (unsigned int) (channel & 0xf));
}

static unsigned int mbox_read(unsigned char channel) {
    while (1) {
        while (mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_EMPTY)
            ;

        unsigned int data = mmio_read(MAIL_BASE + MAIL_READ);
        unsigned char read_channel = (unsigned char) (data & 0xf);
        if (read_channel == channel) {
            return (data & 0xfffffff0);
        }
    }
}

int SetPowerStateOn (unsigned nDeviceId)
{
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    mailbuffer[0] = 8 * 4;              // size of this message
    mailbuffer[1] = 0;                  // this is a request

    mailbuffer[2] = TAG_SET_POWER_STATE;
    mailbuffer[3] = 8;                  // value buffer size
    mailbuffer[4] = 8;                  // request/response
    mailbuffer[5] = PWR_USB_HCD_ID;     // device id
    mailbuffer[6] = POWER_STATE_ON;     // power state

    mailbuffer[7] = 0;
    mbox_write(MAIL_TAGS, mb_addr);

    mbox_read(MAIL_TAGS);
    if (mailbuffer[1] == MAIL_FULL) {
        return 1;
    }

    return 0;
}

int GetMACAddress (unsigned char Buffer[6])
{
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    /* Get the display size */
    mailbuffer[0] = 8 * 4;              // size of this message
    mailbuffer[1] = 0;                  // this is a request

    mailbuffer[2] = 0x00010003;         // get MAC address tag
    mailbuffer[3] = 6;                  // value buffer size
    mailbuffer[4] = 0;                  // request/response
    mailbuffer[5] = 0;                  // space to return value
    mailbuffer[6] = 0;

    mailbuffer[7] = 0;
    mbox_write(MAIL_TAGS, mb_addr);

    mbox_read(MAIL_TAGS);
    if (mailbuffer[1] == MAIL_FULL) {
        volatile unsigned char *ptr = (unsigned char *)&mailbuffer[5];
        Buffer[0] = ptr[0];
        Buffer[1] = ptr[1];
        Buffer[2] = ptr[2];
        Buffer[3] = ptr[3];
        Buffer[4] = ptr[4];
        Buffer[5] = ptr[5];
        return 1;
    }

    return 0;
}

void usDelay (unsigned nMicroSeconds)
{
    unsigned int cur_timer = mmio_read(ARM_SYSTIMER_CLO);
    unsigned int trigger_value = cur_timer + nMicroSeconds;
    unsigned int rollover;

    if (nMicroSeconds > 0)
    {
        if (trigger_value > cur_timer)
            rollover = 0;
        else
            rollover = 1;

        for (;;) {
            cur_timer = mmio_read(ARM_SYSTIMER_CLO);
            if (cur_timer < trigger_value) {
                if (rollover) {
                    rollover = 0;
                }
            }
            else if (!rollover) {
                break;
            }
        }
    }
}

void MsDelay (unsigned nMilliSeconds)
{
    usDelay(nMilliSeconds * 1000);
}

static TInterruptHandler *m_apIRQHandler[IRQ_LINES];
static void              *m_pParam[IRQ_LINES];

static void EnableIRQ (unsigned nIRQ)
{
    mmio_write (ARM_IC_IRQS_ENABLE (nIRQ), ARM_IRQ_MASK (nIRQ));
}

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
    assert (nIRQ < IRQ_LINES);
    assert (m_apIRQHandler[nIRQ] == 0);

    m_apIRQHandler[nIRQ] = pHandler;
    m_pParam[nIRQ] = pParam;

    EnableIRQ (nIRQ);
}

void USPiInterruptHandler (void)
{
    u32 nPendReg = ARM_IC_IRQ_PENDING (ARM_IRQ_USB);
    u32 nIRQMask = ARM_IRQ_MASK (ARM_IRQ_USB);

    if (read32 (nPendReg) & nIRQMask)
    {
        TInterruptHandler *pHandler = m_apIRQHandler[ARM_IRQ_USB];
        if (pHandler != 0)
        {
            (*pHandler) (m_pParam[ARM_IRQ_USB]);
        }
    }


    nPendReg = ARM_IC_IRQ_PENDING (ARM_IRQ_TIMER3);
    nIRQMask = ARM_IRQ_MASK (ARM_IRQ_TIMER3);

    if (read32 (nPendReg) & nIRQMask)
    {
        TInterruptHandler *pHandler = m_apIRQHandler[ARM_IRQ_TIMER3];
        if (pHandler != 0)
        {
            (*pHandler) (m_pParam[ARM_IRQ_TIMER3]);
        }
    }
}

#define KERNEL_TIMERS       20
#define CLOCKHZ             1000000

typedef struct
{
    TKernelTimerHandler *m_pHandler;
    unsigned         m_nElapsesAt;
    void            *m_pParam;
    void            *m_pContext;
} TKernelTimer;

static volatile unsigned     m_nTicks;
static volatile unsigned     m_nTime;
static volatile TKernelTimer m_KernelTimer[KERNEL_TIMERS];  // TODO: should be linked list

static void TimerInterruptHandler (void *pParam)
{
    DataMemBarrier ();

    assert (read32 (ARM_SYSTIMER_CS) & (1 << 3));

    u32 nCompare = read32 (ARM_SYSTIMER_C3) + CLOCKHZ / HZ;
    if (nCompare < read32 (ARM_SYSTIMER_CLO))            // time may drift
    {
        nCompare = read32 (ARM_SYSTIMER_CLO) + CLOCKHZ / HZ;
    }
    write32 (ARM_SYSTIMER_C3, nCompare);

    write32 (ARM_SYSTIMER_CS, 1 << 3);

    DataMemBarrier ();

    if (++m_nTicks % HZ == 0)
    {
        m_nTime++;
    }

    uspi_EnterCritical ();

    for (unsigned hTimer = 0; hTimer < KERNEL_TIMERS; hTimer++)
    {
        volatile TKernelTimer *pTimer = &m_KernelTimer[hTimer];

        TKernelTimerHandler *pHandler = pTimer->m_pHandler;
        if (pHandler != 0)
        {
            if ((int) (pTimer->m_nElapsesAt-m_nTicks) <= 0)
            {
                pTimer->m_pHandler = 0;

                (*pHandler) (hTimer+1, pTimer->m_pParam, pTimer->m_pContext);
            }
        }
    }

    uspi_LeaveCritical ();
}

unsigned TimerInitialize (void)
{
    for (unsigned hTimer = 0; hTimer < KERNEL_TIMERS; hTimer++)
    {
        m_KernelTimer[hTimer].m_pHandler = 0;
    }

    ConnectInterrupt(ARM_IRQ_TIMER3, TimerInterruptHandler, 0);
    mmio_write (ARM_SYSTIMER_C3, mmio_read (ARM_SYSTIMER_CLO) + CLOCKHZ / HZ);

    return 1;
}

unsigned StartKernelTimer (unsigned nDelay,
                   TKernelTimerHandler *pHandler,
                   void *pParam,
                   void *pContext)
{
    uspi_EnterCritical ();

    unsigned hTimer;
    for (hTimer = 0; hTimer < KERNEL_TIMERS; hTimer++)
    {
        if (m_KernelTimer[hTimer].m_pHandler == 0)
        {
            break;
        }
    }

    if (hTimer >= KERNEL_TIMERS)
    {
        uspi_LeaveCritical ();

        return 0;
    }

    assert (pHandler != 0);
    m_KernelTimer[hTimer].m_pHandler    = pHandler;
    m_KernelTimer[hTimer].m_nElapsesAt  = m_nTicks+nDelay;
    m_KernelTimer[hTimer].m_pParam      = pParam;
    m_KernelTimer[hTimer].m_pContext    = pContext;

    uspi_LeaveCritical ();

    return hTimer+1;
}

void CancelKernelTimer (unsigned hTimer)
{
    assert (1 <= hTimer && hTimer <= KERNEL_TIMERS);
    m_KernelTimer[hTimer-1].m_pHandler = 0;
}
