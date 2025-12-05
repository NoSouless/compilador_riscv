#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED
#include "barramento.h"
#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdint.h>
#include <string>

class CPU {
    private:

        uint32_t instrucao; // instrução atual
        std::string nome_instrucao; // nome da instrução atual
        uint32_t rs1; // registrador fonte 1
        uint32_t rs2; // registrador fonte 2
        uint32_t rd;  // registrador destino
        uint32_t imm; // valor imediato
        uint32_t registradores[32]; // array de 32 registradores de uso geral

        uint32_t ula_a; // entrada A da ULA
        uint32_t ula_b; // entrada B da ULA
        uint32_t ula_result; // resultado da ULA
        uint32_t pc; // contador de programa
        uint32_t mepc; // registrador de endereço de exceção
        uint32_t pc_4; // próximo endereço do contador de programa

        Barramento *barramento; // ponteiro para o barramento conectado
        bool pc_updated; // flag para indicar se o PC foi atualizado diretamente

    public:
        // Construtor da classe CPU
        CPU() {
            this->instrucao = 0u;
            this->nome_instrucao = "";
            this->rs1 = this->rs2 = this->rd = 0u;
            this->imm = 0u;
            for (int i = 0; i < 32; ++i) this->registradores[i] = 0x00000000;
            this->ula_a = this->ula_b = this->ula_result = 0u;
            this->pc = 0x00000000;
            this->mepc = 0x00000000;
            this->pc_4 = 0x00000004;
            this->barramento = nullptr;
            this->pc_updated = false;
        }

        void set_pc_updated(bool v) { 
            this->pc_updated = v; 
        }

        bool get_pc_updated() const { 
            return this->pc_updated; 
        }

        // Método para extensão de sinal
        static int32_t sign_extend(uint32_t value, int bits) {
            int shift = 32 - bits;
            return (int32_t)(value << shift) >> shift;
        }

        void set_barramento(Barramento *barramento) {
            this->barramento = barramento;
        }

        void set_instrucao(uint32_t value) {
            this->instrucao = value;
        }

        std::uint32_t get_instrucao() const {
            return this->instrucao;
        }

        void set_nome_instrucao(std::string nome_instrucao) {
            this->nome_instrucao = nome_instrucao;
        }

        std::string get_nome_instrucao() const {
            return this->nome_instrucao;
        }

        void set_rs1(uint32_t value) {
            this->rs1 = value;
        }

        uint32_t get_rs1() const {
            return this->rs1;
        }

        void set_rs2(uint32_t value) {
            this->rs2 = value;
        }

        uint32_t get_rs2() const {
            return this->rs2;
        }

        void set_rd(uint32_t value) {
            this->rd = value;
        }

        uint32_t get_rd() const {
            return this->rd;
        }

        void set_imm(uint32_t value) {
            this->imm = value;
        }

        uint32_t get_imm() const {
            return this->imm;
        }

        // Método para definir um registrador de uso geral
        void set_registrador(int index, uint32_t value) {
            if (index < 0 || index >= 32) {
                std::cerr << "Erro: Indice fora dos limites em set_registrador: " << index << std::endl;
                return;
            }

            if (index != 0) this->registradores[index] = value;
        }

        // Método para obter um registrador específico
        std::uint32_t get_registrador(size_t index) {
            if (index >= 32) {
                std::cerr << "Erro: Indice fora dos limites em get_registrador: " << index << std::endl;
                return 0; // Valor padrão em caso de erro
            }

            return this->registradores[index];
        }

        void set_ula_a(uint32_t value) {
            this->ula_a = value;
        }

        uint32_t get_ula_a() const {
            return this->ula_a;
        }
        
        void set_ula_b(uint32_t value) {
            this->ula_b = value;
        }

        uint32_t get_ula_b() const {
            return this->ula_b;
        }

        void set_ula_result(uint32_t value) {
            this->ula_result = value;
        }

        uint32_t get_ula_result() const {
            return this->ula_result;
        }

        void set_pc(uint32_t value) {
            this->pc = value;
        }

