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
 * Contains the code for the emulated DEC 21143 NIC device.
 *
 * X-1.18       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.17       Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.16       Brian Wheeler                                   10-DEC-2007
 *      Added pthread.h
 *
 * X-1.15       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.14       Camiel Vanderhoeven                             6-DEC-2007
 *      Identifies itself as DE-500BA.
 *
 * X-1.13       Camiel Vanderhoeven                             2-DEC-2007
 *      Receive network data in a separate thread.
 *
 * X-1.12       Camiel Vanderhoeven                             1-DEC-2007
 *      Moved inclusion of StdAfx.h outside conditional block; necessary
 *      for using precompiled headers in Visual C++.
 *
 * X-1.11       Camiel Vanderhoeven                             17-NOV-2007
 *      File should end in a newline.
 *
 * X-1.10       Camiel Vanderhoeven                             17-NOV-2007
 *      Use the standard pcap functions (not the extended windows ones), we
 *      want to be compatible.
 *
 * X-1.9        Camiel Vanderhoeven                             17-NOV-2007
 *      Corrected a small "oops" error in getting the DECnet address.
 *
 * X-1.8        Camiel Vanderhoeven                             17-NOV-2007
 *      Get the adapter and DECnet address to use from the configuration
 *      file.
 *
 * X-1.7        Camiel Vanderhoeven                             17-NOV-2007
 *      Changed the MAC address into the DigitalE-range.
 *
 * X-1.6        Camiel Vanderhoeven                             16-NOV-2007
 *      Change the packet filter less often (only when required).
 *
 * X-1.5        Camiel Vanderhoeven                             16-NOV-2007
 *      Removed some debug messages, and corrected readout of CSR 12.
 *
 * X-1.4        Camiel Vanderhoeven                             16-NOV-2007
 *      BPF filter used for perfect filtering; more correct behaviour of 
 *      registers.
 *
 * X-1.3        Camiel Vanderhoeven                             15-NOV-2007
 *      Use pcap for network access.
 *
 * X-1.2        Camiel Vanderhoeven                             14-NOV-2007
 *      Removed some debug messages.
 *
 * X-1.1        Camiel Vanderhoeven                             14-NOV-2007
 *      Initial version for ES40 emulator.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"

#if !defined(NO_NETWORK)

#include "DEC21143.h"
#include "System.h"

#if !defined(_WIN32)
#include <pthread.h>
#endif

/*  Internal states during MII data stream decode:  */
#define	MII_STATE_RESET				    0
#define	MII_STATE_START_WAIT			1
#define	MII_STATE_READ_OP			    2
#define	MII_STATE_READ_PHYADDR_REGADDR	3
#define	MII_STATE_A				        4
#define	MII_STATE_D				        5
#define	MII_STATE_IDLE				    6

#if defined(_WIN32)
DWORD WINAPI recv_proc(LPVOID lpParam)
#else
#include <pthread.h>
static void * recv_proc(void * lpParam)
#endif
{
  ((CDEC21143 *) lpParam)->receive_process();
  return 0;
}

