/*
 * Compiz opengl plugin, Backlist feature
 *
 * Copyright (c) 2012 Canonical Ltd.
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "gtest/gtest.h"
#include "blacklist.h"

using namespace compiz::opengl;

static const char *recommendedRegex = "(nouveau|Intel).*Mesa 8.0";

TEST (DriverBlacklist, QuantalIntelIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "Intel Open Source Technology Center",
                               "Mesa DRI Intel(R) Sandybridge Desktop",
                               "3.0 Mesa 9.0"));
}

TEST (DriverBlacklist, PreciseIntelIsBad)
{
    EXPECT_TRUE  (blacklisted (recommendedRegex,
                               "Tungsten Graphics, Inc",
                               "Mesa DRI Intel(R) Sandybridge Desktop",
                               "3.0 Mesa 8.0.2"));
}

TEST (DriverBlacklist, QuantalNouveauIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "nouveau",
                               "Gallium 0.4 on NV86",
                               "3.0 Mesa 9.0-devel"));
}

TEST (DriverBlacklist, PreciseNouveauIsBad)
{
    EXPECT_TRUE  (blacklisted (recommendedRegex,
                               "nouveau",
                               "Gallium 0.4 on NVA8",
                               "2.1 Mesa 8.0.2"));
}

TEST (DriverBlacklist, FglrxIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "Advanced Micro Devices, Inc.",
                               "ATI Radeon HD 5450",
                               "4.2.11627 Compatibility Profile Context"));
}

TEST (DriverBlacklist, NvidiaIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "NVIDIA Corporation",
                               "Quadro 1000M/PCIe/SSE2",
                               "4.2.0 NVIDIA 304.48"));
}

TEST (DriverBlacklist, RadeonIsGood1)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "X.Org R300 Project",
                               "Gallium 0.4 on ATI RV350",
                               "2.1 Mesa 8.0.2"));
}

TEST (DriverBlacklist, RadeonIsGood2)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "X.Org",
                               "Gallium 0.4 on AMD CEDAR",
                               "2.1 Mesa 8.0.3"));
}

TEST (DriverBlacklist, RadeonIsGood3)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "X.Org",
                               "Gallium 0.4 on AMD RS880",
                               "2.1 Mesa 8.0.2"));
}

TEST (DriverBlacklist, LLVMpipeIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "VMware, Inc.",
                               "Gallium 0.4 on llvmpipe (LLVM 0x300)",
                               "2.1 Mesa 8.0.4"));
}

TEST (DriverBlacklist, UnknownIsGood)
{
    EXPECT_FALSE (blacklisted (recommendedRegex,
                               "Acme",
                               "Graphics Driver",
                               "4.2 8.0 9.0 123.456"));
}

TEST (DriverBlacklist, NoBlacklist)
{
    EXPECT_FALSE (blacklisted ("",
                               "Tungsten Graphics, Inc",
                               "Mesa DRI Intel(R) Sandybridge Desktop",
                               "3.0 Mesa 8.0.2"));
    EXPECT_FALSE (blacklisted ("", "foo", "bar", "blah"));
    EXPECT_FALSE (blacklisted ("", "", "", ""));
}

TEST (DriverBlacklist, LineContinuation)
{
    EXPECT_FALSE (blacklisted ("alpha",       "beta", "gamma", "delta"));
    EXPECT_FALSE (blacklisted ("betagam",     "beta", "gamma", "delta"));
    EXPECT_TRUE  (blacklisted ("gamma",       "beta", "gamma", "delta"));
    EXPECT_TRUE  (blacklisted ("del",         "beta", "gamma", "delta"));
    EXPECT_TRUE  (blacklisted ("(mag|gam)",   "beta", "gamma", "delta"));
    EXPECT_TRUE  (blacklisted ("beta.*delt",  "beta", "gamma", "delta"));
    EXPECT_FALSE (blacklisted ("beta.*felt",  "beta", "gamma", "delta"));

    EXPECT_TRUE  (blacklisted ("beta\ngamma\ndelta", "beta", "gamma", "delta"));
}

TEST (DriverBlacklist, StraySpaces)
{
    EXPECT_FALSE (blacklisted (" ", "Hello world", "and", "goodbye"));
    EXPECT_FALSE (blacklisted ("  ", " ", "  ", "    "));
    EXPECT_FALSE (blacklisted (" ",
                               "Tungsten Graphics, Inc",
                               "Mesa DRI Intel(R) Sandybridge Desktop",
                               "3.0 Mesa 8.0.2"));
}
