/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
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
 * Contains the code for the emulated Symbios SCSI controller.
 *
 * $Id: Sym53C895.cpp,v 1.28 2008/03/14 15:30:52 iamcamiel Exp $
 *
 * X-1.27       Camiel Vanderhoeven                             14-MAR-2008
 *   1. More meaningful exceptions replace throwing (int) 1.
 *   2. U64 macro replaces X64 macro.
 *
 * X-1.26       Camiel Vanderhoeven                             13-MAR-2008
 *      Create init(), start_threads() and stop_threads() functions.
 *
 * X-1.25       Camiel Vanderhoeven                             11-MAR-2008
 *      Named, debuggable mutexes.
 *
 * X-1.24       Camiel Vanderhoeven                             05-MAR-2008
 *      Multi-threading version.
 *
 * X-1.23       Brian Wheeler                                   27-FEB-2008
 *      Avoid compiler warnings.
 *
 * X-1.22       Camiel Vanderhoeven                             16-FEB-2008
 *      Backported some of the improvements made in the 53C810 code.
 *
 * X-1.21       Camiel Vanderhoeven                             28-JAN-2008
 *      Avoid compiler warnings.
 *
 * X-1.20       Camiel Vanderhoeven                             24-JAN-2008
 *      Use new CPCIDevice::do_pci_read and CPCIDevice::do_pci_write.
 *
 * X-1.19       Camiel Vanderhoeven                             18-JAN-2008
 *      Replaced sext_64 inlines with sext_u64_<bits> inlines for
 *      performance reasons (thanks to David Hittner for spotting this!)
 *
 * X-1.18       Camiel Vanderhoeven                             12-JAN-2008
 *      Use disk's SCSI engine.
 *
 * X-1.17       Camiel Vanderhoeven                             06-JAN-2008
 *      Leave changing the blocksize to the disk itself.
 *
 * X-1.16       Camiel Vanderhoeven                             04-JAN-2008
 *      Less messages fprint'ed.
 *
 * X-1.15       Camiel Vanderhoeven                             02-JAN-2008
 *      Avoid compiler warnings.
 *
 * X-1.14       Camiel Vanderhoeven                             30-DEC-2007
 *      Print file id on initialization.
 *
 * X-1.13       Camiel Vanderhoeven                             29-DEC-2007
 *      Compileable with older compilers (VC 6.0).
 *
 * X-1.12       Camiel Vanderhoeven                             28-DEC-2007
 *      Throw exceptions rather than just exiting when errors occur.
 *
 * X-1.11       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.10       Camiel Vanderhoeven                             20-DEC-2007
 *      Do reselection on read commands.
 *
 * X-1.9        Camiel Vanderhoeven                             19-DEC-2007
 *      Allow for different blocksizes.
 *
 * X-1.8        Camiel Vanderhoeven                             18-DEC-2007
 *      Fixed silly mis-interpretation of "add-with-carry".
 *
 * X-1.7        Camiel Vanderhoeven                             18-DEC-2007
 *      Byte-sized transfers for SCSI controller.
 *
 * X-1.6        Camiel Vanderhoeven                             18-DEC-2007
 *      Removed some messages.
 *
 * X-1.5        Camiel Vanderhoeven                             18-DEC-2007
 *      Selection timeout occurs after the phase is checked the first time.
 *
 * X-1.4        Camiel Vanderhoeven                             17-DEC-2007
 *      Added general timer.
 *
 * X-1.3        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.2        Camiel Vanderhoeven                             16-DEC-2007
 *      Changed register structure.
 *
 * X-1.1        Camiel Vanderhoeven                             14-DEC-2007
 *      Initial version in CVS.
 **/
#if defined(DEBUG_SYM)
#define DEBUG_SYM_REGS
#define DEBUG_SYM_SCRIPTS
#endif
#include "StdAfx.h"
#include "Sym53C895.h"
#include "System.h"
#include "Disk.h"
#include "SCSIBus.h"

#define R_SCNTL0          0x00
#define R_SCNTL0_ARB1     0x80
#define R_SCNTL0_ARB0     0x40
#define R_SCNTL0_START    0x20
#define R_SCNTL0_WATN     0x10
#define R_SCNTL0_EPC      0x08
#define R_SCNTL0_AAP      0x02
#define R_SCNTL0_TRG      0x01
#define SCNTL0_MASK       0xFB

#define R_SCNTL1          0x01
#define R_SCNTL1_CON      0x10
#define R_SCNTL1_RST      0x08
#define R_SCNTL1_IARB     0x02

#define R_SCNTL2          0x02
#define R_SCNTL2_SDU      0x80
#define R_SCNTL2_CHM      0x40
#define R_SCNTL2_SLPMD    0x20
#define R_SCNTL2_SLPHBEN  0x10
#define R_SCNTL2_WSS      0x08
#define R_SCNTL2_VUE0     0x04
#define R_SCNTL2_VUE1     0x02
#define R_SCNTL2_WSR      0x01
#define SCNTL2_MASK       0xF2
#define SCNTL2_W1C        0x09

#define R_SCNTL3          0x03
#define R_SCNTL3_EWS      0x08

#define R_SCID            0x04
#define R_SCID_ID         0x0F
#define SCID_MASK         0x6F

#define R_SXFER           0x05

#define R_SDID            0x06
#define R_SDID_ID         0x0F
#define SDID_MASK         0x0F

#define R_GPREG           0x07
#define GPREG_MASK        0x1F

#define R_SFBR            0x08

#define R_SOCL            0x09
#define R_SOCL_ACK        0x40
#define R_SOCL_ATN        0x20

#define R_SSID            0x0A
#define R_SSID_VAL        0x80
#define R_SSID_ID         0x0F

#define R_SBCL            0x0B
#define R_SBCL_REQ        0x80
#define R_SBCL_ACK        0x40
#define R_SBCL_BSY        0x20
#define R_SBCL_SEL        0x10
#define R_SBCL_ATN        0x08
#define R_SBCL_MSG        0x04
#define R_SBCL_CD         0x02
#define R_SBCL_IO         0x01
#define R_SBCL_PHASE      0x07

#define R_DSTAT           0x0C
#define R_DSTAT_DFE       0x80
#define R_DSTAT_MDPE      0x40
#define R_DSTAT_BF        0x20
#define R_DSTAT_ABRT      0x10
#define R_DSTAT_SSI       0x08
#define R_DSTAT_SIR       0x04
#define R_DSTAT_IID       0x01
#define DSTAT_RC          0x7D
#define DSTAT_FATAL       0x7D

#define R_SSTAT0          0x0D
#define R_SSTAT0_RST      0x02
#define R_SSTAT0_SDP0     0x01

#define R_SSTAT1          0x0E
#define R_SSTAT1_SDP1     0x01

#define R_SSTAT2          0x0F
#define R_SSTAT2_LDSC     0x02

#define R_DSA             0x10

#define R_ISTAT           0x14
#define R_ISTAT_ABRT      0x80
#define R_ISTAT_SRST      0x40
#define R_ISTAT_SIGP      0x20
#define R_ISTAT_SEM       0x10
#define R_ISTAT_CON       0x08
#define R_ISTAT_INTF      0x04
#define R_ISTAT_SIP       0x02
#define R_ISTAT_DIP       0x01
#define ISTAT_MASK        0xF0
#define ISTAT_W1C         0x04

#define R_CTEST0          0x18

#define R_CTEST1          0x19
#define R_CTEST1_FMT      0xF0
#define R_CTEST1_FFL      0x0F

