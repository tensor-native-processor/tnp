#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <cstddef>

// Switch parameters
struct SwitchParam {
    size_t width = 16;
    size_t core_size = 2;
};

// Switch simulation engine
class SwitchSimEngine {
public:
    SwitchSimEngine(const SwitchParam&);

private:
    SwitchParam m_param;
};

#endif
