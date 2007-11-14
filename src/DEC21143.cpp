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
 * X-1.1        Camiel Vanderhoeven                             14-NOV-2007
 *      Initial version for ES40 emulator.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"
#include "DEC21143.h"
#include "System.h"

/*  Internal states during MII data stream decode:  */
#define	MII_STATE_RESET				    0
#define	MII_STATE_START_WAIT			1
#define	MII_STATE_READ_OP			    2
#define	MII_STATE_READ_PHYADDR_REGADDR	3
#define	MII_STATE_A				        4
#define	MII_STATE_D				        5
#define	MII_STATE_IDLE				    6

/**
 * Constructor.
 **/

CDEC21143::CDEC21143(CSystem * c): CSystemComponent(c)
{
  state.mac[0] = 0x12;
  state.mac[1] = 0x12;
  state.mac[2] = 0x12;
  state.mac[3] = 0x12;
  state.mac[4] = 0x12;
  state.mac[5] = 0x12;

  this->net = new CNetwork(NET_INIT_FLAG_GATEWAY,"10.0.0.0",8);

  c->RegisterClock(this, true);

  ResetPCI();

  printf("%%NIC-I-INIT: DEC 21143 network interface card emulator initialized.\n");
}

CDEC21143::~CDEC21143()
{
}

u64 CDEC21143::ReadMem(int index, u64 address, int dsize)
{
  int channel = 0;
  switch(index)
    {
    case 1:
      return endian_bits(config_read(address, dsize), dsize);
    case 2:
    case 3:
      return nic_read(address, dsize);
    }

  return 0;
}

void CDEC21143::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int channel = 0;
  switch(index)
    {
    case 1:
      config_write(address, dsize, endian_bits(data, dsize));
      return;
    case 2:		// CBIO
    case 3:		// CBMA
      nic_write(address, dsize, endian_bits(data, dsize));
      return;
    }
}

int CDEC21143::DoClock()
{
    int asserted;

	if (state.reg[CSR_OPMODE / 8] & OPMODE_ST)
		while (dec21143_tx());

	if (state.reg[CSR_OPMODE / 8] & OPMODE_SR)
		while (dec21143_rx());

	/*  Normal and Abnormal interrupt summary:  */
	state.reg[CSR_STATUS / 8] &= ~(STATUS_NIS | STATUS_AIS);
	if (state.reg[CSR_STATUS / 8] & 0x00004845)
		state.reg[CSR_STATUS / 8] |= STATUS_NIS;
	if (state.reg[CSR_STATUS / 8] & 0x0c0037ba)
		state.reg[CSR_STATUS / 8] |= STATUS_AIS;

	asserted = state.reg[CSR_STATUS / 8] & state.reg[CSR_INTEN / 8] & 0x0c01ffff;

	if (asserted)
        cSystem->interrupt(state.config_data[0x3c], true);
	if (!asserted && state.irq_was_asserted)
		cSystem->interrupt(state.config_data[0x3c], false);

	/*  Remember assertion flag:  */
	state.irq_was_asserted = asserted;


  return 0;
}

u64 CDEC21143::nic_read(u64 address, int dsize)
{
  u64 data;

    printf("nic_read %" LL "x\n",address);

	uint32_t oldreg = 0;
	int regnr = (int)(address >> 3);

	if ((address & 7) == 0 && regnr < 32) {
	  data = state.reg[regnr];
	} else
		fatal("[ dec21143: WARNING! unaligned access (0x%x) ]\n", (int)address);

	switch (address) {

	case CSR_BUSMODE:	/*  csr0  */
		break;

	case CSR_TXPOLL:	/*  csr1  */
		fatal("[ dec21143: UNIMPLEMENTED READ from txpoll ]\n");
		break;

	case CSR_RXPOLL:	/*  csr2  */
		fatal("[ dec21143: UNIMPLEMENTED READ from rxpoll ]\n");
		break;

	case CSR_RXLIST:	/*  csr3  */
	case CSR_TXLIST:	/*  csr4  */
	case CSR_STATUS:	/*  csr5  */
	case CSR_INTEN:		/*  csr7  */
	case CSR_OPMODE:	/*  csr6:  */
	case CSR_MISSED:	/*  csr8  */
	case CSR_MIIROM:	/*  csr9  */
		break;

	case CSR_SIASTAT:	/*  csr12  */
		/*  Auto-negotiation status = Good.  */
		data = SIASTAT_ANS_FLPGOOD;
		break;

	case CSR_SIATXRX:	/*  csr14  */
		/*  Auto-negotiation Enabled  */
		data = SIATXRX_ANE;
		break;

	case CSR_SIACONN:	/*  csr13  */
	case CSR_SIAGEN:	/*  csr15  */
		break;

	default:
        fatal("[ dec21143: read from unimplemented 0x%02x ]\n", (int)address);
	}

  return data;
}

