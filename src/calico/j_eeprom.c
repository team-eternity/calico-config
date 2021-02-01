/*
  CALICO
 
  Jaguar EEPROM Emulation

  The MIT License (MIT)

  Copyright (c) 2016 James Haley

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "../elib/elib.h"
#include "../elib/misc.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "j_eeprom.h"

static FILE *eepromFile;

int startskill  = sk_medium;
int startmap    = 1;
int sfxvolume   = 200;
int musicvolume = 128;
int controltype = 0;
int maxlevel    = 1;

#define IDWORD (('D'<<8)+'1')
#define EEWORDS 8 // MUST BE EVEN!!!!!!
static uint16_t eeprombuffer[EEWORDS];

const char buttona[NUMCONTROLOPTIONS][8] = { "Speed", "Speed", "Fire",  "Fire",  "Use",   "Use"   };
const char buttonb[NUMCONTROLOPTIONS][8] = { "Fire",  "Use",   "Speed", "Use",   "Speed", "Fire"  };
const char buttonc[NUMCONTROLOPTIONS][8] = { "Use",   "Fire",  "Use",   "Speed", "Fire",  "Speed" };

const char *buttons_abc[NUMCONTROLOPTIONS] =
{
    "Speed/Fire/Use",
    "Speed/Use/Fire",
    "Fire/Speed/Use",
    "Fire/Use/Speed",
    "Use/Speed/Fire",
    "Use/Fire/Speed"
};

//
// Open a file to use as the Jaguar EEPROM store.
//
static void J_OpenEEPROM(const char *mode)
{
    char *const fullpath = M_SafeFilePath(hal_medialayer.getWriteDirectory(ELIB_APPNAME), "eeprom.cal");
    if(eepromFile)
    {
        fclose(eepromFile);
        eepromFile = NULL;
    }
    eepromFile = hal_platform.fileOpen(fullpath, mode);
    efree(fullpath);
}

//
// Read from emulated EEPROM
//
static uint16_t eeread(int address)
{
   uint16_t temp, value = 0;

   if(address == 0)
      J_OpenEEPROM("rb");

   if(eepromFile)
   {
      if(fread(&temp, sizeof(temp), 1, eepromFile) == 1)
         value = temp;

      if(address == 7)
      {
         fclose(eepromFile);
         eepromFile = NULL;
      }
   }

   return value;
}

//
// Write to emulated EEPROM
//
int eewrite(int data, int address)
{
   uint16_t store = (uint16_t)data;

   if(address == 0)
      J_OpenEEPROM("wb");

   if(eepromFile)
   {
      fwrite(&store, sizeof(store), 1, eepromFile);
      
      if(address == 7)
      {
         fclose(eepromFile);
         eepromFile = NULL;
      }
   }

   return -1; // always successful (we currently lie if it wasn't...)
}

void ClearEEProm(void)
{
   startskill  = sk_medium;
   startmap    = 1;
   sfxvolume   = 200;
   musicvolume = 128;
   controltype = 0;
   maxlevel    = 1;
}

void ReadEEProm(void)
{
   int i;
   uint16_t checksum;

   checksum = 12345;

   for(i = 0; i < EEWORDS; i++)
   {
      eeprombuffer[i] = eeread(i);
      if(i != EEWORDS-1)
         checksum += eeprombuffer[i];
   }

   if(checksum != eeprombuffer[EEWORDS-1])
   {
      ClearEEProm();
      return;
   }

   startskill = eeprombuffer[1];
   if(startskill < sk_baby || startskill > sk_nightmare)
      startskill = sk_medium;
   startmap = eeprombuffer[2];
   if(startmap < 1 || startmap > 26)
      startmap = 1;
   sfxvolume = eeprombuffer[3];
   if(sfxvolume < 0 || sfxvolume > 255)
      sfxvolume = 200;
   musicvolume = eeprombuffer[4];
   if(musicvolume < 0 || musicvolume > 255)
      musicvolume = 128;
   controltype = eeprombuffer[5];
   if(controltype < 0 || controltype > 5)
      controltype = 0;
   maxlevel = eeprombuffer[6];
   if(maxlevel < 1 || maxlevel > 25)
      maxlevel = 1;
}

void WriteEEProm(void)
{
   int i;

   eeprombuffer[0] = (uint16_t)IDWORD;
   eeprombuffer[1] = (uint16_t)startskill;
   eeprombuffer[2] = (uint16_t)startmap;
   eeprombuffer[3] = (uint16_t)sfxvolume;
   eeprombuffer[4] = (uint16_t)musicvolume;
   eeprombuffer[5] = (uint16_t)controltype;
   eeprombuffer[6] = (uint16_t)maxlevel;

   eeprombuffer[EEWORDS-1] = 12345;

   for(i = 0; i < EEWORDS-1; i++)
      eeprombuffer[EEWORDS-1] += eeprombuffer[i];

   for(i = 0; i < EEWORDS; i++)
      eewrite(eeprombuffer[i], i);
}

// EOF

