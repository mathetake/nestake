#ifndef NESTAKE_CPU
#define NESTAKE_CPU

#include <array>
#include <map>
#include "memory.hpp"

namespace nestake {
    class Cpu {
    public:
        // debug flag
        bool IsDebugMode;

        // pointer to NesMemory interface
        NesMemory* Memory;

        // cpu cycles
        uint64_t Cycles;

        // program counter
        uint16_t PC;

        // stack pointer
        uint8_t SP;

        // accumulator
        uint8_t A;

        // x register
        uint8_t X;

        // y register
        uint8_t Y;

        // c register
        uint8_t C;

        // zero flag
        uint8_t Z;

        // interrupt disable flag
        uint8_t I;

        // decimal mode flag
        uint8_t D;

        //break command flag
        uint8_t B;

        // unused flag
        uint8_t U;

        // overflow flag
        uint8_t V;

        // negative flag
        uint8_t N;

        // interrupt type to execute
        uint8_t interrupt;

        // number of cycles to stall
        int stall;

        // core method for executing instructions
        uint64_t Step();

        // interruption related methods
        void TriggerIRQ();
        void TriggerNMI();
        void irq();
        void nmi();

        // memory push related
        void push(uint8_t);
        void push16(uint16_t);

        // memory pull related
        uint8_t pull();
        uint16_t pull16();

        // memory related
        uint16_t read16(uint16_t);
        uint16_t read16Bug(uint16_t);

        // flag related
        void setZ(uint8_t v);
        void setN(uint8_t v);
        void setZN(uint8_t v);
        void compare(uint8_t a, uint8_t b);
        void setFlags(uint8_t flag);
    };

    // interface of instruction executor
    typedef void (*InstructionExecutor) (Cpu* cpu, uint16_t address, bool is_accumulator);

    // struct consisting of information for instruction execution
    struct InstructionParams {
        // instruction's id
        uint8_t ID;

        // addressing mode
        uint8_t AddressingMode;

        // the size of each instruction in byte
        uint8_t InstructionSizes;

        // the number of cycles used by the instruction
        uint8_t InstructionCycle;

        // the number of page cycles used by the instruction
        uint8_t PageCycle;

        InstructionExecutor executor;
    };
}
#endif
