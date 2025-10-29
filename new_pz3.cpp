#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <cmath>

// пузырьковая сортировка 
void bubble_sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// медианный фильтр 
void median_filter(int filter_size, int height, int width, int** copy_image, int** temp_image) {
    int offset = filter_size / 2;
    int arr_size = filter_size * filter_size;
    int* filter_arr = new int[arr_size];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int index = 0;

            for (int x = -offset; x <= offset; x++) {
                for (int y = -offset; y <= offset; y++) {
                    int nx = i + x;
                    int ny = j + y;
                    if (nx >= 0 && nx < height && ny >= 0 && ny < width) {
                        filter_arr[index] = copy_image[nx][ny];
                        index++;
                    }
                }
            }

            bubble_sort(filter_arr, index);
            temp_image[i][j] = filter_arr[index / 2];
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            copy_image[i][j] = temp_image[i][j];
        }
    }

    delete[] filter_arr;
}


double image_comp(int** image, int** copy_image, int height, int width) {
    double size_pixel = height * width;
    double matching_pixels = 0.0;
    double result;
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            if(abs(image[i][j] - copy_image[i][j]) <= 10) {
                matching_pixels++;
            }
        }
    }
    result = matching_pixels / size_pixel * 100;

    return round(result);
}

// чтение PGM файла 
bool read_pgm(const std::string& filename, int**& image, int& width, int& height, int& max_value) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) return false;

    std::string format;
    file >> format;
    if (format != "P2") return false;

    while (file.peek() == '#' || file.peek() == '\n' || file.peek() == '\r')
        file.ignore(1000, '\n');

    file >> width >> height >> max_value;

    image = new int*[height];
    for (int i = 0; i < height; i++)
        image[i] = new int[width];

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            file >> image[i][j];

    file.close();
    return true;
}

// сохранение PGM 
void save_pgm(const std::string& filename, int** image, int width, int height, int max_value) {
    std::ofstream file(filename.c_str());
    file << "P2\n" << width << " " << height << "\n" << max_value << "\n";
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            file << image[i][j] << " ";
        }
        file << "\n";
    }
    file.close();
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::ofstream csv("results.csv");
    csv << "Файл,Фильтр,PSNR\n";


    const int num_files = 5;
    std::string files[num_files] = {"kasum.pgm", "mqeen.pgm", "kasum2.pgm", "mai.pgm", "makan.pgm"};
    int filter_sizes[3] = {3, 5, 7};

    srand(time(NULL));

    for (int f = 0; f < num_files; f++) {
        std::string filename = files[f];
        std::cout << "\nОбрабатываю файл: " << filename << std::endl;

        int width = 0, height = 0, max_value = 0;
        int** image = nullptr;

        if (!read_pgm(filename, image, width, height, max_value)) {
            std::cout << "Ошибка чтения файла " << filename << std::endl;
            continue;
        }

        int** copy_image = new int*[height];
        int** temp_image = new int*[height];
        for (int i = 0; i < height; i++) {
            copy_image[i] = new int[width];
            temp_image[i] = new int[width];
            for (int j = 0; j < width; j++) {
                copy_image[i][j] = image[i][j];
            }
        }

        // добавляем шум
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                double prob = (rand() % 1000) / 1000.0;
                if (prob > 0.9) { 
                    copy_image[i][j] = rand() % max_value;
                }
            }
        }

        save_pgm("NOISE_" + filename, copy_image, width, height, max_value);

        // фильтруем шумы
        for (int k = 0; k < 3; k++) {
            int size = filter_sizes[k];
            median_filter(size, height, width, copy_image, temp_image);

            std::string out_name = "FILTERED_" + std::to_string(size) + "x" + std::to_string(size) + "_" + filename;
            save_pgm(out_name, copy_image, width, height, max_value);

            double similarity = image_comp(image, copy_image, height, width);
            std::cout << "Фильтр " << size << "x" << size << ": similarity = " << similarity << " %" << std::endl;
            csv << filename << "," << size << "x" << size << "," << similarity << " %" << "\n";
        }

        // освобождаем память
        for (int i = 0; i < height; i++) {
            delete[] image[i];
            delete[] copy_image[i];
            delete[] temp_image[i];
        }
        delete[] image;
        delete[] copy_image;
        delete[] temp_image;
    }

    csv.close();
    std::cout << "\nРезультаты сохранены в results.csv\n";
    return 0;
}
