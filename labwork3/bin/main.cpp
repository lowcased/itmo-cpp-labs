#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#pragma pack(push, 1)  // Выравнивание=1 (без паддингов)

struct BMPHeader {
    uint16_t bfType = 0x4D42;  // 'BM'
    uint32_t bfSize;           // Размер файла в байтах
    uint16_t bfReserved1 = 0;  // Зарезервировано 0
    uint16_t bfReserved2 = 0;  // Зарезервировано 0
    uint32_t bfOffBits = 118;  // Смещение до данных изображения (пикселей)
};

struct DIBHeader {
    uint32_t biSize = 40;  // Размер DIB заголовка
    int32_t biWidth;       // Ширина изображения
    int32_t biHeight;      // Высота изображения
    uint16_t biPlanes = 1;  // Количество цветовых плоскостей
    uint16_t biBitCount = 4;     // 4 бит на цвет
    uint32_t biCompression = 0;  // Без сжатия
    uint32_t biSizeImage;  // Размер изображения в байтах
    int32_t biXPelsPerMeter = 0;  // Горизонтальное разрешение
    int32_t biYPelsPerMeter = 0;  // Вертикальное разрешение
    uint32_t biClrUsed = 16;      // Всего цветов
    uint32_t biClrImportant = 5;  // Количество используемых цветов
};

#pragma pack(pop)

enum ErrorType {
    INPUT_FILE_NOT_FOUND = 1,
    INPUT_FILE_NOT_OPENED,
    INCORRECT_INPUT,
    OUTPUT_FILE_NOT_FOUND,
};

struct Args {
    char* input = nullptr;
    char* output = nullptr;
    int max_iter = -1;
    int freq = 0;
};

struct Node {
    int32_t x;
    int32_t y;
    Node* next;
    Node* prev;
};

struct Stack {
    Node* head = nullptr;

    void push(int32_t xValue, int32_t yValue) {
        Node* newNode = new Node;
        newNode->x = xValue;
        newNode->y = yValue;
        newNode->next = nullptr;
        newNode->prev = head;
        if (head != nullptr) {
            head->next = newNode;
        }
        head = newNode;
    }

    std::pair<int32_t, int32_t> pop() {
        std::pair<uint32_t, uint32_t> coords = {head->x, head->y};
        Node* temp = head;
        head = head->prev;
        if (head != nullptr) {
            head->next = nullptr;
        }
        delete temp;
        return coords;
    }

    bool isEmpty() { return head == nullptr; }
};

struct Row {
    int64_t left_size;
    int64_t right_size;
    uint64_t* data;
    Row() {
        left_size = 10;
        right_size = 10;
        data = new uint64_t[left_size + right_size + 1]();
    }
    void ResizeLeft(uint64_t*& data, int64_t& left_size, int64_t& right_size) {
        left_size *= 2;
        uint64_t* new_data = new uint64_t[left_size + right_size + 1]();
        for (int64_t i = 0; i <= left_size / 2 + right_size; i++) {
            new_data[i + left_size / 2] = data[i];
        }
        delete[] data;
        data = new_data;
    }

    void ResizeRight(uint64_t*& data, int64_t& left_size, int64_t& right_size) {
        right_size *= 2;
        uint64_t* new_data = new uint64_t[left_size + right_size + 1]();
        for (int64_t i = 0; i <= left_size + right_size / 2; i++) {
            new_data[i] = data[i];
        }

        delete[] data;
        data = new_data;
    }

    uint64_t& operator[](int32_t index) {
        if (index >= 0) {
            while (index > right_size) {
                ResizeRight(data, left_size, right_size);
            }
        } else {
            while (-index > left_size) {
                ResizeLeft(data, left_size, right_size);
            }
        }
        return data[index + left_size];
    }
};

struct Grid {
    int64_t top;
    int64_t bottom;
    Row* data;
    Grid(int64_t _b, int64_t _t) {
        bottom = _b;
        top = _t;
        data = new Row[bottom + top + 1];
    }

    void ResizeTop(Row*& data, int64_t& bottom, int64_t& top) {
        top *= 2;
        Row* new_data = new Row[bottom + top + 1]();
        for (int64_t i = 0; i <= bottom + top / 2; i++) {
            new_data[i] = data[i];
        }
        delete[] data;
        data = new_data;
    }

