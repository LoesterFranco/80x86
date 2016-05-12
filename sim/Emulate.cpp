#include "Emulate.h"

#include <cassert>
#include <functional>
#include <stdint.h>
#include "Fifo.h"
#include "Memory.h"

class EmulatorPimpl {
public:
    EmulatorPimpl(RegisterFile *registers);

    size_t emulate();

    void set_instruction_stream(Fifo<uint8_t> *instr_stream)
    {
        this->instr_stream = instr_stream;
    }

    void set_memory(Memory *mem)
    {
        this->mem = mem;
    }

    void set_io(Memory *io)
    {
        this->io = io;
    }
private:
    uint8_t fetch_byte();
    //
    // mov
    //
    void mov88();
    void mov89();
    void mov8a();
    void mov8b();
    void movc6();
    void movc7();
    void movb0_b7();
    void movb8_bf();
    void mova0();
    void mova1();
    void mova2();
    void mova3();
    void mov8e();
    void mov8c();
    //
    // push
    //
    void pushff();
    void push50_57();
    void pushsr();
    //
    // pop
    //
    void popf8();
    void pop58_5f();
    void popsr();
    //
    // xchg
    //
    void xchg86();
    void xchg87();
    void xchg90_97();
    //
    // in
    //
    void ine4();
    void ine5();
    void inec();
    void ined();
    //
    // out
    //
    void oute6();
    void oute7();
    void outee();
    void outef();
    //
    // xlat
    //
    void xlatd7();
    //
    // lea
    //
    void lea8d();
    //
    // lds
    //
    void ldsc5();
    //
    // les
    //
    void lesc4();
    //
    // lahf
    //
    void lahf9f();
    //
    // sahf
    //
    void sahf9e();
    //
    // pushf
    //
    void pushf9c();
    //
    // popf
    //
    void popf9d();
    //
    // add / adc
    //
    void add_adc_sub_sbb_80();
    void add_adc_sub_sbb_81();
    void add_adc_sub_sbb_82();
    void add_adc_sub_sbb_83();
    //
    // add
    //
    template <typename T>
    std::pair<uint16_t, T> do_add(uint16_t v1, uint16_t v2,
                                  uint16_t carry_in=0);
    void add00();
    void add01();
    void add02();
    void add03();
    void add04();
    void add05();
    //
    // adc
    //
    void adc10();
    void adc11();
    void adc12();
    void adc13();
    void adc14();
    void adc15();
    //
    // sub
    //
    template <typename T>
    std::pair<uint16_t, T> do_sub(uint16_t v1, uint16_t v2,
                                  uint16_t carry_in=0);
    void sub28();
    void sub29();
    void sub2a();
    void sub2b();
    void sub2c();
    void sub2d();
    //
    //sbb
    //
    void sbb18();
    void sbb19();
    void sbb1a();
    void sbb1b();
    void sbb1c();
    void sbb1d();
    //
    // inc
    //
    void incfe();
    void incff();
    void inc40_47();
    // Helpers
    void push_inc_ff();
    template <typename T>
    std::pair<uint16_t, T> do_alu(uint16_t v1, uint16_t v2,
                                  uint16_t carry_in,
                                  std::function<uint32_t(uint32_t, uint32_t, uint32_t)> alu_op);
    template <typename T>
        void write_data(T val, bool stack=false);
    template <typename T>
        T read_data(bool stack=false);
    uint16_t fetch_16bit();

    Fifo<uint8_t> *instr_stream;
    Memory *mem;
    Memory *io;
    RegisterFile *registers;
    size_t instr_length = 0;
    std::unique_ptr<ModRMDecoder> modrm_decoder;
    uint8_t opcode;
};

EmulatorPimpl::EmulatorPimpl(RegisterFile *registers)
    : registers(registers)
{
    modrm_decoder = std::make_unique<ModRMDecoder>(
        [&]{ return this->fetch_byte(); },
        this->registers
    );
}