u32 dec21143_cfg_data[64] = {
/*00*/  0x00191011, // CFID: vendor + device
/*04*/  0x02800000, // CFCS: command + status
/*08*/  0x02000041, // CFRV: class + revision
/*0c*/  0x00000000, // CFLT: latency timer + cache line size
/*10*/  0x00000001, // BAR0: CBIO
/*14*/  0x00000000, // BAR1: CBMA
/*18*/  0x00000000, // BAR2: 
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x500b1011, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x281401ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

u32 dec21143_cfg_mask[64] = {
/*00*/  0x00000000, // CFID: vendor + device
/*04*/  0x0000ffff, // CFCS: command + status
/*08*/  0x00000000, // CFRV: class + revision
/*0c*/  0x0000ffff, // CFLT: latency timer + cache line size
/*10*/  0xffffff00, // BAR0
/*14*/  0xffffff00, // BAR1: CBMA
/*18*/  0x00000000, // BAR2: 
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x000000ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/**
 * Constructor.
 **/

CDEC21143::CDEC21143(CConfigurator * confg, CSystem * c, int pcibus, int pcidev): CPCIDevice(confg,c,pcibus,pcidev)
{
  pcap_if_t *alldevs, *d;
  u_int inum, i=0;
  char errbuf[PCAP_ERRBUF_SIZE];// connect to pcap...
  char * cfg;
  int decnet_major;
  int decnet_minor;

  add_function(0,dec21143_cfg_data,dec21143_cfg_mask);


  cfg = myCfg->get_text_value("adapter");

  if (cfg)
  {
    if ( (fp= pcap_open_live(cfg,
                             65536 /*snaplen: capture entire packets*/,
                             1 /*promiscuous*/,
                             1 /*read timeout: 1ms.*/,
                             errbuf)) == NULL)
      FAILURE("Error opening adapter\n");
  }
  else
  {
    printf("\n%%NIC-Q-CHNIC: Choose a network adapter to connect to:\n");
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
      FAILURE("%%NIC-F-PCAPFAD: Error in pcap_findalldevs_ex:");
      FAILURE(errbuf);
    }

    /* Print the list */
    for(d=alldevs; d; d=d->next)
    {
      printf("%d. %s\n    ", ++i, d->name);
      if (d->description)
        printf(" (%s)\n", d->description);
      else
        printf(" (No description available)\n");
    }
        
    if (i==0)
      FAILURE("%%NIC-F-NONIC: No interfaces found! Exiting.\n");
   
    if (i==1)
      inum = 1;
    else
    {
      inum = 0;
      while (inum < 1 || inum > i)
      {
        printf("%%NIC-Q-NICNO: Enter the interface number (1-%d):",i);
        scanf("%d", &inum);
      }
    }
        
    /* Jump to the selected adapter */
    for (d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
        
    /* Open the device */
    if ( (fp= pcap_open_live(d->name,
                             65536 /*snaplen: capture entire packets*/,
                             1 /*flags*/,
                             1 /*read timeout: 1ms.*/,
                             errbuf)) == NULL)
      FAILURE("Error opening adapter\n");
  }

  cfg = myCfg->get_text_value("decnet","1.1");
  decnet_major = atoi(cfg);
  cfg = strchr(cfg,'.')+1;
  decnet_minor = atoi(cfg);

  state.mac[0] = 0xaa;
  state.mac[1] = 0x00;
  state.mac[2] = 0x04;
  state.mac[3] = 0x00;
  state.mac[4] = decnet_minor & 0xff;
  state.mac[5] = ((decnet_minor >> 8) & 0x03) | ((decnet_major << 2) & 0xfc);

  c->RegisterClock(this, true);

  state.cur_rx_buf = NULL;
  state.cur_tx_buf = NULL;

  ResetPCI();

#if defined(_WIN32)
  receive_process_handle = CreateThread(NULL, 0, recv_proc, this, 0, NULL);
#else
  pthread_create(&receive_process_handle, NULL, recv_proc, this);
#endif

  printf("%%NIC-I-INIT: DEC 21143 network interface card emulator initialized.\n");
}

CDEC21143::~CDEC21143()
{
}

u32 CDEC21143::ReadMem_Bar(int func, int bar, u32 address, int dsize)
{
  switch(bar)
  {
  case 0: // CBIO
  case 1: // CBMA
    return nic_read(address, dsize);
  }

  return 0;
}

void CDEC21143::WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data)
{
  switch(bar)
  {
  case 0:       // CBIO
  case 1:		// CBMA
    nic_write(address, dsize, (u32)endian_bits(data, dsize));
    return;
  }
}

int CDEC21143::DoClock()
{
  bool asserted;

  if ((state.reg[CSR_OPMODE / 8] & OPMODE_ST))
	while (dec21143_tx());

//  if (state.reg[CSR_OPMODE / 8] & OPMODE_SR)
//	while (dec21143_rx());

  /*  Normal and Abnormal interrupt summary:  */
  state.reg[CSR_STATUS / 8] &= ~(STATUS_NIS | STATUS_AIS);
  if (state.reg[CSR_STATUS / 8] & state.reg[CSR_INTEN / 8] & 0x00004845)
	state.reg[CSR_STATUS / 8] |= STATUS_NIS;
  if (state.reg[CSR_STATUS / 8] & state.reg[CSR_INTEN / 8] & 0x0c0037ba)
	state.reg[CSR_STATUS / 8] |= STATUS_AIS;

  asserted = (state.reg[CSR_STATUS / 8] & state.reg[CSR_INTEN / 8] & 0x0c01ffff)?true:false;

  if (asserted != state.irq_was_asserted)
  {
    if (do_pci_interrupt(0, asserted))
	  state.irq_was_asserted = asserted;
  }

  return 0;
}

/**
 * Read from the NIC registers.
 **/

void CDEC21143::receive_process()
{
  while (true)
  {
    if (state.reg[CSR_OPMODE / 8] & OPMODE_SR)
      while(dec21143_rx());
    sleep_ms(10);
  }
}

u32 CDEC21143::nic_read(u32 address, int dsize)
{
  u32 data = 0;

    u32 oldreg = 0;
	int regnr = (int)(address >> 3);

	if ((address & 7) == 0 && regnr < 32) {
	  data = state.reg[regnr];
	} else
		fatal("[ dec21143: WARNING! unaligned access (0x%x) ]\n", (int)address);

  return data;
}

/**
 * Write to the NIC registers.
 **/

void CDEC21143::nic_write(u32 address, int dsize, u32 data)
{
	uint32_t oldreg = 0;

    int regnr = (int)(address >> 3);

	if ((address & 7) == 0 && regnr < 32) {
		oldreg = state.reg[regnr];
		switch (regnr) {
		case CSR_STATUS / 8:	/*  Zero-on-write  */
			state.reg[regnr] &= ~((u32)data & 0x0c01ffff);
			break;
		case CSR_MISSED / 8:	/*  Read only  */
			break;
		default:
            state.reg[regnr] = (u32)data;
		}
	} else
		fatal("[ dec21143: WARNING! unaligned access (0x%x) ]\n", (int)address);

	switch (address) {

	case CSR_BUSMODE:	/*  csr0  */
		if (data & BUSMODE_SWR) {
			ResetNIC();
		    data &= ~BUSMODE_SWR;
		}
		break;

	case CSR_TXPOLL:	/*  csr1  */
        /* CaVa interpretation... */
        state.reg[CSR_STATUS/8] &= ~STATUS_TU;
        state.tx_suspend = false;
		state.tx_idling = state.tx_idling_threshold;
        DoClock();
		break;

	case CSR_RXPOLL:	/*  csr2  */
        DoClock();
		break;

	case CSR_RXLIST:	/*  csr3  */
		/* debug("[ dec21143: setting RXLIST to 0x%x ]\n", (int)data); */
		if (data & 0x3)
			fatal("[ dec21143: WARNING! RXLIST not aligned? (0x%" LL "x) ]\n", (long long)data);
		data &= ~0x3;
		state.cur_rx_addr = data;
		break;

	case CSR_TXLIST:	/*  csr4  */
		/* debug("[ dec21143: setting TXLIST to 0x%x ]\n", (int)data); */
		if (data & 0x3)
			fatal("[ dec21143: WARNING! TXLIST not aligned? (0x%" LL "x) ]\n", (long long)data);
		data &= ~0x3;
		state.cur_tx_addr = data;
		break;

	case CSR_STATUS:	/*  csr5  */
	case CSR_INTEN:		/*  csr7  */
		/*  Recalculate interrupt assertion.  */
		DoClock();
		break;

	case CSR_OPMODE:	/*  csr6:  */
		if (data & 0x02000000) {
			/*  A must-be-one bit.  */
			data &= ~0x02000000;
		}
		if (data & OPMODE_ST) {
			data &= ~OPMODE_ST;
		} else {
			/*  Turned off TX? Then idle:  */
			state.reg[CSR_STATUS/8] |= STATUS_TPS;
		}
		if (data & OPMODE_SR) {
			data &= ~OPMODE_SR;
		} else {
			/*  Turned off RX? Then go to stopped state:  */
			state.reg[CSR_STATUS/8] &= ~STATUS_RS;
		}
		data &= ~(OPMODE_HBD | OPMODE_SCR | OPMODE_PCS | OPMODE_PS | OPMODE_SF | OPMODE_TTM | OPMODE_FD | OPMODE_TR);
		if (data & OPMODE_PNIC_IT) {
			data &= ~OPMODE_PNIC_IT;
		    state.tx_idling = state.tx_idling_threshold;
		}
		if (data != 0) {
			fatal("[ dec21143: UNIMPLEMENTED OPMODE bits: 0x%08x ]\n", (int)data);
		}
		DoClock();
		break;

	case CSR_MISSED:	/*  csr8  */
		break;

	case CSR_MIIROM:	/*  csr9  */
		if (data & MIIROM_MDC)
			mii_access(oldreg, (u32)data);
		else
			srom_access(oldreg, (u32)data);
		break;

	case CSR_SIASTAT:	/*  csr12  */
        if (((data & SIASTAT_ANS) == SIASTAT_ANS_START) && (state.reg[CSR_SIATXRX/8] & SIATXRX_ANE))
        {
            // autonegotiation restart... completes immediately in our emulated environment.
          state.reg[CSR_SIASTAT / 8] &= ~SIASTAT_ANS;
          state.reg[CSR_SIASTAT / 8] |= (SIASTAT_ANS_FLPGOOD | SIASTAT_LPN | SIASTAT_LPC);
          state.reg[CSR_STATUS / 8] |= STATUS_LNPANC;
          state.reg[CSR_SIATXRX / 8] &= ~(SIATXRX_TH | SIATXRX_THX | SIATXRX_T4);
          state.reg[CSR_SIATXRX / 8] |= SIATXRX_TXF;
          DoClock();
        }
        else
        {
          state.reg[CSR_SIASTAT / 8] = oldreg;
        }
        break;
	case CSR_SIATXRX:	/*  csr14  */
        break;
	case CSR_SIACONN:	/*  csr13  */
        if ((data & SIACONN_SRL) && (state.reg[CSR_SIATXRX/8] & SIATXRX_ANE))
        {
            // SIA started with autonegotiation... completes immediately in our emulated environment.
          state.reg[CSR_SIASTAT / 8] &= ~SIASTAT_ANS;
          state.reg[CSR_SIASTAT / 8] |= (SIASTAT_ANS_FLPGOOD | SIASTAT_LPN | SIASTAT_LPC);
          state.reg[CSR_STATUS / 8] |= STATUS_LNPANC;
          state.reg[CSR_SIATXRX / 8] &= ~(SIATXRX_TH | SIATXRX_THX | SIATXRX_T4);
          state.reg[CSR_SIATXRX / 8] |= SIATXRX_TXF;
          DoClock();
        }
        break;
	case CSR_SIAGEN:	/*  csr15  */
		break;

	default:
		fatal("[ dec21143: write to unimplemented 0x%02x: 0x%02x ]\n", (int)address, (int)data);
	}
}

/*
 *  mii_access():
 *
 *  This function handles accesses to the MII. Data streams seem to be of the
 *  following format:
 *
 *      vv---- starting delimiter
 *  ... 01 xx yyyyy zzzzz a[a] dddddddddddddddd
 *         ^---- I am starting with mii_bit = 0 here
 *
 *  where x = opcode (10 = read, 01 = write)
 *        y = PHY address
 *        z = register address
 *        a = on Reads: ACK bit (returned, should be 0)
 *            on Writes: _TWO_ dummy bits (10)
 *        d = 16 bits of data (MSB first)
 */
void CDEC21143::mii_access(uint32_t oldreg, uint32_t idata)
{
	int obit, ibit = 0;
	uint16_t tmp;

	/*  Only care about data during clock cycles:  */
	if (!(idata & MIIROM_MDC))
		return;

	if (idata & MIIROM_MDC && oldreg & MIIROM_MDC)
		return;

	/*  fatal("[ mii_access(): 0x%08x ]\n", (int)idata);  */

	if (idata & MIIROM_BR) {
		fatal("[ mii_access(): MIIROM_BR: TODO ]\n");
		return;
	}

	obit = idata & MIIROM_MDO? 1 : 0;

	if (state.mii_state >= MII_STATE_START_WAIT &&
	    state.mii_state <= MII_STATE_READ_PHYADDR_REGADDR &&
	    idata & MIIROM_MIIDIR)
		fatal("[ mii_access(): bad dir? ]\n");

	switch (state.mii_state) {

	case MII_STATE_RESET:
		/*  Wait for a starting delimiter (0 followed by 1).  */
		if (obit)
			return;
		if (idata & MIIROM_MIIDIR)
			return;
		/*  fatal("[ mii_access(): got a 0 delimiter ]\n");  */
		state.mii_state = MII_STATE_START_WAIT;
		state.mii_opcode = 0;
		state.mii_phyaddr = 0;
		state.mii_regaddr = 0;
		break;

	case MII_STATE_START_WAIT:
		/*  Wait for a starting delimiter (0 followed by 1).  */
		if (!obit)
			return;
		if (idata & MIIROM_MIIDIR) {
			state.mii_state = MII_STATE_RESET;
			return;
		}
		/*  fatal("[ mii_access(): got a 1 delimiter ]\n");  */
		state.mii_state = MII_STATE_READ_OP;
		state.mii_bit = 0;
		break;

	case MII_STATE_READ_OP:
		if (state.mii_bit == 0) {
			state.mii_opcode = obit << 1;
			/*  fatal("[ mii_access(): got first opcode bit (%i) ]\n", obit);  */
		} else {
			state.mii_opcode |= obit;
			/*  fatal("[ mii_access(): got opcode = %i ]\n", state.mii_opcode);  */
			state.mii_state = MII_STATE_READ_PHYADDR_REGADDR;
		}
		state.mii_bit ++;
		break;

	case MII_STATE_READ_PHYADDR_REGADDR:
		/*  fatal("[ mii_access(): got phy/reg addr bit nr %i (%i)"
		    " ]\n", state.mii_bit - 2, obit);  */
		if (state.mii_bit <= 6)
			state.mii_phyaddr |= obit << (6-state.mii_bit);
		else
			state.mii_regaddr |= obit << (11-state.mii_bit);
		state.mii_bit ++;
		if (state.mii_bit >= 12) {
			/*  fatal("[ mii_access(): phyaddr=0x%x regaddr=0x"
			    "%x ]\n", state.mii_phyaddr, state.mii_regaddr);  */
			state.mii_state = MII_STATE_A;
		}
		break;

	case MII_STATE_A:
		switch (state.mii_opcode) {
		case MII_COMMAND_WRITE:
			if (state.mii_bit >= 13)
				state.mii_state = MII_STATE_D;
			break;
		case MII_COMMAND_READ:
			ibit = 0;
			state.mii_state = MII_STATE_D;
			break;
		default:debug("[ mii_access(): UNIMPLEMENTED MII opcode %i (probably just a bug in GXemul's MII data stream handling) ]\n", state.mii_opcode);
			state.mii_state = MII_STATE_RESET;
		}
		state.mii_bit ++;
		break;

	case MII_STATE_D:
		switch (state.mii_opcode) {
		case MII_COMMAND_WRITE:
			if (idata & MIIROM_MIIDIR)
				fatal("[ mii_access(): write: bad dir? ]\n");
			obit = obit? (0x8000 >> (state.mii_bit - 14)) : 0;
			tmp = state.mii_phy_reg[(state.mii_phyaddr << 5) +
			    state.mii_regaddr] | obit;
			if (state.mii_bit >= 29) {
				state.mii_state = MII_STATE_IDLE;
//				debug("[ mii_access(): WRITE to phyaddr=0x%x regaddr=0x%x: 0x%04x ]\n", state.mii_phyaddr, state.mii_regaddr, tmp);
			}
			break;
		case MII_COMMAND_READ:
			if (!(idata & MIIROM_MIIDIR))
				break;
			tmp = state.mii_phy_reg[(state.mii_phyaddr << 5) +
			    state.mii_regaddr];
//			if (state.mii_bit == 13)
//				debug("[ mii_access(): READ phyaddr=0x%x regaddr=0x%x: 0x%04x ]\n", state.mii_phyaddr, state.mii_regaddr, tmp);
			ibit = tmp & (0x8000 >> (state.mii_bit - 13));
			if (state.mii_bit >= 28)
				state.mii_state = MII_STATE_IDLE;
			break;
		}
		state.mii_bit ++;
		break;

	case MII_STATE_IDLE:
		state.mii_bit ++;
		if (state.mii_bit >= 31)
			state.mii_state = MII_STATE_RESET;
		break;
	}

	state.reg[CSR_MIIROM / 8] &= ~MIIROM_MDI;
	if (ibit)
		state.reg[CSR_MIIROM / 8] |= MIIROM_MDI;
}


/*
 *  srom_access():
 *
 *  This function handles reads from the Ethernet Address ROM. This is not a
 *  100% correct implementation, as it was reverse-engineered from OpenBSD
 *  sources; it seems to work with OpenBSD, NetBSD, and Linux, though.
 *
 *  Each transfer (if I understood this correctly) is of the following format:
 *
 *	1xx yyyyyy zzzzzzzzzzzzzzzz
 *
 *  where 1xx    = operation (6 means a Read),
 *        yyyyyy = ROM address
 *        zz...z = data
 *
 *  y and z are _both_ read and written to at the same time; this enables the
 *  operating system to sense the number of bits in y (when reading, all y bits
 *  are 1 except the last one).
 */
void CDEC21143::srom_access(uint32_t oldreg, uint32_t idata)
{
	int obit, ibit;

	/*  debug("CSR9 WRITE! 0x%08x\n", (int)idata);  */

	/*  New selection? Then reset internal state.  */
	if (idata & MIIROM_SR && !(oldreg & MIIROM_SR)) {
		state.srom_curbit = 0;
		state.srom_opcode = 0;
		state.srom_opcode_has_started = 0;
		state.srom_addr = 0;
	}

	/*  Only care about data during clock cycles:  */
	if (!(idata & MIIROM_SROMSK))
		return;

	obit = 0;
	ibit = idata & MIIROM_SROMDI? 1 : 0;
	/*  debug("CLOCK CYCLE! (bit %i): ", state.srom_curbit);  */

	/*
	 *  Linux sends more zeroes before starting the actual opcode, than
	 *  OpenBSD and NetBSD. Hopefully this is correct. (I'm just guessing
	 *  that all opcodes should start with a 1, perhaps that's not really
	 *  the case.)
	 */
	if (!ibit && !state.srom_opcode_has_started)
		return;

	if (state.srom_curbit < 3) {
		state.srom_opcode_has_started = 1;
		state.srom_opcode <<= 1;
		state.srom_opcode |= ibit;
		/*  debug("opcode input '%i'\n", ibit);  */
	} else {
		switch (state.srom_opcode) {
		case TULIP_SROM_OPC_READ:
			if (state.srom_curbit < 6 + 3) {
				obit = state.srom_curbit < 6 + 2;
				state.srom_addr <<= 1;
				state.srom_addr |= ibit;
			} else {
				uint16_t romword = state.srom[state.srom_addr*2]
				    + (state.srom[state.srom_addr*2+1] << 8);
//				if (state.srom_curbit == 6 + 3)
//					debug("[ dec21143: ROM read from offset 0x%03x: 0x%04x ]\n", state.srom_addr, romword);
				obit = romword & (0x8000 >> (state.srom_curbit - 6 - 3))? 1 : 0;
			}
			break;
		default:
          fatal("[ dec21243: unimplemented SROM/EEPROM opcode %i ]\n", state.srom_opcode);
		}
		state.reg[CSR_MIIROM / 8] &= ~MIIROM_SROMDO;
		if (obit)
			state.reg[CSR_MIIROM / 8] |= MIIROM_SROMDO;
		/*  debug("input '%i', output '%i'\n", ibit, obit);  */
	}

	state.srom_curbit ++;

	/*
	 *  Done opcode + addr + data? Then restart. (At least NetBSD does
	 *  sequential reads without turning selection off and then on.)
	 */
	if (state.srom_curbit >= 3 + 6 + 16) {
		state.srom_curbit = 0;
		state.srom_opcode = 0;
		state.srom_opcode_has_started = 0;
		state.srom_addr = 0;
	}
}

/*
 *  dec21143_rx():
 *
 *  Receive a packet. (If there is no current packet, then check for newly
 *  arrived ones. If the current packet couldn't be fully transfered the
 *  last time, then continue on that packet.)
 */
int CDEC21143::dec21143_rx()
{
	uint64_t addr = state.cur_rx_addr, bufaddr;
	unsigned char descr[16];
	uint32_t rdes0, rdes1, rdes2, rdes3;
	int bufsize, buf1_size, buf2_size, writeback_len = 4, to_xfer;
    struct pcap_pkthdr * packet_header;
    const u_char * packet_data = NULL;

	/*  No current packet? Then check for new ones.  */
	if (state.cur_rx_buf == NULL) {
  	  /*  Nothing available? Then abort.  */
      if (!pcap_next_ex( fp, &packet_header, &packet_data))
	 	return 0;

//        printf("pcap recv: %d bytes (%d captured) for %02x:%02x:%02x:%02x:%02x:%02x  \n",packet_header->len, packet_header->caplen,packet_data[0],packet_data[1],packet_data[2],packet_data[3],packet_data[4],packet_data[5]);

	    state.cur_rx_buf_len = packet_header->caplen;

		/*  Append a 4 byte CRC:  */
		state.cur_rx_buf_len += 4;
		CHECK_REALLOCATION(state.cur_rx_buf, realloc(state.cur_rx_buf, state.cur_rx_buf_len), u8);

	    /*  Get the next packet into our buffer:  */
	    memcpy(state.cur_rx_buf, packet_data, state.cur_rx_buf_len);

		/*  Well... the CRC is just zeros, for now.  */
		memset(state.cur_rx_buf + state.cur_rx_buf_len - 4, 0, 4);

		state.cur_rx_offset = 0;
	}

    addr = cSystem->PCI_Phys(0,addr&0xffffffff);

    memcpy(descr,cSystem->PtrToMem(addr),sizeof(u32)*4);

	rdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);
	rdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	rdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	rdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

	/*  Only use descriptors owned by the 21143:  */
	if (!(rdes0 & TDSTAT_OWN)) {
		state.reg[CSR_STATUS/8] |= STATUS_RU;
		return 0;
	}

    buf1_size = rdes1 & TDCTL_SIZE1;
	buf2_size = (rdes1 & TDCTL_SIZE2) >> TDCTL_SIZE2_SHIFT;
	bufaddr = buf1_size? rdes2 : rdes3;
	bufsize = buf1_size? buf1_size : buf2_size;

	state.reg[CSR_STATUS/8] &= ~STATUS_RS;

	if (rdes1 & TDCTL_ER)
		state.cur_rx_addr = state.reg[CSR_RXLIST / 8];
	else {
		if (rdes1 & TDCTL_CH)
			state.cur_rx_addr = rdes3;
		else
			state.cur_rx_addr += 16;
	}

  //  fatal("{ dec21143_rx: base = 0x%08x }\n", (int)addr);


//	debug("{ RX (%llx): 0x%08x 0x%08x 0x%x 0x%x: buf %i bytes at 0x%x }\n",
//	    (long long)addr, rdes0, rdes1, rdes2, rdes3, bufsize, (int)bufaddr);

    bufaddr = cSystem->PCI_Phys(0,bufaddr&0xffffffff);

	/*  Turn off all status bits, and give up ownership:  */
	rdes0 = 0x00000000;

	to_xfer = state.cur_rx_buf_len - state.cur_rx_offset;
	if (to_xfer > bufsize)
		to_xfer = bufsize;

	/*  DMA bytes from the packet into emulated physical memory:  */
    memcpy(cSystem->PtrToMem(bufaddr), state.cur_rx_buf + state.cur_rx_offset, to_xfer);

	/*  Was this the first buffer in a frame? Then mark it as such.  */
	if (state.cur_rx_offset == 0)
		rdes0 |= TDSTAT_Rx_FS;

	state.cur_rx_offset += to_xfer;

	/*  Frame completed?  */
	if (state.cur_rx_offset >= state.cur_rx_buf_len) {
//        debug("frame complete.\n");
		rdes0 |= TDSTAT_Rx_LS;

		/*  Set the frame length:  */
		rdes0 |= (state.cur_rx_buf_len << 16) & TDSTAT_Rx_FL;

		/*  Frame too long? (1518 is max ethernet frame length)  */
		if (state.cur_rx_buf_len > 1518)
			rdes0 |= TDSTAT_Rx_TL;

		/*  Cause a receiver interrupt:  */
		state.reg[CSR_STATUS/8] |= STATUS_RI;

		free(state.cur_rx_buf);
		state.cur_rx_buf = NULL;
		state.cur_rx_buf_len = 0;
	}

	/*  Descriptor writeback:  */
	descr[ 0] = (u8) rdes0;       descr[ 1] = (u8) (rdes0 >> 8);
	descr[ 2] = (u8) (rdes0 >> 16); descr[ 3] = (u8) (rdes0 >> 24);
	if (writeback_len > 1) {
		descr[ 4] = (u8) rdes1;       descr[ 5] = (u8) (rdes1 >> 8);
		descr[ 6] = (u8) (rdes1 >> 16); descr[ 7] = (u8) (rdes1 >> 24);
		descr[ 8] = (u8) rdes2;       descr[ 9] = (u8) (rdes2 >> 8);
		descr[10] = (u8) (rdes2 >> 16); descr[11] = (u8) (rdes2 >> 24);
		descr[12] = (u8) rdes3;       descr[13] = (u8) (rdes3 >> 8);
		descr[14] = (u8) (rdes3 >> 16); descr[15] = (u8) (rdes3 >> 24);
	}

    memcpy(cSystem->PtrToMem(addr), descr, sizeof(u32));

	return 1;
}


