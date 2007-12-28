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
 * Contains the code for the configuration file interpreter.
 *
 * X-1.5        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.4        Camiel Vanderhoeven                             14-DEC-2007
 *      Add support for Symbios SCSI controller.
 *
 * X-1.3        Camiel Vanderhoeven                             12-DEC-2007
 *      Add support for file- and RAM-disk.
 *
 * X-1.2        Brian Wheeler                                   10-DEC-2007
 *      Better error reporting.
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h"
#include "Configurator.h"
#include "System.h"
#include "AlphaCPU.h"
#include "Serial.h"
#include "Flash.h"
#include "DPR.h"
#include "AliM1543C.h"
#include "AliM1543C_ide.h"
#include "AliM1543C_usb.h"
#include "DiskFile.h"
#include "DiskRam.h"
#include "Port80.h"
#if defined(USE_CONSOLE)
#include "S3Trio64.h"
#include "Cirrus.h"
#include "gui/plugin.h"
#endif
#if defined(USE_NETWORK)
#include "DEC21143.h"
#endif
#include "Sym53C895.h"

CConfigurator::CConfigurator(class CConfigurator * parent, char * name, char * value, char * text, size_t textlen)
{
  enum { STATE_NONE, 
    STATE_NAME, STATE_NAME_DONE, STATE_IS, 
    STATE_VALUE, STATE_VALUE_DONE, 
    STATE_CHILD } state = STATE_NONE;

  char * cur_name;
  char * cur_value;

  size_t curtext;
  int cmt_depth;
  int child_depth;

  size_t name_start;
  size_t name_len;

  size_t value_start;
  size_t value_len;

  size_t child_start;
  size_t child_len;

  int txt_depth;

  pParent = parent;
  iNumChildren = 0;
  iNumValues = 0;
  myName = name;
  myValue = value;

  for (curtext = 0; curtext < textlen; curtext++)
  {
    //
    // comment
    //
    if (text[curtext] == '/')
    {
      curtext++;
      // may be a comment...
      if (text[curtext] == '*')
      {
        cmt_depth = 1;
        // search end of comment...
        while (cmt_depth)
        {
          curtext++;
          if (text[curtext] == '/')
            if (text[++curtext] =='*')
              cmt_depth++;
            else
              curtext--;
          if (text[curtext] == '*')
            if (text[++curtext] =='/')
              cmt_depth--;
            else
              curtext--;
          
        }
        continue;
      }
      else if (text[curtext] =='/')
      {
        curtext++;
        // line comment
        while (text[curtext]!=0x0d && text[curtext]!= 0x0a)
          curtext++;
        continue;
      }
    }

    if (text[curtext] == '\"' && (state == STATE_VALUE || state == STATE_CHILD))
    {
      txt_depth = 1;
      while (txt_depth)
      {
        curtext++;
        if (text[curtext] == '"')
        {
          if (text[curtext+1] =='"')
            curtext++;
          else
            txt_depth = 0;
        }
      }
      continue;
    }

    switch(state)
    {
    case STATE_NONE:
      if (isalnum((unsigned char)text[curtext]) || text[curtext]=='.' || text[curtext]=='_')
      {
        name_start = curtext;
        state = STATE_NAME;
      }
      break;
    case STATE_NAME:
      if (text[curtext] == '=')
      {
        state = STATE_IS;
        name_len = curtext - name_start;
      }
      else if (isblank(text[curtext]))
      {
        state = STATE_NAME_DONE;
        name_len = curtext - name_start;
      }
      else if (!isalnum((unsigned char)text[curtext]) && text[curtext]!='.' && text[curtext] != '_')
        printf("STATE_NAME: Illegal character: \'%c\'!! (%02x @ %d)\n", text[curtext],text[curtext],curtext);
      break;
    case STATE_NAME_DONE:
      if (text[curtext] == '=')
        state = STATE_IS;
      else if (!isblank(text[curtext]))
        printf("STATE_NAME_DONE: Illegal character: \'%c\'!! (%02x @ %d)\n", text[curtext],text[curtext],curtext);
      break;
    case STATE_IS:
      if (isalnum((unsigned char)text[curtext]) || text[curtext]=='.' || text[curtext] == '_')
      {
        value_start = curtext;
        value_len = 1;
        state = STATE_VALUE;
      }
      if (text[curtext] == '\"')
      {
        value_start = curtext;
        state = STATE_VALUE;
        curtext--;
      }
      break;
    case STATE_VALUE:
      if (text[curtext] == ';')
      {
        value_len = curtext - value_start;
        cur_name = (char*)malloc(name_len+1);
        memcpy(cur_name,&text[name_start],name_len);
        cur_name[name_len] = '\0';
        cur_value = (char*)malloc(value_len+1);
        memcpy(cur_value,&text[value_start],value_len);
        cur_value[value_len] = '\0';

        strip_string(cur_value);
        add_value(cur_name, cur_value);

        state = STATE_NONE;
      }
      else if (text[curtext] == '{')
      {
        value_len = curtext - value_start;
        state = STATE_CHILD;
        child_start = curtext+1;
        child_depth = 1;
      }
      else if (isspace((unsigned char)text[curtext]))
      {
        state = STATE_VALUE_DONE;
        value_len = curtext - value_start;
      }
      else if (!isalnum((unsigned char)text[curtext]) && text[curtext]!='.' && text[curtext] != '_')
        printf("STATE_VALUE: Illegal character: \'%c\'!! (%02x @ %d)\n", text[curtext],text[curtext],curtext);
      break;
    case STATE_VALUE_DONE:
      if (text[curtext] == ';')
      {
        cur_name = (char*)malloc(name_len+1);
        memcpy(cur_name,&text[name_start],name_len);
        cur_name[name_len] = '\0';
        cur_value = (char*)malloc(value_len+1);
        memcpy(cur_value,&text[value_start],value_len);
        cur_value[value_len] = '\0';

        strip_string(cur_value);
        add_value(cur_name,cur_value);

        state = STATE_NONE;
      }
      else if (text[curtext] == '{')
      {
        state = STATE_CHILD;
        child_start = curtext+1;
        child_depth = 1;
      }
      else if (!isspace((unsigned char)text[curtext]))
        printf("STATE_VALUE_DONE: Illegal character: \'%c\'!! (%02x @ %d)\n", text[curtext],text[curtext],curtext);
      break;
    case STATE_CHILD:
      if (text[curtext] == '{')
        child_depth++;
      else if (text[curtext] == '}')
      {
        child_depth--;
        if (!child_depth)
        {
          cur_name = (char*)malloc(name_len+1);
          memcpy(cur_name,&text[name_start],name_len);
          cur_name[name_len] = '\0';
          cur_value = (char*)malloc(value_len+1);
          memcpy(cur_value,&text[value_start],value_len);
          cur_value[value_len] = '\0';
          child_len = curtext-child_start;
          state = STATE_NONE;

          strip_string(cur_value);

          pChildren[iNumChildren++] = new CConfigurator(this, cur_name, cur_value, &text[child_start], child_len);
        }
      }
    }
  }

  int i;
  if (parent==0)
  {
    for (i=0;i<iNumChildren;i++)
    {
      pChildren[i]->initialize();
    }
  }
}

