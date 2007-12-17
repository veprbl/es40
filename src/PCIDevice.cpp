/* ES40 emulator.
 * Copyright (C) 2007 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
 * E-mail : camiel@camicom.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 */

/**
 * \file
 * Contains the code for the PCI device class.
 *
 * X-1.5        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.4        Camiel Vanderhoeven                             10-DEC-2007
 *      Translate a 64-bit PCI access into 2 32-bit accesses.
 *
 * X-1.3        Brian Wheeler                                   10-DEC-2007
 *      More verbose error reporting.
 *
 * X-1.2        Camiel Vanderhoeven                             10-DEC-2007
 *      Removed some printf's.
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h"
#include "PCIDevice.h"
#include "System.h"

CPCIDevice::CPCIDevice(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev) : CSystemComponent(cfg, c)
{
  int i;


  for (i=0;i<8; i++)
    device_at[i] = false;

  myPCIBus = pcibus;
  myPCIDev = pcidev;
}

CPCIDevice::~CPCIDevice(void)
{
}

void CPCIDevice::add_function(int func, u32 data[64], u32 mask[64])
{
  memcpy(this->std_config_data[func],data,64*sizeof(u32));
  memcpy(this->std_config_mask[func],mask,64*sizeof(u32));
  device_at[func] = true;
}

void CPCIDevice::add_legacy_io(int id, u32 base, u32 length)
{
  dev_range_is_io[id] = true;
  cSystem->RegisterMemory(this,id, X64(00000801fc000000)
                                + (X64(0000000200000000) * myPCIBus)
                                + base, length);
}

void CPCIDevice::add_legacy_mem(int id, u32 base, u32 length)
{
  dev_range_is_io[id] = false;
  cSystem->RegisterMemory(this,id, X64(0000080000000000)
                                + (X64(0000000200000000) * myPCIBus)
                                + base, length);
}

u32 CPCIDevice::config_read(int func, u32 address, int dsize)
{
  u8 * x;

  u32 data;

  x = (u8*)pci_state.config_data[func];
  x+= address;

  switch (dsize)
    {
    case 8:
      data = *x & 0xff;
      break;
    case 16:
      data = (*((u16*)x)) & 0xffff;
      break;
    case 32:
      data = (*((u32*)x));
      break;
    }

  data = config_read_custom(func, address, dsize, data);

//  printf("%s(%s).%d config read  %d bytes @ %x = %x\n",myCfg->get_myName(), myCfg->get_myValue(), func,dsize/8,address, data);

  return data;
}

void CPCIDevice::config_write(int func, u32 address, int dsize, u32 data)
{
//  printf("%s(%s).%d config write %d bytes @ %x = %x\n",myCfg->get_myName(), myCfg->get_myValue(), func,dsize/8,address, data);

  u8 * x;
  u8 * y;

  u32 mask;
  u32 old_data;
  u32 new_data;

  x = (u8*)pci_state.config_data[func];
  x+= address;
  y = (u8*)pci_state.config_mask[func];
  y+= address;

  switch (dsize)
    {
    case 8:
      old_data = (*x) & 0xff;
      mask     = (*y) & 0xff;
      new_data = (old_data & ~mask) | data & mask;
      *x = (u8) new_data;
      break;
    case 16:
      old_data = (*((u16*)x)) & 0xffff;
      mask     = (*((u16*)y)) & 0xffff;
      new_data = (old_data & ~mask) | data & mask;
      *((u16*)x) = (u16) new_data;
      break;
    case 32:
      old_data = (*((u32*)x));
      mask     = (*((u32*)y));
      new_data = (old_data & ~mask) | data & mask;
      *((u32*)x) = new_data;
      break;
    }
  if (   dsize==32
      && ((data&mask)!=mask) 
	  && ((data&mask)!=0) )
    switch(address)
      {
      case 0x10:
      case 0x14:
      case 0x18:
      case 0x1c:
      case 0x20:
      case 0x24:
        register_bar(func,(address-0x10)/4, new_data, mask);
        break;
      case 0x30:
        register_bar(func,6, new_data, mask);
        break;
      }
  config_write_custom(func,address,dsize,old_data,new_data,data);
}

void CPCIDevice::register_bar(int func, int bar, u32 data, u32 mask)
{
  int id = PCI_RANGE_BASE + (func*8) + bar;
  u32 length = (~mask) + 1;
  u64 t;

  if (data & 1)
  {
    // io space
    pci_range_is_io[func][bar] = true;

    cSystem->RegisterMemory(this, PCI_RANGE_BASE+(func*8)+bar, 
                               t= X64(00000801fc000000)
                               + (X64(0000000200000000) * myPCIBus)
                               + (data & ~0x3),length);
//    printf("%s(%s).%d PCI BAR %d set to IO  % " LL "x, len %x.\n",myCfg->get_myName(), myCfg->get_myValue(), func,bar,t,length);
  }
  else
  {
    // io space
    pci_range_is_io[func][bar] = true;

    cSystem->RegisterMemory(this, PCI_RANGE_BASE+(func*8)+bar, 
                               t= X64(0000080000000000)
                               + (X64(0000000200000000) * myPCIBus)
                               + (data & ~0xf),length);
//    printf("%s(%s).%d PCI BAR %d set to MEM % " LL "x, len %x.\n",myCfg->get_myName(), myCfg->get_myValue(), func,bar,t,length);
  }
}

void CPCIDevice::ResetPCI()
{
  int i;

  for (i=0; i<8; i++)
  {
    if (device_at[i])
    {
      cSystem->RegisterMemory(this, PCI_RANGE_BASE+(i*8)+7, 
                                    X64(00000801fe000000)
                                 + (X64(0000000200000000) * myPCIBus)
                                 + (X64(0000000000000800) * myPCIDev)
                                 + (X64(0000000000000100) * i),0x100);
      memcpy(pci_state.config_data[i],std_config_data[i],64 * sizeof(u32));
      memcpy(pci_state.config_mask[i],std_config_mask[i],64 * sizeof(u32));

      config_write(i, 0x10, 32, pci_state.config_data[i][4]);
      config_write(i, 0x14, 32, pci_state.config_data[i][5]);
      config_write(i, 0x18, 32, pci_state.config_data[i][6]);
      config_write(i, 0x1c, 32, pci_state.config_data[i][7]);
      config_write(i, 0x20, 32, pci_state.config_data[i][8]);
      config_write(i, 0x24, 32, pci_state.config_data[i][9]);
      config_write(i, 0x30, 32, pci_state.config_data[i][12]);
    }
  }
}

u64 CPCIDevice::ReadMem(int index, u64 address, int dsize) 
{
  int func;
  int bar;

  if (dsize==64)
    return ReadMem(index, address, 32) | (((u64)ReadMem(index, address+4, 32)) << 32);

  if (dsize != 8 && dsize != 16 && dsize != 32)
  {
    printf("ReadMem: %s(%s) Unsupported dsize %d. (%d, %" LL "x)\n",myCfg->get_myName(), myCfg->get_myValue(),dsize,index,address);
    exit(1);
  }
  
  if (index < PCI_RANGE_BASE)
  {
    if (dev_range_is_io[index] && !(pci_state.config_data[0][1]&1))
    {
      printf("%s(%s) Legacy IO access with IO disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue());
      return 0;
    }
    if (!dev_range_is_io[index] && !(pci_state.config_data[0][1]&2))
    {
      printf("%s(%s) Legacy memory access with memory disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue());
      return 0;
    }
//    printf("%s(%s) Calling ReadMem_Legacy(%d).\n",myCfg->get_myName(), myCfg->get_myValue(), index);
    return ReadMem_Legacy(index, address, dsize);
  }

  index -= PCI_RANGE_BASE;

  bar = index & 7;
  func = (index/8) & 7;
  
  if (bar==7)
    return config_read(func,(u32)address,dsize);

  if (pci_range_is_io[func][bar] && !(pci_state.config_data[func][1]&1))
  {
    printf("%s(%s).%d PCI IO access with IO disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue(), func);
    return 0;
  }
  if (!pci_range_is_io[func][bar] && !(pci_state.config_data[func][1]&2))
  {
    printf("%s(%s).%d PCI memory access with memory disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue(), func);
    return 0;
  }

//  printf("%s(%s).%d Calling ReadMem_Bar(%d,%d).\n",myCfg->get_myName(), myCfg->get_myValue(), func,func,bar);
  return ReadMem_Bar(func,bar,address,dsize);
}

void CPCIDevice::WriteMem(int index, u64 address, int dsize, u64 data) 
{
  int func;
  int bar;
  
  if (dsize==64)
  {
    WriteMem(index, address,   32, data & X64(ffffffff));
    WriteMem(index, address+4, 32, (data>>32) & X64(ffffffff));
    return;
  }

  if (dsize != 8 && dsize != 16 && dsize != 32)
  {
    printf("WriteMem: %s(%s) Unsupported dsize %d. (%d,%" LL "x,%" LL "x)\n",myCfg->get_myName(), myCfg->get_myValue(),dsize,index,address,data);
    exit(1);
  }
  
  if (index < PCI_RANGE_BASE)
  {
    if (dev_range_is_io[index] && !(pci_state.config_data[0][1]&1))
    {
      printf("%s(%s) Legacy IO access with IO disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue());
      return;
    }
    if (!dev_range_is_io[index] && !(pci_state.config_data[0][1]&2))
    {
      printf("%s(%s) Legacy memory access with memory disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue());
      return;
    }
    WriteMem_Legacy(index, (u32)address, dsize, (u32)data);
    return;
  }

  index -= PCI_RANGE_BASE;

  bar = index & 7;
  func = (index/8) & 7;

  if (bar==7)
  {
    config_write(func,(u32)address,dsize,(u32)data);
    return;
  }

  if (pci_range_is_io[func][bar] && !(pci_state.config_data[func][1]&1))
  {
    printf("%s(%s).%d PCI IO access with IO disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue(), func);
    return;
  }
  if (!pci_range_is_io[func][bar] && !(pci_state.config_data[func][1]&2))
  {
    printf("%s(%s).%d PCI memory access with memory disabled from PCI config.\n",myCfg->get_myName(), myCfg->get_myValue(), func);
    return;
  }
  WriteMem_Bar(func,bar,(u32)address,dsize,(u32)data);
}

bool CPCIDevice::do_pci_interrupt(int func, bool asserted)
{
  if ((pci_state.config_data[func][0x0f] & 0xff) != 0xff)
  {
    cSystem->interrupt(pci_state.config_data[func][0x0f] & 0xff, asserted);
    return true;
  }
  else
    return false;
}

static u32 pci_magic1 = 0xC1095A78;
static u32 pci_magic2 = 0x87A5901C;

/**
 * Save state to a Virtual Machine State file.
 **/

