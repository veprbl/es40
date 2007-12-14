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
 * Contains the definitions for the configuration file interpreter.
 *
 * X-1.3        Camiel Vanderhoeven                             14-DEC-2007
 *      Add support for Symbios SCSI controller.
 *
 * X-1.2        Camiel Vanderhoeven                             12-DEC-2007
 *      Add support for file- and RAM-disk.
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS.
 **/

#if !defined(__CONFIGURATOR_H__)
#define __CONFIGURATOR_H__

#define CFG_MAX_CHILDREN  25
#define CFG_MAX_VALUES    50

typedef enum {c_none,
              // chipsets
              c_tsunami,
              // system devices
              c_ev68cb,
              c_serial,
              // pci devices
              c_ali,
              c_ali_ide,
              c_ali_usb,
              c_s3,
              c_cirrus,
              c_dec21143,
              c_sym53c895,
              // disk devices
              c_file,
              c_ramdisk,
              // gui's
              c_sdl} classid;

class CConfigurator
{
public:
  CConfigurator(class CConfigurator * parent, char * name, char * value, char * text, int textlen);
  ~CConfigurator(void);

  char * strip_string(char * c);
  void add_value(char * n, char * v);

  char * get_text_value(char * n) { return get_text_value(n, (char*)0); };
  char * get_text_value(char * n, char * def);

  bool get_bool_value(char * n) { return get_bool_value(n, false); };
  bool get_bool_value(char * n, bool def);

  int get_int_value(char * n) { return get_int_value(n, 0); };
  int get_int_value(char * n, int def);

  classid get_class_id() { return myClassId; };
  void * get_device() { return myDevice; };
  int get_flags() { return myFlags; };

  char * get_myName() { return myName; };
  char * get_myValue() { return myValue; };
  CConfigurator * get_myParent() { return pParent; };

  void initialize();

private:
  class CConfigurator * pParent;
  class CConfigurator * pChildren[CFG_MAX_CHILDREN];
  int iNumChildren;
  char * myName;
  char * myValue;
  void * myDevice;
  classid myClassId;
  int myFlags;
  int iNumValues;
  struct {
    char * name;
    char * value;
  } pValues[CFG_MAX_VALUES];

};

#endif //!defined(__CONFIGURATOR_H__)
