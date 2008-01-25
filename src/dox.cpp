/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * Website: http://sourceforge.net/projects/es40
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
 * Contains extra comments for Doxygen.
 *
 * You could read the documentation from this file; but it would probably
 * be easier to go to http://es40.sourceforge.net.
 *
 * $Id: dox.cpp,v 1.10 2008/01/25 09:19:15 iamcamiel Exp $
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

/**
 * \mainpage ES40 Emulator Documentation
 *
 * \section intro Introduction
 * Welcome to the documentation for the ES40 Emulator Project. For the
 * main project page, goto http://sourceforge.net/projects/es40.
 *
 * On these pages here, you can find the documented source code. In the future,
 * you might also find release notes, installation instructions, and compilation
 * instructions here.
 **/

/**
 * \page faq Frequently Asked Questions
 * Here, you can find the answers to a few frequently asked questions.
 * 
 * \section a A. Compilation
 * Compilation related issues.
 *
 * \subsection a1 A1. SDL not found
 * Q: I get warnings about SDL.H, sdl.lib, sdlmain.lib or sdl.dll not being 
 * found. What do I need?
 *
 * A: You need the Simple DirectMedia Layer (SDL) libraries, if you want to
 * use one of the emulated VGA cards. Visit http://www.libsdl.org. To run 
 * the emulator you need the runtime libraries, to compile the emulator you
 * need the development libraries. Make sure your compiler/linker can find
 * the header and library files.
 *
 * \subsection a2 A2. SDL not available
 * Q: SDL is not available for my system, or I don't want to install it. 
 * What are my options?
 *
 * A: Compile without HAVE_SDL defined. For Windows, you can download the
 * no_sdl version of the binary package. If you're on Windows, change the
 * configuration file (gui=win32), if your system supports X-Windows (as is
 * the case on most Linux/BSD/UNIX distributions, change it to gui=X11.
 * If your system is not supported, remove the gui section alltogether, as
 * well as any vga-card sections. Graphics support will nbot be available
 * in this case.
 *
 * \subsection a3 A3. PCAP not found
 * Q: I get warnings about pcap.h, wpcap.lib, wpcap.dll, or libpcap not being 
 * found. What do I need?
 *
 * A: You need the libpcap (Linux and others) or winpcap (Windows) packet
 * capture libraries, if you want to use one of the emulated network cards.
 * Visit http://www.tcpdump.org or http://www.winpcap.org. To run the emulator
 * you need the runtime libraries, to compile the emulator you need the 
 * developer's pack. Make sure your compiler/linker can find the header and
 * library files.
 *
 * \subsection a4 A4. PCAP not available
 * Q: PCAP is not available for my system, or I don't want to install it. 
 * What are my options?
 *
 * A: Your only option is to disable network support, by compiling without
 * HAVE_PCAP defined. For Windows, you can download the no_net version of
 * the binary package.
 *
 * \subsection a5 A5. Endianess issues
 * Q: I'm running the ES40 emulator on a big endian host machine (PowerPC,
 * Sparc) and I get all these strange errors. Network, graphics and disks
 * don't seem to work properly.
 *
 * A: Support for big-endian host architectures has been available in the
 * past, but since the primary developer of the ES40 Emulator no longer
 * has a big endian machine, it has become difficult to support and
 * maintain. If you can, please help us get the emulator working on big-
 * endian architectures again!
 *
 * \section b B. Configuration
 * Configuration related issues.
 *
 * \subsection b1 B1. Which VGA card?
 * Q: Which of the two emulated VGA cards should I choose? S3 Trio64 or
 * Cirrus?
 *
 * A: Right now, both cards are nearly identical (except for the PCI
 * configuration ID, which tells the OS what card this is). In the future,
 * the emulated Cirrus card will evolve into a more complete card, 
 * supporting various SVGA features. For the S3 Trio, this is more difficult,
 * because documentation is harder to find. Also, we chose to continue with
 * Cirrus, because the Bochs emulator emulates a Cirrus card, and we can
 * borrow bits of their code.
 *
 * \subsection b2 B2. CD-ROM on Windows?
 * Q: How do I connect my real CD drive under Windows so that I can have ES40
 * boot a real AlphaVMS CD?
 *
 * A: I'm planning to create a DiskDevice class that does this, but I
 * haven't gotten around to it yet... Right now, it isn't possible yet.
 * You could use a CD-burning tool to create an ISO image, and boot using
 * that image. That's what I do.
 *
 * UPDATE: The current version in CVS, and the next release will make this
 * possibe; check the sample configuration file in CVS for information.
 */

/**
 * \page con_ack Contributors & Acknowledgements
 *
 * \section con Contributors
 * In order of appearance.
 *  - Camiel Vanderhoeven (camiel@camicom.com): main architect/developer.
 *  - Brian Wheeler: UNIX port, initial config file, initial VGA, new IDE, and contributions to overall quality.
 *  - Eduardo Marcelo Serrat: Major bugfixes. 
 *  - Fang Zhe: OS X port.
 *  - David Hittner: Ethernet controller fixes, many wise suggestions in other places.
 *  - Fausto Saporito: Bugfixes.
 *  .
 *
 * \section Acknowledgements
 * In order of appearance.
 *  - Gerrit Woertman (HP)
 *  - Jur van der Burg (helped out with IDE problems)
 *  - Sue Skonetski (HP) (publicity)
 *  - Andy Goldstein (HP)
 *  - Ian Miller (publicity: http://www.openvms.org)
 *  - Anders Gavare (allowing us to re-use parts of GXEmul)
 *  - Bob Supnik (author of the SIMH Alpha pre-implementation)
 *  .
 *
 * If we missed anyone, please let us know!
 **/