int CPCIDevice::SaveState(FILE *f)
{
  long ss = sizeof(pci_state);

  fwrite(&pci_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&pci_state,sizeof(pci_state),1,f);
  fwrite(&pci_magic2,sizeof(u32),1,f);
  printf("%s: %d PCI bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CPCIDevice::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  size_t r;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != pci_magic1)
  {
    printf("%s: PCI MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(pci_state))
  {
    printf("%s: PCI STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&pci_state,sizeof(pci_state),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m2 != pci_magic2)
  {
    printf("%s: PCI MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  printf("%s: %d PCI bytes restored.\n",devid_string,ss);
  return 0;
}

u32 CPCIDevice::ReadMem_Legacy(int index, u32 address, int dsize) 
{
  printf("%s(%s) No Legacy read handler installed!\n",myCfg->get_myName(), myCfg->get_myValue());
  exit(1);
  return 0;
}

void CPCIDevice::WriteMem_Legacy(int index, u32 address, int dsize, u32 data) 
{
  printf("%s(%s) No Legacy write handler installed!\n",myCfg->get_myName(), myCfg->get_myValue());
  exit(1);
  return;
}

u32 CPCIDevice::ReadMem_Bar(int func,int bar, u32 address, int dsize) 
{ 
  printf("%s(%s).%d No BAR read handler installed!\n",myCfg->get_myName(), myCfg->get_myValue(), func);
  exit(1);
  return 0; 
}

void CPCIDevice::WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data) 
{
  printf("%s(%s).%d No BAR write handler installed!\n",myCfg->get_myName(), myCfg->get_myValue(), func);
  exit(1);
  return; 
}
