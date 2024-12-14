#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <numeric>
#include <chrono>

class ShannonFano
{
private:
    std::unordered_map<char, std::string> codes;
    std::unordered_map<std::string, char> reverse_codes;

    void build_tree(std::vector<std::pair<char, int>> &frequencies, const std::string &current_code = "")
    {
        if (frequencies.size() == 1)
        {
            codes[frequencies[0].first] = current_code + "0"; 
            return;
        }

        int total = std::accumulate(frequencies.begin(), frequencies.end(), 0,
            [](int sum, const std::pair<char, int> &p)
            { return sum + p.second; });

        int acc = 0;
        size_t split = 0;
        for (size_t i = 0; i < frequencies.size(); ++i)
        {
            if (acc + frequencies[i].second > total / 2)
            {
                break;
            }
            acc += frequencies[i].second;
            split = i;
        }

        auto left = std::vector<std::pair<char, int>>(frequencies.begin(), frequencies.begin() + split + 1);
        auto right = std::vector<std::pair<char, int>>(frequencies.begin() + split + 1, frequencies.end());

        build_tree(left, current_code + "0");
        build_tree(right, current_code + "1");
    }

public:
    std::string compress(const std::string &text)
    {
        if (text.empty())
        {
            std::cerr << "El texto está vacío." << std::endl;
            return "";
        }

        std::unordered_map<char, int> freq_map;
        for (char c : text)
        {
            freq_map[c]++;
        }

        std::vector<std::pair<char, int>> frequencies(freq_map.begin(), freq_map.end());
        std::sort(frequencies.begin(), frequencies.end(), [](const auto &a, const auto &b)
                  { return b.second < a.second; });

        build_tree(frequencies);
        for (const auto &p : codes)
        {
            reverse_codes[p.second] = p.first;
        }

        std::string compressed;
        for (char c : text)
        {
            compressed += codes[c];
        }

        return compressed;
    }

    std::string decompress(const std::string &compressed)
    {
        if (compressed.empty())
        {
            std::cerr << "El texto está vacío." << std::endl;
            return "";
        }

        std::string decoded;
        std::string current;
        for (char bit : compressed)
        {
            current += bit;
            if (reverse_codes.find(current) != reverse_codes.end())
            {
                decoded += reverse_codes[current];
                current.clear();
            }
        }

        return decoded;
    }
};

void processShannonFano(const std::string &inputFileName, const std::string &compressedFileName, const std::string &decompressedFileName, bool compress)
{
    ShannonFano sf;

    if (compress)
    {
        // Lee el archivo de texto original
        std::ifstream inputFile(inputFileName);
        if (!inputFile.is_open())
        {
            std::cerr << "No se pudo abrir el archivo de entrada." << std::endl;
            return;
        }

        std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        inputFile.close();

        if (text.empty())
        {
            std::cerr << "El archivo de entrada está vacío." << std::endl;
            return;
        }

        // Comprimir el contenido
        auto startCompress = std::chrono::high_resolution_clock::now();
        std::string compressed = sf.compress(text);
        auto endCompress = std::chrono::high_resolution_clock::now();

        if (compressed.empty())
        {
            std::cerr << "Error durante la compresión." << std::endl;
            return;
        }

        // Guardar el archivo comprimido
        std::ofstream compressedFile(compressedFileName, std::ios::binary); 
        if (!compressedFile.is_open())
        {
            std::cerr << "No se pudo abrir el archivo comprimido para escribir." << std::endl;
            return;
        }
        compressedFile << compressed;
        compressedFile.close();

        // Calcular la tasa de compresión
        double compressionRate = (double)compressed.size() / (text.size() * 8) * 100.0;
        std::cout << "Compresión completada. \nTasa de compresión: " << compressionRate << "%" << std::endl;
        std::cout << "Tiempo de compresión: " << std::chrono::duration_cast<std::chrono::milliseconds>(endCompress - startCompress).count() << " ms." << std::endl;
    }
    else
    {
        // Lee el archivo comprimido
        std::ifstream compressedFile(compressedFileName, std::ios::binary); 
        if (!compressedFile.is_open())
        {
            std::cerr << "No se pudo abrir el archivo comprimido." << std::endl;
            return;
        }

        std::string compressed((std::istreambuf_iterator<char>(compressedFile)), std::istreambuf_iterator<char>());
        compressedFile.close();

        if (compressed.empty())
        {
            std::cerr << "El archivo comprimido está vacío." << std::endl;
            return;
        }

        // Descomprimir el contenido
        auto startDecompress = std::chrono::high_resolution_clock::now();
        std::string decompressed = sf.decompress(compressed);
        auto endDecompress = std::chrono::high_resolution_clock::now();

        if (decompressed.empty())
        {
            std::cerr << "Error durante la descompresión." << std::endl;
            return;
        }

        // Guardar el archivo descomprimido
        std::ofstream outputFile(decompressedFileName);
        if (!outputFile.is_open())
        {
            std::cerr << "No se pudo abrir el archivo descomprimido para escribir." << std::endl;
            return;
        }
        outputFile << decompressed;
        outputFile.close();

        std::cout << "Descompresión completada." << std::endl;
        std::cout << "Tiempo de descompresión: " << std::chrono::duration_cast<std::chrono::milliseconds>(endDecompress - startDecompress).count() << " ms." << std::endl;
    }
}

int main()
{
    system("chcp 65001");

    setlocale(LC_ALL, "es_ES.UTF-8");

    int choice;
    std::string inputFileName, compressedFileName, decompressedFileName;

    while (true)
    {
        std::cout << "\nSeleccione una opción:\n1. Comprimir archivo\n2. Descomprimir archivo\n3. Salir\nOpción: ";
        std::cin >> choice;

        if (choice == 1)
        {
            std::cout << "Ingrese el nombre del archivo de entrada: ";
            std::cin >> inputFileName;
            std::cout << "Ingrese el nombre del archivo comprimido (con extensión .compressed): ";
            std::cin >> compressedFileName;
            processShannonFano(inputFileName, compressedFileName, "", true);
        }
        else if (choice == 2)
        {
            std::cout << "Ingrese el nombre del archivo comprimido (con extensión .compressed): ";
            std::cin >> compressedFileName;
            std::cout << "Ingrese el nombre del archivo descomprimido: ";
            std::cin >> decompressedFileName;
            processShannonFano("", compressedFileName, decompressedFileName, false);
        }
        else if (choice == 3)
        {
            break; 
        }
        else
        {
            std::cerr << "Opción no válida." << std::endl;
        }
    }

    return 0;
}
