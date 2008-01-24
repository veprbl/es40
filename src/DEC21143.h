/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
 * E-mail : camiel@camicom.com
 * 
 *  This file is based upon GXemul.
 *
 *  Copyright (C) 2004-2007  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

/**
 * \file 
 * Contains the definitions for the emulated DEC 21143 NIC device.
 *
 * $Id: DEC21143.h,v 1.12 2008/01/24 12:40:27 iamcamiel Exp $
 *
 * X-1.12       Camiel Vanderhoeven                             24-JAN-2008
 *      Use new CPCIDevice::do_pci_read and CPCIDevice::do_pci_write.
 *
 * X-1.11       David Hittner                                   04-JAN-2008
 *      MAC address configurable.
 *
 * X-1.10       Camiel Vanderhoeven                             02-JAN-2008
 *      Comments.
 *
 * X-1.9        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.8        Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.7        Camiel Vanderhoeven                             2-DEC-2007
 *      Receive network data in a separate thread.
 *
 * X-1.6        Camiel Vanderhoeven                             1-DEC-2007
 *      Moved inclusion of StdAfx.h outside conditional block; necessary
 *      for using precompiled headers in Visual C++.
 *
 * X-1.5        Camiel Vanderhoeven                             17-NOV-2007
 *      Don't define REMOTE before including pcap.h; we want to be 
 *      compatible.
 *
 * X-1.4        Camiel Vanderhoeven                             17-NOV-2007
 *      Cleanup.
 *
 * X-1.3        Camiel Vanderhoeven                             16-NOV-2007
 *      BPF filter used for perfect filtering.
 *
 * X-1.2        Camiel Vanderhoeven                             15-NOV-2007
 *      Use pcap for network access.
 *
 * X-1.1        Camiel Vanderhoeven                             14-NOV-2007
 *      Initial version for ES40 emulator.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_DEC21143_H_)
#define INCLUDED_DEC21143_H

#include "PCIDevice.h"
#include "DEC21143_mii.h"
#include "DEC21143_tulipreg.h"
#include <pcap.h>

/**
 * \brief Emulated DEC 21143 NIC device.
 *
 * Documentation consulted:
 *  - 21143 PCI/Cardbus 10/100Mb/s Ethernet LAN Controller Hardware Reference Manual  [HRM].
 *    (http://download.intel.com/design/network/manuals/27807401.pdf)
 *  .
 **/

class CDEC21143 : public CPCIDevice  
{
 public:
  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);
  void instant_tick();
  //	void interrupt(int number);
  virtual int DoClock();
  virtual void WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Bar(int func, int bar, u32 address, int dsize);

  CDEC21143(CConfigurator * confg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CDEC21143();
  virtual void ResetPCI();
  void ResetNIC();
  void SetupFilter();
  void receive_process();

 private:
  static int nic_num;
#if defined(_WIN32)
  HANDLE receive_process_handle;
#else
  pthread_t receive_process_handle;
#endif

  u32 nic_read(u32 address, int dsize);
  void nic_write(u32 address, int dsize, u32 data);
  void mii_access(uint32_t oldreg, uint32_t idata);
  void srom_access(uint32_t oldreg, uint32_t idata);

  int dec21143_rx();
  int dec21143_tx();

  pcap_t *fp;
  struct bpf_program fcode;
  bool shutting_down;

  /// The state structure contains all elements that need to be saved to the statefile.
  struct SNIC_state {
   
    bool irq_was_asserted;      /**< remember state of IRQ */

	u8		mac[6];             /**< ethernet address */
    u8      setup_filter[192];  /**< filter for perfect filtering */

	/// SROM emulation
    struct SNIC_srom {
	  u8		data[1 << (7)];
	  int		curbit;
	  int		opcode;
	  int		opcode_has_started;
	  int		addr;
      } srom;

	/// MII PHY emulation
    struct SNIC_mii {
	  u16	    phy_reg[MII_NPHY * 32];
	  int		state;
	  int		bit;
	  int		opcode;
	  int		phyaddr;
	  int		regaddr;
      } mii;

	u32	    reg[32]; /**< 21143 registers */

    /// Internal TX state
    struct SNIC_tx {
	  u32	    cur_addr;
	  unsigned char	*cur_buf;
	  int		cur_buf_len;
	  int		idling;
	  int		idling_threshold;
      bool suspend;
      } tx;

	/// Internal RX state
    struct SNIC_rx {
	  u32	    cur_addr;
	  unsigned char	*cur_buf;
	  int		cur_buf_len;
	  int		cur_offset;
      } rx;
  } state;
};

#endif // !defined(INCLUDED_DEC21143_H)
