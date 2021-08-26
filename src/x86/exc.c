/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <x86/exc.h>
#include <mutex.h>
#include <io.h>
// kernel default process
extern process_t* __shell;
extern idt_entry_t __idt[];
exc_t __exc_table[EXC_TABLE_SIZE];
extern void exc0(); extern void exc1();
extern void exc2(); extern void exc3();
extern void exc4(); extern void exc5();
extern void exc6(); extern void exc7();
extern void exc8(); extern void exc9();
extern void exc10(); extern void exc11();
extern void exc12(); extern void exc13();
extern void exc14(); extern void exc15();
extern void exc16(); extern void exc17();
extern void exc18(); extern void exc19();
extern void exc20(); extern void exc21();
extern void exc22(); extern void exc23();
extern void exc24(); extern void exc25();
extern void exc26(); extern void exc27();
extern void exc28(); extern void exc29();
extern void exc30(); extern void exc31();

void set_exc_handler(u32_t __handler, u8_t __enum){
    if(__enum >= EXC_TABLE_SIZE || __enum < 0)
        return;
    __idt[__enum].offset0_15 = (u16_t)(__handler & 0x0000ffff);
    __idt[__enum].offset16_31 = (u16_t)((__handler >> 16) & 0x0000ffff);
    __idt[__enum].flags = INTGATE;
}

void register_execption_handler(exc_t __exc, u8_t __num){
	if(__num >= EXC_TABLE_SIZE || __num < 0)
		return;
	__exc_table[__num] = __exc;
}

void final_exc_handler(registers_t* __regs){
	if(!__exc_table[__regs->intno])
		return;
	exc_t __handler = __exc_table[__regs->intno];
	__handler(__regs);
}

void exc_divide_by_zero(registers_t* __regs){
	kprintf("arithmetic exception: Divide by 0\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);
}

void exc_debug(registers_t* __regs){
	kprintf("debug exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_nmi(registers_t* __regs){
	kprintf("nmi exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);		
}

void exc_brp(registers_t* __regs){
	kprintf("breakpoint exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_overflow(registers_t* __regs){
	kprintf("overflow exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_bound(registers_t* __regs){
	kprintf("bound exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_invopcode(registers_t* __regs){
	kprintf("invalid opcode exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_device_not_avail(registers_t* __regs){
	kprintf("device not avail exception\n");
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_double_fault(registers_t* __regs){
	kprintf("double fault exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_coproc(registers_t* __regs){
	kprintf("coprocessor exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_invtss(registers_t* __regs){
	kprintf("invalid tss exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_segment_not_present(registers_t* __regs){
	kprintf("segment not present exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_ssf(registers_t* __regs){
	kprintf("stack segment exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_gpf(registers_t* __regs){
	kprintf("general protection fault \n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_pf(registers_t* __regs){
	// get error code on the stack
	int __error = 0;
	int * __ebp = NULL;
	__asm__ __volatile__("movl %%ebp, %0":"=m"(__ebp));
	__error = __ebp[1];
	int __addr = 0;
	__asm__ __volatile__("movl %%cr2, %0":"=a"(__addr));

	// 
	if(__addr < USER_OFFSET || __addr >= SHM_UP)
		kprintf("segmentation fault: @%x\n",__addr);
	else{
		if(!(__error & PAGE_PRESENT) || (__error & PAGE_READ_WRITE) || (__error & PAGE_USER_SUPERUSER))
			kprintf("core dumped: @%x\n",__addr);
	}
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void exc_reserved(registers_t* __regs){
	kprintf("reserved exception\n");
	// send SIGKILL to process
	process_kill(process_pid(),SIGKILL);
	// switch to __sehll process
	switch_process(__shell);	
}

void init_exc(){
	kprintf("Installing Exception...");
	set_exc_handler((u32_t)exc0,0x0); register_execption_handler(exc_divide_by_zero,0x0);
	set_exc_handler((u32_t)exc1,0x1); register_execption_handler(exc_debug,0x01);
	set_exc_handler((u32_t)exc2,0x2); register_execption_handler(exc_nmi,0x02);
	set_exc_handler((u32_t)exc3,0x3); register_execption_handler(exc_brp,0x03);
	set_exc_handler((u32_t)exc4,0x4); register_execption_handler(exc_overflow,0x04);
	set_exc_handler((u32_t)exc5,0x5); register_execption_handler(exc_bound,0x05);
	set_exc_handler((u32_t)exc6,0x6); register_execption_handler(exc_invopcode,0x06);
	set_exc_handler((u32_t)exc7,0x7); register_execption_handler(exc_device_not_avail,0x07);
	set_exc_handler((u32_t)exc8,0x8); register_execption_handler(exc_double_fault,0x08);
	set_exc_handler((u32_t)exc9,0x9); register_execption_handler(exc_coproc,0x09);
	set_exc_handler((u32_t)exc10,0xa); register_execption_handler(exc_invtss,0x0a);
	set_exc_handler((u32_t)exc11,0xb); register_execption_handler(exc_segment_not_present,0x0b);
	set_exc_handler((u32_t)exc12,0xc); register_execption_handler(exc_ssf,0x0c);
	set_exc_handler((u32_t)exc13,0xd); register_execption_handler(exc_gpf,0x0d);
	set_exc_handler((u32_t)exc14,0xe); register_execption_handler(exc_pf,0x0e);
	set_exc_handler((u32_t)exc15,0xf); register_execption_handler(exc_reserved,0x0f);
	set_exc_handler((u32_t)exc16,0x10);
	set_exc_handler((u32_t)exc17,0x11);
	set_exc_handler((u32_t)exc18,0x12);
	set_exc_handler((u32_t)exc19,0x13);
	set_exc_handler((u32_t)exc20,0x14);
	set_exc_handler((u32_t)exc21,0x15);
	set_exc_handler((u32_t)exc22,0x16);
	set_exc_handler((u32_t)exc23,0x17);
	set_exc_handler((u32_t)exc24,0x18);
	set_exc_handler((u32_t)exc25,0x19);
	set_exc_handler((u32_t)exc26,0x1a);
	set_exc_handler((u32_t)exc27,0x1b);
	set_exc_handler((u32_t)exc28,0x1c);
	set_exc_handler((u32_t)exc29,0x1d);
	set_exc_handler((u32_t)exc30,0x1e);
	set_exc_handler((u32_t)exc31,0x1f);
	kprintf("[ok]\n");
}