size_t EmulatorPimpl::emulate()
{
    instr_length = 0;

    opcode = fetch_byte();
    switch (opcode) {
    // mov
    case 0x88: mov88(); break;
    case 0x89: mov89(); break;
    case 0x8a: mov8a(); break;
    case 0x8b: mov8b(); break;
    case 0xc6: movc6(); break;
    case 0xc7: movc7(); break;
    case 0xb0 ... 0xb7: movb0_b7(); break;
    case 0xb8 ... 0xbf: movb8_bf(); break;
    case 0xa0: mova0(); break;
    case 0xa1: mova1(); break;
    case 0xa2: mova2(); break;
    case 0xa3: mova3(); break;
    case 0x8e: mov8e(); break;
    case 0x8c: mov8c(); break;
    // push
    case 0xff: push_inc_ff(); break;
    case 0x50 ... 0x57: push50_57(); break;
    case 0x06: // fallthrough
    case 0x0e: // fallthrough
    case 0x16: // fallthrough
    case 0x1e: pushsr(); break;
    // pop
    case 0xf8: popf8(); break;
    case 0x58 ... 0x5f: pop58_5f(); break;
    case 0x07: // fallthrough
    case 0x0f: // fallthrough
    case 0x17: // fallthrough
    case 0x1f: popsr(); break;
    // xchg
    case 0x86: xchg86(); break;
    case 0x87: xchg87(); break;
    case 0x90 ... 0x97: xchg90_97(); break;
    // in
    case 0xe4: ine4(); break;
    case 0xe5: ine5(); break;
    case 0xec: inec(); break;
    case 0xed: ined(); break;
    // out
    case 0xe6: oute6(); break;
    case 0xe7: oute7(); break;
    case 0xee: outee(); break;
    case 0xef: outef(); break;
    // xlat
    case 0xd7: xlatd7(); break;
    // lea
    case 0x8d: lea8d(); break;
    // lds
    case 0xc5: ldsc5(); break;
    // les
    case 0xc4: lesc4(); break;
    // lahf
    case 0x9f: lahf9f(); break;
    // sahf
    case 0x9e: sahf9e(); break;
    // pushf
    case 0x9c: pushf9c(); break;
    // popf
    case 0x9d: popf9d(); break;
    // add / adc / sub / sbb
    case 0x80: add_adc_sub_sbb_80(); break;
    case 0x81: add_adc_sub_sbb_81(); break;
    case 0x82: add_adc_sub_sbb_82(); break;
    case 0x83: add_adc_sub_sbb_83(); break;
    // add
    case 0x00: add00(); break;
    case 0x01: add01(); break;
    case 0x02: add02(); break;
    case 0x03: add03(); break;
    case 0x04: add04(); break;
    case 0x05: add05(); break;
    // adc
    case 0x10: adc10(); break;
    case 0x11: adc11(); break;
    case 0x12: adc12(); break;
    case 0x13: adc13(); break;
    case 0x14: adc14(); break;
    case 0x15: adc15(); break;
    // sub
    case 0x28: sub28(); break;
    case 0x29: sub29(); break;
    case 0x2a: sub2a(); break;
    case 0x2b: sub2b(); break;
    case 0x2c: sub2c(); break;
    case 0x2d: sub2d(); break;
    // sbb
    case 0x18: sbb18(); break;
    case 0x19: sbb19(); break;
    case 0x1a: sbb1a(); break;
    case 0x1b: sbb1b(); break;
    case 0x1c: sbb1c(); break;
    case 0x1d: sbb1d(); break;
    // inc
    case 0xfe: incfe(); break;
    case 0x40 ... 0x47: inc40_47(); break;
    }

    return instr_length;
}

// mov m/r, r (8-bit)
void EmulatorPimpl::mov88()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    auto source = modrm_decoder->reg();
    auto val = registers->get(source);

    write_data<uint8_t>(val);
}

// mov m/r, r (16-bit)
void EmulatorPimpl::mov89()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    auto source = modrm_decoder->reg();
    auto val = registers->get(source);

    write_data<uint16_t>(val);
}

// mov r, m/r (8-bit)
void EmulatorPimpl::mov8a()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t val = read_data<uint8_t>();

    auto dest = modrm_decoder->reg();
    registers->set(dest, val);
}

// mov r, m/r (16-bit)
void EmulatorPimpl::mov8b()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t val = read_data<uint16_t>();

    auto dest = modrm_decoder->reg();
    registers->set(dest, val);
}

// mov r/m, immediate (reg == 0), 8-bit
void EmulatorPimpl::movc6()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0) {
        uint8_t immed = fetch_byte();
        write_data<uint8_t>(immed);
    }
}

