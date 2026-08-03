#pragma once
#include "core/defs.hpp"

// Static Exchange Evaluation.
// Simulates the full capture sequence on a move's target square and
// returns the net material result (positive = good for the side moving).
int see(int move);

// Fast reject: is this capture's SEE >= threshold? Avoids running the
// full swap loop when you only need a yes/no (e.g. quiescence pruning).
bool seeGe(int move, int threshold);