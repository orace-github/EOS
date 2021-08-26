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


#include <driver/ata.h>
static u16_t ide[256];
static prd_t bus0prd __attribute__((aligned(sizeof(u32_t))));
static prd_t bus1prd __attribute__((aligned(sizeof(u32_t))));

// kernel data
static ata_t* __hd = NULL;
static struct list __atalist;

ata_t* new_ata(u16_t __bus, u16_t __drive, u16_t __subcode, u16_t __basecode)
{
    ata_t* __atadev = NULL;

    // find ata device by __bus
    // and __drive
    struct list_head* __it = NULL;
    list_for_each(__it,&__atalist.head)
    {
        __atadev = list_entry(__it,ata_t,head);
        if(__atadev->bus == __bus && __atadev->drive == __drive)
        {
            __hd = __atadev;
            return __hd;
        }
    }

    // whether find failed then
    // create new ata device
    __atadev = sys_alloc(sizeof(ata_t));
    __atadev->bus = __bus;
    __atadev->drive = __drive;
    __atadev->pcidev = find_pcidev(__subcode,__basecode,0,0,PCI_FIND_BY_CLASS);
    if(!__atadev->pcidev)
    {
        __hd = NULL;
        return __hd;
    }
    // check bus line
    if(!__bus)
        __atadev->io_base = ATA_PRIMARY_IO;
    else
        __atadev->io_base = ATA_SECONDARY_IO;
     // enable ata device pci bus
    write_pcidev(__atadev->pcidev,COMMAND_REG_OFF,0x05);
    // registering bus master register  
    __atadev->bmr = read_pcidev(__atadev->pcidev,BUS_MASTER_REG_OFF);
    __atadev->bmr &= ~1;
    // insert to ata device list
    list_insert(&__atalist,&__atadev->head);
    // make as current ata device
    __hd = __atadev;
    return __hd;
}

ata_t* find_ata(u16_t __bus, u16_t __drive)
{
    struct list_head* __it = NULL;
    list_for_each(__it,&__atalist.head)
    {
        ata_t* __atadev = list_entry(__it,ata_t,head);
        if(__atadev->bus == __bus && __atadev->drive == __drive)
            return __atadev;
    }

    return NULL;
}

void ata_delay()
{
    // poll while ATA controller is busy
    while(inb(__hd->io_base+ATA_REG_STATUS) & ATA_SR_BSY);
}

void ata_reset(u16_t __io)
{
    outb(__io+ATA_REG_CONTROL,0x04);
    ide_400ns_delay(__io);
    outb(__io+ATA_REG_CONTROL,0x00);
}

void ide_select_drive(u8_t __bus, u8_t __drive)
{
    // select ata device
    if(__bus == ATA_PRIMARY)
        if(__drive == ATA_MASTER)
            outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL,0xa0);
        else  
            outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL,0xb0);
    else
        if(__drive == ATA_MASTER)
            outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL,0xa0);
        else
            outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL,0xb0);
}

u8_t ide_identify(u16_t __bus, u16_t __drive)
{
    u16_t __io = 0;
    if(__bus == ATA_PRIMARY)
        __io = ATA_PRIMARY_IO;
    else
        __io = ATA_SECONDARY_IO;

    ata_reset(__io);
    ide_400ns_delay(__io);
    // select ata device
    ide_select_drive(__bus,__drive);
    outb(__io+ATA_REG_SECCOUNT0,0);
    outb(__io+ATA_REG_LBA0,0);
    outb(__io+ATA_REG_LBA1,0);
    outb(__io+ATA_REG_LBA2,0);

    /* Now send identify */
    outb(__io+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);

    /*Now read status port */
    u8_t __status = inb(__io+ATA_REG_STATUS);

    if(__status)
    {
        while(inb(__io+ATA_REG_STATUS) & ATA_SR_BSY !=0);
    loop:   __status = inb(__io+ATA_REG_STATUS);
        if(__status & ATA_SR_ERR)
            return 0;

        while(!(__status & ATA_SR_DRQ)) goto loop;
        /* Now read data */
        for(int i = 0; i < 256; i++) 
            ide[i] = inw(__io+ATA_REG_DATA);
        return 1;
    }

    return 0;
}