/**
 * Write to the PCI configuration space.
 **/

void CDEC21143::nic_write(u64 address, int dsize, u64 data)
{
	uint32_t oldreg = 0;

    printf("nic_write %" LL "x, %" LL "x\n",address,data);
    
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
			ResetPCI();
		    data &= ~BUSMODE_SWR;
		}
		break;

	case CSR_TXPOLL:	/*  csr1  */
		state.tx_idling = state.tx_idling_threshold;
        DoClock();
		break;

	case CSR_RXPOLL:	/*  csr2  */
        DoClock();
		break;

	case CSR_RXLIST:	/*  csr3  */
		debug("[ dec21143: setting RXLIST to 0x%x ]\n", (int)data);
		if (data & 0x3)
			fatal("[ dec21143: WARNING! RXLIST not aligned? (0x%" LL "x) ]\n", (long long)data);
		data &= ~0x3;
		state.cur_rx_addr = data;
		break;

	case CSR_TXLIST:	/*  csr4  */
		debug("[ dec21143: setting TXLIST to 0x%x ]\n", (int)data);
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
		data &= ~(OPMODE_HBD | OPMODE_SCR | OPMODE_PCS | OPMODE_PS | OPMODE_SF | OPMODE_TTM | OPMODE_FD);
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
	case CSR_SIATXRX:	/*  csr14  */
	case CSR_SIACONN:	/*  csr13  */
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
			/*  fatal("[ mii_access(): got first opcode bit "
			    "(%i) ]\n", obit);  */
		} else {
			state.mii_opcode |= obit;
			/*  fatal("[ mii_access(): got opcode = %i ]\n",
			    state.mii_opcode);  */
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
		default:debug("[ mii_access(): UNIMPLEMENTED MII opcode "
			    "%i (probably just a bug in GXemul's "
			    "MII data stream handling) ]\n", state.mii_opcode);
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
				debug("[ mii_access(): WRITE to phyaddr=0x%x "
				    "regaddr=0x%x: 0x%04x ]\n", state.mii_phyaddr,
				    state.mii_regaddr, tmp);
			}
			break;
		case MII_COMMAND_READ:
			if (!(idata & MIIROM_MIIDIR))
				break;
			tmp = state.mii_phy_reg[(state.mii_phyaddr << 5) +
			    state.mii_regaddr];
			if (state.mii_bit == 13)
				debug("[ mii_access(): READ phyaddr=0x%x "
				    "regaddr=0x%x: 0x%04x ]\n", state.mii_phyaddr,
				    state.mii_regaddr, tmp);
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
				if (state.srom_curbit == 6 + 3)
					debug("[ dec21143: ROM read from offset 0x%03x: 0x%04x ]\n",
					    state.srom_addr, romword);
				obit = romword & (0x8000 >>
				    (state.srom_curbit - 6 - 3))? 1 : 0;
			}
			break;
		default:fatal("[ dec21243: unimplemented SROM/EEPROM opcode %i ]\n", state.srom_opcode);
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
	int bufsize, buf1_size, buf2_size, i, writeback_len = 4, to_xfer;

	/*  No current packet? Then check for new ones.  */
	if (state.cur_rx_buf == NULL) {
		/*  Nothing available? Then abort.  */
		if (!net->net_ethernet_rx_avail())
			return 0;

		/*  Get the next packet into our buffer:  */
		net->net_ethernet_rx(&state.cur_rx_buf, &state.cur_rx_buf_len);

		/*  Append a 4 byte CRC:  */
		state.cur_rx_buf_len += 4;
		CHECK_ALLOCATION(state.cur_rx_buf = (u8 *) realloc(state.cur_rx_buf,
		    state.cur_rx_buf_len));

		/*  Well... the CRC is just zeros, for now.  */
		memset(state.cur_rx_buf + state.cur_rx_buf_len - 4, 0, 4);

		state.cur_rx_offset = 0;
	}

	/*  fatal("{ dec21143_rx: base = 0x%08x }\n", (int)addr);  */
	addr &= 0x7fffffff;

    memcpy(descr,cSystem->PtrToMem(addr),sizeof(u32));
