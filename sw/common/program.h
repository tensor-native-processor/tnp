#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <vector>
#include <map>
#include <string>

// Program binary definition
typedef std::vector<std::byte> TNPProgramBinary;
void SaveProgram(const TNPProgramBinary&, const std::string&);
TNPProgramBinary LoadProgram(const std::string&);

// Core size definition
typedef unsigned long CoreValue_t;


// Single generic core instruction data
template<class CoreInstDefn>
class BaseCoreInst {
public:
    typename CoreInstDefn::Opcode opcode;
    std::map<typename CoreInstDefn::Operand, CoreValue_t> operands;
};


// Generic core program
template<class CoreInstDefn, class CoreInstSize>
class BaseCoreProgram {
public:
    typedef BaseCoreInst<CoreInstDefn> CoreInst;

    BaseCoreProgram(): m_isizes() {}
    BaseCoreProgram(const CoreInstSize& c): m_isizes(c) {}

    // Binary is in little-endian
    TNPProgramBinary toBinary() const;
    std::string toText() const;
    void fromBinary(const TNPProgramBinary&);
    void fromText(const std::string&);

    void append(const CoreInst&);
    size_t size() const;
    const CoreInst& operator[](size_t) const;

private:
    CoreInstSize m_isizes;
    std::vector<CoreInst> m_insts;

	static TNPProgramBinary encodeBinary(CoreValue_t, size_t);
	static CoreValue_t decodeBinary(const TNPProgramBinary&);
};


#include "program.inl"

#endif