CConfigurator::~CConfigurator(void)
{
}

char * CConfigurator::strip_string(char * c)
{
  char * pos = c;
  char * org = c+1;
  bool end_it = false;

  if (c[0] =='\"')
  {
    while (!end_it)
    {
      if (*org=='\"')
      {
        org++;
        if (*org=='\"')
          *pos++ = *org++;
        else
          end_it = true;
      }
      else
        *pos++ = *org++;
    }
    *pos = '\0';
  }

  return c;
}

void CConfigurator::add_value(char * n, char * v)
{
  pValues[iNumValues].name = n;
  pValues[iNumValues].value = v;
  iNumValues++;
}

char * CConfigurator::get_text_value(char * n, char * def)
{
  int i;
  for (i=0;i<iNumValues;i++)
  {
    if (!strcmp(pValues[i].name, n))
      return pValues[i].value;
  }
  return def;
}

bool CConfigurator::get_bool_value(char * n, bool def)
{
  int i;
  for (i=0;i<iNumValues;i++)
  {
    if (!strcmp(pValues[i].name, n))
    {
      switch (pValues[i].value[0])
      {
      case 't':
      case 'T':
      case 'y':
      case 'Y':
      case '1':
        return true;
      case 'f':
      case 'F':
      case 'n':
      case 'N':
      case '0':
        return false;
      default:
        printf("Illegal boolean value: %s   \n", pValues[i].value);
      }
    }
  }
  return def;
}

int CConfigurator::get_int_value(char * n, int def)
{
  int i;
  for (i=0;i<iNumValues;i++)
  {
    if (!strcmp(pValues[i].name, n))
      return atoi(pValues[i].value);
  }
  return def;
}

// THIS IS WHERE THINGS GET COMPLICATED...

#define NO_FLAGS 0
#define IS_CHIPSET 1
#define HAS_PCI 2
#define IS_PCI 4
#define HAS_ISA 8
#define IS_ISA 16
#define HAS_DISK 32
#define IS_DISK 64
#define IS_GUI 128

