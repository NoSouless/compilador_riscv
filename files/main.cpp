#include <stdio.h>
#include <iostream>
#include "cpu.h"
#include "ram.h"
#include "barramento.h"
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <bitset>
#include <fstream>
#include <cctype>


//https://eseo-tech.github.io/emulsiV/

int main() {
    CPU minhaCPU;
    RAM minhaRAM;
    Barramento meuBarramento;

    minhaCPU.set_barramento(&meuBarramento);
    meuBarramento.set_cpu(&minhaCPU);
    meuBarramento.set_ram(&minhaRAM);

    uint32_t endereco_byte = 0u;
    const uint32_t RAM_SIZE_BYTES = RAM::BACKING_SIZE;
    int opcao;
    std::cout << "Escolha uma opcao para inserir as instrucoes na RAM:" << std::endl;
    std::cout << "1 - Inserir manualmente" << std::endl;
    std::cout << "2 - Carregar de arquivo txt" << std::endl;
    std::cout << "Digite 1 ou 2: ";
    std::cin >> opcao;

    if (opcao == 1) {
        do {
            uint32_t instrucao;
            std::cout << "Digite uma instrucao RISC-V em hexadecimal para armazenar na RAM (0 para sair): ";
            std::cin >> std::hex >> instrucao;
            if (!std::cin) {
                std::cerr << "Entrada invalida." << std::endl;
                break;
            }
            if (instrucao == 0) break;
            if (endereco_byte >= RAM_SIZE_BYTES) {
                std::cerr << "Erro: RAM cheia. Nao e possivel armazenar mais instrucoes." << std::endl;
                break;
            }
            minhaRAM.set_endereco_por_byte(endereco_byte, instrucao);
            endereco_byte += 4u;
        } while (true);
    } else if (opcao == 2) {
        std::string nome_arquivo;      
        std::cout << "Digite o nome do arquivo txt (com extensao) na pasta 'commands': ";   
        std::cin >> nome_arquivo;
        std::ifstream arquivo("commands/" + nome_arquivo);
        if (!arquivo.is_open()) {
            std::cerr << "Erro ao abrir o arquivo." << std::endl;
            return 1;
        }
        std::string linha;
        while (std::getline(arquivo, linha)) {
            if (endereco_byte >= RAM_SIZE_BYTES) {
                std::cerr << "Erro: RAM cheia. Nao e possivel armazenar mais instrucoes." << std::endl;
                break;
            }
            try {
                uint32_t instrucao = std::stoul(linha, nullptr, 16);
                minhaRAM.set_endereco_por_byte(endereco_byte, instrucao);
                endereco_byte += 4u;
            } catch (const std::invalid_argument& e) {
                std::cerr << "Linha invalida no arquivo: " << linha << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Valor fora do alcance no arquivo: " << linha << std::endl;
            }
        } 
        arquivo.close();
    } else {
        std::cerr << "Opcao invalida." << std::endl;
        return 1;
    }

    while (true) {
        meuBarramento.set_addr(minhaCPU.get_pc());
        meuBarramento.ler_da_ram();
        minhaCPU.set_instrucao(meuBarramento.get_data());
        if(!minhaCPU.decodificar_instrucao()) break;
    }

    char dump_choice;
    std::cout << std::endl;
    std::cout << "Deseja fazer dump dos registradores? (s/n): ";
    std::cin >> dump_choice;
    //Perguntar se quer em hexadecimal ou decimal
    char format_choice;
    std::cout << "Deseja ver os registradores em hexadecimal ou decimal? (h/d): ";
    std::cin >> format_choice;
    if (std::tolower(dump_choice) == 's') {
        minhaCPU.dump_registradores(format_choice);
    }

    char dump_ram_choice;
    std::cout << std::endl;
    std::cout << "Deseja fazer dump da RAM? (s/n): ";
    std::cin >> dump_ram_choice;
    if (std::tolower(dump_ram_choice) == 's') {
        minhaRAM.dump_ram();
    }

    return 0;
}