#define R_CTEST2          0x1A
#define R_CTEST2_DDIR     0x80
#define R_CTEST2_SIGP     0x40
#define R_CTEST2_CIO      0x20
#define R_CTEST2_CM       0x10
#define R_CTEST2_SRTCH    0x08
#define R_CTEST2_TEOP     0x04
#define R_CTEST2_DREQ     0x02
#define R_CTEST2_DACK     0x01
#define CTEST2_MASK       0x08
#define R_CTEST3          0x1B
#define R_CTEST3_REV      0xf0
#define R_CTEST3_FLF      0x08
#define R_CTEST3_CLF      0x04
#define R_CTEST3_FM       0x02
#define CTEST3_MASK       0x0B

#define R_TEMP            0x1C
#define R_DFIFO           0x20
#define R_CTEST4          0x21

#define R_CTEST5          0x22
#define R_CTEST5_ADCK     0x80
#define R_CTEST5_BBCK     0x40
#define CTEST5_MASK       0x3F

#define R_DBC             0x24
#define R_DCMD            0x27
#define R_DNAD            0x28
#define R_DSP             0x2C
#define R_DSPS            0x30
#define R_SCRATCHA        0x34

#define R_DMODE           0x38
#define R_DMODE_MAN       0x01

#define R_DIEN            0x39
#define DIEN_MASK         0x7D

#define R_SBR             0x3A

#define R_DCNTL           0x3B
#define R_DCNTL_SSM       0x10
#define R_DCNTL_STD       0x04
#define R_DCNTL_IRQD      0x02
#define R_DCNTL_COM       0x01
#define DCNTL_MASK        0xFB

#define R_ADDER           0x3C

#define R_SIEN0           0x40
#define SIEN0_MASK        0xFF
#define R_SIEN1           0x41
#define SIEN1_MASK        0x17

#define R_SIST0           0x42
#define R_SIST0_MA        0x80
#define R_SIST0_CMP       0x40
#define R_SIST0_SEL       0x20
#define R_SIST0_RSL       0x10
#define R_SIST0_SGE       0x08
#define R_SIST0_UDC       0x04
#define R_SIST0_RST       0x02
#define R_SIST0_PAR       0x01
#define SIST0_RC          0xFF
#define SIST0_FATAL       0x8F

#define R_SIST1           0x43
#define R_SIST1_SBMC      0x10
#define R_SIST1_STO       0x04
#define R_SIST1_GEN       0x02
#define R_SIST1_HTH       0x01
#define SIST1_RC          0x17
#define SIST1_FATAL       0x14

#define R_MACNTL          0x46
#define MACNTL_MASK       0x0F
#define R_GPCNTL          0x47
#define R_STIME0          0x48
#define R_STIME1          0x49
#define R_STIME1_GEN      0x0F
#define STIME1_MASK       0x7F

#define R_RESPID          0x4A

#define R_STEST0          0x4C
#define R_STEST1          0x4D
#define STEST1_MASK       0xCC

#define R_STEST2          0x4E
#define R_STEST2_SCE      0x80
#define R_STEST2_ROF      0x40
#define R_STEST2_DIF      0x20
#define R_STEST2_SLB      0x10
#define R_STEST2_SZM      0x08
#define R_STEST2_AWS      0x04
#define R_STEST2_EXT      0x02
#define R_STEST2_LOW      0x01
#define STEST2_MASK       0xBF

#define R_STEST3          0x4F
#define R_STEST3_TE       0x80
#define R_STEST3_STR      0x40
#define R_STEST3_HSC      0x20
#define R_STEST3_DSI      0x10
#define R_STEST3_S16      0x08
#define R_STEST3_TTM      0x04
#define R_STEST3_CSF      0x02
#define R_STEST3_STW      0x01
#define STEST3_MASK       0xFF

#define R_STEST4          0x52

#define R_SBDL            0x58
#define R_SCRATCHB        0x5C
#define R_SCRATCHC        0x60

#define R8(a)             state.regs.reg8[R_##a]
#define R16(a)            state.regs.reg16[R_##a / 2]
#define R32(a)            state.regs.reg32[R_##a / 4]

// test bit in register
#define TB_R8(a, b) ((R8(a) & R_##a##_##b) == R_##a##_##b)

//set bit in register
#define SB_R8(a, b, c)  R8(a) = (R8(a) &~R_##a##_##b) | (c ? R_##a##_##b : 0)

// write with mask
#define WRM_R8(a, b)  R8(a) = (R8(a) &~a##_MASK) | ((b) & a##_MASK)

