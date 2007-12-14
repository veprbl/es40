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
 * Contains the definitions for the emulated Symbios SCSI controller.
 *
 * X-1.1        Camiel Vanderhoeven                             14-DEC-2007
 *      Initial version in CVS
 **/

#if !defined(INCLUDED_SYM53C895_H_)
#define INCLUDED_SYM53C895_H_

#include "DiskController.h"
#include "Configurator.h"

class CSym53C895 : public CDiskController  
{
 public:
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  virtual int DoClock();

  virtual void WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Bar(int func,int bar, u32 address, int dsize);

  virtual u32 config_read_custom(int func, u32 address, int dsize, u32 data);
  virtual void config_write_custom(int func, u32 address, int dsize, u32 old_data, u32 new_data, u32 data);

  CSym53C895(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CSym53C895();

 private:

  u8 read_b_scntl0();
  void write_b_scntl0(u8 value);
  u8 read_b_scntl1();
  void write_b_scntl1(u8 value);
  u8 read_b_scntl2();
  void write_b_scntl2(u8 value);
  u8 read_b_scntl3();
  void write_b_scntl3(u8 value);
  u8 read_b_scid();
  void write_b_scid(u8 value);
  u8 read_b_sxfer();
  void write_b_sxfer(u8 value);
  u8 read_b_istat();
  void write_b_istat(u8 value);
  u8 read_b_ctest3();
  void write_b_ctest3(u8 value);
  u8 read_b_ctest4();
  void write_b_ctest4(u8 value);
  u8 read_b_ctest5();
  void write_b_ctest5(u8 value);
  u8 read_b_dmode();
  void write_b_dmode(u8 value);
  u8 read_b_dien();
  void write_b_dien(u8 value);
  u8 read_b_dstat();
  u8 read_b_dcntl();
  void write_b_dcntl(u8 value);
  u8 read_b_sien0();
  void write_b_sien0(u8 value);
  u8 read_b_sien1();
  void write_b_sien1(u8 value);
  u8 read_b_sist0();
  u8 read_b_sist1();
  u8 read_b_stest1();
  void write_b_stest1(u8 value);
  u8 read_b_stest2();
  void write_b_stest2(u8 value);
  u8 read_b_stest3();
  void write_b_stest3(u8 value);
  u8 read_b_stest4();
  u8 read_b_respid0();
  void write_b_respid0(u8 value);
  u8 read_b_respid1();
  void write_b_respid1(u8 value);
  u8 read_b_stime0();
  void write_b_stime0(u8 value);
  u8 read_b_stime1();
  void write_b_stime1(u8 value);
  u8 read_b_gpreg();
  void write_b_gpreg(u8 value);
  u8 read_b_gpcntl();
  void write_b_gpcntl(u8 value);

  u32 read_d_dsa();
  void write_d_dsa(u32 value);
  u32 read_d_dsp();
  void write_d_dsp(u32 value);
  u32 read_d_dsps();
  void write_d_dsps(u32 value);

  void select_target(int target);
  void byte_to_target(u8 value);
  u8 byte_from_target();
  void end_xfer();
  int do_command();

// The state structure contains all elements that need to be saved to the statefile.
  struct SSym53C895State {

    struct {
      int arb; 
      bool start;
      bool watn;
      bool epc;
      bool aap;
      bool trg;
      bool exc;
      bool adb;
      bool dhp;
      bool con;
      bool rst;
      bool aesp;
      bool iarb;
      bool sst;
      bool sdu;
      bool chm;
      bool slpmd;
      bool slphben;
      bool wss;
      bool vue0;
      bool vue1;
      bool wsr;
      bool ultra;
      int scf;
      bool ews;
      int ccf;

      bool rre;
      bool sre;

      int my_scsi_id;
      u16 response_id;
    } scntl;  // SCSI control

    struct {
      int tp;
      int mo;
    } sxfer; // SCSI transfer

    struct {
      bool abrt; // Abort Operation
      bool srst; // Software Reset
      bool sigp; // Signal Process
      bool sem;  // Semaphore
      bool con;  // Connected
      bool intf; // Interrupt on the Fly
      bool sip;  // SCSI Interrupt Pending
      bool dip;  // DMA Interrupt Pending
    } istat; // Interrupt Status; register 14

    struct {
      bool bdis;
      bool zmod;
      bool zsd;
      bool srtm;
      bool mpee;
      int fbl;
      bool masr;
      bool srtch;
      bool flf;
      bool fm;
      bool wrie;
    } ctest;

    struct {
      bool ddir; // dma direction
      bool dfs; // dma fifo size
      int bl; // burst length
      int bo; // byte offset
      bool siom;
      bool diom;
      bool erl;
      bool ermp;
      bool bof;
      bool man;
      bool clse;
      bool pff;
      bool pfen;
      bool ssm;
      bool irqm;
      bool irqd;
      bool com;
    } dma;

    struct {
      bool dfe;
      bool mdpe;
      bool bf;
      bool abrt;
      bool ssi;
      bool sir;
      bool iid;
    } dien,dstat;

    struct {
      bool ma;
      bool cmp;
      bool sel;
      bool rsl;
      bool sge;
      bool udc;
      bool rst;
      bool par;
      bool sbmc;
      bool sto;
      bool gen;
      bool hth;
    } sien, sist;

    struct {
      bool te;
      bool str;
      bool hsc;
      bool dsi;
      bool s16;
      bool ttm;
      bool stw;
      bool sce;
      bool rof;
      bool dif;
      bool slb;
      bool szm;
      bool aws;
      bool ext;
      bool low;
      bool sclk;
      bool siso;
      bool qen;
      bool qsel;
    } stest;

    struct {
      int hth;
      int sel;
      bool hthba;
      bool gensf;
      bool hthsf;
      int gen;
    } stime;

    struct {
      bool ack;
      bool atn;
    } socl;

    struct {
      bool carry;
    } alu;

    struct {
      bool me;
      bool fe;
      int gpio_en;
      int gpio;
    } gp; //general purpose

    u8 scratch[10*4]; // scratch registers A..J

    u32 dsa; // data structure address
    u32 dsp; // dma scripts pointer

    u8 dcmd;
    u32 dbc;
    u32 dsps;
    u32 temp;
    u32 dnad;
    u8 sfbr;

    u8 ram[4096];

    bool executing;

    int phase;
    int target;

    struct {
      // msgi: Message In Phase (disk -> controller)
      u8 msgi[10];
      int msgi_len;
      int msgi_ptr;

      // msgo: Message Out Phase (controller -> disk)
      u8 msgo[10];
      int msgo_len;

      bool msg_err;

      // cmd: Command phase 
      u8 cmd[20];
      int cmd_len;

      u8 dati[512];
      int dati_ptr;
      int dati_len;
      bool dati_off_disk;

      u8 dato[512];
      int dato_ptr;
      int dato_len;
      bool dato_to_disk;

      int stat[10];
      int stat_ptr;
      int stat_len;

    } per_target[16];

  } state;

};

#endif // !defined(INCLUDED_SYM_H)
