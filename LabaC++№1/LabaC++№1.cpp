#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* file_path = argv[1];
    const std::vector<uint8_t> data = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};

    // Создание и запись файла
    std::ofstream out_file(file_path, std::ios::binary);
    if (!out_file) {
        std::cerr << "Failed to create file: " << file_path << std::endl;
        return EXIT_FAILURE;
    }

    out_file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!out_file) {
        std::cerr << "Failed to write all data" << std::endl;
        return EXIT_FAILURE;
    }
    out_file.close();

    // Чтение файла с побайтовым выводом
    std::ifstream in_file(file_path, std::ios::binary);
    if (!in_file) {
        std::cerr << "Failed to open file for reading: " << file_path << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Reading file byte by byte:" << std::endl;
    std::cout << "================================" << std::endl;

    char byte;
    size_t pos = 0;
    while (in_file.get(byte)) {
        std::cout << "Byte " << std::setw(2) << pos++ << ": " 
                  << static_cast<int>(static_cast<uint8_t>(byte)) 
                  << " (0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(static_cast<uint8_t>(byte)) << ")" << std::dec << std::endl;
        
        // Вывод состояния потока
        std::cout << "  Stream state:" << std::endl;
        std::cout << "    eof flag: " << in_file.eof() << std::endl;
        std::cout << "    fail flag: " << in_file.fail() << std::endl;
        std::cout << "    bad flag: " << in_file.bad() << std::endl;
        std::cout << "    Current position: " << in_file.tellg() << std::endl;
        std::cout << "  -------------------------" << std::endl;
    }

    std::cout << "\nEnd of file reached. eof: " << in_file.eof() << std::endl;
    in_file.close();

    // Повторное открытие и seekg
    std::cout << "\n=== Second read with seekg ===" << std::endl;
    in_file.open(file_path, std::ios::binary);
    if (!in_file) {
        std::cerr << "Failed to open file for second reading" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Position before seekg: " << in_file.tellg() << std::endl;
    
    in_file.seekg(3, std::ios::beg);
    if (!in_file) {
        std::cerr << "seekg failed" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "Position after seekg to 3: " << in_file.tellg() << std::endl;

    std::vector<uint8_t> buffer(4);
    in_file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    std::streamsize bytes_read = in_file.gcount();

    std::cout << "\nResult of read:" << std::endl;
    std::cout << "  Bytes requested: " << buffer.size() << std::endl;
    std::cout << "  Bytes actually read: " << bytes_read << std::endl;
    std::cout << "  Buffer contents: ";
    
    if (bytes_read > 0) {
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            std::cout << static_cast<int>(buffer[i]) << " ";
        }
        std::cout << std::endl;
        
        // Объяснение результата
        std::cout << "\nAnswer: The buffer contains ";
        for (std::streamsize i = 0; i < bytes_read; ++i) {
            std::cout << static_cast<int>(buffer[i]);
            if (i < bytes_read - 1) std::cout << ", ";
        }
        std::cout << " (bytes at positions 3, 4, 5, 6 in the file)" << std::endl;
    } else {
        std::cout << "No bytes read" << std::endl;
    }

    std::cout << "\nState after read:" << std::endl;
    std::cout << "  Current position: " << in_file.tellg() << std::endl;
    std::cout << "  eof flag: " << in_file.eof() << std::endl;
    std::cout << "  fail flag: " << in_file.fail() << std::endl;

    in_file.close();
    
    return EXIT_SUCCESS;
}