/**
 * \page cons Consulted Documentation
 * The following documents have been useful to us:
 *  - Alpha 21264/EV68CB and 21264/EV68DC Microprocessor Hardware Reference Manual [HRM] (http://download.majix.org/dec/21264ev68cb_ev68dc_hrm.pdf)
 *  - DS-0026A-TE: Alpha 21264B Microprocessor Hardware Reference Manual [HRM] (http://ftp.digital.com/pub/Digital/info/semiconductor/literature/21264hrm.pdf)
 *  - Alpha Architecture Handbook [AHB] (http://ftp.digital.com/pub/Digital/info/semiconductor/literature/alphaahb.pdf)
 *  - Alpha Architecture Reference Manual, fourth edition [ARM] (http://download.majix.org/dec/alpha_arch_ref.pdf)
 *  - Tsunami/Typhoon 21272 Chipset Hardware Reference Manual [HRM] (http://download.majix.org/dec/tsunami_typhoon_21272_hrm.pdf)
 *  - AlphaServer ES40 and AlphaStation ES40 Service Guide [SG] (http://www.dec-store.com/PD_00158.aspx)
 *  - 21143 PCI/Cardbus 10/100Mb/s Ethernet LAN Controller Hardware Reference Manual  [HRM] (http://download.intel.com/design/network/manuals/27807401.pdf)
 *  - AT Attachment with Packet Interface - 5 (ATA/ATAPI-5) (http://www.t13.org/Documents/UploadedDocuments/project/d1321r3-ATA-ATAPI-5.pdf)
 *  - Programming Interface for Bus Master IDE COntroller (http://suif.stanford.edu/%7Ecsapuntz/specs/idems100.ps)
 *  - FDC37C669 PC 98/99 Compliant Super I/O Floppy Disk Controller (http://www.smsc.com/main/datasheets/37c669.pdf)
 *  - SCSI 2 (http://www.t10.org/ftp/t10/drafts/s2/s2-r10l.pdf)
 *  - SCSI 3 Multimedia Commands (MMC) (http://www.t10.org/ftp/t10/drafts/mmc/mmc-r10a.pdf)
 *  - SYM53C895 PCI-Ultra2 SCSI I/O Processor (http://www.datasheet4u.com/html/S/Y/M/SYM53C895_LSILogic.pdf.html)
 *  - Ali M1543C B1 South Bridge Version 1.20 (http://mds.gotdns.com/sensors/docs/ali/1543dScb1-120.pdf)
 *  - VGADOC4b (http://home.worldonline.dk/~finth/)
 *  .
 **/

/**
 * \page guest_os Guest OS Notes
 *
 * Status for various guest OSes under ES40
 *
 * \section hp DEC/Compaq/HP OS-es
 *
 * \subsection vms OpenVMS
 * Test 8.3
 *   - Using newide, all devices are recognized and use DMA mode, including CDs.
 *   - There is a permissions violation during install.  Do not abort it, and the
 *     install will work anyway. (note Camiel: differing results of installation
 *     reported. A lot seems to depend on timing).
 *   .
 *
 * \subsection tru64 Tru64 Unix
 * Tested using 5.1B
 *   - Using the newide device:
 *       - Devices are probed correctly.
 *       - DMA is supported.
 *       - CD-ROM devices crash es40 due to unhandled SCSI commands.
 *       - Crash during install due to timing issues in disk code.
 *       .
 *   - Using the ali_ide device
 *       - devices timeout on probe.
 *       .
 *   - ES40 crashes if ali_usb device is present, due to incomplete emulation.
 *   .
 *
 * \section free Free OS'es
 *
 * \subsection fbsd FreeBSD
 * Tested with 6.2
 *   - Using newide
 *      - devices are probed and DMA is used.
 *      - CD-ROM device works, but occasionally times out.
 *      .
 *   .
 *
 * \subsection nbsd NetBSD
 * Tested 3.1.1
 *   - Panics if cirrus device is present.
 *   - Using newide 
 *       - all devices are recognized and use DMA mode.
 *       - CD-ROM devices are supported and work for install and normal use
 *       .
 *   - Using ali_ide
 *       - all devices are recognized and pio mode is used.
 *       - CD-ROM devices are not supported
 *       .
 *   .
 *
 * Tested 4.0
 *   - Panics if cirrus device is present.
 *   - Using newide
 *       - recognizes all devices and uses DMA
 *       - CD-ROM cannot be used due to apparent cdrom corruption.
 *       .
 *   - Using ali_ide
 *       - Unknown at this time.
 *       .
 *   .
 *
 * \subsection linux Linux
 * Tested using Debian 4.0 rc 0, centos 4.2, gentoo 2007.0.
 * Always boot with "boot <device> -flags 1" when using a serial console.
 *   - Gentoo + ali_ide: hang after hda size display
 *   - Gentoo + newide: hang after hda size display
 *   - Centos:  hangs after serial probe.
 *   - Debian:  hangs after serial probe.
 *   .
 *
 * \section srm SRM
 * Ok, its not really an OS, but it acts like one.
 *   - Can boot CDROMs with newide device (with cdrom=true)
 *   - Cannot boot CDROMs with ali_ide device (segfault)
 *   .
 * When booting CDROM-type media with the ali_ide device,
 * set the disk is cdrom=false.
 **/