typedef struct {
  char * name;
  classid id;
  int flags;
} classinfo;

  
classinfo classes[] = 
{
  {"tsunami", c_tsunami, IS_CHIPSET | HAS_PCI},
  {"ev68cb",  c_ev68cb,  NO_FLAGS},
  {"ali",     c_ali,     IS_PCI | HAS_ISA},
  {"ali_ide", c_ali_ide, IS_PCI | HAS_DISK},
  {"ali_usb", c_ali_usb, IS_PCI},
  {"serial",  c_serial,  NO_FLAGS},
  {"s3",      c_s3,      IS_PCI},
  {"cirrus",  c_cirrus,  IS_PCI},
  {"dec21143",c_dec21143,IS_PCI},
  {"sym53c895", c_sym53c895, IS_PCI | HAS_DISK},
  {"file",    c_file,    IS_DISK},
  {"ramdisk", c_ramdisk, IS_DISK},
  {"sdl",     c_sdl,     IS_GUI},
  {0,c_none,0}
};

void CConfigurator::initialize()
{
  myClassId = c_none;
  int i = 0;
  int pcibus = 0;
  int pcidev = 0;
  int idedev = 0;
  int idebus = 0;
  int number;
  char * pt;

  for(i=0;classes[i].id != c_none; i++)
  {
    if (!strcmp(myValue,classes[i].name))
    {
      myClassId = classes[i].id;
      myFlags = classes[i].flags;
      break;
    }
  }

  if (myClassId == c_none)
  {
    printf("Class %s not known!!\n",myValue);
    exit(1);
  }

  if (myFlags & IS_PCI)
  {
    if (strncmp(myName,"pci",3))
    {
      printf("Error: name of PCI device %s should be pci<bus>.<device>, %s found.\n",myValue,myName);
      exit(1);
    }
    if (!(pParent->get_flags() & HAS_PCI))
    {
      printf("Error: parent of PCI device %s(%s) should be a pci-bus capable device.\n",myName,myValue);
      exit(1);
    }

    pt = &myName[3];
    pcibus = atoi(pt);
    pt = strchr(pt,'.');
    if (!pt)
    {
      printf("Error: name of PCI device %s should be pci<bus>.<device>, %s found.\n",myValue,myName);
      exit(1);
    }
    pt++;
    pcidev = atoi(pt);
  }

  if (myFlags & IS_DISK)
  {
    if (strncmp(myName,"disk",4))
    {
      printf("Error: name of Disk device %s should be disk<channel>.<device>, %s found.\n",myValue,myName);
      exit(1);
    }
    if (!(pParent->get_flags() & HAS_DISK))
    {
      printf("Error: parent of disk device %s(%s) should be a disk controller.\n",myName,myValue);
      exit(1);
    }

    pt = &myName[4];
    idebus = atoi(pt);
    pt = strchr(pt,'.');
    if (!pt)
    {
      printf("Error: name of Disk device %s should be disk<controller>.<device>, %s found.\n",myValue,myName);
      exit(1);
    }
    pt++;
    idedev = atoi(pt);
  }

  switch(myClassId)
  {
  case c_tsunami:
    myDevice = new CSystem(this);
    new CDPR(this,(CSystem *)myDevice);
    new CFlash(this,(CSystem *)myDevice);
    break;
  case c_ev68cb:
    if (pParent->get_flags() & IS_CHIPSET)
      myDevice = new CAlphaCPU(this,(CSystem *)pParent->get_device());
    break;
  case c_ali:
    myDevice = new CAliM1543C(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    new CPort80(this,(CSystem *)pParent->get_device());
    break;
  case c_ali_ide:
    myDevice = new CAliM1543C_ide(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_ali_usb:
    myDevice = new CAliM1543C_usb(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_s3:
    myDevice = new CS3Trio64(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_cirrus:
    myDevice = new CCirrus(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_dec21143:
    myDevice = new CDEC21143(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_sym53c895:
    myDevice = new CSym53C895(this,(CSystem *)pParent->get_device(),pcibus,pcidev);
    break;
  case c_file:
    myDevice = new CDiskFile(this,(CDiskController *)pParent->get_device(),idebus,idedev);
    break;
  case c_ramdisk:
    myDevice = new CDiskRam(this,(CDiskController *)pParent->get_device(),idebus,idedev);
    break;
  case c_serial:
    if (pParent->get_flags() & IS_CHIPSET)
    {
      number = 0;
      if (!strncmp(myName,"serial",6))
      {
        pt = &myName[6];
        number = atoi(pt);
      }
      myDevice = new CSerial(this,(CSystem *)pParent->get_device(),number);
    }
    break;
#if defined(USE_CONSOLE)
  case c_sdl:
    PLUG_load_plugin (this, sdl);
    break;
#endif // USE_CONSOLE

  }

  for (i=0;i<iNumChildren;i++)
  {
    pChildren[i]->initialize();
  }
}
