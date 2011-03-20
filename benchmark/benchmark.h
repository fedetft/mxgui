
#ifndef BENCHMARK_H
#define	BENCHMARK_H

#ifdef _MIOSIX

#include <vector>
#include "mxgui/display.h"
#include "miosix.h"


/**
 * The result of a benchmark
 */
class BenchmarkResult
{
public:
    /**
     * \param name name of benchmark
     * \param time time (in microseconds) taken to do the benchmark
     * \param fps number of fps resulting
     */
    BenchmarkResult(const char str[25], unsigned int time, double fps);

    /**
     * \return the benchmark name
     */
    const char *getName() const;

    /**
     * \return the benchmark time, in microseconds
     */
    unsigned int getTime() const;

    /**
     * \return the benchmark resulting fps
     */
    unsigned int getFps() const;

    /**
     * Print the result on the display d, at point p
     */
    void print(mxgui::Display& d, mxgui::Point p);

private:
    char name[25];
    unsigned int time;
    double fps;
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

    mxgui::Display& display;
    std::vector<BenchmarkResult> results;
    miosix::Timer timer;
};

#endif //_MIOSIX

#endif	/* BENCHMARK_H */