        uint32_t get_pc() const {
            return this->pc;
        }

        void set_pc_4(uint32_t value) {
            this->pc_4 = value;
        }

        uint32_t get_pc_4() const {
            return this->pc_4;
        }

        void set_mepc(uint32_t value) {
            this->mepc = value;
        }

        uint32_t get_mepc() const {
            return this->mepc;
        }

        // Método para extrair bits de um registrador
        std::uint32_t extrair_bits(uint32_t valor, int inicio, int fim) {
            int bits = fim - inicio + 1;
            if (bits <= 0 || bits > 32) {
                std::cerr << "Erro: contagem de bits inválida." << std::endl;
                return 0;
            }
            uint64_t mask64;
            if (bits >= 32) {
                mask64 = 0xFFFFFFFFULL;
            } else {
                mask64 = ((1ULL << bits) - 1ULL) << inicio;
            }

            uint32_t mascara = static_cast<uint32_t>(mask64 & 0xFFFFFFFFULL);
            uint32_t resultado = (valor & mascara) >> inicio;

            return resultado;
        }

        // Método que decodifica instruções
        bool decodificar_instrucao() {
            this->set_nome_instrucao("-");
            this->set_imm(0);
            uint32_t opcode = this->extrair_bits(this->get_instrucao(), 0, 6);
            switch(opcode) {
                case 0x33: // R-type -> 0110011
                    this->decodificar_instrucao_R();
                    break;
                case 0x13: // I-type -> 0010011
                case 0x03: // I-type -> 0000011
                case 0x73: // I-type -> 1110011
                case 0x67: // I-type -> 1100111
                    this->decodificar_instrucao_I();
                    break;
                case 0x23: // S-type -> 0100011
                    this->decodificar_instrucao_S();
                    break;
                case 0x63: // B-type -> 1100011
                    this->decodificar_instrucao_B();
                    break;
                case 0x6F: // J-type -> 1101111
                    this->decodificar_instrucao_J();
                    break;
                case 0x37: // U-type -> 0110111
                case 0x17: // U-type -> 0010111
                    this->decodificar_instrucao_U();
                    break;
            }
            if (this->get_nome_instrucao() == "-") {
                return false;
            }
            this->executar_instrucao();
            if (!this->get_pc_updated()) {
                this->set_pc(this->get_pc() + 4u);
            } else {
                this->set_pc_updated(false);
            }
            return true;
        }

        // Método que decodifica instruções do tipo R
        void decodificar_instrucao_R() {
            uint32_t instrucao = this->get_instrucao();
            uint32_t funct3 = this->extrair_bits(instrucao, 12, 14);
            uint32_t funct7 = this->extrair_bits(instrucao, 25, 31);
            this->set_rs1(this->extrair_bits(instrucao, 15, 19));
            this->set_rs2(this->extrair_bits(instrucao, 20, 24));
            this->set_rd(this->extrair_bits(instrucao, 7, 11));
            this->set_imm(0);
            switch(funct3) {
                case 0x0:
                    this->set_nome_instrucao(funct7 == 0x00 ? "ADD" : "SUB");
                    break;
                case 0x4:
                    this->set_nome_instrucao("XOR");
                    break;
                case 0x6:
                    this->set_nome_instrucao("OR");
                    break;
                case 0x7:
                    this->set_nome_instrucao("AND");
                    break;
                case 0x1:
                    this->set_nome_instrucao("SLL");
                    break;
                case 0x5:
                    this->set_nome_instrucao((funct7 == 0x00) ? "SRL" : "SRA");
                    break;
                case 0x2:
                    this->set_nome_instrucao("SLT");
                    break;
                case 0x3:
                    this->set_nome_instrucao("SLTU");
                    break;
                default:
                    this->set_nome_instrucao("-");
                    this->set_imm(0);
                    break;
            }
        }