// write with mask and write-1-to-clear
#define WRMW1C_R8(a, b)                    \
    R8(a) = (R8(a) &~a##_MASK &~a##_W1C) | \
    ((b) & a##_MASK) |                     \
    (R8(a) &~(b) & a##_W1C)
#define RAISE(a, b) set_interrupt(R_##a, R_##a##_##b)
#define RDCLR_R8(a) R8(a) &= ~a##_RC

#define GET_DEST()  (R8(SDID) & R_SCID_ID)
#define SET_DEST(a) R8(SDID) = (a) & R_SCID_ID

#define GET_DBC()   (R32(DBC) & 0x00ffffff)
#define SET_DBC(a)                       \
    R32(DBC) = (R32(DBC) & 0xff000000) | \
    ((a) & 0x00ffffff)
#define PT  state.per_target[GET_DEST()]
#define PTD get_disk(0, GET_DEST())

  u32 sym_cfg_data[64] =
  {

    /*00*/
    0x000c1000,       // CFID: vendor + device

  /*04*/
  0x02000001,         // CFCS: command + status

  /*08*/
  0x01000000,         // CFRV: class + revision

  /*0c*/
  0x00000000,         // CFLT: latency timer + cache line size

  /*10*/
  0x00000001,         // BAR0: IO Space

  /*14*/
  0x00000000,         // BAR1: Memory space

  /*18*/
  0x00000000,         // BAR2: RAM space

  /*1c*/
  0x00000000,         // BAR3:

  /*20*/
  0x00000000,         // BAR4:

  /*24*/
  0x00000000,         // BAR5:

  /*28*/
  0x00000000,         // CCIC: CardBus

  /*2c*/
  0x00000000,         // CSID: subsystem + vendor

  /*30*/
  0x00000000,         // BAR6: expansion rom base

  /*34*/
  0x00000000,         // CCAP: capabilities pointer

  /*38*/
  0x00000000,

  /*3c*/
  0x401101ff,         // CFIT: interrupt configuration
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

u32 sym_cfg_mask[64] = {
  /*00*/ 0x00000000,  // CFID: vendor + device
  /*04*/ 0x00000157,  // CFCS: command + status
  /*08*/ 0x00000000,  // CFRV: class + revision
  /*0c*/ 0x0000ffff,  // CFLT: latency timer + cache line size
  /*10*/ 0xffffff00,  // BAR0: IO space (256 bytes)
  /*14*/ 0xffffff00,  // BAR1: Memory space (256 bytes)
  /*18*/ 0xfffff000,  // BAR2: RAM space (4KB)
  /*1c*/ 0x00000000,  // BAR3:
  /*20*/ 0x00000000,  // BAR4:
  /*24*/ 0x00000000,  // BAR5:
  /*28*/ 0x00000000,  // CCIC: CardBus
  /*2c*/ 0x00000000,  // CSID: subsystem + vendor
  /*30*/ 0x00000000,  // BAR6: expansion rom base
  /*34*/ 0x00000000,  // CCAP: capabilities pointer
  /*38*/ 0x00000000,
  /*3c*/ 0x000000ff,  // CFIT: interrupt configuration
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/**
 * Thread entry point.
 **/
void CSym53C895::run()
{
  try
  {
    for(;;)
    {
      mySemaphore.wait();
      if(StopThread)
        return;
      while(state.executing)
      {
        MUTEX_LOCK(myRegLock);
        execute();
        MUTEX_UNLOCK(myRegLock);
      }
    }
  }

  catch(Poco::Exception & e)
  {
    printf("Exception in SYM thread: %s.\n", e.displayText().c_str());

    // Let the thread die...
  }
}

/**
 * Constructor.
 **/
CSym53C895::CSym53C895(CConfigurator* cfg, CSystem* c, int pcibus, int pcidev) : CDiskController(cfg, c, pcibus, pcidev, 1, 16), mySemaphore(0, 1)
{

  // create scsi bus
  CSCSIBus*   a = new CSCSIBus(cfg, c);
  scsi_register(0, a, 7); // scsi id 7 by default
}

/**
 * Initialize the Symbios device.
 **/
void CSym53C895::init()
{
  add_function(0, sym_cfg_data, sym_cfg_mask);

  ResetPCI();

  chip_reset();

  myRegLock = new CMutex("sym-reg");

  myThread = 0;

  printf("%s: $Id: Sym53C895.cpp,v 1.28 2008/03/14 15:30:52 iamcamiel Exp $\n",
         devid_string);
}

void CSym53C895::start_threads()
{
  if(!myThread)
  {
    myThread = new Poco::Thread("sym");
    printf(" %s", myThread->getName().c_str());
    StopThread = false;
    myThread->start(*this);
    if(state.executing)
      mySemaphore.set();
  }
}

void CSym53C895::stop_threads()
{
  StopThread = true;
  if(myThread)
  {
    printf(" %s", myThread->getName().c_str());
    mySemaphore.set();
    myThread->join();
    delete myThread;
    myThread = 0;
  }
}

CSym53C895::~CSym53C895()
{
  stop_threads();
  delete scsi_bus[0];
}

void CSym53C895::chip_reset()
{
  state.executing = false;
  state.wait_reselect = false;
  state.irq_asserted = false;
  state.gen_timer = 0;
  memset(state.regs.reg32, 0, sizeof(state.regs.reg32));
  R8(SCNTL0) = R_SCNTL0_ARB1 | R_SCNTL0_ARB0; // 810
  R8(DSTAT) = R_DSTAT_DFE;    // DMA FIFO empty // 810

  //  R8(SSTAT2) = R_SSTAT2_LDSC; // 810
  R8(CTEST1) = R_CTEST1_FMT;  // 810
  R8(CTEST2) = R_CTEST2_DACK; // 810
  R8(CTEST3) = (u8) (pci_state.config_data[0][2] << 4) & R_CTEST3_REV;  // Chip rev.
  R8(MACNTL) = 0xD0;  // 895 type ID
  R8(GPCNTL) = 0x0F;  // 810
  R8(STEST0) = 0x03;  // 810
}

void CSym53C895::register_disk(class CDisk* dsk, int bus, int dev)
{
  CDiskController::register_disk(dsk, bus, dev);
  dsk->scsi_register(0, scsi_bus[0], dev);
}

static u32  sym_magic1 = 0x53C895CC;
static u32  sym_magic2 = 0xCC53C895;

/**
 * Save state to a Virtual Machine State file.
 **/
int CSym53C895::SaveState(FILE* f)
{
  long  ss = sizeof(state);
  int   res;

  if(res = CPCIDevice::SaveState(f))
    return res;

  fwrite(&sym_magic1, sizeof(u32), 1, f);
  fwrite(&ss, sizeof(long), 1, f);
  fwrite(&state, sizeof(state), 1, f);
  fwrite(&sym_magic2, sizeof(u32), 1, f);
  printf("%s: %d bytes saved.\n", devid_string, (int) ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/
int CSym53C895::RestoreState(FILE* f)
{
  long    ss;
  u32     m1;
  u32     m2;
  int     res;
  size_t  r;

  if(res = CPCIDevice::RestoreState(f))
    return res;

  r = fread(&m1, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(m1 != sym_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n", devid_string);
    return -1;
  }

  fread(&ss, sizeof(long), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n", devid_string);
    return -1;
  }

  fread(&state, sizeof(state), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  r = fread(&m2, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("%s: unexpected end of file!\n", devid_string);
    return -1;
  }

  if(m2 != sym_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n", devid_string);
    return -1;
  }

  printf("%s: %d bytes restored.\n", devid_string, (int) ss);
  return 0;
}

void CSym53C895::WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data)
{
  void*   p;

  switch(bar)
  {
  case 0:
  case 1:
    address &= 0x7f;
    switch(dsize)
    {
    case 8:
      MUTEX_LOCK(myRegLock);
#if defined(DEBUG_SYM_REGS)
      printf("SYM: Write to register %02x: %02x.   \n", address, data);
#endif
      if(address >= R_SCRATCHC)
      {
        state.regs.reg8[address] = (u8) data;
        MUTEX_UNLOCK(myRegLock);
        break;
      }

      switch(address)
      {

      // SIMPLE CASES: JUST WRITE
      case R_SXFER:             // 05
      case R_DSA:               // 10
      case R_DSA + 1:           // 11
      case R_DSA + 2:           // 12
      case R_DSA + 3:           // 13
      case R_CTEST0:            // 18
      case R_TEMP:              // 1C
      case R_TEMP + 1:          // 1D
      case R_TEMP + 2:          // 1E
      case R_TEMP + 3:          // 1F
      case R_DSP:               // 2C
      case R_DSP + 1:           // 2D
      case R_DSP + 2:           // 2E
      case R_DSPS:              // 30
      case R_DSPS + 1:          // 31
      case R_DSPS + 2:          // 32
      case R_DSPS + 3:          // 33
      case R_SCRATCHA:          // 34
      case R_SCRATCHA + 1:      // 35
      case R_SCRATCHA + 2:      // 36
      case R_SCRATCHA + 3:      // 37
      case R_DMODE:             // 38
      case R_SBR:               // 3A     // 810
      case R_GPCNTL:            // 47
      case R_STIME0:            // 48
      case R_RESPID:            // 4A
      case R_RESPID + 1:        // 4B
      case R_STEST0:            // 4C
      case R_SCRATCHB:          // 5C
      case R_SCRATCHB + 1:      // 5D
      case R_SCRATCHB + 2:      // 5E
      case R_SCRATCHB + 3:      // 5F
        state.regs.reg8[address] = (u8) data;
        break;

      case R_SCNTL0:            // 00
        // side effects: start arbitration bit
        write_b_scntl0((u8) data);
        break;

      case R_SCNTL1:            //01
        // side effects: start immediate arbitration bit
        write_b_scntl1((u8) data);
        break;

      case R_SCNTL2:            //02
        WRMW1C_R8(SCNTL2, (u8) data);
        break;

      case R_SCNTL3:            //03
        // side effects: clearing EWS
        write_b_scntl3((u8) data);
        break;

      case R_SCID:              // 04
        WRM_R8(SCID, (u8) data);
        break;

      case R_SDID:              // 06
        WRM_R8(SDID, (u8) data);
        break;

      case R_GPREG:             // 07
        WRM_R8(GPREG, (u8) data);
        break;

      case R_ISTAT:             // 14
        write_b_istat((u8) data);
        break;

      case R_CTEST2:            // 1A
        WRM_R8(CTEST2, (u8) data);
        break;

      case R_CTEST3:            // 1B
        write_b_ctest3((u8) data);
        break;

      case R_CTEST4:            // 21
        write_b_ctest4((u8) data);
        break;

      case R_CTEST5:            // 22
        write_b_ctest5((u8) data);
        break;

      case R_DSP + 3:           // 2F
        state.regs.reg8[address] = (u8) data;
        post_dsp_write();
        break;

      case R_DIEN:              // 39
        WRM_R8(DIEN, (u8) data);
        eval_interrupts();
        break;

      case R_DCNTL:             // 3B
        write_b_dcntl((u8) data);
        break;

      case R_SIEN0:             // 40
        R8(SIEN0) = (u8) data;
        eval_interrupts();
        break;

      case R_SIEN1:             // 41
        WRM_R8(SIEN1, (u8) data);
        eval_interrupts();
        break;

      case R_MACNTL:            // 46     // 810
        WRM_R8(MACNTL, (u8) data);
        break;

      case R_STIME1:            // 49
        WRM_R8(STIME1, (u8) data);
        state.gen_timer = (R8(STIME1) & R_STIME1_GEN) * 30;
        break;

      case R_STEST1:            // 4D
        WRM_R8(STEST1, (u8) data);
        break;

      case R_STEST2:            // 4E
        write_b_stest2((u8) data);
        break;

      case R_STEST3:            // 4F
        write_b_stest3((u8) data);
        break;

      case R_DSTAT:             // 0C
      case R_SSTAT0:            // 0D
      case R_SSTAT1:            // 0E
      case R_SSTAT2:            // 0F
        //printf("SYM: Write to read-only memory at %02x. FreeBSD driver cache test.\n" ,address);
        break;

      default:
        FAILURE_2(NotImplemented,
                  "SYM: Write to unknown register at %02x with %08x.\n", address,
                  data);
      }

      MUTEX_UNLOCK(myRegLock);
      break;

    case 16:
      WriteMem_Bar(0, 1, address + 0, 8, (data >> 0) & 0xff);
      WriteMem_Bar(0, 1, address + 1, 8, (data >> 8) & 0xff);
      break;

    case 32:
      WriteMem_Bar(0, 1, address + 0, 8, (data >> 0) & 0xff);
      WriteMem_Bar(0, 1, address + 1, 8, (data >> 8) & 0xff);
      WriteMem_Bar(0, 1, address + 2, 8, (data >> 16) & 0xff);
      WriteMem_Bar(0, 1, address + 3, 8, (data >> 24) & 0xff);
      break;
    }
    break;

  case 2:
    p = (u8*) state.ram + address;
    switch(dsize)
    {
    case 8:   *((u8*) p) = (u8) data; break;
    case 16:  *((u16*) p) = (u16) data; break;
    case 32:  *((u32*) p) = (u32) data; break;
    }
    break;
  }
}

u32 CSym53C895::ReadMem_Bar(int func, int bar, u32 address, int dsize)
{
  u32     data = 0;
  void*   p;

  switch(bar)
  {
  case 0:
  case 1:
    address &= 0x7f;
    switch(dsize)
    {
    case 8:
      MUTEX_LOCK(myRegLock);
      if(address >= R_SCRATCHC)
      {
        data = state.regs.reg8[address];
        MUTEX_UNLOCK(myRegLock);
        break;
      }

      switch(address)
      {
      case R_SCNTL0:            // 00
      case R_SCNTL1:            // 01
      case R_SCNTL2:            // 02
      case R_SCNTL3:            // 03
      case R_SCID:              // 04
      case R_SXFER:             // 05
      case R_SDID:              // 06
      case R_GPREG:             // 07
      case R_SFBR:              // 08
      case R_SSID:              // 0A
      case R_SBCL:              // 0B
      case R_SSTAT0:            // 0D
      case R_SSTAT1:            // 0E
      case R_SSTAT2:            // 0F
      case R_DSA:               // 10
      case R_DSA + 1:           // 11
      case R_DSA + 2:           // 12
      case R_DSA + 3:           // 13
      case R_ISTAT:             // 14
      case R_CTEST0:            // 18
      case R_CTEST1:            // 19
      case R_CTEST3:            // 1B
      case R_TEMP:              // 1C
      case R_TEMP + 1:          // 1D
      case R_TEMP + 2:          // 1E
      case R_TEMP + 3:          // 1F
      case R_CTEST4:            // 21
      case R_CTEST5:            // 22
      case R_DBC:               // 24  // 810
      case R_DBC + 1:           // 25  // 810
      case R_DBC + 2:           // 26  // 810
      case R_DCMD:              // 27  // 810
      case R_DNAD:              // 28  // 810
      case R_DNAD + 1:          // 29  // 810
      case R_DNAD + 2:          // 2A  // 810
      case R_DNAD + 3:          // 2B  // 810
      case R_DSP:               // 2C
      case R_DSP + 1:           // 2D
      case R_DSP + 2:           // 2E
      case R_DSP + 3:           // 2F
      case R_DSPS:              // 30
      case R_DSPS + 1:          // 31
      case R_DSPS + 2:          // 32
      case R_DSPS + 3:          // 33
      case R_DMODE:             // 38
      case R_DIEN:              // 39
      case R_SBR:               // 3A     // 810
      case R_DCNTL:             // 3B
      case R_SIEN0:             // 40
      case R_SIEN1:             // 41
      case R_MACNTL:            // 46     // 810
      case R_GPCNTL:            // 47
      case R_STIME0:            // 48
      case R_STIME1:            // 49
      case R_RESPID:            // 4A
      case R_RESPID + 1:        // 4B
      case R_STEST0:            // 4C
      case R_STEST1:            // 4D
      case R_STEST2:            // 4E
      case R_STEST3:            // 4F
      case R_STEST4:            // 52
      case R_SBDL:              // 58
      case R_SBDL + 1:          // 59
        data = state.regs.reg8[address];
        break;

      case R_DSTAT:             // 0C
        data = read_b_dstat();
        break;

      case R_CTEST2:            // 1A
        data = read_b_ctest2();
        break;

      case R_DFIFO:             // 20
        data = R8(DBC) & 0x7f;  // 810 - fake the DFIFO count
        break;

      case R_SCRATCHA:          // 34
      case R_SCRATCHA + 1:      // 35
      case R_SCRATCHA + 2:      // 36
      case R_SCRATCHA + 3:      // 37
        data = read_b_scratcha(address - R_SCRATCHA);
        break;

      case R_SIST0:         // 42
      case R_SIST1:         // 43
        data = read_b_sist(address - R_SIST0);
        break;

      case R_SCRATCHB:      // 5C
      case R_SCRATCHB + 1:  // 5D
      case R_SCRATCHB + 2:  // 5E
      case R_SCRATCHB + 3:  // 5F
        data = read_b_scratchb(address - R_SCRATCHB);
        break;

      default:
        FAILURE_2(NotImplemented,
                  "SYM: Attempt to read from unknown register at %02x\n", dsize,
                  address);
      }

      MUTEX_UNLOCK(myRegLock);
#if defined(DEBUG_SYM_REGS)
      printf("SYM: Read frm register %02x: %02x.   \n", address, data);
#endif
      break;

    case 16:
      data = (ReadMem_Bar(0, 1, address + 0, 8) << 0) & 0x00ff;
      data |= (ReadMem_Bar(0, 1, address + 1, 8) << 8) & 0xff00;
      break;

    case 32:
      data = (ReadMem_Bar(0, 1, address + 0, 8) << 0) & 0x000000ff;
      data |= (ReadMem_Bar(0, 1, address + 1, 8) << 8) & 0x0000ff00;
      data |= (ReadMem_Bar(0, 1, address + 2, 8) << 16) & 0x00ff0000;
      data |= (ReadMem_Bar(0, 1, address + 3, 8) << 24) & 0xff000000;
      break;
    }
    break;

  case 2:
    p = (u8*) state.ram + address;
    switch(dsize)
    {
    case 8:   return *((u8*) p);
    case 16:  return *((u16*) p);
    case 32:  return *((u32*) p);
    }
    break;
  }

  return data;
}

u32 CSym53C895::config_read_custom(int func, u32 address, int dsize, u32 data)
{
  if(address >= 0x80)
    return ReadMem_Bar(func, 1, address - 0x80, dsize);
  else
    return data;
}

void CSym53C895::config_write_custom(int func, u32 address, int dsize,
                                     u32 old_data, u32 new_data, u32 data)
{
  if(address >= 0x80)
    WriteMem_Bar(func, 1, address - 0x80, dsize, data);
}

void CSym53C895::write_b_scntl0(u8 value)
{
  bool  old_start = TB_R8(SCNTL0, START);

  WRM_R8(SCNTL0, value);

  if(TB_R8(SCNTL0, START) && !old_start)
    FAILURE(NotImplemented, "SYM: Don't know how to start arbitration sequence");

  if(TB_R8(SCNTL0, TRG))
    FAILURE(NotImplemented, "SYM: Don't know how to operate in target mode");
}

void CSym53C895::write_b_scntl1(u8 value)
{
  bool  old_iarb = TB_R8(SCNTL1, IARB);
  bool  old_con = TB_R8(SCNTL1, CON);
  bool  old_rst = TB_R8(SCNTL1, RST);

  R8(SCNTL1) = value;

  //  if (TB_R8(SCNTL1,CON) != old_con)
  //    printf("SYM: Don't know how to forcibly connect or disconnect\n");
  if(TB_R8(SCNTL1, RST) != old_rst)
  {
    SB_R8(SSTAT0, SDP0, false);
    SB_R8(SSTAT1, SDP1, false);
    R16(SBDL) = 0;
    R8(SBCL) = 0;

    SB_R8(SSTAT0, RST, !old_rst);

    //    printf("SYM: %s SCSI bus reset.\n",old_rst?"end":"start");
    if(!old_rst)
      RAISE(SIST0, RST);
  }

  if(TB_R8(SCNTL1, IARB) && !old_iarb)
    FAILURE(NotImplemented,
            "SYM: Don't know how to start immediate arbitration sequence.\n");
}

void CSym53C895::write_b_scntl3(u8 value)
{
  R8(SCNTL3) = value;

  if(!TB_R8(SCNTL3, EWS))
    SB_R8(SCNTL2, WSR, false);
}

void CSym53C895::write_b_istat(u8 value)
{
  bool  old_srst = TB_R8(ISTAT, SRST);
  bool  old_sem = TB_R8(ISTAT, SEM);
  bool  old_sigp = TB_R8(ISTAT, SIGP);

  WRMW1C_R8(ISTAT, value);

  if(TB_R8(ISTAT, ABRT))
  {

    //    printf("SYM: Aborting on request.\n");
    RAISE(DSTAT, ABRT);
  }

  if(TB_R8(ISTAT, SRST) && !old_srst)
  {

    //    printf("SYM: Resetting on request.\n");
    chip_reset();
  }

  //  if (TB_R8(ISTAT,SEM) != old_sem)
  //    printf("SYM: SEM %s.\n",old_sem?"reset":"set");
  //  if (TB_R8(ISTAT,SIGP) != old_sigp)
  //    printf("SYM: SIGP %s.\n",old_sigp?"reset":"set");
  if(TB_R8(ISTAT, SIGP))
  {
    if(state.wait_reselect)
    {

      //      printf("SYM: SIGP while wait_reselect. Jumping...\n");
      R32(DSP) = state.wait_jump;
      state.wait_reselect = false;
      state.executing = true;
      mySemaphore.set();
    }
  }

  eval_interrupts();
}

u8 CSym53C895::read_b_ctest2()
{
  SB_R8(CTEST2, CIO, pci_state.config_data[0][4] != 0);
  SB_R8(CTEST2, CM, pci_state.config_data[0][5] != 0);
  SB_R8(CTEST2, SIGP, TB_R8(ISTAT, SIGP));
  SB_R8(ISTAT, SIGP, false);

  //  printf("SYM: SIGP cleared by CTEST2 read.\n");
  return R8(CTEST2);
}

void CSym53C895::write_b_ctest3(u8 value)
{
  WRM_R8(CTEST3, value);

  //if ((value>>3) & 1)
  //  printf("SYM: Don't know how to flush DMA FIFO\n");
  //if ((value>>2) & 1)
  //  printf("SYM: Don't know how to clear DMA FIFO\n");
  if((value >> 1) & 1)
    FAILURE(NotImplemented, "SYM: Don't know how to handle FM mode");
}

void CSym53C895::write_b_ctest4(u8 value)
{
  R8(CTEST4) = value;

  if((value >> 4) & 1)
    FAILURE(NotImplemented, "SYM: Don't know how to handle SRTM mode");
}

void CSym53C895::write_b_ctest5(u8 value)
{
  WRM_R8(CTEST5, value);

  if((value >> 7) & 1)
    FAILURE(NotImplemented, "SYM: Don't know how to do Clock Address increment");

  if((value >> 6) & 1)
    FAILURE(NotImplemented,
            "SYM: Don't know how to do Clock Byte Counter decrement");
}

u8 CSym53C895::read_b_dstat()
{
  u8  retval = R8(DSTAT);

  RDCLR_R8(DSTAT);

  //printf("Read DSTAT --> eval int\n");
  eval_interrupts();

  //printf("Read DSTAT <-- eval int; retval: %02x; dstat: %02x.\n",retval,R8(DSTAT));
  return retval;
}

u8 CSym53C895::read_b_sist(int id)
{
  u8  retval = state.regs.reg8[R_SIST0 + id];

  if(id)
    RDCLR_R8(SIST1);
  else
    RDCLR_R8(SIST0);

  eval_interrupts();

  return retval;
}

void CSym53C895::write_b_dcntl(u8 value)
{
  WRM_R8(DCNTL, value);

  // start operation
  if(value & R_DCNTL_STD)
  {
    state.executing = true;
    mySemaphore.set();
  }

  //IRQD bit...
  eval_interrupts();
}

u8 CSym53C895::read_b_scratcha(int reg)
{
  if(TB_R8(CTEST2, SRTCH))
  {

    //printf("SYM: SCRATCHA from PCI\n");
    return(u8) (pci_state.config_data[0][4] >> (reg * 8)) & 0xff;
  }
  else
    return state.regs.reg8[R_SCRATCHA + reg];
}

u8 CSym53C895::read_b_scratchb(int reg)
{
  if(TB_R8(CTEST2, SRTCH))
  {

    //printf("SYM: SCRATCHB from PCI\n");
    return(u8) (pci_state.config_data[0][5] >> (reg * 8)) & 0xff;
  }
  else
    return state.regs.reg8[R_SCRATCHB + reg];
}

void CSym53C895::write_b_stest2(u8 value)
{
  WRM_R8(STEST2, value);

  //  if (value & R_STEST2_ROF)
  //    printf("SYM: Don't know how to reset SCSI offset!\n");
  if(TB_R8(STEST2, LOW))
    FAILURE(NotImplemented, "SYM: I don't like LOW level mode");
}

void CSym53C895::write_b_stest3(u8 value)
{
  WRM_R8(STEST3, value);

  //  if (value & R_STEST3_CSF)
  //    printf("SYM: Don't know how to clear the SCSI fifo.\n");
}

void CSym53C895::post_dsp_write()
{
  if(!TB_R8(DMODE, MAN))
  {
    state.executing = true;
    mySemaphore.set();

    //printf("SYM: Execution started @ %08x.\n",R32(DSP));
  }
}

/**
 * Check if threads are still running.
 **/
void CSym53C895::check_state()
{
  if(myThread && !myThread->isRunning())
    FAILURE(Thread, "SYM thread has died");

  if(state.gen_timer)
  {
    state.gen_timer--;
    if(!state.gen_timer)
    {
      state.gen_timer = (R8(STIME1) & R_STIME1_GEN) * 30;
      RAISE(SIST1, GEN);
      return;
    }
  }

  /**

  if (state.wait_reselect && PT.disconnected)
  {
    state.executing = true;
    state.wait_reselect = false;
    PT.disconnected = false;
    //PT.disconnect_priv = false;
    //PT.will_disconnect = false;
    PT.reselected = true;
    state.phase = 7; // msg in //PT.disconnect_phase;
    R8(SSID) = GET_DEST() | R_SSID_VAL; // valid scsi selector id
    if (TB_R8(DCNTL,COM))
      R8(SFBR) = GET_DEST();
    // don't expect a disconnect.
    SB_R8(SCNTL2,SDU,true);
    //RAISE(SIST0,RSL);
    return 0;
  }
 
 **/
  if(state.disconnected)
  {
    if(!TB_R8(SCNTL2, SDU))
    {

      // disconnect expected
      //printf("SYM: Disconnect expected. stopping disconnect timer at %d.\n",state.disconnected);
      state.disconnected = 0;
      return;
    }

    state.disconnected--;
    if(!state.disconnected)
    {

      //printf("SYM: Disconnect unexpected. raising interrupt!\n");
      //printf(">");
      //getchar();
      RAISE(SIST0, UDC);
      return;
    }
  }
}

void CSym53C895::execute()
{
  int optype;

  // single step mode
  if(TB_R8(DCNTL, SSM))
  {
#if defined(DEBUG_SYM_SCRIPTS)
    printf("SYM: Single step...\n");
#endif
    RAISE(DSTAT, SSI);
  }

#if defined(DEBUG_SYM_SCRIPTS)

  //    printf("SYM: EXECUTING SCRIPT\n");
  //    printf("SYM: INS @ %x, %x   \n",R32(DSP), R32(DSP)+4);
#endif
  do_pci_read(R32(DSP), &R32(DBC), 4, 1);
  do_pci_read(R32(DSP) + 4, &R32(DSPS), 4, 1);

  R32(DSP) += 8;

#if defined(DEBUG_SYM_SCRIPTS)

  //printf("SYM: INS @ %" LL "x, %" LL "x   \n",cmda0, cmda1);
  printf("SYM: INS = %x, %x, %x   \n", R8(DCMD), GET_DBC(), R32(DSPS));
#endif
  optype = (R8(DCMD) >> 6) & 3;
  switch(optype)
  {
  case 0:
    {
      bool  indirect = (R8(DCMD) >> 5) & 1;
      bool  table_indirect = (R8(DCMD) >> 4) & 1;
      int   opcode = (R8(DCMD) >> 3) & 1;
      int   scsi_phase = (R8(DCMD) >> 0) & 7;
      int   real_phase = scsi_get_phase(0);

#if defined(DEBUG_SYM_SCRIPTS)
      printf("SYM: INS = Block Move (i %d, t %d, opc %d, phase %d\n", indirect,
             table_indirect, opcode, scsi_phase);
#endif
      if(real_phase == SCSI_PHASE_ARBITRATION)
      {

        // selection timeout...?
#if defined(DEBUG_SYM_SCRIPTS)
        printf("Phase check... selection time-out!\n");
#endif
        RAISE(SIST1, STO);    // select time-out
        scsi_free(0);
        state.select_timeout = false;
        return;
      }

      if(real_phase == SCSI_PHASE_FREE && state.disconnected)
      {
#if defined(DEBUG_SYM_SCRIPTS)
        printf("Phase check... disconnected!\n");
#endif
        state.disconnected = 1;
        R32(DSP) -= 8;
        return;
      }

      if(real_phase == scsi_phase)
      {
#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: Ready for transfer.\n");
#endif

        u32 start;
        u32 count;

        if(table_indirect)
        {
          u32 add = R32(DSA) + sext_u32_24(R32(DSPS));

          add &= ~0x03;       // 810
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Reading table at DSA(%08x)+DSPS(%08x) = %08x.\n",
                 R32(DSA), R32(DSPS), add);
#endif
          do_pci_read(add, &count, 4, 1);
          count &= 0x00ffffff;
          do_pci_read(add + 4, &start, 4, 1);
        }
        else if(indirect)
        {
          FAILURE(NotImplemented, "SYM: Unsupported: indirect addressing");
        }
        else
        {
          start = R32(DSPS);
          count = GET_DBC();
        }

#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: %08x: MOVE Start/count %x, %x\n", R32(DSP) - 8, start,
               count);
#endif
        R32(DNAD) = start;
        SET_DBC(count);       // page 5-32
        if(count == 0)
        {

          //printf("SYM: Count equals zero!\n");
          RAISE(DSTAT, IID);  // page 5-32
          return;
        }

        if((size_t) count > scsi_expected_xfer(0))
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: xfer %d bytes, max %d expected, in phase %d.\n", count,
                 scsi_expected_xfer(0), scsi_phase);
#endif
          count = (u32) scsi_expected_xfer(0);
        }

        u8*   scsi_data_ptr = (u8*) scsi_xfer_ptr(0, count);
        u8*   org_sdata_ptr = scsi_data_ptr;

        switch(scsi_phase)
        {
        case SCSI_PHASE_COMMAND:
        case SCSI_PHASE_DATA_OUT:
        case SCSI_PHASE_MSG_OUT:
          do_pci_read(R32(DNAD), scsi_data_ptr, 1, count);
          R32(DNAD) += count;
          break;

        case SCSI_PHASE_STATUS:
        case SCSI_PHASE_DATA_IN:
        case SCSI_PHASE_MSG_IN:
          do_pci_write(R32(DNAD), scsi_data_ptr, 1, count);
          R32(DNAD) += count;
          break;
        }

        R8(SFBR) = *org_sdata_ptr;
        scsi_xfer_done(0);
        return;
      }
    }
    break;

  case 1:
    {
      int opcode = (R8(DCMD) >> 3) & 7;

      if(opcode < 5)
      {
        bool  relative = (R8(DCMD) >> 2) & 1;
        bool  table_indirect = (R8(DCMD) >> 1) & 1;
        bool  atn = (R8(DCMD) >> 0) & 1;
        int   destination = (GET_DBC() >> 16) & 0x0f;
        bool  sc_carry = (GET_DBC() >> 10) & 1;
        bool  sc_target = (GET_DBC() >> 9) & 1;
        bool  sc_ack = (GET_DBC() >> 6) & 1;
        bool  sc_atn = (GET_DBC() >> 3) & 1;

        //HACK?? DOCS UNCLEAR: TRY-THIS
        R32(DNAD) = R32(DSPS);

        u32 dest_addr = R32(DNAD);

        if(relative)
          dest_addr = R32(DSP) + sext_u32_24(R32(DNAD));

#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: INS = I/O (opc %d, r %d, t %d, a %d, dest %d, sc %d%d%d%d\n",
               opcode, relative, table_indirect, atn, destination, sc_carry,
               sc_target, sc_ack, sc_atn);
#endif
        if(table_indirect)
        {
          u32 io_addr = R32(DSA) + sext_u32_24(GET_DBC());
          io_addr &= ~3;      //810
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Reading table at DSA(%08x)+DBC(%08x) = %08x.\n",
                 R32(DSA), sext_u32_24(GET_DBC()), io_addr);
#endif

          u32 io_struc;
          do_pci_read(io_addr, &io_struc, 4, 1);
          destination = (io_struc >> 16) & 0x0f;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: table indirect. io_struct = %08x, new dest = %d.\n",
                 io_struc, destination);
#endif
        }

        switch(opcode)
        {
        case 0:
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: SELECT %d.\n", R32(DSP) - 8, destination);
#endif
          SET_DEST(destination);
          if(!scsi_arbitrate(0))
          {

            // scsi bus busy, try again next clock...
            printf("scsi bus busy...\n");
            R32(DSP) -= 8;
            return;
          }

          state.select_timeout = !scsi_select(0, destination);
          if(!state.select_timeout)   // select ok
            SB_R8(SCNTL2, SDU, true); // don't expect a disconnect
          return;

        case 1:
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: WAIT DISCONNECT\n", R32(DSP) - 8);
#endif

          // maybe we need to do more??
          scsi_free(0);
          return;

        case 2:
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: WAIT RESELECT\n", R32(DSP) - 8);
#endif
          if(TB_R8(ISTAT, SIGP))
          {
#if defined(DEBUG_SYM_SCRIPTS)
            printf("SYM: SIGP set before wait reselect; jumping!\n");
#endif
            R32(DSP) = dest_addr;
          }
          else
          {
            state.wait_reselect = true;
            state.wait_jump = dest_addr;
            state.executing = false;
          }

          return;

        case 3:
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: SET %s%s%s%s\n", R32(DSP) - 8,
                 sc_carry ? "carry " : "", sc_target ? "target " : "",
                 sc_ack ? "ack " : "", sc_atn ? "atn " : "");
#endif
          if(sc_ack)
            SB_R8(SOCL, ACK, true);
          if(sc_atn)
          {
            if(!TB_R8(SOCL, ATN))
            {
              SB_R8(SOCL, ATN, true);

              //printf("SET ATN.\n");
              //printf(">");
              //getchar();
            }
          }

          if(sc_target)
            SB_R8(SCNTL0, TRG, true);
          if(sc_carry)
            state.alu.carry = true;
          return;

        case 4:
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: CLEAR %s%s%s%s\n", R32(DSP) - 8,
                 sc_carry ? "carry " : "", sc_target ? "target " : "",
                 sc_ack ? "ack " : "", sc_atn ? "atn " : "");
#endif
          if(sc_ack)
            SB_R8(SOCL, ACK, false);
          if(sc_atn)
          {
            if(TB_R8(SOCL, ATN))
            {
              SB_R8(SOCL, ATN, false);

              //printf("RESET ATN.\n");
              //printf(">");
              //getchar();
            }
          }

          if(sc_target)
            SB_R8(SCNTL0, TRG, false);
          if(sc_carry)
            state.alu.carry = false;
          return;

          break;
        }
      }
      else
      {
        int   oper = (R8(DCMD) >> 0) & 7;
        bool  use_data8_sfbr = (GET_DBC() >> 23) & 1;
        int   reg_address = ((GET_DBC() >> 16) & 0x7f); //| (GET_DBC() & 0x80); // manual is unclear about bit 7.
        u8    imm_data = (u8) (GET_DBC() >> 8) & 0xff;
        u8    op_data;

#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: INS = R/W (opc %d, oper %d, use %d, add %d, imm %02x\n",
               opcode, oper, use_data8_sfbr, reg_address, imm_data);
#endif
        if(use_data8_sfbr)
          imm_data = R8(SFBR);

        if(oper != 0)
        {
          if(opcode == 5 || reg_address == 0x08)
          {
            op_data = R8(SFBR);
#if defined(DEBUG_SYM_SCRIPTS)
            printf("SYM: %08x: sfbr (%02x) ", R32(DSP) - 8, op_data);
#endif
          }
          else
          {
            op_data = (u8) ReadMem_Bar(0, 1, reg_address, 8);
#if defined(DEBUG_SYM_SCRIPTS)
            printf("SYM: %08x: reg%02x (%02x) ", R32(DSP) - 8, reg_address,
                   op_data);
#endif
          }
        }

        u16 tmp16;

        switch(oper)
        {
        case 0:
          op_data = imm_data;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: %02x ", R32(DSP) - 8, imm_data);
#endif
          break;

        case 1:
          tmp16 = (op_data << 1) + (state.alu.carry ? 1 : 0);
          state.alu.carry = (tmp16 >> 8) & 1;
          op_data = tmp16 & 0xff;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("<< 1 = %02x ", op_data);
#endif
          break;

        case 2:
          op_data |= imm_data;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("| %02x = %02x ", imm_data, op_data);
#endif
          break;

        case 3:
          op_data ^= imm_data;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("^ %02x = %02x ", imm_data, op_data);
#endif
          break;

        case 4:
          op_data &= imm_data;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("& %02x = %02x ", imm_data, op_data);
#endif
          break;

        case 5:
          tmp16 = (op_data >> 1) + (state.alu.carry ? 0x80 : 0x00);
          state.alu.carry = op_data & 1;
          op_data = tmp16 & 0xff;
#if defined(DEBUG_SYM_SCRIPTS)
          printf(">> 1 = %02x ", op_data);
#endif
          break;

        case 6:
          tmp16 = op_data + imm_data;
          state.alu.carry = (tmp16 > 0xff);
          op_data = tmp16 & 0xff;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("+ %02x = %02x (carry %d) ", imm_data, op_data, state.alu.carry);
#endif
          break;

        case 7:
          tmp16 = op_data + imm_data + (state.alu.carry ? 1 : 0);
          state.alu.carry = (tmp16 > 0xff);
          op_data = tmp16 & 0xff;
#if defined(DEBUG_SYM_SCRIPTS)
          printf("+ %02x (w/carry) = %02x (carry %d) ", imm_data, op_data,
                 state.alu.carry);
#endif
          break;
        }

        if(opcode == 6 || reg_address == 0x08)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("-> sfbr.\n");
#endif
          R8(SFBR) = op_data;
        }
        else
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("-> reg%02x.\n", reg_address);
#endif
          WriteMem_Bar(0, 1, reg_address, 8, op_data);
        }

        return;
      }
    }
    break;

  case 2:
    {
      int   opcode = (R8(DCMD) >> 3) & 7;
      int   scsi_phase = (R8(DCMD) >> 0) & 7;
      bool  relative = (GET_DBC() >> 23) & 1;
      bool  carry_test = (GET_DBC() >> 21) & 1;
      bool  interrupt_fly = (GET_DBC() >> 20) & 1;
      bool  jump_if = (GET_DBC() >> 19) & 1;
      bool  cmp_data = (GET_DBC() >> 18) & 1;
      bool  cmp_phase = (GET_DBC() >> 17) & 1;

      // wait_valid can be safely ignored, phases are always valid in this ideal world...
      // bool wait_valid = (GET_DBC()>>16) & 1;
      int   cmp_mask = (GET_DBC() >> 8) & 0xff;
      int   cmp_dat = (GET_DBC() >> 0) & 0xff;

      u32   dest_addr = R32(DSPS);

      bool  do_it;

      if(relative)
        dest_addr = R32(DSP) + sext_u32_24(R32(DSPS));

#if defined(DEBUG_SYM_SCRIPTS)
      printf("SYM: %08x: if (", R32(DSP) - 8);
#endif
      if(carry_test)
      {
#if defined(DEBUG_SYM_SCRIPTS)
        printf("(%scarry)", jump_if ? "" : "!");
#endif
        do_it = (state.alu.carry == jump_if);
      }
      else if(cmp_data || cmp_phase)
      {
        do_it = true;
        if(cmp_data)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("((data & 0x%02x) %s 0x%02x)", (~cmp_mask) & 0xff,
                 jump_if ? "==" : "!=", cmp_dat &~cmp_mask);
#endif
          if(((R8(SFBR) &~cmp_mask) == (cmp_dat &~cmp_mask)) != jump_if)
            do_it = false;
#if defined(DEBUG_SYM_SCRIPTS)
          if(cmp_phase)
            printf(" && ");
#endif
        }

        if(cmp_phase)
        {
          int real_phase = scsi_get_phase(0);
#if defined(DEBUG_SYM_SCRIPTS)
          printf("(phase %s %d)", jump_if ? "==" : "!=", scsi_phase);
#endif
          if(real_phase == SCSI_PHASE_ARBITRATION)
          {

            // selection timeout...?
#if defined(DEBUG_SYM_SCRIPTS)
            printf("Phase check... selection time-out!\n");
#endif
            RAISE(SIST1, STO);  // select time-out
            scsi_free(0);
            state.select_timeout = false;
            return;
          }

          if(real_phase == SCSI_PHASE_FREE && state.disconnected)
          {
#if defined(DEBUG_SYM_SCRIPTS)
            printf("Phase check... disconnected!\n");
#endif
            state.disconnected = 1;
            R32(DSP) -= 8;
            return;
          }

          if((real_phase == scsi_phase) != jump_if)
            do_it = false;
        }
      }
      else
      {

        // no comparison
        do_it = jump_if;
      }