/*
 *  dec21143_tx():
 *
 *  Transmit a packet, if the guest OS has marked a descriptor as containing
 *  data to transmit.
 */
int CDEC21143::dec21143_tx()
{
	uint64_t addr = state.cur_tx_addr, bufaddr;
	unsigned char descr[16];
	uint32_t tdes0, tdes1, tdes2, tdes3;
	int bufsize, buf1_size, buf2_size;

    if (state.tx_suspend)
        return 0;

    addr = cSystem->PCI_Phys(0,addr & 0xffffffff);

    memcpy(descr, cSystem->PtrToMem(addr), 16);

	tdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);
	tdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	tdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	tdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

	/*  fatal("{ dec21143_tx: base=0x%08x, tdes0=0x%08x }\n", (int)addr, (int)tdes0);  */

	/*  Only process packets owned by the 21143:  */
	if (!(tdes0 & TDSTAT_OWN)) {
		if (state.tx_idling > state.tx_idling_threshold) {
			state.reg[CSR_STATUS/8] |= STATUS_TU;
            state.tx_suspend = true;
			state.tx_idling = 0;
		} else
			state.tx_idling ++;
		return 0;
	}

	buf1_size = tdes1 & TDCTL_SIZE1;
	buf2_size = (tdes1 & TDCTL_SIZE2) >> TDCTL_SIZE2_SHIFT;
	bufaddr = buf1_size? tdes2 : tdes3;
	bufsize = buf1_size? buf1_size : buf2_size;

	state.reg[CSR_STATUS/8] &= ~STATUS_TS;

	if (tdes1 & TDCTL_ER)
		state.cur_tx_addr = state.reg[CSR_TXLIST / 8];
	else {
		if (tdes1 & TDCTL_CH)
			state.cur_tx_addr = tdes3;
		else
			state.cur_tx_addr += 4 * sizeof(uint32_t);
	}

	/*
	fatal("{ TX (%llx): 0x%08x 0x%08x 0x%x 0x%x: buf %i bytes at 0x%x }\n",
	  (long long)addr, tdes0, tdes1, tdes2, tdes3, bufsize, (int)bufaddr);
	*/
    bufaddr = cSystem->PCI_Phys(0,bufaddr&0xffffffff);

	/*  Assume no error:  */
	tdes0 &= ~ (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO | TDSTAT_ES);

	if (tdes1 & TDCTL_Tx_SET) {
		/*
		 *  Setup Packet.
		 *
		 *  TODO. For now, just ignore it, and pretend it worked.
		 */
//		fatal("{ TX: setup packet }\n");
        if (bufsize != 192)
			fatal("[ dec21143: setup packet len = %i, should be 192! ]\n", (int)bufsize);
  //      else
  //        for (i=0;i<192;i+=12)
  //            printf("%02" LL "x:%02" LL "x:%02" LL "x:%02" LL "x:%02" LL "x:%02" LL "x \n",cSystem->ReadMem(bufaddr+i,8),cSystem->ReadMem(bufaddr+i+1,8),cSystem->ReadMem(bufaddr+i+4,8),cSystem->ReadMem(bufaddr+i+5,8),cSystem->ReadMem(bufaddr+i+8,8),cSystem->ReadMem(bufaddr+i+9,8));
        if (memcmp(state.setup_filter, cSystem->PtrToMem(bufaddr),192))
        {
          memcpy(state.setup_filter, cSystem->PtrToMem(bufaddr),192);
          SetupFilter();
        }
		if (tdes1 & TDCTL_Tx_IC)
			state.reg[CSR_STATUS/8] |= STATUS_TI;
		/*  New descriptor values, according to the docs:  */
		tdes0 = 0x7fffffff; tdes1 = 0xffffffff;
		tdes2 = 0xffffffff; tdes3 = 0xffffffff;
	} else {
		/*
		 *  Data Packet.
		 */
//		fatal("{ TX: data packet: ");
		if (tdes1 & TDCTL_Tx_FS) {
			/*  First segment. Let's allocate a new buffer:  */
			/*  fatal("new frame }\n");  */

			CHECK_ALLOCATION(state.cur_tx_buf = (u8 *)malloc(bufsize));
			state.cur_tx_buf_len = 0;
		} else {
			/*  Not first segment. Increase the length of the current buffer:  */
			/*  fatal("continuing last frame }\n");  */

			if (state.cur_tx_buf == NULL)
				fatal("[ dec21143: WARNING! tx: middle segment, but no first segment?! ]\n");

			CHECK_REALLOCATION(state.cur_tx_buf, realloc(state.cur_tx_buf, state.cur_tx_buf_len + bufsize), u8);
		}

		/*  "DMA" data from emulated physical memory into the buf:  */
        memcpy(state.cur_tx_buf + state.cur_tx_buf_len, cSystem->PtrToMem(bufaddr), bufsize);

		state.cur_tx_buf_len += bufsize;

		/*  Last segment? Then actually transmit it:  */
		if (tdes1 & TDCTL_Tx_LS) {
			/*  fatal("{ TX: data frame complete. }\n");  */
            if (pcap_sendpacket(fp, state.cur_tx_buf, state.cur_tx_buf_len))
              fatal("Error sending the packet: %s\n",pcap_geterr);

//            printf("pcap send: %d bytes   \n",state.cur_tx_buf_len);

			free(state.cur_tx_buf);
			state.cur_tx_buf = NULL;
			state.cur_tx_buf_len = 0;

			/*  Interrupt, if Tx_IC is set:  */
			if (tdes1 & TDCTL_Tx_IC)
				state.reg[CSR_STATUS/8] |= STATUS_TI;
		}

		/*  We are done with this segment.  */
		tdes0 &= ~TDSTAT_OWN;
	}

	/*  Error summary:  */
	if (tdes0 & (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO))
		tdes0 |= TDSTAT_ES;

	/*  Descriptor writeback:  */
	descr[ 0] = (u8) tdes0;       descr[ 1] = (u8) (tdes0 >> 8);
	descr[ 2] = (u8) (tdes0 >> 16); descr[ 3] = (u8) (tdes0 >> 24);
	descr[ 4] = (u8) tdes1;       descr[ 5] = (u8) (tdes1 >> 8);
	descr[ 6] = (u8) (tdes1 >> 16); descr[ 7] = (u8) (tdes1 >> 24);
	descr[ 8] = (u8) tdes2;       descr[ 9] = (u8) (tdes2 >> 8);
	descr[10] = (u8) (tdes2 >> 16); descr[11] = (u8) (tdes2 >> 24);
	descr[12] = (u8) tdes3;       descr[13] = (u8) (tdes3 >> 8);
	descr[14] = (u8) (tdes3 >> 16); descr[15] = (u8) (tdes3 >> 24);

    memcpy(cSystem->PtrToMem(addr),descr,sizeof(u32)*4);

	return 1;
}


