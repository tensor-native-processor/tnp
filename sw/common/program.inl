#include <sstream>

// Convert MatProgram to/from text
template<class CoreInstDefn, class CoreInstSize>
std::string BaseCoreProgram<CoreInstDefn, CoreInstSize>::toText() const {
    std::ostringstream oss;
    for (auto const &inst : m_insts) {
        oss << CoreInstDefn::getOpcodeName(inst.opcode) << " ";
        auto operands = CoreInstDefn::getOpcodeOperands(inst.opcode);

        for (auto const& opr : operands) {
            oss << inst.operands.at(opr) << " ";
        }
        oss << "\n";
    }
    return oss.str();
}

template<class CoreInstDefn, class CoreInstSize>
void BaseCoreProgram<CoreInstDefn, CoreInstSize>::fromText(const std::string& str) {
    std::istringstream iss(str);
    // Clear existing program
    m_insts.clear();
    CoreInst inst;
    std::string opName;
    while (iss >> opName) {
        inst.opcode = CoreInstDefn::findOpcodeByName(opName);
        auto operands = CoreInstDefn::getOpcodeOperands(inst.opcode);

        for (auto const& opr : operands) {
            iss >> inst.operands.at(opr);
        }
        m_insts.push_back(inst);
    }
}

// Convert BaseCoreProgram to/from binary
template<class CoreInstDefn, class CoreInstSize>
TNPProgramBinary BaseCoreProgram<CoreInstDefn, CoreInstSize>::toBinary() const {
    TNPProgramBinary bin;
    for (auto const &inst : m_insts) {
        auto opcBin = encodeBinary(inst.opcode, m_isizes[CoreInstDefn::OPCODE_TYPE]);
        bin.insert(bin.end(), opcBin.begin(), opcBin.end());

        auto operands = CoreInstDefn::getOpcodeOperands(inst.opcode);
        for (auto const& opr : operands) {
            auto oprBin = encodeBinary(inst.operands.at(opr), m_isizes[CoreInstDefn::getOperandType(opr)]);
            bin.insert(bin.end(), oprBin.begin(), oprBin.end());
        }
    }
    return bin;
}

template<class CoreInstDefn, class CoreInstSize>
void BaseCoreProgram<CoreInstDefn, CoreInstSize>::fromBinary(const TNPProgramBinary& bin) {
    // Clear existing program
    m_insts.clear();
    CoreInst inst;

    auto iter = bin.begin();
    while (iter != bin.end()) {
        size_t size;

        // Fetch opcode
        size = m_isizes[CoreInstDefn::OPCODE_TYPE];
        inst.opcode = (typename CoreInstDefn::Opcode)decodeBinary(TNPProgramBinary(iter, iter + size));
        iter += size;

        // Fetch operands
        auto operands = CoreInstDefn::getOpcodeOperands(inst.opcode);
        for (auto const& opr : operands) {
            size = m_isizes[CoreInstDefn::getOperandType(opr)];
            inst.operands[opr] = decodeBinary(TNPProgramBinary(iter, iter + size));
            iter += size;
        }
        m_insts.push_back(inst);
    }
}

// Append one instruction to mat program
template<class CoreInstDefn, class CoreInstSize>
void BaseCoreProgram<CoreInstDefn, CoreInstSize>::append(const CoreInst& inst) {
    m_insts.push_back(inst);
}

template<class CoreInstDefn, class CoreInstSize>
std::vector<BaseCoreInst<CoreInstDefn>> BaseCoreProgram<CoreInstDefn, CoreInstSize>::getInsts() {
    return m_insts;
}

// Encode and decode binary content
template<class CoreInstDefn, class CoreInstSize>
TNPProgramBinary BaseCoreProgram<CoreInstDefn, CoreInstSize>::encodeBinary(CoreValue_t value, size_t size) {
	TNPProgramBinary binary;
	for (size_t i = 0;i < size;i++) {
        std::byte b = std::byte(value & 0xFF);
		binary.push_back(b);
		value >>= 8;
	}
	return binary;
}

template<class CoreInstDefn, class CoreInstSize>
CoreValue_t BaseCoreProgram<CoreInstDefn, CoreInstSize>::decodeBinary(const TNPProgramBinary& binary) {
	CoreValue_t value = 0;
	for (size_t i = 0;i < binary.size();i++) {
		CoreValue_t byte = (CoreValue_t)(binary[i]);
		value |= byte << 8 * i;
	}
	return value;
}

// Fetch instruction from program
template<class CoreInstDefn, class CoreInstSize>
size_t BaseCoreProgram<CoreInstDefn, CoreInstSize>::size() const {
    return m_insts.size();
}

template<class CoreInstDefn, class CoreInstSize>
const typename BaseCoreProgram<CoreInstDefn, CoreInstSize>::CoreInst&
BaseCoreProgram<CoreInstDefn, CoreInstSize>::operator[](size_t index) const {
    return m_insts[index];
}
