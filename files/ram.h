#ifndef RAM_H_INCLUDED
#define RAM_H_INCLUDED
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

class RAM{
    private:
        std::vector<uint8_t> mem; // array de bytes representando a RAM
    public:
        // 0x00000 - 0x7FFFF : 512 KB Main RAM
        static constexpr uint32_t RAM_MAIN_BASE = 0x00000u;
        static constexpr uint32_t RAM_MAIN_SIZE = 0x80000u; // 512 KB
        // 0x80000 - 0x8FFFF : 64 KB VRAM (video)
        static constexpr uint32_t VRAM_BASE = 0x80000u;
        static constexpr uint32_t VRAM_SIZE = 0x10000u; // 64 KB
        // 0x90000 - 0x9FBFF : 60 KB Extra RAM para expansão
        static constexpr uint32_t RAM_EXTRA_BASE = 0x90000u;
        static constexpr uint32_t RAM_EXTRA_SIZE = 0x0FC00u; // PERIPHERAL_BASE - 0x90000
        // 0x9FC00 - 0x9FFFF : 1 KB para periféricos (MMIO)
        static constexpr uint32_t PERIPHERAL_BASE = 0x9FC00u;
        static constexpr uint32_t PERIPHERAL_SIZE = 0x400u; // 1 KB

        // Number of backing bytes in the RAM backing store (up to peripherals base)
        static constexpr uint32_t BACKING_SIZE = PERIPHERAL_BASE; // bytes backed by RAM storage

        // Construtor da classe RAM
        RAM() {
            mem.resize(BACKING_SIZE, 0u);
        }

        // Escreve uma palavra de 32 bits em um endereço de byte
        void set_endereco_por_byte(uint32_t endereco_byte, uint32_t value) {
            if (endereco_byte % 4 != 0) {
                std::cerr << "Erro: endereco nao alinhado (deve ser multiplo de 4): 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return;
            }
            if (endereco_byte >= BACKING_SIZE) {
                std::cerr << "Erro: endereco de escrita fora do espaco RAM backing: 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return;
            }
            if (endereco_byte + 3 >= BACKING_SIZE) {
                std::cerr << "Erro: escrita atravessaria limite da RAM backing: 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return;
            }
            mem[endereco_byte + 0] = static_cast<uint8_t>(value & 0xFFu);
            mem[endereco_byte + 1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
            mem[endereco_byte + 2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
            mem[endereco_byte + 3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
        }

        // Lê uma palavra de 32 bits em formato little-endian no endereço de byte
        std::uint32_t get_endereco_por_byte(uint32_t endereco_byte) {
            if (endereco_byte % 4 != 0) {
                std::cerr << "Erro: endereco nao alinhado (deve ser multiplo de 4): 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return 0;
            }
            if (endereco_byte >= BACKING_SIZE) {
                std::cerr << "Erro: endereco de leitura fora do espaco RAM backing: 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return 0;
            }
            if (endereco_byte + 3 >= BACKING_SIZE) {
                std::cerr << "Erro: leitura atravesaria limite da RAM backing: 0x" << std::hex << endereco_byte << std::dec << std::endl;
                return 0;
            }
            uint32_t b0 = mem[endereco_byte + 0];
            uint32_t b1 = mem[endereco_byte + 1];
            uint32_t b2 = mem[endereco_byte + 2];
            uint32_t b3 = mem[endereco_byte + 3];
            return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
        }

        // Método para exibir o conteúdo da RAM
        void dump_ram() {
            std::cout << "Dump da RAM (enderecos e valores em hexadecimal):" << std::endl;
            //Apenas dos endereços que foram escritos
            for (uint32_t addr = 0; addr < BACKING_SIZE; addr += 4) {
                uint32_t word = get_endereco_por_byte(addr);
                if (word != 0) { // Apenas exibe palavras não nulas
                    std::cout << "0x" << std::hex << addr << ": 0x" << word << std::dec << std::endl;
                }
            }
        }
};

#endif // RAM_H_INCLUDED
