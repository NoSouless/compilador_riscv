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
    CPU minhaCPU; // cria uma instância da CPU
    RAM minhaRAM; // cria uma instância da RAM
    Barramento meuBarramento; // cria uma instância do Barramento

    minhaCPU.set_barramento(&meuBarramento); // conecta o barramento à CPU
    meuBarramento.set_cpu(&minhaCPU); // conecta a CPU ao barramento
    meuBarramento.set_ram(&minhaRAM); // conecta a RAM ao barramento

    uint32_t endereco_byte = 0u; // endereço inicial para armazenar instruções na RAM
    const uint32_t RAM_SIZE_BYTES = RAM::BACKING_SIZE; // tamanho total da RAM em bytes
    int opcao; // opção do usuário para inserir instruções
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
            minhaRAM.set_endereco_por_byte(endereco_byte, instrucao); // armazena a instrução na RAM
            endereco_byte += 4u; // avança para o próximo endereço (4 bytes por instrução)
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
                uint32_t instrucao = std::stoul(linha, nullptr, 16); // converte a linha de string para uint32_t em hexadecimal
                minhaRAM.set_endereco_por_byte(endereco_byte, instrucao); // armazena a instrução na RAM
                endereco_byte += 4u; // avança para o próximo endereço (4 bytes por instrução)
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

    while (true) { // loop principal de execução
        meuBarramento.set_addr(minhaCPU.get_pc()); // define o endereço no barramento com o PC da CPU
        meuBarramento.ler_da_ram(); // lê a instrução da RAM através do barramento
        minhaCPU.set_instrucao(meuBarramento.get_data()); // define a instrução na CPU
        if(!minhaCPU.decodificar_instrucao()) break; // decodifica e executa a instrução; sai se for instrução de parada
    }

    char dump_choice;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Deseja fazer dump dos registradores? (s/n): ";
    std::cin >> dump_choice;
    char format_choice;
    std::cout << "Deseja ver os registradores em hexadecimal ou decimal? (h/d): ";
    std::cin >> format_choice;
    if (std::tolower(dump_choice) == 's') {
        minhaCPU.dump_registradores(format_choice); // faz dump dos registradores no formato escolhido
    }

    char dump_ram_choice;
    std::cout << std::endl;
    std::cout << "Deseja fazer dump da RAM? (s/n): ";
    std::cin >> dump_ram_choice;
    if (std::tolower(dump_ram_choice) == 's') {
        minhaRAM.dump_ram(); // faz dump da RAM
    }

    return 0;
}