#if defined(DEBUG_SYM_SCRIPTS)
      printf(") ");
#endif
      switch(opcode)
      {
      case 0:
#if defined(DEBUG_SYM_SCRIPTS)
        printf("jump %x\n", R32(DSPS));
#endif
        if(do_it)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Jumping %08x...\n", dest_addr);
#endif
          R32(DSP) = dest_addr;
        }

        return;
        break;

      case 1:
#if defined(DEBUG_SYM_SCRIPTS)
        printf("call %d\n", R32(DSPS));
#endif
        if(do_it)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Calling %08x...\n", dest_addr);
#endif
          R32(TEMP) = R32(DSP);
          R32(DSP) = dest_addr;
        }

        return;
        break;

      case 2:
#if defined(DEBUG_SYM_SCRIPTS)
        printf("return %d\n", R32(DSPS));
#endif
        if(do_it)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Returning %08x...\n", R32(TEMP));
#endif
          R32(DSP) = R32(TEMP);
        }

        return;
        break;

      case 3:
#if defined(DEBUG_SYM_SCRIPTS)
        printf("interrupt%s.\n", interrupt_fly ? " on the fly" : "");
#endif
        if(do_it)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: Interrupt with vector %x...\n", R32(DSPS));
#endif
          if(interrupt_fly)
            RAISE(ISTAT, INTF);
          else
            RAISE(DSTAT, SIR);
        }

        return;
        break;

      default:
        FAILURE_1(NotImplemented,
                  "SYM: Transfer Control Instruction with opcode %d is RESERVED.\n",
                  opcode);
      }
    }

  case 3:
    {
      bool  load_store = (R8(DCMD) >> 5) & 1;
      if(load_store)
      {
        bool  is_load = (R8(DCMD) >> 0) & 1;
        bool  no_flush = (R8(DCMD) >> 1) & 1;
        bool  dsa_relative = (R8(DCMD) >> 4) & 1;
        int   regaddr = (GET_DBC() >> 16) & 0x7f;
        int   byte_count = (GET_DBC() >> 0) & 7;
        u32   memaddr;

        if(dsa_relative)
          memaddr = R32(DSA) + sext_u32_24(R32(DSPS));
        else
          memaddr = R32(DSPS);

#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: dsa_rel: %d, DSA: %04x, DSPS: %04x, mem %04x.\n",
               dsa_relative, R32(DSA), R32(DSPS), memaddr);
#endif
        if(is_load)
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: Load reg%02x", R32(DSP) - 8, regaddr);
          if(byte_count > 1)
            printf("..%02x", regaddr + byte_count - 1);
          printf("from %x.\n", memaddr);
#endif
          for(int i = 0; i < byte_count; i++)
          {
            u8  dat;
            do_pci_read(memaddr + i, &dat, 1, 1);
#if defined(DEBUG_SYM_SCRIPTS)
            printf("SYM: %02x -> reg%02x\n", dat, regaddr + i);
#endif
            WriteMem_Bar(0, 1, regaddr + i, 8, dat);
          }
        }
        else
        {
#if defined(DEBUG_SYM_SCRIPTS)
          printf("SYM: %08x: Store reg%02x", R32(DSP) - 8, regaddr);
          if(byte_count > 1)
            printf("..%02x", regaddr + byte_count - 1);
          printf("to %x.\n", memaddr);
#endif
          for(int i = 0; i < byte_count; i++)
          {
            u8  dat = (u8) ReadMem_Bar(0, 1, regaddr + i, 8);
#if defined(DEBUG_SYM_SCRIPTS)
            printf("SYM: %02x <- reg%02x\n", dat, regaddr + i);
#endif
            do_pci_write(memaddr + i, &dat, 1, 1);
          }
        }

        return;
      }
      else
      {

        // memory move
        u32 temp_shadow;
        do_pci_read(R32(DSP), &temp_shadow, 4, 1);
        R32(DSP) += 4;

#if defined(DEBUG_SYM_SCRIPTS)
        printf("SYM: %08x: Memory Move %06x bytes from %08x to %08x.\n",
               R32(DSP) - 12, GET_DBC(), R32(DSPS), temp_shadow);
#endif

        void*   buf = malloc(GET_DBC());
        do_pci_read(R32(DSPS), buf, 1, GET_DBC());
        do_pci_write(temp_shadow, buf, 1, GET_DBC());
        free(buf);
        return;
      }
    }
    break;
  }

  FAILURE(Logic, "SCSI should never get here");
}