// mov r/m, immediate (reg == 0), 16-bit
void EmulatorPimpl::movc7()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0)
        write_data<uint16_t>(fetch_16bit());
}

// mov r, immediate, 8-bit
void EmulatorPimpl::movb0_b7()
{
    uint8_t immed = fetch_byte();
    auto reg = static_cast<GPR>(static_cast<int>(AL) + (opcode & 0x7));
    registers->set(reg, immed);
}

// mov r, immediate, 16-bit
void EmulatorPimpl::movb8_bf()
{
    uint16_t immed = fetch_16bit();
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    registers->set(reg, immed);
}

// mov al, m, 8-bit
void EmulatorPimpl::mova0()
{
    auto displacement = fetch_16bit();
    auto addr = get_phys_addr(registers->get(DS), displacement);
    auto val = mem->read<uint8_t>(addr);
    registers->set(AL, val);
}

// mov ax, m, 16-bit
void EmulatorPimpl::mova1()
{
    auto displacement = fetch_16bit();
    auto addr = get_phys_addr(registers->get(DS), displacement);
    auto val = mem->read<uint16_t>(addr);
    registers->set(AX, val);
}

// mov m, al 8-bit
void EmulatorPimpl::mova2()
{
    auto displacement = fetch_16bit();
    auto val = registers->get(AL);
    auto addr = get_phys_addr(registers->get(DS), displacement);
    mem->write<uint8_t>(addr, val);
}

// mov m, al 16-bit
void EmulatorPimpl::mova3()
{
    auto displacement = fetch_16bit();
    auto val = registers->get(AX);
    auto addr = get_phys_addr(registers->get(DS), displacement);
    mem->write<uint16_t>(addr, val);
}

// mov sr, r/m
void EmulatorPimpl::mov8e()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() & (1 << 2))
        return;

    uint16_t val = read_data<uint16_t>();
    auto segnum = modrm_decoder->raw_reg();
    auto reg = static_cast<GPR>(static_cast<int>(ES) + segnum);

    registers->set(reg, val);
}

// mov r/m, sr
void EmulatorPimpl::mov8c()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() & (1 << 2))
        return;

    auto segnum = modrm_decoder->raw_reg();
    auto reg = static_cast<GPR>(static_cast<int>(ES) + segnum);
    uint16_t val = registers->get(reg);

    write_data<uint16_t>(val);
}

void EmulatorPimpl::push_inc_ff()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 6)
        pushff();
    else if (modrm_decoder->raw_reg() == 0)
        incff();
}

// push r/m
void EmulatorPimpl::pushff()
{
    assert(modrm_decoder->raw_reg() == 6);

    auto val = read_data<uint16_t>();
    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// push r
void EmulatorPimpl::push50_57()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    auto val = registers->get(reg);

    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// push sr
void EmulatorPimpl::pushsr()
{
    auto reg = static_cast<GPR>(static_cast<int>(ES) + ((opcode >> 3) & 0x3));
    auto val = registers->get(reg);

    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// pop r/m
void EmulatorPimpl::popf8()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0)
        return;

    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);
    registers->set(SP, registers->get(SP) + 2);

    write_data<uint16_t>(val);
}

// pop r
void EmulatorPimpl::pop58_5f()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);

    registers->set(SP, registers->get(SP) + 2);
    registers->set(reg, val);
}

// pop sr
void EmulatorPimpl::popsr()
{
    auto reg = static_cast<GPR>(static_cast<int>(ES) + ((opcode >> 3) & 0x3));
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);

    registers->set(reg, val);
    registers->set(SP, registers->get(SP) + 2);
}

// xchg r, r/m, 8-bit
void EmulatorPimpl::xchg86()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    auto v1 = read_data<uint8_t>();
    auto v2 = registers->get(modrm_decoder->reg());

    write_data<uint8_t>(v2);
    registers->set(modrm_decoder->reg(), v1);
}

// xchg r, r/m, 16-bit
void EmulatorPimpl::xchg87()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    auto v1 = read_data<uint16_t>();
    auto v2 = registers->get(modrm_decoder->reg());

    write_data<uint16_t>(v2);
    registers->set(modrm_decoder->reg(), v1);
}

