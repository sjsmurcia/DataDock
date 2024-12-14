#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>

class LZWCompression {
public:
    std::vector<int> compress(const std::string& text) {
        std::unordered_map<std::string, int> dictionary;
        for (int i = 0; i < 256; ++i) {
            dictionary[std::string(1, i)] = i;
        }

        std::string current;
        std::vector<int> compressed;
        int code = 256;

        for (char c : text) {
            current += c;
            if (dictionary.find(current) == dictionary.end()) {
                dictionary[current] = code++;
                current.pop_back();
                compressed.push_back(dictionary[current]);
                current = c;
            }
        }

        if (!current.empty()) {
            compressed.push_back(dictionary[current]);
        }

        return compressed;
    }

    std::string decompress(const std::vector<int>& compressed) {
        std::unordered_map<int, std::string> dictionary;
        for (int i = 0; i < 256; ++i) {
            dictionary[i] = std::string(1, i);
        }

        std::string current = dictionary[compressed[0]];
        std::string decompressed = current;
        int code = 256;

        for (size_t i = 1; i < compressed.size(); ++i) {
            std::string entry;
            if (dictionary.find(compressed[i]) != dictionary.end()) {
                entry = dictionary[compressed[i]];
            } else if (compressed[i] == code) {
                entry = current + current[0];
            } else {
                throw std::runtime_error("Error en la descompresión: código no encontrado.");
            }

            decompressed += entry;
            dictionary[code++] = current + entry[0];
            current = entry;
        }

        return decompressed;
    }
};

void compressFile(const std::string& inputFileName, const std::string& compressedFileName) {
    LZWCompression lzw;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo original." << std::endl;
        return;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> compressed = lzw.compress(text);
    auto end = std::chrono::high_resolution_clock::now();

    std::ofstream compressedFile(compressedFileName, std::ios::binary);
    if (!compressedFile.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo comprimido." << std::endl;
        return;
    }

    for (int code : compressed) {
        compressedFile.write(reinterpret_cast<const char*>(&code), sizeof(code));
    }
    compressedFile.close();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Archivo comprimido en: " << compressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
    std::cout << "Tamaño original: " << text.size() << " bytes, Tamaño comprimido: " << compressed.size() * sizeof(int) << " bytes." << std::endl;
}

void decompressFile(const std::string& compressedFileName, const std::string& decompressedFileName) {
    LZWCompression lzw;
    std::ifstream compressedFile(compressedFileName, std::ios::binary);
    if (!compressedFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo comprimido." << std::endl;
        return;
    }

    std::vector<int> compressed;
    int code;
    while (compressedFile.read(reinterpret_cast<char*>(&code), sizeof(code))) {
        compressed.push_back(code);
    }
    compressedFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    std::string decompressed = lzw.decompress(compressed);
    auto end = std::chrono::high_resolution_clock::now();

    std::ofstream decompressedFile(decompressedFileName);
    if (!decompressedFile.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo descomprimido." << std::endl;
        return;
    }

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
        std::cout << "\n--- Menu LZW Compression ---" << std::endl;
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
