/*  ES40 emulator.
 *
 *  This file is based upon Bochs.
 *
 *  Copyright (C) 2002  MandrakeSoft S.A.
 *
 *    MandrakeSoft S.A.
 *    43, rue d'Aboukir
 *    75002 Paris - France
 *    http://www.linux-mandrake.com/
 *    http://www.mandrakesoft.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/**
 *
 * X-1.2        Camiel Vanderhoeven                             7-DEC-2007
 *      Code cleanup.
 *
 * X-1.1        Camiel Vanderhoeven                             6-DEC-2007
 *      Initial version for ES40 emulator.
 *
 **/

/////////////////////////////////////////////////////////////////////////
//
// Methods of bx_keymap_c :
//
// - loadKeymap(u32 convertStringToSymbol(const char*));
//   loads the configuration specified keymap file if keymapping is enabled
//   using convertStringToSymbol to convert strings to client constants
//
// - loadKeymap(u32 convertStringToSymbol(const char*), const char* filename);
//   loads the specified keymap file 
//   using convertStringToSymbol to convert strings to client constants
//
// - isKeymapLoaded () returns true if the keymap contains any valid key
//   entries.
//
// - convertStringToBXKey
//   convert a null-terminate string to a BX_KEY code
//
// - findHostKey(u32 key)
// - findAsciiChar(u8 ch)
//   Each of these methods returns a pointer to a BXKeyEntry structure
//   corresponding to a key.  findHostKey() finds an entry whose hostKey
//   value matches the target value, and findAsciiChar() finds an entry
//   whose ASCII code matches the search value.

// In case of unknown symbol
#define BX_KEYMAP_UNKNOWN   0xFFFFFFFF

// Structure of an element of the keymap table
typedef struct { 
  u32 baseKey;   // base key
  u32 modKey;   // modifier key that must be held down
  s32 ascii;    // ascii equivalent, if any
  u32 hostKey;  // value that the host's OS or library recognizes
  } BXKeyEntry;

class bx_keymap_c {
public:
  bx_keymap_c(void);
  ~bx_keymap_c(void);

  void   loadKeymap(u32(*)(const char*));
  void   loadKeymap(u32(*)(const char*),const char *filename);
  bool isKeymapLoaded ();

  BXKeyEntry *findHostKey(u32 hostkeynum);
  BXKeyEntry *findAsciiChar(u8 ascii);
  char *getBXKeyName(u32 key);

private:
  u32 convertStringToBXKey(const char *);
 
  BXKeyEntry *keymapTable;
  u16   keymapCount;
  };

extern bx_keymap_c bx_keymap;
