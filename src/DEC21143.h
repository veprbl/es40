/*  ES40 emulator.
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

#include "SystemComponent.h"
#include "DEC21143_mii.h"
#include "DEC21143_tulipreg.h"
#include <pcap.h>

/**
 * Emulated DEC 21143 NIC device.
 **/

class CDEC21143 : public CSystemComponent  
{
 public:
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  void instant_tick();
  //	void interrupt(int number);
  virtual int DoClock();
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);

  virtual u64 ReadMem(int index, u64 address, int dsize);
  CDEC21143(class CSystem * c);
  virtual ~CDEC21143();
  virtual void ResetPCI();
  void ResetNIC();
  void SetupFilter();
 private:
  u64 config_read(u64 address, int dsize);
  void config_write(u64 address, int dsize, u64 data);
  u64 nic_read(u64 address, int dsize);
  void nic_write(u64 address, int dsize, u64 data);
  void mii_access(uint32_t oldreg, uint32_t idata);
  void srom_access(uint32_t oldreg, uint32_t idata);

  int dec21143_rx();
  int dec21143_tx();

  pcap_t *fp;
  struct bpf_program fcode;
  // The state structure contains all elements that need to be saved to the statefile.
  struct SDEC21143State {
    
    /* PCI configuration registers */  
    u8 config_data[256];
    u8 config_mask[256];

    bool		irq_was_asserted;

    /*  Ethernet address, and a network which we are connected to:  */
	u8		mac[6];
    u8      setup_filter[192];

	/*  SROM emulation:  */
	u8		srom[1 << (7)];
	int		srom_curbit;
	int		srom_opcode;
	int		srom_opcode_has_started;
	int		srom_addr;

	/*  MII PHY emulation:  */
	u16	    mii_phy_reg[MII_NPHY * 32];
	int		mii_state;
	int		mii_bit;
	int		mii_opcode;
	int		mii_phyaddr;
	int		mii_regaddr;

	/*  21143 registers:  */
	u32	    reg[32];

    /*  Internal TX state:  */
	u64	    cur_tx_addr;
	unsigned char	*cur_tx_buf;
	int		cur_tx_buf_len;
	int		tx_idling;
	int		tx_idling_threshold;
    bool tx_suspend;

	/*  Internal RX state:  */
	u64	    cur_rx_addr;
	unsigned char	*cur_rx_buf;
	int		cur_rx_buf_len;
	int		cur_rx_offset;
  } state;
};

#endif // !defined(INCLUDED_DEC21143_H)
