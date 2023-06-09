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
// System object header file                                                //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and inline code for   //
// the system object, this object if what binds together all of the Handy   //
// hardware enmulation objects, its the glue that holds the system together //
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

#ifndef SYSTEM_H
#define SYSTEM_H

#pragma inline_depth(255)
#pragma inline_recursion(on)

#ifdef _LYNXDBG

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #undef THIS_FILE
// static char THIS_FILE[] = __FILE__;
// #endif

#endif

#include "machine.h"
#include "global.h"

#define HANDY_SYSTEM_FREQ 16000000
#define HANDY_TIMER_FREQ 20
#define HANDY_AUDIO_SAMPLE_FREQ 22050
#define HANDY_AUDIO_SAMPLE_PERIOD (HANDY_SYSTEM_FREQ / HANDY_AUDIO_SAMPLE_FREQ)
#define HANDY_AUDIO_WAVESHAPER_TABLE_LENGTH 0x200000

#ifdef LINUX_PATCH
// #define HANDY_AUDIO_BUFFER_SIZE					4096	// Needed for SDL 8bit MONO
// #define HANDY_AUDIO_BUFFER_SIZE					8192	// Needed for SDL STEREO 8bit
#define HANDY_AUDIO_BUFFER_SIZE 16384 // Needed for SDL STEREO 16bit
#else
#define HANDY_AUDIO_BUFFER_SIZE 8 * (HANDY_AUDIO_SAMPLE_FREQ / 4)
#endif

#define HANDY_FILETYPE_LNX 0
#define HANDY_FILETYPE_HOMEBREW 1
#define HANDY_FILETYPE_SNAPSHOT 2
#define HANDY_FILETYPE_ILLEGAL 3
#define HANDY_FILETYPE_RAW 4

#define HANDY_SCREEN_WIDTH 160
#define HANDY_SCREEN_HEIGHT 102
//
// Define the global variable list
//

typedef struct lssfile
{
    UBYTE *memptr;
    ULONG index;
    ULONG index_limit;
} LSS_FILE;

int lss_read(void *dest, int varsize, int varcount, LSS_FILE *fp);

//
// Define the interfaces before we start pulling in the classes
// as many classes look for articles from the interfaces to
// allow compilation

#include "sysbase.h"

class CSystem;

//
// Now pull in the parts that build the system
//
#include "lynxbase.h"
#include "memfault.h"
#include "ram.h"
#include "rom.h"
#include "memmap.h"
#include "cart.h"
#include "eeprom.h"
#include "susie.h"
#include "mikie.h"
#include "c65c02.h"

#define TOP_START 0xfc00
#define TOP_MASK 0x03ff
#define TOP_SIZE 0x400
#define SYSTEM_SIZE 65536

#define LSS_VERSION_OLD "LSS2"
#define LSS_VERSION "LSS3"

class CSystem : public CSystemBase
{
  public:
    CSystem(const char *gamefile, const char *romfile, bool useEmu);
    ~CSystem();
    void SaveEEPROM(void);

  public:
    void HLE_BIOS_FE00(void);
    void HLE_BIOS_FE19(void);
    void HLE_BIOS_FE4A(void);
    void HLE_BIOS_FF80(void);
    void Reset(void);
    bool ContextSave(const char *context);
    bool ContextLoad(const char *context);
    bool IsZip(const char *filename);
    bool ReloadCart();
    bool ReadCart();

    void RegisterMemoryAccessCallback(std::function<void(LynxMemBank, uint16_t, bool)> callback)
    {
        mMemoryAccessCallback = callback;
    }

    ULONG Update(void)
    {
        ULONG cycles = mSystemCycleCount;
        //
        // Only update if there is a predicted timer event
        //
        if (mSystemCycleCount >= mNextTimerEvent)
        {
            mMikie->Update();
        }
        //
        // Step the processor through 1 instruction
        //
        mCpu->Update();

#ifdef _LYNXDBG
        // Check breakpoint
        static ULONG lastcycle = 0;
        if (lastcycle < mCycleCountBreakpoint && gSystemCycleCount >= mCycleCountBreakpoint)
            gBreakpointHit = TRUE;
        lastcycle = gSystemCycleCount;

        // Check single step mode
        if (gSingleStepMode)
            gBreakpointHit = TRUE;
#endif

        //
        // If the CPU is asleep then skip to the next timer event
        //
        if (mSystemCPUSleep)
        {
            mSystemCycleCount = mNextTimerEvent;
        }

        return mSystemCycleCount - cycles;
    }

    //
    // We MUST have separate CPU & RAM peek & poke handlers as all CPU accesses must
    // go thru the address generator at $FFF9
    //
    // BUT, Mikie video refresh & Susie see the whole system as RAM
    //
    // Complete and utter wankers, its taken me 1 week to find the 2 lines
    // in all the documentation that mention this fact, the mother of all
    // bugs has been found and FIXED.......

