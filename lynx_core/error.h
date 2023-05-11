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
// Generic error handler class                                              //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides error handler facilities for the Lynx emulator, I    //
// shamelessly lifted most of the code from Stella by Brad Mott.            //
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

#ifndef ERROR_H
#define ERROR_H

#include <cstring>
#include <iostream>
#include <strstream>

#define MAX_ERROR_MSG 512
#define MAX_ERROR_DESC 2048

enum LynxErrors_
{
  LynxErrors_None = 0,
  LynxErrors_Illegal_Opcode = 1,
  LynxErrors_MAX
};

// class CLynxException : public CException
class CLynxException
{
  public:
    // Constructor
    CLynxException() : mError { LynxErrors_None }
    {
    }

    // Copy Constructor
    CLynxException(CLynxException &err)
    {
        int MsgCount, DescCount;

        mError = err.mError;

        MsgCount = (int)err.Message().pcount() + 1;
        DescCount = (int)err.Description().pcount() + 1;
        if (MsgCount > MAX_ERROR_MSG)
            MsgCount = MAX_ERROR_MSG;
        if (DescCount > MAX_ERROR_DESC)
            DescCount = MAX_ERROR_DESC;

        strncpy(mMsg, err.Message().str(), MsgCount);
        mMsg[MsgCount - 1] = '\0';
        strncpy(mDesc, err.Description().str(), DescCount);
        mDesc[DescCount - 1] = '\0';
    }

    // Destructor
    virtual ~CLynxException()
    {
        mMsgStream.rdbuf()->freeze(0);
        mDescStream.rdbuf()->freeze(0);
    }

  public:
    // Answer stream which should contain the one line error message
    std::ostrstream &Message()
    {
        return mMsgStream;
    }

    // Answer stream which should contain the multiple line description
    std::ostrstream &Description()
    {
        return mDescStream;
    }

    LynxErrors_ &Error()
    {
        return mError;
    }

  public:
    // Overload the assignment operator
    CLynxException &operator=(CLynxException &err)
    {
        mMsgStream.seekp(0);

        mMsgStream.write(err.Message().str(), err.Message().pcount());
        err.Message().rdbuf()->freeze(0);

        mDescStream.seekp(0);

        mDescStream.write(err.Description().str(), err.Description().pcount());
        err.Description().rdbuf()->freeze(0);

        return *this;
    }

    // Overload the I/O output operator
    friend std::ostream &operator<<(std::ostream &out, CLynxException &err)
    {
        out.write(err.Message().str(), err.Message().pcount());
        err.Message().rdbuf()->freeze(0);

        if (err.Description().pcount() != 0)
        {
            out << std::endl << std::endl;

            out.write(err.Description().str(), err.Description().pcount());
            err.Description().rdbuf()->freeze(0);
        }

        return out;
    }

  private:
    // Contains the one line error code message
    std::ostrstream mMsgStream;

    // Contains a multiple line description of the error and ways to
    // solve the problem
    std::ostrstream mDescStream;
    LynxErrors_ mError;

  public:
    // CStrings to hold the data after its been thrown

    char mMsg[MAX_ERROR_MSG];
    char mDesc[MAX_ERROR_DESC];

};
#endif
