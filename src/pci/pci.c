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


#include <pci.h>
static int __pcidevice = 0;
static pcidev_t* __pcidev[TOTAL_PCI_DEVICE] __attribute__((section(".data")));

u32_t r_pcidev(u32_t __bus, u32_t __dev, u32_t __function, u32_t __offset)
{
   u32_t __value =
    1<<31 | __bus<<16 | __dev<<11 | __function<<8 | (__offset & 0xfc);
    outl(PCI_CONFIG_ADDR,__value);
    u32_t __result = inl(PCI_DATA_ADDR);
    return __result>>(8*(__offset%4));
}

void w_pcidev(u32_t __bus, u32_t __dev, u32_t __function, u32_t __offset, u32_t __data)
{
	u32_t __value =
    1<<31 | __bus<<16 | __dev<<11 | __function<<8 | (__offset & 0xfc);
    outl(PCI_CONFIG_ADDR,__value);
    outl(PCI_DATA_ADDR,__data);
}

void name_pcidev(pcidev_t* __pcidev, u8_t* __name)
{
    strncpy(__pcidev->name,__name,PCI_DEV_NAME);
}

u32_t read_pcidev(pcidev_t* __pcidev, u8_t __offset)
{
    return r_pcidev(__pcidev->bus,__pcidev->device,__pcidev->function,__offset);
}

void write_pcidev(pcidev_t* __pcidev, u8_t __offset, u32_t __data)
{
    w_pcidev(__pcidev->bus,__pcidev->device,__pcidev->function,__offset,
        __data);
}


pcidev_t* find_pcidev(u8_t __subclass, u8_t __baseclass, u16_t __vendorID, u16_t __deviceID, u8_t __type)
{
    // find a pci device by
    // subclass and baseclass
    // code
    if(__type == PCI_FIND_BY_CLASS)
    {
        for(int i = 0; i < __pcidevice; i++)
        if(__pcidev[i]->subcode == __subclass && __pcidev[i]->basecode == __baseclass)
            return __pcidev[i];
        return NULL;    
    }
    // find a pci device by vendor ID and device ID
    if (__type == PCI_FIND_BY_ID)
    {
        for(int i = 0; i < __pcidevice; i++)
        if(__pcidev[i]->vendorID == __vendorID && __pcidev[i]->deviceID == __deviceID)
            return __pcidev[i];
        return NULL;       
    }
    // return NULL if
    // no result has found
    return NULL;
}


void lookup_pcidev()
{
    // poll until all pci device
    // installed are known
    for(int __bus = 0; __bus < 8; __bus++)
    {
        for(int __dev = 0; __dev < 32; __dev++)
        {
            for(int __function = 0; __function < 8; __function++)
            {
                u16_t __value = r_pcidev(__bus,__dev,__function,VENDOR_ID_REG_OFF);
                if(__value == 0xffff || __value == 0x0000)
                    break;
                // registering pci device 
                __pcidev[__pcidevice] = (pcidev_t*)sys_alloc(sizeof(pcidev_t));
                __pcidev[__pcidevice]->bus = __bus;
                __pcidev[__pcidevice]->device = __dev;
                __pcidev[__pcidevice]->function = __function;
                __pcidev[__pcidevice]->basecode = read_pcidev(__pcidev[__pcidevice],BASECLASS_REG_OFF);
                __pcidev[__pcidevice]->subcode = read_pcidev(__pcidev[__pcidevice],SUBCLASS_REG_OFF);
                __pcidev[__pcidevice]->bmr = read_pcidev(__pcidev[__pcidevice],BUS_MASTER_REG_OFF);
                __pcidev[__pcidevice]->vendorID = read_pcidev(__pcidev[__pcidevice],VENDOR_ID_REG_OFF);
                __pcidev[__pcidevice]->deviceID = read_pcidev(__pcidev[__pcidevice],DEVICE_ID_REG_OFF);
                // increase __pcidevice value
                __pcidevice++;
            }
        }
    }

}