void CDEC21143::SetupFilter()
{
  u8 mac[16][6];
  char mac_txt[16][20];
  char filter[1000];
  int i,j;
  int numUnique;
  int unique[16];
  bool u;
  //printf("Building a filter...\n");
  for (i=0;i<16;i++)
  {
    mac[i][0] = state.setup_filter[i*12];
    mac[i][1] = state.setup_filter[i*12+1];
    mac[i][2] = state.setup_filter[i*12+4];
    mac[i][3] = state.setup_filter[i*12+5];
    mac[i][4] = state.setup_filter[i*12+8];
    mac[i][5] = state.setup_filter[i*12+9];
    sprintf(mac_txt[i],"%02x:%02x:%02x:%02x:%02x:%02x",mac[i][0],mac[i][1],mac[i][2],mac[i][3],mac[i][4],mac[i][5]);
  //  printf("MAC[%d] = %s. \n",i,mac_txt[i]);
  }

  /*
  printf("Filter mode: ");
  if (state.reg[CSR_OPMODE/8] & OPMODE_PR)
    printf("promiscuous.\n");
  else
  {
    if (state.reg[CSR_OPMODE/8] & OPMODE_IF)
      printf("inverse ");
    if (state.reg[CSR_OPMODE/8] & OPMODE_HP)
    {
      printf("hash ");
      if (state.reg[CSR_OPMODE/8] & OPMODE_HO)
        printf("only ");
    }
    else
      printf("perfect ");
    printf("filtering.\n");
  }
  */
  numUnique = 0;
  for (i=0;i<16;i++)
  {
    u = true;
    for(j=0;j<numUnique;j++)
    {
      if (mac[i][0]==mac[unique[j]][0] && mac[i][1]==mac[unique[j]][1] && mac[i][2]==mac[unique[j]][2] && mac[i][3]==mac[unique[j]][3] && mac[i][4]==mac[unique[j]][4] && mac[i][5]==mac[unique[j]][5])
      {
        u = false;
        break;
      }
    }
    if (u)
    {
      unique[numUnique] = i;
      numUnique++;
    }
  }
  /*
  for (i=0;i<numUnique;i++)
    printf("Unique MAC[%d] = %s. \n",i,mac_txt[unique[i]]);
  */
  filter[0] = '\0';
  strcat(filter,"ether broadcast");
  for (i=0;i<numUnique;i++)
  {
    strcat(filter," or ether dst ");
    strcat(filter,mac_txt[unique[i]]);
  }
  //printf("FILTER = %s.   \n",filter);

  if (pcap_compile(fp,&fcode,filter,1,0xffffffff)<0)
    FAILURE("Unable to compile the packet filter. Check the syntax.\n");

  if (pcap_setfilter(fp, &fcode)<0)
    FAILURE("Error setting the filter.\n");

//  getchar();
}


