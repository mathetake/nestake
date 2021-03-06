#include "gtest/gtest.h"
#include "cpu.cpp"

#include <iostream>

TEST(CPUTest, Initialization) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    // TODO: change `0` to appropriate one after implementing mapper
    EXPECT_EQ(0, cpu.PC);
    EXPECT_EQ(0xFD, cpu.SP);
    EXPECT_EQ(36, cpu.getFlag());
}


TEST(CPUTest, isPageCrossed) {
    bool actual = nestake::isPageCrossed(0, 1);
    EXPECT_EQ(false, actual);

    actual = nestake::isPageCrossed(0, 0x00FF);
    EXPECT_EQ(false, actual);

    actual = nestake::isPageCrossed(0, 0x0100);
    EXPECT_EQ(true, actual);
}

TEST(CPUTest, compare) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.compare(0x0000, 0x0000);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(1, cpu.Z);
    EXPECT_EQ(0, cpu.N);

    cpu.compare(0x0000, 0x0001);
    EXPECT_EQ(0, cpu.C);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(1, cpu.N);

}

TEST(CPUTest, READ16) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 0x02;
    mem->RAM[1] = 0x10;

    uint16_t actual = cpu.read16(0x00);
    EXPECT_EQ(0x1002, actual);
}


TEST(CPUTest, READ16Bug) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0x0000] = 0x02;
    mem->RAM[0x0100] = 0x33;

    // address = 0x0000, _address = 0x0001 <<8 = 0x0100
    uint16_t actual = cpu.read16Bug(0x0000);
    EXPECT_EQ(0x3302, actual);
}

TEST(CPUTest, PUSH) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 0x10;
    cpu.push(0x99);

    // 0x10 - 1 = 0x0F
    EXPECT_EQ(0x0F, cpu.SP);

    // address = (0x0100 | 0x0010) = 0x0110
    EXPECT_EQ(0x99, cpu.mem->RAM[0x0110]);
}

TEST(CPUTest, PULL) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 0x10;
    cpu.mem->RAM[0x0111] = 0x96;
    uint8_t actual = cpu.pull();

    EXPECT_EQ(0x11, cpu.SP);
    EXPECT_EQ(0x96, actual);
}

TEST(CPUTest, PULL16) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 0x10;
    cpu.mem->RAM[0x0111] = 0x96;
    cpu.mem->RAM[0x0112] = 0x98;
    uint16_t actual = cpu.pull16();

    EXPECT_EQ(0x12, cpu.SP);
    EXPECT_EQ(0x9896, actual);
}

TEST(CPUTest, GetFlag) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.C = 1;
    cpu.Z = 0;
    cpu.I = 0;
    cpu.D = 1;
    cpu.B = 0;
    cpu.U = 0;
    cpu.V = 0;
    cpu.N = 1;
    uint8_t actual = cpu.getFlag();
    EXPECT_EQ(0b10001001, actual);
}


TEST(CPUTest, SetFlags) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.setFlags(0b10001001);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(0, cpu.I);
    EXPECT_EQ(1, cpu.D);
    EXPECT_EQ(0, cpu.B);
    EXPECT_EQ(0, cpu.U);
    EXPECT_EQ(0, cpu.V);
    EXPECT_EQ(1, cpu.N);
}

TEST(CPUTest, ADC) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    // Neither overflow nor carry case
    cpu.A = 0;
    cpu.C = 0;
    mem->RAM[0x0000] = 1;
    cpu.ExecADC(0x0000, false);

    EXPECT_EQ(1, cpu.A);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(1, cpu.N);

    // NOT overflow BUT carry
    cpu.A = 1;
    cpu.C = 0;
    mem->RAM[0x0000] = 0xFF;
    cpu.ExecADC(0x0000, false);

    EXPECT_EQ(0x00, cpu.A);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0, cpu.V);

    // NOT carry BUT overflow
    cpu.A = 0b01000000;
    cpu.C = 0;
    mem->RAM[0x0000] = 0b01000000;
    cpu.ExecADC(0x0000, false);

    EXPECT_EQ(0b10000000, cpu.A);
    EXPECT_EQ(0, cpu.C);
    EXPECT_EQ(1, cpu.V);

    // overflow AND carry
    cpu.A = 0b10000000;
    cpu.C = 0;
    mem->RAM[0x0000] = 0b10000000;
    cpu.ExecADC(0x0000, false);

    EXPECT_EQ(0, cpu.A);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(1, cpu.V);
}