// xchg accumulator, r
void EmulatorPimpl::xchg90_97()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));

    auto v1 = registers->get(AX);
    auto v2 = registers->get(reg);

    registers->set(AX, v2);
    registers->set(reg, v1);
}

// in al, data8
void EmulatorPimpl::ine4()
{
    auto port_num = fetch_byte();

    registers->set(AL, io->read<uint8_t>(port_num));
}

// in ax, data8
void EmulatorPimpl::ine5()
{
    auto port_num = fetch_byte();

    registers->set(AX, io->read<uint16_t>(port_num));
}

// in al, dx
void EmulatorPimpl::inec()
{
    registers->set(AL, io->read<uint8_t>(registers->get(DX)));
}

// in ax, dx
void EmulatorPimpl::ined()
{
    registers->set(AX, io->read<uint16_t>(registers->get(DX)));
}

// out data8, al
void EmulatorPimpl::oute6()
{
    auto port_num = fetch_byte();

    io->write<uint8_t>(port_num, registers->get(AL));
}

// out data8, ax
void EmulatorPimpl::oute7()
{
    auto port_num = fetch_byte();

    io->write<uint16_t>(port_num, registers->get(AX));
}

// out dx, al
void EmulatorPimpl::outee()
{
    io->write<uint8_t>(registers->get(DX), registers->get(AL));
}

// out dx, ax
void EmulatorPimpl::outef()
{
    io->write<uint16_t>(registers->get(DX), registers->get(AX));
}

// xlat
void EmulatorPimpl::xlatd7()
{
    auto v = registers->get(AL);
    auto table_addr = registers->get(BX);
    auto xlated_val = mem->read<uint8_t>(get_phys_addr(registers->get(DS),
                                                       table_addr + v));
    registers->set(AL, xlated_val);
}

// lea r, r/m
void EmulatorPimpl::lea8d()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    registers->set(modrm_decoder->reg(), modrm_decoder->effective_address());
}

// lds r, m
void EmulatorPimpl::ldsc5()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->rm_type() == OP_REG)
        return;

    auto p32 = get_phys_addr(registers->get(DS), modrm_decoder->effective_address());
    auto displacement = mem->read<uint16_t>(p32);
    auto seg = mem->read<uint16_t>(p32 + 2);

    registers->set(modrm_decoder->reg(), displacement);
    registers->set(DS, seg);
}

// les r, m
void EmulatorPimpl::lesc4()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->rm_type() == OP_REG)
        return;

    auto p32 = get_phys_addr(registers->get(DS), modrm_decoder->effective_address());
    auto displacement = mem->read<uint16_t>(p32);
    auto seg = mem->read<uint16_t>(p32 + 2);

    registers->set(modrm_decoder->reg(), displacement);
    registers->set(ES, seg);
}

// lahf
void EmulatorPimpl::lahf9f()
{
    auto flags = registers->get_flags();
    registers->set(AH, flags & 0xff);
}

// sahf
void EmulatorPimpl::sahf9e()
{
    auto new_flags = registers->get(AH);
    auto old_flags = registers->get_flags();
    registers->set_flags((old_flags & 0xff00) | new_flags);
}

// pushf
void EmulatorPimpl::pushf9c()
{
    auto val = registers->get_flags();
    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// popf
void EmulatorPimpl::popf9d()
{
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);
    registers->set_flags(val);

    registers->set(SP, registers->get(SP) + 2);
}

template <typename T>
std::pair<uint16_t, T>
EmulatorPimpl::do_alu(uint16_t v1, uint16_t v2, uint16_t carry_in,
                      std::function<uint32_t(uint32_t, uint32_t, uint32_t)> alu_op)
{
    uint16_t flags = registers->get_flags();

    flags &= ~(AF | CF | OF | PF | SF | ZF);

    uint32_t result32 = alu_op(static_cast<uint32_t>(v1),
                               static_cast<uint32_t>(v2),
                               static_cast<uint32_t>(carry_in));
    bool af = !!(alu_op(v1 & 0xf, v2 & 0xf, 0) & (1 << 4));

    auto sign_bit = (8 * sizeof(T)) - 1;
    auto carry_bit = (8 * sizeof(T));

    if (af)
        flags |= AF;
    if (result32 & (1 << carry_bit))
        flags |= CF;
    if (result32 & (1 << sign_bit))
        flags |= SF;
    if ((result32 & static_cast<T>(-1)) == 0)
        flags |= ZF;
    if (!__builtin_parity(result32 & static_cast<T>(-1)))
        flags |= PF;
    if ((v1 & (1 << sign_bit)) == (v2 & (1 << sign_bit)) &&
        (v1 & (1 << sign_bit)) != (result32 & (1 << sign_bit)))
        flags |= OF;

    return std::make_pair(flags, static_cast<T>(result32));
}