    //
    // CPU
    //
    inline void Poke_CPU(ULONG addr, UBYTE data)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr, true);
        }
        mMemoryHandlers[addr]->Poke(addr, data);
    };
    inline UBYTE Peek_CPU(ULONG addr)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr, false);
        }
        return mMemoryHandlers[addr]->Peek(addr);
    };
    inline void PokeW_CPU(ULONG addr, UWORD data)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr, true);
        }
        mMemoryHandlers[addr]->Poke(addr, data & 0xff);
        addr++;
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr, true);
        }
        mMemoryHandlers[addr]->Poke(addr, data >> 8);
    };
    inline UWORD PeekW_CPU(ULONG addr)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr, false);
            mMemoryAccessCallback(mMemoryHandlers[addr]->GetBankType(), addr + 1, false);
        }
        return ((mMemoryHandlers[addr]->Peek(addr)) + (mMemoryHandlers[addr]->Peek(addr + 1) << 8));
    };

    //
    // RAM
    //
    inline void Poke_RAM(ULONG addr, UBYTE data)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_RAM, addr, true);
        }
        mRam->Poke(addr, data);
    };
    inline UBYTE Peek_RAM(ULONG addr)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_RAM, addr, false);
        }
        return mRam->Peek(addr);
    };
    inline void PokeW_RAM(ULONG addr, UWORD data)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_RAM, addr, true);
        }
        mRam->Poke(addr, data & 0xff);
        addr++;
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_RAM, addr, true);
        }
        mRam->Poke(addr, data >> 8);
    };
    inline UWORD PeekW_RAM(ULONG addr)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_RAM, addr, false);
            mMemoryAccessCallback(LynxMemBank_RAM, addr + 1, false);
        }
        return ((mRam->Peek(addr)) + (mRam->Peek(addr + 1) << 8));
    };

    // High level cart access for debug etc

    inline void Poke_CART(ULONG addr, UBYTE data)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_CART, addr, true);
        }
        mCart->Poke(addr, data);
    };
    inline UBYTE Peek_CART(ULONG addr)
    {
        if (mMemoryAccessCallback)
        {
            mMemoryAccessCallback(LynxMemBank_CART, addr, false);
        }
        return mCart->Peek(addr);
    };
    inline void CartBank(EMMODE bank)
    {
        mCart->BankSelect(bank);
    };
    inline ULONG CartSize(void)
    {
        return mCart->ObjectSize();
    };
    inline const char *CartGetName(void)
    {
        return mCart->CartGetName();
    };
    inline const char *CartGetManufacturer(void)
    {
        return mCart->CartGetManufacturer();
    };
    inline ULONG CartGetRotate(void)
    {
        return mCart->CartGetRotate();
    };

    // Low level cart access for Suzy, Mikey

    inline void Poke_CARTB0(UBYTE data)
    {
        mCart->Poke0(data);
    };
    inline void Poke_CARTB1(UBYTE data)
    {
        mCart->Poke1(data);
    };
    inline void Poke_CARTB0A(UBYTE data)
    {
        mCart->Poke0A(data);
    };
    inline void Poke_CARTB1A(UBYTE data)
    {
        mCart->Poke1A(data);
    };
    inline UBYTE Peek_CARTB0(void)
    {
        return mCart->Peek0();
    }
    inline UBYTE Peek_CARTB1(void)
    {
        return mCart->Peek1();
    }
    inline UBYTE Peek_CARTB0A(void)
    {
        return mCart->Peek0A();
    }
    inline UBYTE Peek_CARTB1A(void)
    {
        return mCart->Peek1A();
    }
    inline void CartAddressStrobe(bool strobe)
    {
        mCart->CartAddressStrobe(strobe);
    };
    inline void CartAddressData(bool data)
    {
        mCart->CartAddressData(data);
    };

    // Low level CPU access

    void SetRegs(C6502_REGS &regs)
    {
        mCpu->SetRegs(regs);
    };
    void GetRegs(C6502_REGS &regs)
    {
        mCpu->GetRegs(regs);
    };

    // Mikey system interfacing

    void DisplaySetAttributes(void (*DisplayCallback)(void *objref, uint8_t *ramPointer, uint8_t *palettePointer), void *objref)
    {
        mMikie->DisplaySetAttributes(DisplayCallback, objref);
    };

    void ComLynxCable(int status)
    {
        mMikie->ComLynxCable(status);
    };
    void ComLynxRxData(int data)
    {
        mMikie->ComLynxRxData(data);
    };
    void ComLynxTxCallback(void (*function)(int data, void *objref), void *objref)
    {
        mMikie->ComLynxTxCallback(function, objref);
    };

    // Suzy system interfacing

    ULONG PaintSprites(void)
    {
        return mSusie->PaintSprites();
    };

    // Miscellaneous

    void SetButtonData(LYNXBUTTONS data)
    {
        mSusie->SetButtonData(data);
    };
    LYNXBUTTONS GetButtonData(void)
    {
        return mSusie->GetButtonData();
    };
    void SetCycleBreakpoint(ULONG breakpoint)
    {
        mCycleCountBreakpoint = breakpoint;
    };
    UBYTE *GetRamPointer(void)
    {
        return mRam->GetRamPointer();
    };

#ifdef _LYNXDBG
    void DebugTrace(int address);

    void DebugSetCallback(void (*function)(ULONG objref, char *message), ULONG objref);

    void (*mpDebugCallback)(ULONG objref, char *message);
    ULONG mDebugCallbackObject;
#endif

  public:
    std::filesystem::path mGamefile;
    ULONG mCycleCountBreakpoint;
    CLynxBase *mMemoryHandlers[SYSTEM_SIZE];
    std::function<void(LynxMemBank, uint16_t, bool)> mMemoryAccessCallback{};
    CCart *mCart{};
    CRom *mRom{};
    CMemMap *mMemMap{};
    CRam *mRam{};
    C65C02 *mCpu{};
    CMikie *mMikie{};
    CSusie *mSusie{};
    CEEPROM *mEEPROM{};

    ULONG mFileType;
};

#endif
