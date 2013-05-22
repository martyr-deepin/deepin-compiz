/*
 * Compiz Core: OutputDevices class
 *
 * Copyright (c) 2012 Canonical Ltd.
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

#include <core/output.h>
#include <core/rect.h>
#include <core/region.h>
#include <core/screen.h>

namespace compiz
{
namespace core
{

class OutputDevices
{
public:
    OutputDevices();

    void setCurrentOutput(unsigned int outputNum);

    CompOutput& getCurrentOutputDev() { return outputDevs[currentOutputDev]; }

    bool hasOverlappingOutputs() const { return overlappingOutputs; }

    void computeWorkAreas(CompRect& workArea, bool& workAreaChanged,
	    CompRegion& allWorkArea, const CompWindowList& windows);

    const CompOutput& getOutputDev(unsigned int outputNum) const
    { return outputDevs[outputNum]; }

    // TODO breaks encapsulation horribly ought to be const at least
    // Even better, use begin() and end() return const_iterators
    // BUT this is exported directly through API - which makes changing
    // it a PITA.
    CompOutput::vector& getOutputDevs() { return outputDevs; }

    int outputDeviceForGeometry(const CompWindow::Geometry& gm, int strategy,
	    CompSize* screen) const;
    void updateOutputDevices(CoreOptions& coreOptions, CompSize* screen);

    void setGeometryOnDevice(unsigned int nOutput, int x, int y,
	    const int width, const int height);
    void adoptDevices(unsigned int nOutput, CompSize* screen);

private:
    static CompRect computeWorkareaForBox(const CompRect& box,
	    const CompWindowList& windows);

    CompOutput::vector outputDevs;
    bool               overlappingOutputs;
    int	           currentOutputDev;
};

} // namespace core
} // namespace compiz
