#pragma once

#include "3d_types.h"
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <intrin.h>

/** Compile everything out when using the PROFILING_ON flag */

#ifdef PROFILING_ON
typedef struct
{
	uint64 cycle_count;
	std::chrono::milliseconds elapsed_time;
	uint64 calls;
} debug_cycle_counter;

enum debug_counters
{
	debug_cycle_counters_FrameTime,
	debug_cycle_counters_FrameLoop,
	debug_cycle_counters_Input,
	debug_cycle_counters_Update,
	debug_cycle_counters_Render,
	debug_cycle_counters_BresenhamLine,
	debug_cycle_counters_RenderNormals,
	debug_cycle_counters_RenderTriangles,
};

std::map<debug_counters, debug_cycle_counter> counters;
std::map<debug_counters, std::string> counter_names = {
	{debug_cycle_counters_FrameTime, "Frame time"},
	{debug_cycle_counters_FrameLoop, "Frame loop"},
	{debug_cycle_counters_Input, "Input"},
	{debug_cycle_counters_Update, "Update"},
	{debug_cycle_counters_Render, "Render"},
	{debug_cycle_counters_BresenhamLine, "Bresenham line"},
	{debug_cycle_counters_RenderNormals, "Render normals"},
	{debug_cycle_counters_RenderTriangles, "Render triangles"},
};

#define BEGIN_TIMED_BLOCK(ID) \
	auto start_time##ID = std::chrono::high_resolution_clock::now(); \
	uint64_t start_cycle_count##ID = __rdtsc(); \
    counters[debug_cycle_counters_##ID].calls++;

#define END_TIMED_BLOCK(ID) \
	auto end_time##ID = std::chrono::high_resolution_clock::now(); \
	uint64_t end_cycle_count##ID = __rdtsc(); \
	counters[debug_cycle_counters_##ID].cycle_count += end_cycle_count##ID - start_cycle_count##ID; \
	counters[debug_cycle_counters_##ID].elapsed_time += std::chrono::duration_cast<std::chrono::milliseconds>(end_time##ID - start_time##ID);
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

void dump_cycle_counters()
{
#ifdef PROFILING_ON
	// move cursor to row 1, column 1
	std::cout << "\033[1;1H";
	for (const auto& counter : counters)
	{
		// use std::endl to flush the output
		std::cout << counter_names[counter.first] << ": "
			<< counter.second.cycle_count << " cycles ("
			<< counter.second.elapsed_time.count() << " ms)" << std::endl;
	}
	counters.clear();
#endif
}