void ide_400ns_delay(u16_t __io)
{
    // wait for 400 ns
    for(int i = 0; i < 4; i++)
        inb(__io+ATA_REG_ALTSTATUS);
}

void ide_poll(u16_t __io)
{
    ide_400ns_delay(__io);
    retry:;
    u8_t __status = inb(__io+ATA_REG_STATUS);
    if(__status & ATA_SR_BSY)
		goto retry;

    retry2: __status = inb(__io+ATA_REG_STATUS);
    if(__status & ATA_SR_ERR)
    {
        #ifdef __DEBUG__
            kprintf("ATA STATUS ERR\n");
        #endif // __DEBUG__
    }

    if(!(__status & ATA_SR_DRQ))
        goto retry2;
    
    return;
}

void __isr14(registers_t* __regs){
    inb(__hd->bmr+ATA_BMS_SR_1);
    inb(__hd->io_base+ATA_REG_STATUS);
    outb(__hd->bmr+ATA_BMS_CR_1,0x0);
}

void __isr15(registers_t* __regs)
{
    inb(__hd->bmr+ATA_BMS_SR_2);
    inb(__hd->io_base+ATA_REG_STATUS);
    outb(__hd->bmr+ATA_BMS_CR_2,0x0);
}

/* 48 bit adressage mode */
int ide_read_write(char* __buf, u32_t __lba, u8_t __read){// read == 1 --> disk_write
                                                                          // read == 0 --> disk_read
    if(!__hd)
    {
        #ifdef __DEBUG__
            kprintf("PANIC: ATA DEVICE NOT FOUND.\n");
        #endif //__DEBUG__
        return 0;
    }

    u8_t __cmd = (__hd->drive == ATA_MASTER) ? 0x40 : 0x50;

    /* Now reset bus master command register */
    if(__hd->bus == ATA_PRIMARY)
        outb(__hd->bmr + ATA_BMS_CR_1,0x0);
    else
        outb(__hd->bmr + ATA_BMS_CR_2,0x0);

    /* Now setup prd */
    if(__hd->bus == ATA_PRIMARY)
    {
        bus0prd.phy_addr = (u32_t)__buf;
        bus0prd.bytes = 512;
        bus0prd.reserved = (1<<15);
        outl(__hd->bmr+ATA_BMS_PRD_R_1,(u32_t)&bus0prd);
    }else
    {
        bus1prd.phy_addr = (u32_t)__buf;
        bus1prd.bytes = 512;
        bus1prd.reserved = (1<<15);
        outl(__hd->bmr+ATA_BMS_PRD_R_2,(u32_t)&bus1prd);
    }

    /* Now setup sector location */
    outb(__hd->io_base + ATA_REG_HDDEVSEL,__cmd);
    outb(__hd->io_base + ATA_REG_SECCOUNT0,(1 >> 8) & 0xff);
    outb(__hd->io_base + ATA_REG_LBA0,(__lba>>24) & 0xff);
    outb(__hd->io_base + ATA_REG_LBA1,0);
    outb(__hd->io_base + ATA_REG_LBA2,0);

    outb(__hd->io_base + ATA_REG_SECCOUNT0,1 & 0xff);
    outb(__hd->io_base + ATA_REG_LBA0, __lba & 0xff);
    outb(__hd->io_base + ATA_REG_LBA1,(__lba >> 8) & 0xff);
    outb(__hd->io_base + ATA_REG_LBA2,(__lba >> 16) & 0xff);

    if(__read)
        outb(__hd->io_base + ATA_REG_COMMAND,ATA_CMD_READ_DMA_EXT);
    else
        outb(__hd->io_base + ATA_REG_COMMAND,ATA_CMD_WRITE_DMA_EXT);

    /* Now select operation direction */
    if(__hd->bus == ATA_PRIMARY)
        if(__read)
            outb(__hd->bmr+ATA_BMS_CR_1,ATA_DMA_READ|0x01);
        else
            outb(__hd->bmr+ATA_BMS_CR_1,ATA_DMA_WRITE|0x01);
    else
        if(__read)
            outb(__hd->bmr+ATA_BMS_CR_2,ATA_DMA_READ|0x01);
        else
            outb(__hd->bmr+ATA_BMS_CR_2,ATA_DMA_WRITE|0x01);

    // wait until ata controller will be free
    while(1)
    {
        u8_t __bmrstate = (__hd->bus == ATA_PRIMARY)? inb(__hd->bmr+ATA_BMS_SR_1):inb(__hd->bmr+ATA_BMS_SR_2);
        u8_t __atastate = inb(__hd->io_base+ATA_REG_STATUS);

        if(!(__bmrstate & 0x04))
            continue;

        if(!(__atastate & ATA_SR_BSY))
            break;
    }

    //kprintf("ata driver %s %d\n",__read?"read":"write",__count*512); 
    return 1;
}