void CDEC21143::ResetPCI()
{
  CPCIDevice::ResetPCI();

  ResetNIC();
}

void CDEC21143::ResetNIC()
{
  int leaf;

	if (state.cur_rx_buf != NULL)
		free(state.cur_rx_buf);
	if (state.cur_tx_buf != NULL)
		free(state.cur_tx_buf);
	state.cur_rx_buf = state.cur_tx_buf = NULL;

	memset(state.reg, 0, sizeof(uint32_t) * 32);
	memset(state.srom, 0, sizeof(state.srom));
	memset(state.mii_phy_reg, 0, sizeof(state.mii_phy_reg));

	/*  Register values at reset, according to the manual:  */
	state.reg[CSR_BUSMODE / 8] = 0xfe000000;	/*  csr0   */
	state.reg[CSR_MIIROM  / 8] = 0xfff483ff;	/*  csr9   */
	state.reg[CSR_SIACONN / 8] = 0xffff0000;	/*  csr13  */
	state.reg[CSR_SIATXRX / 8] = 0xffffffff;	/*  csr14  */
	state.reg[CSR_SIAGEN  / 8] = 0x8ff00000;	/*  csr15  */

	state.tx_idling_threshold = 10;
	state.cur_rx_addr = state.cur_tx_addr = 0;

	/*  Version (= 1) and Chip count (= 1):  */
	state.srom[TULIP_ROM_SROM_FORMAT_VERION] = 1;
	state.srom[TULIP_ROM_CHIP_COUNT] = 1;

	/*  Set the MAC address:  */
	memcpy(state.srom + TULIP_ROM_IEEE_NETWORK_ADDRESS, state.mac, 6);

	leaf = 30;
	state.srom[TULIP_ROM_CHIPn_DEVICE_NUMBER(0)] = 0;
	state.srom[TULIP_ROM_CHIPn_INFO_LEAF_OFFSET(0)] = leaf & 255;
	state.srom[TULIP_ROM_CHIPn_INFO_LEAF_OFFSET(0)+1] = leaf >> 8;

	state.srom[leaf+TULIP_ROM_IL_SELECT_CONN_TYPE] = 0; /*  Not used?  */
	state.srom[leaf+TULIP_ROM_IL_MEDIA_COUNT] = 2;
	leaf += TULIP_ROM_IL_MEDIAn_BLOCK_BASE;

	state.srom[leaf] = 7;	/*  descriptor length  */
	state.srom[leaf+1] = TULIP_ROM_MB_21142_SIA;
	state.srom[leaf+2] = TULIP_ROM_MB_MEDIA_100TX;
	/*  here comes 4 bytes of GPIO control/data settings  */
	leaf += state.srom[leaf];

	state.srom[leaf] = 15;	/*  descriptor length  */
	state.srom[leaf+1] = TULIP_ROM_MB_21142_MII;
	state.srom[leaf+2] = 0;	/*  PHY nr  */
	state.srom[leaf+3] = 0;	/*  len of select sequence  */
	state.srom[leaf+4] = 0;	/*  len of reset sequence  */
	/*  5,6, 7,8, 9,10, 11,12, 13,14 = unused by GXemul  */
	leaf += state.srom[leaf];

	/*  MII PHY initial state:  */
	state.mii_state = MII_STATE_RESET;

	/*  PHY #0:  */
	state.mii_phy_reg[MII_BMSR] = BMSR_100TXFDX | BMSR_10TFDX | BMSR_ACOMP | BMSR_ANEG | BMSR_LINK;

    state.tx_suspend = false;

//    SetupFilter();
}

static u32 nic_magic1 = 0xDEC21143;
static u32 nic_magic2 = 0x21143DEC;

/**
 * Save state to a Virtual Machine State file.
 **/

int CDEC21143::SaveState(FILE *f)
{
  long ss = sizeof(state);
  int res;

  if (res = CPCIDevice::SaveState(f))
    return res;

  fwrite(&nic_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&nic_magic2,sizeof(u32),1,f);
  printf("%s: %d bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CDEC21143::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  int res;
  size_t r;

  if (res = CPCIDevice::RestoreState(f))
    return res;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != nic_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&state,sizeof(state),1,f);
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
  if (m2 != nic_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  printf("%s: %d bytes restored.\n",devid_string,ss);
  return 0;
}

#endif //!defined(NO_NETWORK)
