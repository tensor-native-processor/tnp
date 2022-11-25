#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <cstddef>
#include <vector>

// Switch parameters
struct SwitchParam {
    size_t width = 16;
    size_t core_size = 2;
};

// Switch simulation engine
class SwitchSimEngine {
public:
    SwitchSimEngine(const SwitchParam&);

    // Switch simulateStep should be called after core simulateStep
    void simulateStep();

    // Interface with cores
    bool sendRequest(size_t self, size_t core_idx);
    bool recvRequest(size_t self, size_t core_idx);

private:
    SwitchParam m_param;

    std::vector<std::vector<bool>> m_transit_empty;
    std::vector<std::pair<size_t, size_t>> m_toggle_events;
};

#endif