void CSym53C895::set_interrupt(int reg, u8 interrupt)
{

  //
  //printf("set interrupt %02x, %02x.\n",reg,interrupt);
  switch(reg)
  {
  case R_DSTAT:
    if(TB_R8(ISTAT, DIP) || TB_R8(ISTAT, SIP))
    {
      state.dstat_stack |= interrupt;

      //printf("DSTAT stacked.\n");
    }
    else
    {
      R8(DSTAT) |= interrupt;

      //printf("DSTAT.\n");
    }
    break;

  case R_SIST0:
    if(TB_R8(ISTAT, DIP) || TB_R8(ISTAT, SIP))
    {
      state.sist0_stack |= interrupt;

      //printf("SIST0 stacked.\n");
    }
    else
    {
      R8(SIST0) |= interrupt;

      //printf("SIST0.\n");
    }
    break;

  case R_SIST1:
    if(TB_R8(ISTAT, DIP) || TB_R8(ISTAT, SIP))
    {
      state.sist1_stack |= interrupt;

      //printf("SIST1 stacked.\n");
    }
    else
    {
      R8(SIST1) |= interrupt;

      //printf("SIST1.\n");
    }
    break;

  case R_ISTAT:

    //printf("ISTAT.\n");
    R8(ISTAT) |= interrupt;
    break;

  default:
    FAILURE_1(NotImplemented, "set_interrupt reg %02x!!\n", reg);
  }

  //printf("--> eval int\n");
  eval_interrupts();

  //printf("<-- eval_int\n");
}

