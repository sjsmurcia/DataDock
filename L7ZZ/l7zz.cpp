#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

struct LZ77Token {
    int offset;
    int length;
    char nextChar;
};

class LZ77 {
private:
    static const int WINDOW_SIZE = 1024; 

public:
    std::vector<LZ77Token> compress(const std::string& text) {
        std::vector<LZ77Token> tokens;
        size_t cursor = 0;

        while (cursor < text.size()) {
            int bestOffset = 0;
            int bestLength = 0;
            
            int searchStart = std::max(0, static_cast<int>(cursor) - WINDOW_SIZE);

            for (size_t searchCursor = searchStart; searchCursor < cursor; ++searchCursor) {
                size_t matchLength = 0;
                while (cursor + matchLength < text.size() &&
                       text[searchCursor + matchLength] == text[cursor + matchLength]) {
                    matchLength++;
                    if (searchCursor + matchLength >= cursor) {
                        break;
                    }
                }

                if (matchLength > bestLength) {
                    bestOffset = cursor - searchCursor;
                    bestLength = matchLength;
                }
            }

            char nextChar = (cursor + bestLength < text.size()) ? text[cursor + bestLength] : '\0';
            tokens.push_back({bestOffset, bestLength, nextChar});

            cursor += bestLength + 1;
        }

        return tokens;
    }

    std::string decompress(const std::vector<LZ77Token>& tokens) {
        std::string decompressed;

        for (const auto& token : tokens) {
            size_t start = decompressed.size() - token.offset;
            for (int i = 0; i < token.length; ++i) {
                decompressed += decompressed[start + i];
            }
            if (token.nextChar != '\0') {
                decompressed += token.nextChar;
            }
        }

        return decompressed;
    }
};

void saveCompressedFile(const std::vector<LZ77Token>& tokens, const std::string& compressedFileName) {
    std::ofstream outFile(compressedFileName, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error al escribir el archivo comprimido." << std::endl;
        return;
    }

    size_t tokenCount = tokens.size();
    outFile.write(reinterpret_cast<const char*>(&tokenCount), sizeof(tokenCount));
    for (const auto& token : tokens) {
        outFile.write(reinterpret_cast<const char*>(&token.offset), sizeof(token.offset));
        outFile.write(reinterpret_cast<const char*>(&token.length), sizeof(token.length));
        outFile.put(token.nextChar);
    }

    outFile.close();
}

std::vector<LZ77Token> loadCompressedFile(const std::string& compressedFileName) {
    std::ifstream inFile(compressedFileName, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error al leer el archivo comprimido." << std::endl;
        return {};
    }

    size_t tokenCount;
    inFile.read(reinterpret_cast<char*>(&tokenCount), sizeof(tokenCount));

    std::vector<LZ77Token> tokens(tokenCount);
    for (auto& token : tokens) {
        inFile.read(reinterpret_cast<char*>(&token.offset), sizeof(token.offset));
        inFile.read(reinterpret_cast<char*>(&token.length), sizeof(token.length));
        token.nextChar = inFile.get();
    }

    inFile.close();
    return tokens;
}

void compressFile(const std::string& inputFileName, const std::string& compressedFileName) {
    LZ77 lz77;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "No se pudo abrir el archivo original." << std::endl;
        return;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    auto tokens = lz77.compress(text);
    auto end = std::chrono::high_resolution_clock::now();

    saveCompressedFile(tokens, compressedFileName);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    size_t originalSize = text.size();
    size_t compressedSize = tokens.size() * (sizeof(int) * 2 + sizeof(char));

    double compressionRate = 1.0 - static_cast<double>(compressedSize) / originalSize;

    std::cout << "Archivo comprimido en: " << compressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
    std::cout << "Tasa de compresion: " << (compressionRate * 100) << "%" << std::endl;
}

void decompressFile(const std::string& compressedFileName, const std::string& decompressedFileName) {
    LZ77 lz77;
    auto tokens = loadCompressedFile(compressedFileName);
    if (tokens.empty()) {
        std::cerr << "Error al cargar el archivo comprimido." << std::endl;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto decompressed = lz77.decompress(tokens);
    auto end = std::chrono::high_resolution_clock::now();

    std::ofstream decompressedFile(decompressedFileName);
    decompressedFile << decompressed;
    decompressedFile.close();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Archivo descomprimido en: " << decompressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
}

int main() {

    system("chcp 65001");

    // Configurar el entorno para mostrar caracteres especiales
    setlocale(LC_ALL, "es_ES.UTF-8");
    std::string inputFileName;
    std::string compressedFileName;
    std::string decompressedFileName;

    while (true) {
        std::cout << "\n--- Menu LZ77 ---" << std::endl;
          std::cout <<"\n !Importante: \n La extension del archivo a comprimir\n debe ser '.txt'\n y la extension del archivo de salidad\npuede ser .sf o .compressed\n"<<std::endl;
       
        std::cout << "1. Comprimir archivo" << std::endl;
        std::cout << "2. Descomprimir archivo" << std::endl;
        std::cout << "3. Salir" << std::endl;
        std::cout << "Seleccione una opcion: ";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            std::cout << "Ingrese el nombre del archivo a comprimir: ";
            std::cin >> inputFileName;
            std::cout << "Ingrese el nombre del archivo comprimido de salida: ";
            std::cin >> compressedFileName;
            compressFile(inputFileName, compressedFileName);
        } else if (choice == 2) {
            std::cout << "Ingrese el nombre del archivo comprimido: ";
            std::cin >> compressedFileName;
            std::cout << "Ingrese el nombre del archivo descomprimido de salida: ";
            std::cin >> decompressedFileName;
            decompressFile(compressedFileName, decompressedFileName);
        } else if (choice == 3) {
            std::cout << "Saliendo..." << std::endl;
            break;
        } else {
            std::cout << "Opcion invalida. Intente nuevamente." << std::endl;
        }
    }

    return 0;
}