TEST(CPUTest, AND) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.A = 0b10000001;
    cpu.mem->RAM[0x0000] = 0b10000000;

    cpu.ExecAND(0x0000, false);
    EXPECT_EQ(0b10000000, cpu.A);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(1, cpu.N);
}

TEST(CPUTest, ASL) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    // accumulator mode
    cpu.A = 0b10000001;
    cpu.C = 0b00000000;

    cpu.ExecASL(0, true);
    EXPECT_EQ(0b00000010, cpu.A);
    EXPECT_EQ(1, cpu.C);

    // non accumulator mode
    mem->RAM[0] = 0b10000001;

    cpu.ExecASL(0, false);
    EXPECT_EQ(0b00000010, mem->RAM[0]);
    EXPECT_EQ(1, cpu.C);
}

TEST(CPUTest, BCC) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.C = 0;
    cpu.Cycles = 0;

    // page crossing
    cpu.ExecBCC(0x1000, false);
    EXPECT_EQ(2, cpu.Cycles);
    EXPECT_EQ(0x1000, cpu.PC);

    // not page crossing
    cpu.ExecBCC(0x1001, false);
    EXPECT_EQ(3, cpu.Cycles);
    EXPECT_EQ(0x1001, cpu.PC);
}

TEST(CPUTest, BIT) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.A = 0b00000001;
    mem->RAM[0] = 0b11000000;
    cpu.ExecBIT(0, false);
    EXPECT_EQ(1, cpu.Z);
    EXPECT_EQ(1, cpu.N);
    EXPECT_EQ(1, cpu.V);
}

TEST(CPUTest, CMP) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.A = 1;
    mem->RAM[0] = 0;
    cpu.ExecCMP(0, false);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(1, cpu.N);

}

TEST(CPUTest, DEC) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 1;

    cpu.ExecDEC(0, false);
    EXPECT_EQ(0, mem->RAM[0]);
    EXPECT_EQ(1, cpu.Z);
    EXPECT_EQ(0, cpu.N);
}

TEST(CPUTest, DEX) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.X = 100;

    cpu.ExecDEX(0, false);
    EXPECT_EQ(99, cpu.X);
    EXPECT_EQ(1, cpu.N);
    EXPECT_EQ(0, cpu.Z);
}

TEST(CPUTest, EOR) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 0b10000001;
    cpu.A   = 0b10001000;

    cpu.ExecEOR(0, false);
    EXPECT_EQ(0b00001001, cpu.A);
    EXPECT_EQ(1, cpu.N);
    EXPECT_EQ(0, cpu.Z);
}

TEST(CPUTest, INC) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 1;

    cpu.ExecINC(0, false);
    EXPECT_EQ(2, mem->RAM[0]);
    EXPECT_EQ(0, cpu.Z);
    EXPECT_EQ(1, cpu.N);
}

TEST(CPUTest, JSR) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.PC = 100;
    cpu.ExecJSR(0, false);
    EXPECT_EQ(0, cpu.PC);
}

TEST(CPUTest, LDA) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 100;

    cpu.ExecLDA(0, false);
    EXPECT_EQ(100, cpu.A);
}

TEST(CPUTest, LSR) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    //  accumulator mode
    cpu.A = 0b00000011;
    cpu.ExecLSR(0, true);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0b00000001, cpu.A);

    //  non accumulator mode
    mem->RAM[0] = 0b00000011;
    cpu.ExecLSR(0, false);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0b00000001, mem->RAM[0]);
}