        // Método que decodifica instruções do tipo I
        void decodificar_instrucao_I() {
            uint32_t instrucao = this->get_instrucao();
            uint32_t opcode = extrair_bits(instrucao, 0, 6);
            uint32_t funct3 = extrair_bits(instrucao, 12, 14);
            uint32_t funct7 = extrair_bits(instrucao, 25, 31);
            this->set_rs1(this->extrair_bits(instrucao, 15, 19));
            this->set_rd(this->extrair_bits(instrucao, 7, 11));
            this->set_imm(this->extrair_bits(instrucao, 20, 31));
            switch (opcode) {
                case 0x13: // I-type -> 0010011
                    switch(funct3) {
                        case 0x0:
                            this->set_nome_instrucao("ADDI");
                            break;
                        case 0x7:
                            this->set_nome_instrucao("ANDI");
                            break;
                        case 0x6:
                            this->set_nome_instrucao("ORI");
                            break;
                        case 0x4:
                            this->set_nome_instrucao("XORI");
                            break;
                        case 0x2:
                            this->set_nome_instrucao("SLTI");
                            break;
                        case 0x3:
                            this->set_nome_instrucao("SLTIU");
                            break;
                        case 0x1:
                            this->set_nome_instrucao((funct7 == 0x00) ? "SLLI" : "-");
                            break;
                        case 0x5:
                            this->set_nome_instrucao((funct7 == 0x00) ? "SRLI" : (funct7 == 0x20) ? "SRAI" : "-");
                            break;
                        default:
                            this->set_nome_instrucao("-");
                            this->set_imm(0);
                    }
                    break;
                case 0x03: // I-type -> 0000011 (Load instructions)
                    switch(funct3) {
                        case 0x0:
                            this->set_nome_instrucao("LB");
                            break;
                        case 0x1:
                            this->set_nome_instrucao("LH");
                            break;
                        case 0x2:
                            this->set_nome_instrucao("LW");
                            break;
                        case 0x4:
                            this->set_nome_instrucao("LBU");
                            break;
                        case 0x5:
                            this->set_nome_instrucao("LHU");
                            break;
                        default:
                            this->set_nome_instrucao("-");
                            this->set_imm(0);
                    }
                    break;
                case 0x73: // I-type -> 1110011 (System instructions)
                    switch(funct3) {
                        case 0x0:
                            this->set_nome_instrucao("ECALL/EBREAK");
                            this->set_imm(0);
                            break;
                        default:
                            this->set_nome_instrucao("-");
                            this->set_imm(0);
                            break;
                    }
                    break;
                case 0x67: // I-type -> 1100111 (JALR instruction)
                    this->set_nome_instrucao("JALR");
                    break;
                default:
                    this->set_nome_instrucao("-");
                    this->set_imm(0);
                    break;
            }
        }

        // Método que decodifica instruções do tipo S
        void decodificar_instrucao_S() {
            uint32_t instrucao = this->get_instrucao();
            uint32_t funct3 = extrair_bits(instrucao, 12, 14);
            this->set_rs1(this->extrair_bits(instrucao, 15, 19));
            this->set_rs2(this->extrair_bits(instrucao, 20, 24));
            this->set_imm((this->extrair_bits(instrucao, 7, 11)) | (this->extrair_bits(instrucao, 25, 31) << 5));
            switch(funct3) {
                case 0x0:
                    this->set_nome_instrucao("SB");
                    break;
                case 0x1:
                    this->set_nome_instrucao("SH");
                    break;
                case 0x2:
                    this->set_nome_instrucao("SW");
                    break;
                default:
                    this->set_nome_instrucao("-");
                    this->set_imm(0);
                    break;
            }
        }

        // Método que decodifica instruções do tipo B
        void decodificar_instrucao_B() {
            uint32_t instrucao = this->get_instrucao();
            uint32_t funct3 = extrair_bits(instrucao, 12, 14);
            this->set_imm((this->extrair_bits(instrucao, 8, 11) << 1) |
                    (this->extrair_bits(instrucao, 25, 30) << 5) |
                    (this->extrair_bits(instrucao, 7, 7) << 11) |
                    (this->extrair_bits(instrucao, 31, 31) << 12));
            this->set_rs1(this->extrair_bits(instrucao, 15, 19));
            this->set_rs2(this->extrair_bits(instrucao, 20, 24));
            switch(funct3) {
                case 0x0:
                    this->set_nome_instrucao("BEQ");
                    break;
                case 0x1:
                    this->set_nome_instrucao("BNE");
                    break;
                case 0x4:
                    this->set_nome_instrucao("BLT");
                    break;
                case 0x5:
                    this->set_nome_instrucao("BGE");
                    break;
                case 0x6:
                    this->set_nome_instrucao("BLTU");
                    break;
                case 0x7:
                    this->set_nome_instrucao("BGEU");
                    break;
                default:
                    this->set_nome_instrucao("-");
                    this->set_imm(0);
                    break;
            }
        }

