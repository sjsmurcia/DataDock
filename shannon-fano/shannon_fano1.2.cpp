#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <filesystem>

class ShannonFano {
private:
    std::unordered_map<char, std::string> codes;
    std::unordered_map<std::string, char> reverse_codes;

    void build_tree(std::vector<std::pair<char, int>>& frequencies, const std::string& prefix) {
        if (frequencies.size() == 1) {
            codes[frequencies[0].first] = prefix;
            return;
        }

        int total = std::accumulate(frequencies.begin(), frequencies.end(), 0,
                                    [](int sum, const std::pair<char, int>& p) { return sum + p.second; });

        int acc = 0;
        size_t split = 0;
        for (size_t i = 0; i < frequencies.size(); ++i) {
            if (acc + frequencies[i].second > total / 2) {
                break;
            }
            acc += frequencies[i].second;
            split = i;
        }

        auto left = std::vector<std::pair<char, int>>(frequencies.begin(), frequencies.begin() + split + 1);
        auto right = std::vector<std::pair<char, int>>(frequencies.begin() + split + 1, frequencies.end());

        build_tree(left, prefix + "0");
        build_tree(right, prefix + "1");
    }

public:
    void clear_codes() {
        codes.clear();
        reverse_codes.clear();
    }

    std::string compress(const std::string& text) {
        clear_codes();
        std::unordered_map<char, int> freq_map;
        for (char c : text) {
            freq_map[c]++;
        }

        std::vector<std::pair<char, int>> frequencies(freq_map.begin(), freq_map.end());
        std::sort(frequencies.begin(), frequencies.end(), [](const auto& a, const auto& b) {
            return b.second > a.second;
        });

        build_tree(frequencies, "");
        for (const auto& p : codes) {
            reverse_codes[p.second] = p.first;
        }

        std::string compressed;
        for (char c : text) {
            compressed += codes[c];
        }

        return compressed;
    }

    std::string decompress(const std::string& compressed, const std::unordered_map<std::string, char>& loaded_codes) {
        reverse_codes = loaded_codes;

        std::string decoded;
        std::string current;
        for (char bit : compressed) {
            current += bit;
            if (reverse_codes.find(current) != reverse_codes.end()) {
                decoded += reverse_codes[current];
                current.clear();
            }
        }

        return decoded;
    }

    const std::unordered_map<char, std::string>& get_codes() const {
        return codes;
    }
};

void saveCompressedFile(const std::string& compressed, const std::unordered_map<char, std::string>& codes, const std::string& compressedFileName) {
    std::ofstream outFile(compressedFileName, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error al escribir el archivo comprimido." << std::endl;
        return;
    }

    // Guardar el tamaño del mapa de códigos
    size_t mapSize = codes.size();
    outFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    // Guardar los códigos
    for (const auto& pair : codes) {
        outFile.put(pair.first);
        size_t codeLength = pair.second.size();
        outFile.write(reinterpret_cast<const char*>(&codeLength), sizeof(codeLength));
        outFile.write(pair.second.data(), codeLength);
    }

    // Guardar los datos comprimidos
    outFile.write(compressed.data(), compressed.size());
    outFile.close();
}

std::pair<std::unordered_map<std::string, char>, std::string> loadCompressedFile(const std::string& compressedFileName) {
    std::ifstream inFile(compressedFileName, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error al leer el archivo comprimido." << std::endl;
        return {{}, ""};
    }

    std::unordered_map<std::string, char> loaded_codes;

    // Leer el tamaño del mapa de códigos
    size_t mapSize;
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    // Leer los códigos
    for (size_t i = 0; i < mapSize; ++i) {
        char character = inFile.get();
        size_t codeLength;
        inFile.read(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        std::string code(codeLength, ' ');
        inFile.read(&code[0], codeLength);
        loaded_codes[code] = character;
    }

    // Leer los datos comprimidos
    std::string compressed((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    return {loaded_codes, compressed};
}

void compressFile(const std::string& inputFileName, const std::string& compressedFileName) {
    ShannonFano sf;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "No se pudo abrir el archivo original." << std::endl;
        return;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    std::string compressed = sf.compress(text);
    auto end = std::chrono::high_resolution_clock::now();

    saveCompressedFile(compressed, sf.get_codes(), compressedFileName);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    size_t originalSize = text.size();
    size_t compressedSize = compressed.size() / 8 + sf.get_codes().size() * sizeof(char); // Estimación del tamaño comprimido

    double compressionRate = 1.0 - static_cast<double>(compressedSize) / originalSize;

    std::cout << "Archivo comprimido en: " << compressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
    std::cout << "Tasa de compresion: " << (compressionRate * 100) << "%" << std::endl;
}

void decompressFile(const std::string& compressedFileName, const std::string& decompressedFileName) {
    ShannonFano sf;
    auto [loaded_codes, compressed] = loadCompressedFile(compressedFileName);
    if (compressed.empty()) {
        std::cerr << "Error al cargar el archivo comprimido." << std::endl;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();
    std::string decompressed = sf.decompress(compressed, loaded_codes);
    auto end = std::chrono::high_resolution_clock::now();

    std::ofstream decompressedFile(decompressedFileName);
    decompressedFile << decompressed;
    decompressedFile.close();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Archivo descomprimido en: " << decompressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
}

int main() {
    std::string inputFileName;
    std::string compressedFileName;
    std::string decompressedFileName;

    while (true) {
        std::cout << "\n--- Menu ---" << std::endl;
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
