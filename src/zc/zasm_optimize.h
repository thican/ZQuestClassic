#ifndef ZASM_OPTIMIZE_H_
#define ZASM_OPTIMIZE_H_

#include "base/zdefs.h"

#include <map>
#include <string>

struct OptimizationPass
{
	std::string name;
	uint32_t instructions_saved;
	// microseconds
	uint32_t elapsed;
};

struct OptimizeResults
{
	uint32_t instructions_saved;
	// microseconds
	uint32_t elapsed;
	std::vector<OptimizationPass> passes;
};

bool zasm_optimize_enabled();
OptimizeResults zasm_optimize(zasm_script* script);
void zasm_optimize_and_log(zasm_script* script);
OptimizeResults zasm_optimize();
bool zasm_optimize_test();
void zasm_optimize_run_for_file(std::string path);

#endif