//	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t),
//	    MEM_READ, PHYSICAL | NO_EXCEPTIONS)) {
//		fatal("[ dec21143_rx: memory_rw failed! ]\n");
//		return 0;
//	}

	rdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);

	/*  Only use descriptors owned by the 21143:  */
	if (!(rdes0 & TDSTAT_OWN)) {
		state.reg[CSR_STATUS/8] |= STATUS_RU;
		return 0;
	}

    memcpy(descr+sizeof(u32),cSystem->PtrToMem(addr+sizeof(u32)),sizeof(u32)*3);
//    if (!cpu->memory_rw(cpu, cpu->mem, addr + sizeof(uint32_t), descr +
//	    sizeof(uint32_t), sizeof(uint32_t) * 3, MEM_READ, PHYSICAL |
//	    NO_EXCEPTIONS)) {
//		fatal("[ dec21143_rx: memory_rw failed! ]\n");
//		return 0;
//	}

	rdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	rdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	rdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

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
			state.cur_rx_addr += 4 * sizeof(uint32_t);
	}

	debug("{ RX (%llx): 0x%08x 0x%08x 0x%x 0x%x: buf %i bytes at 0x%x }\n",
	    (long long)addr, rdes0, rdes1, rdes2, rdes3, bufsize, (int)bufaddr);
	bufaddr &= 0x7fffffff;

	/*  Turn off all status bits, and give up ownership:  */
	rdes0 = 0x00000000;

	to_xfer = state.cur_rx_buf_len - state.cur_rx_offset;
	if (to_xfer > bufsize)
		to_xfer = bufsize;

	/*  DMA bytes from the packet into emulated physical memory:  */
	for (i=0; i<to_xfer; i++) {
        memcpy(cSystem->PtrToMem(bufaddr+i), state.cur_rx_buf + state.cur_rx_offset + i, 1);
//		cpu->memory_rw(cpu, cpu->mem, bufaddr + i,
//		    state.cur_rx_buf + state.cur_rx_offset + i, 1, MEM_WRITE,
//		    PHYSICAL | NO_EXCEPTIONS);
		/*  fatal(" %02x", state.cur_rx_buf[state.cur_rx_offset + i]);  */
	}

	/*  Was this the first buffer in a frame? Then mark it as such.  */
	if (state.cur_rx_offset == 0)
		rdes0 |= TDSTAT_Rx_FS;

	state.cur_rx_offset += to_xfer;

	/*  Frame completed?  */
	if (state.cur_rx_offset >= state.cur_rx_buf_len) {
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
//	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t)
//	    * writeback_len, MEM_WRITE, PHYSICAL | NO_EXCEPTIONS)) {
//		fatal("[ dec21143_rx: memory_rw failed! ]\n");
//		return 0;
//	}

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
	int bufsize, buf1_size, buf2_size, i;

	addr &= 0x7fffffff;

    memcpy(descr, cSystem->PtrToMem(addr), sizeof(u32));
//	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t),
//	    MEM_READ, PHYSICAL | NO_EXCEPTIONS)) {
//		fatal("[ dec21143_tx: memory_rw failed! ]\n");
//		return 0;
//	}

	tdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);

	/*  fatal("{ dec21143_tx: base=0x%08x, tdes0=0x%08x }\n",
	    (int)addr, (int)tdes0);  */

	/*  Only process packets owned by the 21143:  */
	if (!(tdes0 & TDSTAT_OWN)) {
		if (state.tx_idling > state.tx_idling_threshold) {
			state.reg[CSR_STATUS/8] |= STATUS_TU;
			state.tx_idling = 0;
		} else
			state.tx_idling ++;
		return 0;
	}

    memcpy(descr + sizeof(u32), cSystem->PtrToMem(addr + sizeof(u32)), sizeof(u32)*3);
