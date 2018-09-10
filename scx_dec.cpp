#include "scx_dec.hpp"

bool scx_dec::operator>(const scx_dec &other) {
	return score >= other.score;
}