int disk_read(u32_t lba, u32_t offset, void* buf, u32_t bytes)
{
    if(!bytes)
        return 0;

    char* __bl_buffer = NULL;
    u32_t __lba = lba + (offset / 512);
    u32_t __count = bytes / 512;
    if(bytes % 512)
        __count++;
    __bl_buffer = sys_alloc(__count * 512);
    char* __memup = __bl_buffer;

    for(int i = 0; i < __count; i++)
    {
        ide_read_write(__memup,__lba+i,1);
        __memup += 512;    
    }
    
    memcpy(buf,__bl_buffer + (offset % 512),bytes);
    sys_free(__bl_buffer);
    return bytes;
}

int disk_write(u32_t lba, u32_t offset, void* buf, u32_t bytes)
{
    if(!bytes)
        return bytes;

    char* __bl_buffer = NULL;
    u32_t __lba = lba + (offset / 512);
    u32_t __count = bytes / 512;
    if(bytes % 512)
        __count++;
    __bl_buffer = sys_alloc(__count * 512);
    char* __memup = __bl_buffer;
    if(offset)
    {
        for(int i = 0; i < __count; i++)
        {
            ide_read_write(__memup,__lba+i,1);
            __memup += 512;    
        }
        __memup = __bl_buffer;    
    }
    
    memcpy(__bl_buffer + (offset % 512),buf,bytes);

    for(int i = 0; i < __count; i++)
    {
        ide_read_write(__memup,__lba+i,0);
        __memup += 512;    
    }
    
    sys_free(__bl_buffer);
    return bytes;
}


void ata_init()
{
    init_list(&__atalist);
    kprintf("Installing PCI devices context...");
    lookup_pcidev(); kprintf("[ok]\n");
    
    for(int __bus = 0; __bus < 1; __bus++)
    {
        for(int __drive = 0; __drive < 2; __drive++)
        {
            kprintf("Detecting %s %s ...",(__bus == 0)? "ATA_PRIMARY":"ATA_SECONDRY",
                (__drive == 0)? "ATA_MASTER":"ATA_SLAVE");

            if(ide_identify(__bus,__drive))
            {
                new_ata(__bus,__drive,0x01,0x01);
                kprintf("[ok]\n");
                //break;
            }
            else
                kprintf("[failed]\n");
        }
    }
    
    // set default hd to ata primary master
    __hd = new_ata(0x0,0x0,0x01,0x01);
    kprintf("Installing ATA IRQ handler...");
    register_interrupt_handler(__isr14,ATA_PRIMARY_ISR);
    register_interrupt_handler(__isr15,ATA_SECONDARY_ISR);
    kprintf("[ok]\n");
}