//	if (!cpu->memory_rw(cpu, cpu->mem, addr + sizeof(uint32_t), descr +
//	    sizeof(uint32_t), sizeof(uint32_t) * 3, MEM_READ, PHYSICAL |
//	    NO_EXCEPTIONS)) {
//		fatal("[ dec21143_tx: memory_rw failed! ]\n");
//		return 0;
//	}

	tdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	tdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	tdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

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
	bufaddr &= 0x7fffffff;

	/*  Assume no error:  */
	tdes0 &= ~ (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC
	    | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO | TDSTAT_ES);

	if (tdes1 & TDCTL_Tx_SET) {
		/*
		 *  Setup Packet.
		 *
		 *  TODO. For now, just ignore it, and pretend it worked.
		 */
		/*  fatal("{ TX: setup packet }\n");  */
		if (bufsize != 192)
			fatal("[ dec21143: setup packet len = %i, should be"
			    " 192! ]\n", (int)bufsize);
		if (tdes1 & TDCTL_Tx_IC)
			state.reg[CSR_STATUS/8] |= STATUS_TI;
		/*  New descriptor values, according to the docs:  */
		tdes0 = 0x7fffffff; tdes1 = 0xffffffff;
		tdes2 = 0xffffffff; tdes3 = 0xffffffff;
	} else {
		/*
		 *  Data Packet.
		 */
		/*  fatal("{ TX: data packet: ");  */
		if (tdes1 & TDCTL_Tx_FS) {
			/*  First segment. Let's allocate a new buffer:  */
			/*  fatal("new frame }\n");  */

			CHECK_ALLOCATION(state.cur_tx_buf = (u8 *)malloc(bufsize));
			state.cur_tx_buf_len = 0;
		} else {
			/*  Not first segment. Increase the length of
			    the current buffer:  */
			/*  fatal("continuing last frame }\n");  */

			if (state.cur_tx_buf == NULL)
				fatal("[ dec21143: WARNING! tx: middle segment, but no first segment?! ]\n");

			CHECK_ALLOCATION(state.cur_tx_buf = (u8 *) realloc(state.cur_tx_buf,
			    state.cur_tx_buf_len + bufsize));
		}

		/*  "DMA" data from emulated physical memory into the buf:  */
		for (i=0; i<bufsize; i++) {
            memcpy(state.cur_tx_buf + state.cur_tx_buf_len + i, cSystem->PtrToMem(bufaddr + i), 1);
//			cpu->memory_rw(cpu, cpu->mem, bufaddr + i,
//			    state.cur_tx_buf + state.cur_tx_buf_len + i, 1, MEM_READ,
//			    PHYSICAL | NO_EXCEPTIONS);
			/*  fatal(" %02x", state.cur_tx_buf[
			    state.cur_tx_buf_len + i]);  */
		}

		state.cur_tx_buf_len += bufsize;

		/*  Last segment? Then actually transmit it:  */
		if (tdes1 & TDCTL_Tx_LS) {
			/*  fatal("{ TX: data frame complete. }\n");  */
			net->net_ethernet_tx(state.cur_tx_buf, state.cur_tx_buf_len);

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
	if (tdes0 & (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC
	    | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO))
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
//	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t)
//	    * 4, MEM_WRITE, PHYSICAL | NO_EXCEPTIONS)) {
//		fatal("[ dec21143_tx: memory_rw failed! ]\n");
//		return 0;
//	}

	return 1;
}

/**
 * Read from the PCI configuration space.
 **/

u64 CDEC21143::config_read(u64 address, int dsize)
{
    
  u64 data;
  void * x;

  x = &(state.config_data[address]);

  switch (dsize)
    {
    case 8:
      data = (u64)(*((u8*)x))&0xff;
      break;
    case 16:
      data = (u64)(*((u16*)x))&0xffff;
      break;
    case 32:
      data = (u64)(*((u32*)x))&0xffffffff;
      break;
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  return data;

}

/**
 * Write to the PCI configuration space.
 **/

void CDEC21143::config_write(u64 address, int dsize, u64 data)
{
  void * x;
  void * y;

  printf("cfg_write %" LL "x, %" LL "x\n",address,data);


  x = &(state.config_data[address]);
  y = &(state.config_mask[address]);

  switch (dsize)
    {
    case 8:
      *((u8*)x) = (*((u8*)x) & ~*((u8*)y)) | ((u8)data & *((u8*)y));
      break;
    case 16:
      *((u16*)x) = (*((u16*)x) & ~*((u16*)y)) | ((u16)data & *((u16*)y));
      break;
    case 32:
      *((u32*)x) = (*((u32*)x) & ~*((u32*)y)) | ((u32)data & *((u32*)y));
      break;
    case 64:
      *((u64*)x) = (*((u64*)x) & ~*((u64*)y)) | ((u64)data & *((u64*)y));
      break;
    }
  if (   ((data&0xffffffff)!=0xffffffff) 
	 && ((data&0xffffffff)!=0x00000000) 
	 && ( dsize           ==32        ))
    switch(address)
      {
      // CBIO
      case 0x10:
  	  cSystem->RegisterMemory(this,2, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 256);
      printf("cbio @ %016" LL "x\n",X64(00000801fc000000) + (endian_32(data)&0x00fffffe));
	  return;
      // CBMA
      case 0x14:
	  cSystem->RegisterMemory(this,3, X64(0000080000000000) + (endian_32(data)&0xfffffffe), 256);
      printf("cbma @ %016" LL "x\n",X64(0000080000000000) + (endian_32(data)&0xfffffffe));
	  return;
      }
}

void CDEC21143::ResetPCI()
{
  int i;

  cSystem->RegisterMemory(this, 1, X64(00000801fe003000),0x100);

  for (i=0;i<256;i++) 
    {
      state.config_data[i] = 0;
      state.config_mask[i] = 0;
    }
  state.config_data[0x00] = 0x11; // vendor id 1011h
  state.config_data[0x01] = 0x10;
  state.config_data[0x02] = 0x19; // device id 0019h
  state.config_data[0x03] = 0x00;
  state.config_data[0x04] = 0x00; // cfcs 02800000h
  state.config_data[0x05] = 0x00;
  state.config_data[0x06] = 0x80;
  state.config_data[0x07] = 0x02; 
  state.config_data[0x08] = 0x41; // cfrv 02000041h
  state.config_data[0x09] = 0x00;
  state.config_data[0x0a] = 0x00;
  state.config_data[0x0b] = 0x02;
  state.config_data[0x10] = 0x01; state.config_mask[0x10] = 0x00; // cbio xxxxxxx1h
  state.config_data[0x11] = 0x00; state.config_mask[0x11] = 0xff;	
  state.config_data[0x12] = 0x00; state.config_mask[0x12] = 0xff;
  state.config_data[0x13] = 0x00; state.config_mask[0x13] = 0xff;
  state.config_data[0x14] = 0x00; state.config_mask[0x14] = 0x00; // cbma xxxxxxx0h
  state.config_data[0x15] = 0x00; state.config_mask[0x15] = 0xff;	
  state.config_data[0x16] = 0x00; state.config_mask[0x16] = 0xff;
  state.config_data[0x17] = 0x00; state.config_mask[0x17] = 0xff;
  state.config_data[0x3c] = 0xff; state.config_mask[0x3c] = 0xff; // cfit 281401xxh
  state.config_data[0x3d] = 0x01;	
  state.config_data[0x3e] = 0x14;
  state.config_data[0x3f] = 0x28;

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


}

/**
 * Save state to a Virtual Machine State file.
 **/

void CDEC21143::SaveState(FILE *f)
{
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CDEC21143::RestoreState(FILE *f)
{
  fread(&state,sizeof(state),1,f);

  // allocations 
  config_write(0x10,32,(*((u32*)(&state.config_data[0x10])))&~1);
  config_write(0x14,32,(*((u32*)(&state.config_data[0x14])))&~1);
}