    void ResizeBottom(Row*& data, int64_t& bottom, int64_t& top) {
        bottom *= 2;
        Row* new_data = new Row[bottom + top + 1]();
        for (int64_t i = 0; i <= bottom / 2 + top; i++) {
            new_data[i + bottom / 2] = data[i];
        }
        delete[] data;
        data = new_data;
    }

    Row& operator[](int32_t index) {
        if (index >= 0) {
            while (index > top) {
                ResizeTop(data, bottom, top);
            }
        } else {
            while (-index > bottom) {
                ResizeBottom(data, bottom, top);
            }
        }
        return data[index + bottom];
    }
};

struct SandSize {
    int32_t min_x = INT16_MAX;
    int32_t min_y = INT16_MAX;
    int32_t max_x = INT16_MIN;
    int32_t max_y = INT16_MIN;
};

int saveBMP(char* filename, Grid*& grid, SandSize& sizes, int iter_count) {
    int width;
    int height;
    if (sizes.min_x < 0) {
        if (sizes.max_x < 0) {
            width = -sizes.min_x - sizes.max_x;
        } else {
            width = -sizes.min_x + sizes.max_x;
        }
    } else {
        width = sizes.min_x + sizes.max_x;
    }
    if (sizes.min_y < 0) {
        if (sizes.max_y < 0) {
            height = -sizes.min_y - sizes.max_y;
        } else {
            height = -sizes.min_y + sizes.max_y;
        }
    } else {
        height = sizes.min_y + sizes.max_y;
    }
    width++;
    height++;
    BMPHeader bmpHeader;
    DIBHeader dibHeader;
    int rowSize = (width + 1) / 2;
    rowSize = (rowSize + 3) / 4 * 4;  // Округляем до 4
    int pixelDataSize = rowSize * height;
    bmpHeader.bfSize = 118 + 16 * 4 + pixelDataSize;
    dibHeader.biWidth = width;
    dibHeader.biHeight = height;
    dibHeader.biSizeImage = pixelDataSize;
    uint8_t palette[16][4] = {{255, 255, 255, 0},  // белый
                              {0, 255, 0, 0},      // зеленый
                              {128, 0, 128, 0},    // фиолетовый
                              {255, 255, 0, 0},    // желтый
                              {0, 0, 0, 0},        // черный
                              };
    char buf[15];
    sprintf(buf, "%d.bmp", iter_count);
    int length = strlen(filename);
    filename = strcat(filename, buf);
    std::ofstream file(filename, std::ios::binary);
    filename[length] = '\0';
    if (!file) {
        return ErrorType::OUTPUT_FILE_NOT_FOUND; 
    }
    file.write(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    file.write(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));
    file.write(reinterpret_cast<char*>(palette), sizeof(palette));

    uint8_t** pixels = new uint8_t*[height];
    for (int i = 0; i < height; i++) {
        pixels[i] = new uint8_t[rowSize]();
    }
    int pixel_x = -sizes.min_x, pixel_y = -sizes.min_y;
    for (int y = sizes.min_y; y <= sizes.max_y; y++) {
        for (int x = sizes.min_x; x <= sizes.max_x; x++) {
            uint64_t value = (*grid)[y][x];
            if (value > 4) {
                value = 4;
            }
            uint8_t color = static_cast<uint8_t>(value);
            pixels[y + pixel_y][(x + pixel_x) / 2] |=
                (color << (4 * (1 - ((x + pixel_x) % 2))));
        }
    }
    for (int y = height - 1; y >= 0; y--) {
        file.write(reinterpret_cast<char*>(pixels[y]), rowSize);
    }
    for (int i = 0; i < height; ++i) {
        delete[] pixels[i];
    }
    delete[] pixels;
    file.close();
    return 0;
}

int ParseArgs(Args& args, int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        char* parameter = strchr(argv[i], '=');
        if (parameter != nullptr) {
            *parameter = '\0';
            parameter += 1;
        } else if (i + 1 < argc) {
            parameter = argv[i + 1];
        }
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (parameter) {
                args.input = parameter;
            } else {
                return ErrorType::INPUT_FILE_NOT_FOUND;  // не указан файл с
                                                         // входными данными
            }
        } else if (strcmp(argv[i], "-o") == 0 ||
                   strcmp(argv[i], "--output") == 0) {
            if (parameter) {
                args.output = parameter;
            }
        } else if (strcmp(argv[i], "-m") == 0 ||
                   strcmp(argv[i], "--max-iter") == 0) {
            if (parameter) {
                args.max_iter = std::stoi(parameter);
            }
        } else if (strcmp(argv[i], "-f") == 0 ||
                   strcmp(argv[i], "--freq") == 0) {
            if (parameter) {
                args.freq = std::stoi(parameter);
            }
        }
    }
    return 0;
}

