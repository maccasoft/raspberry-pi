/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

.global _start

.global dmb
.global dsb
.global flush_cache

_start:
    /* kernel.img is loaded at 0x8000
     *
     * 0x2c00 - 0x3c00  User/system stack
     * 0x2800 - 0x2c00  IRQ stack
     * 0x2400 - 0x2800  Abort stack
     * 0x2000 - 0x2400  Supervisor (SWI/SVC) stack
     *
     * All stacks grow down; decrement then store
     *
     * Stack addresses are stored in the stack pointers as
     * 0x80000000+address, as this means the stack pointer doesn't have
     * to change when the MMU is turned on (before the MMU is on, accesses
     * to 0x80000000 go to 0x00000000, and so on). Eventually, the stacks
     * will be given a proper home
     */

    mov     r4, #0x80000000

    /* SVC stack (for SWIs) at 0x2000 */
    /* The processor appears to start in this mode, but change to it
     * anyway
     */
    cps     #0x13       /* Change to supervisor (SVC) mode */
    add     sp, r4, #0x2400

    /* ABORT stack at 0x2400 */
    cps     #0x17       /* Change to Abort mode */
    add     sp, r4, #0x2800

    /* IRQ stack at 0x2800 */
    cps     #0x12       /* Change to IRQ mode */
    ldr     sp, =__irq_stack_top__

    /* System stack at 0x2c00 */
    cps     #0x1f       /* Change to system mode */
    ldr     sp, =__c_stack_top__

    /* Stay in system mode from now on */

    /* Zero bss section */
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    mov     r2, #0
bss_zero_loop:
    cmp     r0,r1
    it      lt
    strlt   r2,[r0], #4
    blt     bss_zero_loop

    /* Enable the FPU */
    mrc     p15, 0, r0, c1, c0, 2
    orr     r0, r0, #0x300000            /* single precision */
    orr     r0, r0, #0xC00000            /* double precision */
    mcr     p15, 0, r0, c1, c0, 2
    mov     r0, #0x40000000
    fmxr    fpexc, r0

    /* Turn on unaligned memory access */
    mrc     p15, #0, r4, c1, c0, #0
    orr     r4, #0x400000   /* 1<22 */
    mcr     p15, #0, r4, c1, c0, #0

    /* Enable MMU */
    ldr     r1, =ttbr0              // addr(TTBR0)

    ldr     r2, =0x0000040E
    mov     r3, #0                  // from 0x00000000
    mov     r4, #0x200              //   to 0x1FFFFFFF
    bl      set_pgtbl_entry

    ldr     r2, =0x00002416
    mov     r3, #0x200              // from 0x20000000 (incl. peripherals)
    mov     r4, #0x1000             //   to 0xFFFFFFFF
    bl      set_pgtbl_entry

    ldr     r2, =0x0000040E
    mov     r3, #0x480              // framebuffer at 0x48000000
    mov     r4, #0x490              // make 16 Mbyte cacheable
    bl      set_pgtbl_entry

    mov     r3, #3
    mcr     p15, #0, r3, c3, c0, #0 // set domain 0 to master

    mcr     p15, #0, r1, c2, c0, #0 // set TTBR0 (addr of ttbr0)  (ptblwlk inner non cacheable,
                                    // outer non-cacheable, not shareable memory)
    /* Start L1 Cache */
    mov     r3, #0
    mcr     p15, #0, r3, c7, c7, #0 /* Invalidate data cache and flush prefetch buffer */
    mcr     p15, #0, r3, c8, c7, #0 /* Invalidate TLB */
    mrc     p15, #0, r2, c1, c0, #0 /* Read Control Register Configuration Data */
    orr     r2, #0x00800000
    orr     r2, #0x00001000         /* Instruction */
    orr     r2, #0x00000800         /* Branch Prediction */
    orr     r2, #0x00000004         /* Data */
    orr     r2, #0x00000001
    mcr     p15, #0, r2, c1, c0, #0 /* Write Control Register Configuration Data */

    /* Enable interrupts */
    ldr     r4, =interrupt_vectors
    mcr     p15, #0, r4, c12, c0, #0
    cpsie   i

    /* Call constructors of all global objects */
    ldr     r0, =__init_array_start
    ldr     r1, =__init_array_end
globals_init_loop:
    cmp     r0, r1
    it      lt
    ldrlt   r2, [r0], #4
    blxlt   r2
    blt     globals_init_loop

    /* Jump to main */
    bl      main

    /* Hang if main function returns */
hang:
    b       hang

set_pgtbl_entry:
    lsl     r0, r3, #20             // = r3 * 0x100000 (1M)
    orr     r0, r2
    str     r0, [r1, r3, lsl #2]
    add     r3, #1
    cmp     r3, r4
    bne     set_pgtbl_entry
    mov     pc, lr

/*
 * Data memory barrier
 * No memory access after the DMB can run until all memory accesses before it
 * have completed
 */
dmb:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c10, #5
    mov     pc, lr

/*
 * Data synchronisation barrier
 * No instruction after the DSB can run until all instructions before it have
 * completed
 */
dsb:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c10, #4
    mov     pc, lr

/*
 * Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
flush_cache:
    mov     r0, #0
    mcr     p15, #0, r0, c7, c14, #0
    mov     pc, lr

/*
 * Interrupt vectors table
 */
    .align  5
interrupt_vectors:
    b       bad_exception /* RESET */
    b       bad_exception /* UNDEF */
    b       interrupt_swi
    b       interrupt_prefetch_abort
    b       interrupt_data_abort
    b       bad_exception /* Unused vector */
    b       interrupt_irq
    b       bad_exception /* FIQ */

    .section .data

    .align 14
ttbr0:
    .space  4 << 12                        // 4 bytes * 4096 entries
