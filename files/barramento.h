#ifndef BARRAMENTO_H_INCLUDED
#define BARRAMENTO_H_INCLUDED
#include "ram.h"
class CPU;
#include <utility>
#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdint.h>
#include <string>
#include <unordered_map>

class Barramento{
    private:
        uint32_t endereco; // endereço em bytes, para barramento de endereços
        uint32_t dados; // dados em 32 bits, para barramento de dados
        bool controle; // sinal de controle (ex: IRQ), para barramento de controle

        CPU *cpu; // ponteiro para a CPU conectada
        RAM *ram; // ponteiro para a RAM conectada
        std::unordered_map<uint32_t, uint32_t> mmio_state; // estado simples para MMIO
    public:
        // Construtor da classe Barramento
        Barramento() {
            this->set_addr(0x00000000);
            this->set_data(0x00000000);
            this->set_irq(false);
        }

        void set_cpu(CPU *cpu) {
            this->cpu = cpu;
        }

        void set_ram(RAM *ram) {
            this->ram = ram;
        }

        void set_addr(uint32_t endereco) {
            this->endereco = endereco;
        }

        uint32_t get_addr() const {
            return endereco;
        }

        void set_data(uint32_t dados) {
            this->dados = dados;
        }

        uint32_t get_data() const {
            return dados;
        }

        void set_irq(bool interrupcao) {
            controle = interrupcao;
        }

        bool get_irq() const {
            return controle;
        }

        // Ler da RAM através do barramento usando endereço em bytes
        void ler_da_ram() {
            if (!ram) {
                std::cerr << "Erro: RAM nao conectada ao barramento." << std::endl;
                return;
            }
            if (this->get_addr() >= RAM::PERIPHERAL_BASE || this->get_addr() >= RAM::BACKING_SIZE) {
                this->set_data(0u);
                return;
            }
            this->set_data(this->ram->get_endereco_por_byte(this->get_addr()));
        }

        // Escrever na RAM através do barramento usando endereço em bytes
        void escrever_na_ram(uint32_t endereco_byte, uint32_t valor) {
            if (!ram) {
                std::cerr << "Erro: RAM nao conectada ao barramento." << std::endl;
                return;
            }

            if (endereco_byte >= RAM::PERIPHERAL_BASE) {
                uint32_t aligned_addr = endereco_byte & ~0x3u;
                uint32_t old = 0u;
                auto it = mmio_state.find(aligned_addr);
                if (it != mmio_state.end()) old = it->second;
                uint32_t new_word = valor;
                for (int i = 0; i < 4; ++i) {
                    uint8_t new_b = static_cast<uint8_t>((new_word >> (i*8)) & 0xFFu);
                    if (new_b != 0) {
                        std::cout << static_cast<char>(new_b);
                    }
                }
                std::cout.flush();
                mmio_state[aligned_addr] = new_word;
                return;
            }
            if (endereco_byte >= RAM::BACKING_SIZE) {
                std::cerr << "Erro: endereco de escrita fora do espaco RAM: 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return;
            }
            this->ram->set_endereco_por_byte(endereco_byte, valor);
        }
};

#endif // BARRAMENTO_H_INCLUDED
