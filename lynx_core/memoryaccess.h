
#pragma once
#include "machine.h"

class IMemoryAccess
{
  public:
    IMemoryAccess()
    {
    }
    virtual ~IMemoryAccess()
    {
    }

    virtual void Poke(ULONG addr, UBYTE data) = 0;
    virtual UBYTE Peek(ULONG addr) = 0;
};