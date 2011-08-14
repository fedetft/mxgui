/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifndef BENCHMARK_H
#define	BENCHMARK_H

#ifdef _MIOSIX

#include "mxgui/display.h"
#include "miosix.h"

/**
 * The result of a benchmark
 */
class BenchmarkResult
{
public:

    BenchmarkResult(): time(0) { name[0]='\0'; }

    /**
     * \param name name of benchmark
     * \param time time (in microseconds) taken to do the benchmark
     */
    BenchmarkResult(const char str[20], unsigned int time);

    /**
     * \return the benchmark name
     */
    const char *getName() const { return name; }

    /**
     * \return the benchmark time, in microseconds
     */
    unsigned int getTime() const { return time; }

    /**
     * \return number of fps(multiplied by 100, so that the last
     * two digits are fractional fps values).
     */
    unsigned int getFps() const { return time==0 ? 99999 : (1000000*100)/time; }

    /**
     * Print the result on the display d, at point p
     */
    void print(mxgui::DrawingContext& dc, mxgui::Point p);

private:
    char name[20];
    unsigned int time;
};

/**
 * Benchmark code is here. Benchmark is designed for a 240x320 screen,
 * orientation vertical
 */
class Benchmark
{
public:
    /**
     * \param display the display that will be benchmarked
     */
    Benchmark(mxgui::Display& display);

    /**
     * Starts the benchmark.
     * At the end result are directly printed on screen, but it is possible
     * to get them with getResults()
     */
    void start();

private:
    void fixedWidthTextBenchmark();

    void variableWidthTextBenchmark();

    void antialiasingBenchmark();

    void horizontalLineBenchmark();

    void verticalLineBenchmark();

    void obliqueLineBenchmark();

    void clearScreenBenchmark();
	
    void imageBenchmark();

    void scanLineBenchmark();

    void clippedDrawBenchmark();

    void clippedWriteBenchmark();

    void resourceImageBenchmark();

    static const unsigned int numBenchmarks=12;
    mxgui::Display& display;
    BenchmarkResult results[numBenchmarks];
    miosix::Timer timer;
};

#endif //_MIOSIX

#endif	/* BENCHMARK_H */