template <typename T>
std::pair<uint16_t, T> EmulatorPimpl::do_add(uint16_t v1, uint16_t v2,
                                             uint16_t carry_in)
{
    return do_alu<T>(v1, v2, carry_in,
        [](uint32_t a, uint32_t b, uint32_t c) -> uint32_t {
            return a + b + c;
        });
}

template <typename T>
std::pair<uint16_t, T> EmulatorPimpl::do_sub(uint16_t v1, uint16_t v2,
                                             uint16_t carry_in)
{
    return do_alu<T>(v1, v2, carry_in,
        [](uint32_t a, uint32_t b, uint32_t c) -> uint32_t {
            return a - b - c;
        });
}

// add r, r/m, 8-bit
void EmulatorPimpl::add00()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2);

    registers->set_flags(flags);
    write_data<uint8_t>(result & 0xff);
}

// add r, r/m, 16-bit
void EmulatorPimpl::add01()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    uint16_t result, flags;
    std::tie(flags, result) = do_add<uint16_t>(v1, v2);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

// add r/m, r, 8-bit
void EmulatorPimpl::add02()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xff);
}

// add r/m, r, 16-bit
void EmulatorPimpl::add03()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint16_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void EmulatorPimpl::add_adc_sub_sbb_80()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3)
        return;

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = fetch_byte();
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint8_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);
    else
        std::tie(flags, result) = do_sub<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint8_t>(result & 0xff);
}

void EmulatorPimpl::add_adc_sub_sbb_82()
{
    // The 's' bit has no effect for 8-bit add immediate.
    add_adc_sub_sbb_80();
}

// add r/m, immediate, 16-bit
void EmulatorPimpl::add_adc_sub_sbb_81()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3)
        return;

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = fetch_16bit();
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint16_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint16_t>(v1, v2, carry_in);
    else
        std::tie(flags, result) = do_sub<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

// add r/m, immediate, 8-bit, sign-extended
void EmulatorPimpl::add_adc_sub_sbb_83()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2 &&
        modrm_decoder->raw_reg() != 5 &&
        modrm_decoder->raw_reg() != 3)
        return;

    uint16_t v1 = read_data<uint16_t>();
    int16_t immed = fetch_byte();
    immed <<= 8;
    immed >>= 8;
    bool carry_in = modrm_decoder->raw_reg() == 2 || modrm_decoder->raw_reg() == 3 ?
        !!(registers->get_flags() & CF) : 0;

    uint16_t result;
    uint16_t flags;
    if (modrm_decoder->raw_reg() == 0 ||
        modrm_decoder->raw_reg() == 2)
        std::tie(flags, result) = do_add<uint16_t>(v1, immed, carry_in);
    else
        std::tie(flags, result) = do_sub<uint16_t>(v1, immed, carry_in);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

void EmulatorPimpl::add04()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(AL, result);
}

void EmulatorPimpl::add05()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint16_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(AX, result);
}

// adc r, r/m, 8-bit
void EmulatorPimpl::adc10()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint8_t>(result & 0xff);
}

// adc r, r/m, 16-bit
void EmulatorPimpl::adc11()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

// adc r/m, r, 8-bit
void EmulatorPimpl::adc12()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xff);
}

// adc r/m, r, 16-bit
void EmulatorPimpl::adc13()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void EmulatorPimpl::adc14()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    bool carry_in = !!(registers->get_flags() & CF);
    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(AL, result);
}

void EmulatorPimpl::adc15()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    bool carry_in = !!(registers->get_flags() & CF);
    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_add<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(AX, result);
}

// sub r, r/m, 8-bit
void EmulatorPimpl::sub28()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2);

    registers->set_flags(flags);
    write_data<uint8_t>(result & 0xff);
}

// sub r, r/m, 16-bit
void EmulatorPimpl::sub29()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

// sub r/m, r, 8-bit
void EmulatorPimpl::sub2a()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xff);
}