int ReadFile(char* filename, Grid*& grid, Stack*& drop, SandSize& sizes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return ErrorType::INPUT_FILE_NOT_OPENED;  // не удалось открыть файл
    } else {
        const int buffer_size = 128;
        char buffer[buffer_size];
        int16_t x, y;
        uint64_t sand_count;
        while (fgets(buffer, buffer_size, file)) {
            if (sscanf(buffer, "%hd\t%hd\t%llu", &x, &y, &sand_count) == 3) {
                (*grid)[y][x] = sand_count;
                if (sand_count > 3) {
                    (*drop).push(x, y);
                }
                sizes.max_x = std::max(x, (int16_t)sizes.max_x);
                sizes.min_x = std::min(x, (int16_t)sizes.min_x);
                sizes.max_y = std::max(y, (int16_t)sizes.max_y);
                sizes.min_y = std::min(y, (int16_t)sizes.min_y);
            } else {
                return ErrorType::INCORRECT_INPUT;  
            }
        }
    }
    fclose(file);
    return 0;
}

void iteration(Grid*& grid, Stack*& drop, SandSize& sizes) {
    Stack* new_drop = new Stack;
    while (!(*drop).isEmpty()) {
        std::pair<int32_t, int32_t> coords = (*drop).pop();
        int32_t x = coords.first;
        int32_t y = coords.second;
        sizes.max_x = std::max(x, sizes.max_x);
        sizes.min_x = std::min(x, sizes.min_x);
        sizes.max_y = std::max(y, sizes.max_y);
        sizes.min_y = std::min(y, sizes.min_y);
        if ((*grid)[y][x] > 3) {
            (*grid)[y][x] -= 4;
            (*grid)[y + 1][x]++;
            (*grid)[y - 1][x]++;
            (*grid)[y][x + 1]++;
            (*grid)[y][x - 1]++;
            sizes.max_x = std::max(x + 1, sizes.max_x);
            sizes.min_x = std::min(x - 1, sizes.min_x);
            sizes.max_y = std::max(y + 1, sizes.max_y);
            sizes.min_y = std::min(y - 1, sizes.min_y);
        }
        if ((*grid)[y][x] > 3) {
            (*new_drop).push(x, y);
        }
        if ((*grid)[y + 1][x] == 4) {
            (*new_drop).push(x, y + 1);
        }
        if ((*grid)[y - 1][x] == 4) {
            (*new_drop).push(x, y - 1);
        }
        if ((*grid)[y][x + 1] == 4) {
            (*new_drop).push(x + 1, y);
        }
        if ((*grid)[y][x - 1] == 4) {
            (*new_drop).push(x - 1, y);
        }
    }
    drop = new_drop;
}

int main(int argc, char** argv) {
    Args args;
    int error = ParseArgs(args, argc, argv);
    if (error == 0) {
        if (args.output == nullptr) {
            delete[] args.output;
            std::filesystem::path path =
                std::filesystem::absolute(std::string(argv[0]));
            std::filesystem::path directory = path.parent_path();
            size_t length = directory.native().size();
            args.output = new char[length + 1 + 20];
            for (size_t i = 0; i < length; i++) {
                args.output[i] = directory.native()[i];
            }
            args.output[length] = '\\';
            for (int i = length + 1; i <= length + 20; i++) {
                args.output[i] = '\0';
            }
        }
        Grid* grid = new Grid(10, 10);
        Stack* drop = new Stack;
        SandSize sizes;
        error = ReadFile(args.input, grid, drop, sizes);
        if(error!=0){
            if(error==ErrorType::INPUT_FILE_NOT_OPENED){
                std::cout << "Failed to open the file";
            }
            else{
                std::cout << "Incorrect input";
            }
            return 0;
        }
        int iter_count = 0;
        while ((iter_count < args.max_iter || args.max_iter == -1) &&
               (!(*drop).isEmpty())) {
            iteration(grid, drop, sizes);
            if (args.freq != 0 && iter_count % args.freq == 0) {
                error=saveBMP(args.output, grid, sizes, iter_count);
                if(error!=0){
                    std::cout << "Output file not found";
                    return 0;
                }
            }
            iter_count++;
        }
        if (args.freq == 0) {
            saveBMP(args.output, grid, sizes, 0);
        }
    } else {
        std::cout << "Input file not found";
    }
}