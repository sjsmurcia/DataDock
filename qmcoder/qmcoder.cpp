#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <chrono>

class QMCoder {
private:
    std::map<char, double> probabilities;
    std::map<char, std::pair<double, double>> ranges;

    void calculateProbabilities(const std::string& text) {
        std::map<char, int> frequency;
        for (char c : text) {
            frequency[c]++;
        }

        double total = text.size();
        double cumulative = 0.0;

        for (const auto& pair : frequency) {
            double prob = pair.second / total;
            probabilities[pair.first] = prob;
            ranges[pair.first] = {cumulative, cumulative + prob};
            cumulative += prob;
        }
    }

    void loadRanges(const std::map<char, double>& loadedProbabilities) {
        double cumulative = 0.0;
        probabilities = loadedProbabilities;

        for (const auto& pair : probabilities) {
            ranges[pair.first] = {cumulative, cumulative + pair.second};
            cumulative += pair.second;
        }
    }

public:
    std::pair<std::string, std::map<char, double>> compress(const std::string& text) {
        calculateProbabilities(text);

        double low = 0.0;
        double high = 1.0;

        for (char c : text) {
            double range = high - low;
            high = low + range * ranges[c].second;
            low = low + range * ranges[c].first;
        }

        double encodedValue = (low + high) / 2;
        std::ostringstream oss;
        oss << std::setprecision(16) << encodedValue; // Alta precisión
        return {oss.str(), probabilities};
    }

    std::string decompress(const std::string& compressed, const std::map<char, double>& loadedProbabilities, size_t originalSize) {
        loadRanges(loadedProbabilities);

        double value = std::stod(compressed);
        std::string decoded;

        for (size_t i = 0; i < originalSize; ++i) {
            for (const auto& pair : ranges) {
                if (value >= pair.second.first && value < pair.second.second) {
                    decoded += pair.first;
                    double range = pair.second.second - pair.second.first;
                    value = (value - pair.second.first) / range;
                    break;
                }
            }
        }

        return decoded;
    }
};

void compressFile(const std::string& inputFileName, const std::string& compressedFileName) {
    QMCoder qm;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo original." << std::endl;
        return;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    auto [compressed, probabilities] = qm.compress(text);
    auto end = std::chrono::high_resolution_clock::now();

    std::ofstream compressedFile(compressedFileName, std::ios::binary);
    if (!compressedFile.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo comprimido." << std::endl;
        return;
    }

    compressedFile << compressed << "\n";

    size_t mapSize = probabilities.size();
    compressedFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    for (const auto& pair : probabilities) {
        compressedFile.put(pair.first);
        double probability = pair.second;
        compressedFile.write(reinterpret_cast<const char*>(&probability), sizeof(probability));
    }

    size_t originalSize = text.size();
    compressedFile.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));
    compressedFile.close();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Archivo comprimido en: " << compressedFileName << " (Tiempo: " << duration << " ms)" << std::endl;
}

void decompressFile(const std::string& compressedFileName, const std::string& decompressedFileName) {
    QMCoder qm;
    std::ifstream compressedFile(compressedFileName, std::ios::binary);
    if (!compressedFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo comprimido." << std::endl;
        return;
    }

    std::string compressed;
    std::getline(compressedFile, compressed);

    size_t mapSize;
    compressedFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    std::map<char, double> probabilities;
    for (size_t i = 0; i < mapSize; ++i) {
        char character = compressedFile.get();
        double probability;
        compressedFile.read(reinterpret_cast<char*>(&probability), sizeof(probability));
        probabilities[character] = probability;
    }

    size_t originalSize;
    compressedFile.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));
    compressedFile.close();

    auto start = std::chrono::high_resolution_clock::now();
    std::string decompressed = qm.decompress(compressed, probabilities, originalSize);
    auto end = std::chrono::high_resolution_clock::now();

    if (decompressed.empty()) {
        std::cerr << "Error: La descompresión resultó en un texto vacío." << std::endl;
        return;
    }

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
    std::string inputFileName;
    std::string compressedFileName;
    std::string decompressedFileName;

    while (true) {
        std::cout << "\n--- Menu QM Coder ---" << std::endl;
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