// sub r/m, r, 16-bit
void EmulatorPimpl::sub2b()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void EmulatorPimpl::sub2c()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(AL, result);
}

void EmulatorPimpl::sub2d()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2);

    registers->set_flags(flags);
    registers->set(AX, result);
}

void EmulatorPimpl::sbb18()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint8_t>(result & 0xff);
}

// sbb r, r/m, 16-bit
void EmulatorPimpl::sbb19()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    write_data<uint16_t>(result & 0xffff);
}

// sbb r/m, r, 8-bit
void EmulatorPimpl::sbb1a()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xff);
}

// sbb r/m, r, 16-bit
void EmulatorPimpl::sbb1b()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void EmulatorPimpl::sbb1c()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    bool carry_in = !!(registers->get_flags() & CF);
    uint8_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint8_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(AL, result);
}

void EmulatorPimpl::sbb1d()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    bool carry_in = !!(registers->get_flags() & CF);
    uint16_t result;
    uint16_t flags;
    std::tie(flags, result) = do_sub<uint16_t>(v1, v2, carry_in);

    registers->set_flags(flags);
    registers->set(AX, result);
}

// inc r/m, 8-bit
void EmulatorPimpl::incfe()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0)
        return;

    uint8_t v = read_data<uint8_t>();
    uint8_t result;
    uint16_t flags, old_flags = registers->get_flags();

    std::tie(flags, result) = do_add<uint8_t>(v, 1);

    old_flags &= ~(OF | SF | ZF | AF | PF);
    flags &= (OF | SF | ZF | AF | PF);
    registers->set_flags(flags | old_flags);
    write_data<uint8_t>(result);
}

// inc r/m, 16-bit
void EmulatorPimpl::incff()
{
    assert(modrm_decoder->raw_reg() == 0);

    uint16_t v = read_data<uint16_t>();
    uint16_t result;
    uint16_t flags, old_flags = registers->get_flags();

    std::tie(flags, result) = do_add<uint16_t>(v, 1);

    old_flags &= ~(OF | SF | ZF | AF | PF);
    flags &= (OF | SF | ZF | AF | PF);
    registers->set_flags(flags | old_flags);
    write_data<uint16_t>(result);
}

// inc r, 16-bit
void EmulatorPimpl::inc40_47()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    auto v = registers->get(reg);
    uint16_t result;
    uint16_t flags, old_flags = registers->get_flags();

    std::tie(flags, result) = do_add<uint16_t>(v, 1);

    old_flags &= ~(OF | SF | ZF | AF | PF);
    flags &= (OF | SF | ZF | AF | PF);
    registers->set_flags(flags | old_flags);
    registers->set(reg, result);
}

uint8_t EmulatorPimpl::fetch_byte()
{
    ++instr_length;

    return instr_stream->pop();
}

uint16_t EmulatorPimpl::fetch_16bit()
{
    uint16_t immed = (static_cast<uint16_t>(fetch_byte()) |
                      (static_cast<uint16_t>(fetch_byte()) << 8));
    return immed;
}

template <typename T>
void EmulatorPimpl::write_data(T val, bool stack)
{
    if (modrm_decoder->rm_type() == OP_REG) {
        auto dest = modrm_decoder->rm_reg();
        registers->set(dest, val);
    } else {
        auto ea = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), ea);
        mem->write<T>(addr, val);
    }
}

template <typename T>
T EmulatorPimpl::read_data(bool stack)
{
    if (modrm_decoder->rm_type() == OP_MEM) {
        auto displacement = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), displacement);

        return mem->read<T>(addr);
    } else {
        auto source = modrm_decoder->rm_reg();
        return registers->get(source);
    }
}

Emulator::Emulator(RegisterFile *registers)
    : pimpl(std::make_unique<EmulatorPimpl>(registers))
{
}

Emulator::~Emulator()
{
}

size_t Emulator::emulate()
{
    return pimpl->emulate();
}

void Emulator::set_instruction_stream(Fifo<uint8_t> *instr_stream)
{
    pimpl->set_instruction_stream(instr_stream);
}

void Emulator::set_memory(Memory *mem)
{
    pimpl->set_memory(mem);
}

void Emulator::set_io(Memory *io)
{
    pimpl->set_io(io);
}
