//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// ROM emulation class                                                      //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the system ROM (512B), the interface is pretty       //
// simple: constructor, reset, peek, poke.                                  //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define ROM_CPP

// #include <crtdbg.h>
// #define   TRACE_ROM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "rom.h"

extern CErrorInterface *gError;

CRom::CRom(CSystemBase &parent, const char *romfile, bool useEmu)
    : mSystem{parent}
{
    mWriteEnable = FALSE;
    mValid = TRUE;
    SetBankType(LynxMemBank_ROM);
    strncpy(mFileName, romfile, 1024);
    Reset();

    // Initialise ROM
    for (int loop = 0; loop < ROM_SIZE; loop++)
        mRomData[loop] = DEFAULT_ROM_CONTENTS;
    // actually not part of Boot ROM but uninitialized otherwise
    // Reset Vector etc
    mRomData[0x1F8] = 0x00;
    mRomData[0x1F9] = 0x80;
    mRomData[0x1FA] = 0x00;
    mRomData[0x1FB] = 0x30;
    mRomData[0x1FC] = 0x80;
    mRomData[0x1FD] = 0xFF;
    mRomData[0x1FE] = 0x80;
    mRomData[0x1FF] = 0xFF;

    if (useEmu)
    {
        mValid = FALSE;
    }
    else
    {
        // Load up the file

        FILE *fp;

        if ((fp = fopen(mFileName, "rb")) == NULL)
        {
            CLynxException lynxerr;

            lynxerr.Message() << "The Lynx Boot ROM image couldn't be located!";
            lynxerr.Description()
                << "The lynx emulator can run without the Boot ROM image." << std::endl
                << "\"" << romfile << "\" was not found in the lynx emulator " << std::endl
                << "directory (see the LynxEmu User Guide for more information).";
            // throw(lynxerr);
            //      fprintf(stdout, "The Lynx Boot ROM image couldn't be located! Using built-in replacement\n");
            mValid = FALSE;
        }
        else
        {
            // Read in the 512 bytes

            if (fread(mRomData, sizeof(char), ROM_SIZE, fp) != ROM_SIZE)
            {
                CLynxException lynxerr;

                lynxerr.Message() << "The Lynx Boot ROM image couldn't be loaded!";
                lynxerr.Description()
                    << "The lynx emulator can run without the Boot ROM image." << std::endl
                    << "It appears that your BOOT image may be corrupted or there is" << std::endl
                    << "some other error.(see the LynxEmu User Guide for more information)";
                //      throw(lynxerr);
                //         fprintf(stdout, "The Lynx Boot ROM image couldn't be loaded! Using built-in replacement\n");
                mValid = FALSE;
            }
            if (fp)
                fclose(fp);
        }

        // Check the code that has been loaded and report an error if its a
        // fake version (from handy distribution) of the bootrom
        // would be more intelligent to make a crc

        if (mRomData[0x1FE] != 0x80 || mRomData[0x1FF] != 0xFF)
        {
            /*            gError->Warning("FAKE LYNXBOOT.IMG - CARTRIDGES WILL NOT WORK\n\n"
                                        "PLEASE READ THE ACCOMPANYING README.TXT FILE\n\n"
                                        "(Do not email the author asking for this image)\n");
            */
            //      fprintf(stdout, "The Lynx Boot ROM image is invalid! Using built-in replacement\n");
            mValid = FALSE;
        }

        if (mValid == FALSE)
        {
            throw std::runtime_error("The chosen bootrom is not existing or invalid. Switching now to bootrom emulation. (Un)check the menu item to get rid of this message and/or select a valid rom image.");
        }
    }
}

void CRom::Reset(void)
{
    // Nothing to do here
}

bool CRom::ContextSave(FILE *fp)
{
    if (!fprintf(fp, "CRom::ContextSave"))
        return 0;
    if (!fwrite(mRomData, sizeof(UBYTE), ROM_SIZE, fp))
        return 0;
    return 1;
}

bool CRom::ContextLoad(LSS_FILE *fp)
{
    char teststr[100] = "XXXXXXXXXXXXXXXXX";
    if (!lss_read(teststr, sizeof(char), 17, fp))
        return 0;
    if (strcmp(teststr, "CRom::ContextSave") != 0)
        return 0;

    if (!lss_read(mRomData, sizeof(UBYTE), ROM_SIZE, fp))
        return 0;
    return 1;
}

// END OF FILE