TEST(CPUTest, ORA) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    mem->RAM[0] = 0b00000011;
    cpu.A = 0b10000000;


    cpu.ExecORA(0, false);
    EXPECT_EQ(0b10000011, cpu.A);
}

TEST(CPUTest, PHP) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.C = 1;
    cpu.Z = 0;
    cpu.I = 0;
    cpu.D = 1;
    cpu.B = 0;
    cpu.U = 0;
    cpu.V = 0;
    cpu.N = 1;
    EXPECT_EQ(0b10001001, cpu.getFlag());

    cpu.SP = 100;
    cpu.ExecPHP(0, false);
    EXPECT_EQ(0b10011001, mem->RAM[0x0100|100]);
    EXPECT_EQ(99, cpu.SP);
}

TEST(CPUTest, PLP) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 0b10000000;
    mem->RAM[0x100|0b10000001] = 0b00000010;

    cpu.ExecPLP(0, false);
    EXPECT_EQ(0b00100010, cpu.getFlag());
}

TEST(CPUTest, ROL) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    // accumulator mode
    cpu.A = 0b01111110;
    cpu.C = 1;
    cpu.ExecROL(0, true);
    EXPECT_EQ(0b11111101, cpu.A);

    // non accumulator mode
    mem->RAM[0] = 0b01111110;
    cpu.C = 1;
    cpu.ExecROL(0, false);
    EXPECT_EQ(0b11111101, mem->RAM[0]);
}

TEST(CPUTest, ROR) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    // accumulator mode
    cpu.A = 0b01111110;
    cpu.C = 1;
    cpu.ExecROR(0, true);
    EXPECT_EQ(0b10111111, cpu.A);

    // non accumulator mode
    mem->RAM[0] = 0b01111110;
    cpu.C = 1;
    cpu.ExecROR(0, false);
    EXPECT_EQ(0b10111111, mem->RAM[0]);
}

TEST(CPUTest, RTI) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 0;
    mem->RAM[0b00000001|0x100] = 0b00000011;
    cpu.ExecRTI(0, false);

    uint8_t actual = cpu.getFlag();
    EXPECT_EQ(0b00100011, actual);
}

TEST(CPUTest, RTS) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);
    cpu.SP = 10;
    mem->RAM[0x100|11] = 0x55;
    mem->RAM[0x100|12] = 0x99;
    cpu.ExecRTS(0, false);
    EXPECT_EQ(12, cpu.SP);
    EXPECT_EQ(0x9956, cpu.PC);
}

TEST(CPUTest, SBC) {
    std::shared_ptr<nestake::CPUMemory> mem(std::make_shared<nestake::CPUMemory>());
    nestake::Cpu cpu = nestake::Cpu(mem);

    // Neither overflow nor carry case
    cpu.A = 1;
    cpu.C = 1;
    mem->RAM[0x0000] = 1;
    cpu.ExecSBC(0x0000, false);

    EXPECT_EQ(0, cpu.A);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(0, cpu.V);

    // NOT overflow BUT carry
    cpu.A = 1;
    cpu.C = 1;
    mem->RAM[0x0000] = 2;
    cpu.ExecSBC(0x0000, false);

    EXPECT_EQ(0xFF, cpu.A);
    EXPECT_EQ(0, cpu.C);
    EXPECT_EQ(0, cpu.V);

    // NOT carry BUT overflow
    cpu.A = 0b10000000;
    cpu.C = 1;
    mem->RAM[0x0000] = 1;
    cpu.ExecSBC(0x0000, false);

    EXPECT_EQ(0b01111111, cpu.A);
    EXPECT_EQ(1, cpu.C);
    EXPECT_EQ(1, cpu.V);

    // overflow AND carry
    cpu.A = 0b10000000;
    cpu.C = 1;
    mem->RAM[0x0000] = 0b10000001;
    cpu.ExecSBC(0x0000, false);

    EXPECT_EQ(0xFF, cpu.A);
    EXPECT_EQ(0, cpu.C);
    EXPECT_EQ(0, cpu.V);
}