        // Método que decodifica instruções do tipo U
        void decodificar_instrucao_U() {
            uint32_t instrucao = this->get_instrucao();
            uint32_t opcode = extrair_bits(instrucao, 0, 6);
            this->set_imm(this->extrair_bits(instrucao, 12, 31) << 12);
            this->set_rd(this->extrair_bits(instrucao, 7, 11));
            switch (opcode) {
                case 0x37: // U-type -> 0110111
                    this->set_nome_instrucao("LUI");
                    break;
                case 0x17: // U-type -> 0010111
                    this->set_nome_instrucao("AUIPC");
                    break;
                default:
                    this->set_nome_instrucao("-");
                    this->set_imm(0);
                    break;
            }
        }

        // Método que decodifica instruções do tipo J
        void decodificar_instrucao_J() {
            uint32_t instrucao = this->get_instrucao();
            this->set_imm((this->extrair_bits(instrucao, 21, 30) << 1) |
                    (this->extrair_bits(instrucao, 20, 20) << 11) |
                    (this->extrair_bits(instrucao, 12, 19) << 12) |
                    (this->extrair_bits(instrucao, 31, 31) << 20));
            this->set_rd(this->extrair_bits(instrucao, 7, 11));
            this->set_nome_instrucao("JAL");
        }

        // Método que executa a instrução decodificada
        void executar_instrucao() {
            std::string instr = this->get_nome_instrucao();
            if (instr == "ADD") { // Adição
                uint32_t resultado = this->get_registrador(this->get_rs1()) + this->get_registrador(this->get_rs2());
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SUB") { // Subtração
                uint32_t resultado = this->get_registrador(this->get_rs1()) - this->get_registrador(this->get_rs2());
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "XOR") { // XOR
                uint32_t resultado = this->get_registrador(this->get_rs1()) ^ this->get_registrador(this->get_rs2());
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "OR") { // OR
                uint32_t resultado = this->get_registrador(this->get_rs1()) | this->get_registrador(this->get_rs2());
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "AND") { // AND
                uint32_t resultado = this->get_registrador(this->get_rs1()) & this->get_registrador(this->get_rs2());
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLL") { // Shift Left Logical (Shift lógico à esquerda)
                uint32_t resultado = this->get_registrador(this->get_rs1()) << (this->get_registrador(this->get_rs2()) & 0x1F);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SRL") { // Shift Right Logical (Shift lógico à direita)
                uint32_t resultado = this->get_registrador(this->get_rs1()) >> (this->get_registrador(this->get_rs2()) & 0x1F);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SRA") { // Shift Right Arithmetic (Shift aritmético à direita)
                uint32_t resultado = static_cast<uint32_t>(static_cast<int32_t>(this->get_registrador(this->get_rs1())) >> (this->get_registrador(this->get_rs2()) & 0x1F));
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLT") { // Set Less Than (Comparação com sinal)
                uint32_t resultado = (static_cast<int32_t>(this->get_registrador(this->get_rs1())) < static_cast<int32_t>(this->get_registrador(this->get_rs2()))) ? 1u : 0u;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLTU") { // Set Less Than Unsigned (Comparação sem sinal)
                uint32_t resultado = (this->get_registrador(this->get_rs1()) < this->get_registrador(this->get_rs2())) ? 1u : 0u;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "ADDI") { // Adição Imediata
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t resultado = this->get_registrador(this->get_rs1()) + static_cast<uint32_t>(imm);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "ANDI") { // AND Imediato
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t resultado = this->get_registrador(this->get_rs1()) & static_cast<uint32_t>(imm);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "ORI") { // OR Imediato
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t resultado = this->get_registrador(this->get_rs1()) | static_cast<uint32_t>(imm);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "XORI") { // XOR Imediato
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t resultado = this->get_registrador(this->get_rs1()) ^ static_cast<uint32_t>(imm);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLTI") { // Set Less Than Imediato (Comparação com sinal)
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t resultado = (static_cast<int32_t>(this->get_registrador(this->get_rs1())) < imm) ? 1u : 0u;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLTIU") { // Set Less Than Imediato Unsigned (Comparação sem sinal)
                uint32_t imm = this->get_imm();
                uint32_t resultado = (this->get_registrador(this->get_rs1()) < imm) ? 1u : 0u;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SLLI") { // Shift Left Logical Imediato
                uint32_t shamt = this->get_imm() & 0x1F;
                uint32_t resultado = this->get_registrador(this->get_rs1()) << shamt;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SRLI") { // Shift Right Logical Imediato
                uint32_t shamt = this->get_imm() & 0x1F;
                uint32_t resultado = this->get_registrador(this->get_rs1()) >> shamt;
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "SRAI") { // Shift Right Arithmetic Imediato
                uint32_t shamt = this->get_imm() & 0x1F;
                uint32_t resultado = static_cast<uint32_t>(static_cast<int32_t>(this->get_registrador(this->get_rs1())) >> shamt);
                this->set_registrador(this->get_rd(), resultado);
            } else if (instr == "LB" || instr == "LBU" || instr == "LH" || instr == "LHU" || instr == "LW") {
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t addr = this->get_registrador(this->get_rs1()) + static_cast<uint32_t>(imm);
                uint32_t aligned = addr & ~0x3u;
                uint32_t offset = addr & 0x3u;
                if (!this->barramento) {
                    std::cerr << "Erro: barramento nao conectado ao CPU para load." << std::endl;
                } else {
                    this->barramento->set_addr(aligned);
                    this->barramento->ler_da_ram();
                    uint32_t word = this->barramento->get_data();
                    uint32_t value = 0u;
                    if (instr == "LW") { // Load Word
                        value = word;
                    } else if (instr == "LB" || instr == "LBU") {
                        uint8_t b = static_cast<uint8_t>((word >> (offset * 8)) & 0xFFu);
                        if (instr == "LB") { // Load Byte
                            int8_t sb = static_cast<int8_t>(b);
                            value = static_cast<uint32_t>(static_cast<int32_t>(sb));
                        } else { // Load Byte Unsigned
                            value = static_cast<uint32_t>(b);
                        }
                    } else if (instr == "LH" || instr == "LHU") {
                        uint16_t half;
                        if (offset == 0u) {
                            half = static_cast<uint16_t>(word & 0xFFFFu);
                        } else if (offset == 2u) {
                            half = static_cast<uint16_t>((word >> 16) & 0xFFFFu);
                        } else {
                            half = 0u;
                        }
                        if (instr == "LH") { // Load Halfword
                            int16_t sh = static_cast<int16_t>(half);
                            value = static_cast<uint32_t>(static_cast<int32_t>(sh));
                        } else { // Load Halfword Unsigned
                            value = static_cast<uint32_t>(half);
                        }
                    }
                    this->set_registrador(this->get_rd(), value);
                }
            } else if (instr == "SB" || instr == "SH" || instr == "SW") {
                int32_t imm = sign_extend(this->get_imm(), 12);
                uint32_t addr = this->get_registrador(this->get_rs1()) + static_cast<uint32_t>(imm);
                uint32_t aligned = addr & ~0x3u;
                uint32_t offset = addr & 0x3u;
                uint32_t data = this->get_registrador(this->get_rs2());
                if (!this->barramento) {
                    std::cerr << "Erro: barramento nao conectado ao CPU para store." << std::endl;
                } else {
                    this->barramento->set_addr(aligned);
                    this->barramento->ler_da_ram();
                    uint32_t word = this->barramento->get_data();
                    uint32_t new_word = word;
                    if (instr == "SW" && offset == 0u) { // Store Word
                        new_word = data;
                    } else if (instr == "SB") { // Store Byte
                        uint32_t shift = offset * 8u;
                        uint32_t mask = 0xFFu << shift;
                        new_word = (word & ~mask) | ((data & 0xFFu) << shift);
                    } else if (instr == "SH") { // Store Halfword
                        if (offset == 0u || offset == 2u) {
                            uint32_t shift = offset * 8u;
                            uint32_t mask = 0xFFFFu << shift;
                            new_word = (word & ~mask) | ((data & 0xFFFFu) << shift);
                        } 
                    }
                    this->barramento->escrever_na_ram(aligned, new_word);
                }
            } else if (instr == "ECALL/EBREAK") { // Tratamento de exceção
                this->set_mepc(this->get_pc());
                this->set_pc(0x00000010);
                this->set_pc_updated(true);
            } else if (instr == "JAL") { // Jump and Link
                int32_t offset = sign_extend(this->get_imm(), 21);
                if (offset == 0 && this->get_rd() == 0u) {
                    this->set_nome_instrucao("-");
                    return;
                }
                uint32_t ret = this->get_pc() + 4u;
                this->set_registrador(this->get_rd(), ret);
                this->set_pc(static_cast<uint32_t>(static_cast<int32_t>(this->get_pc()) + offset));
                this->set_pc_updated(true);
            } else if (instr == "JALR") { // Jump and Link Register
                uint32_t ret = this->get_pc() + 4u;
                this->set_registrador(this->get_rd(), ret);
                int32_t offset = sign_extend(this->get_imm(), 12);
                uint32_t target = static_cast<uint32_t>(static_cast<int32_t>(this->get_registrador(this->get_rs1())) + offset) & ~1u;
                this->set_pc(target);
                this->set_pc_updated(true);
            } else if (instr == "BEQ" || instr == "BNE" || instr == "BLT" || instr == "BGE" || instr == "BLTU" || instr == "BGEU") {
                int32_t imm = sign_extend(this->get_imm(), 13);
                uint32_t v1 = this->get_registrador(this->get_rs1());
                uint32_t v2 = this->get_registrador(this->get_rs2());
                bool take = false;
                if (instr == "BEQ") take = (v1 == v2); // Branch if Equal
                else if (instr == "BNE") take = (v1 != v2); // Branch if Not Equal
                else if (instr == "BLT") take = (static_cast<int32_t>(v1) < static_cast<int32_t>(v2)); // Branch if Less Than
                else if (instr == "BGE") take = (static_cast<int32_t>(v1) >= static_cast<int32_t>(v2)); // Branch if Greater or Equal
                else if (instr == "BLTU") take = (v1 < v2); // Branch if Less Than Unsigned
                else if (instr == "BGEU") take = (v1 >= v2); // Branch if Greater or Equal Unsigned
                if (take) {
                    this->set_pc(static_cast<uint32_t>(static_cast<int32_t>(this->get_pc()) + imm));
                    this->set_pc_updated(true);
                }
            } else if (instr == "LUI") { // Load Upper Immediate
                this->set_registrador(this->get_rd(), this->get_imm());
            } else if (instr == "AUIPC") { // Add Upper Immediate to PC
                this->set_registrador(this->get_rd(), this->get_pc() + this->get_imm());
            }
        }

        // Método para exibir o conteúdo dos registradores
        void dump_registradores(char format_choice) {
            std::cout << "Dump dos registradores:" << std::endl;
            for (int i = 0; i < 32; ++i) {
                if (this->registradores[i] != 0) {
                    if (format_choice == 'd' || format_choice == 'D')
                        std::cout << "x" << i << ": " << this->get_registrador(i) << std::endl;
                    else
                        std::cout << "x" << i << ": 0x" << std::hex << std::uppercase << this->get_registrador(i) << std::dec << std::nouppercase << std::endl;
                }
            }
        }

};

#endif // CPU_H_INCLUDED
