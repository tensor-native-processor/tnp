#include "switch.h"

// Init switch sim engine
SwitchSimEngine::SwitchSimEngine(const SwitchParam& param)
: m_param(param),
  m_transit_empty(param.core_size,
                  std::vector<bool>(param.core_size, true)),
  m_toggle_events()
{}

// Simulate step
void SwitchSimEngine::simulateStep() {
    for (auto const& [s, r] : m_toggle_events) {
        m_transit_empty[s][r] = !m_transit_empty[s][r];
    }
    m_toggle_events.clear();
}

// Send and receive data
bool SwitchSimEngine::sendRequest(size_t self, size_t core_idx) {
    if (m_transit_empty[self][core_idx]) {
        m_toggle_events.push_back(std::make_pair(self, core_idx));
        return true;
    } else {
        return false;
    }
}

bool SwitchSimEngine::recvRequest(size_t self, size_t core_idx) {
    if (!m_transit_empty[core_idx][self]) {
        m_toggle_events.push_back(std::make_pair(core_idx, self));
        return true;
    } else {
        return false;
    }
}