void CSym53C895::eval_interrupts()
{
  bool  will_assert = false;
  bool  will_halt = false;

  if(!R8(SIST0) && !R8(SIST1) && !R8(DSTAT))
  {
    R8(SIST0) |= state.sist0_stack;
    R8(SIST1) |= state.sist1_stack;
    R8(DSTAT) |= state.dstat_stack;
    state.sist0_stack = 0;
    state.sist1_stack = 0;
    state.dstat_stack = 0;
  }

  if(R8(DSTAT) & DSTAT_FATAL)
  {
    will_halt = true;

    //printf("  will halt(DSTAT).\n");
    SB_R8(ISTAT, DIP, true);
    if(R8(DSTAT) & R8(DIEN) & DSTAT_FATAL)
    {
      will_assert = true;

      //printf("  will assert(DSTAT).\n");
    }
  }
  else
    SB_R8(ISTAT, DIP, false);

  if(R8(SIST0) || R8(SIST1))
  {
    SB_R8(ISTAT, SIP, true);
    if((R8(SIST0) & (SIST0_FATAL | R8(SIEN0)))
     || (R8(SIST1) & (SIST1_FATAL | R8(SIEN1))))
    {
      will_halt = true;

      //printf("  will halt(SIST).\n");
      if((R8(SIST0) & R8(SIEN0)) || (R8(SIST1) & R8(SIEN1)))
      {
        will_assert = true;

        //printf("  will assert(SIST).\n");
      }
    }
  }
  else
    SB_R8(ISTAT, SIP, false);

  if(TB_R8(ISTAT, INTF))
  {
    will_assert = true;

    //printf("  will assert(INTF).\n");
  }

  if(TB_R8(DCNTL, IRQD))
  {
    will_assert = false;

    //printf("  won't assert(IRQD).\n");
  }

  if(will_halt)
    state.executing = false;

  if(will_assert != state.irq_asserted)
  {

    //printf("  doing...%d\n",will_assert);
    do_pci_interrupt(0, will_assert);
    state.irq_asserted = will_assert;
  }
}
