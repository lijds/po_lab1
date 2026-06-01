#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <windows.h>

// Функція для генерації випадкової квадратної матриці
void generateMatrix(std::vector<std::vector<int>>& matrix, size_t N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 99);

    matrix.assign(N, std::vector<int>(N));
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
}

// Послідовний алгоритм
void processSequential(std::vector<std::vector<int>>& matrix, size_t N) {
    for (size_t i = 0; i < N; ++i) {
        size_t max_j = 0;
        for (size_t j = 1; j < N; ++j) {
            if (matrix[i][j] > matrix[i][max_j]) {
                max_j = j;
            }
        }
        std::swap(matrix[i][i], matrix[i][max_j]);
    }
}

// Функція для потоку
void processRowsWorker(std::vector<std::vector<int>>& matrix, size_t N, size_t start_row, size_t end_row) {
    for (size_t i = start_row; i < end_row; ++i) {
        size_t max_j = 0;
        for (size_t j = 1; j < N; ++j) {
            if (matrix[i][j] > matrix[i][max_j]) {
                max_j = j;
            }
        }
        std::swap(matrix[i][i], matrix[i][max_j]);
    }
}

// Паралельний алгоритм
void processParallel(std::vector<std::vector<int>>& matrix, size_t N, size_t num_threads) {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    size_t rows_per_thread = N / num_threads;
    size_t remainder = N % num_threads;
    size_t start_row = 0;

    for (size_t i = 0; i < num_threads; ++i) {
        size_t current_rows = rows_per_thread + (i < remainder ? 1 : 0);
        size_t end_row = start_row + current_rows;

        if (current_rows > 0) {
            threads.emplace_back(processRowsWorker, std::ref(matrix), N, start_row, end_row);
        }
        start_row = end_row;
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

// Функція для виведення матриці
void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const auto& row : matrix) {
        for (int val : row) {
            std::cout << std::setw(4) << val;
        }
        std::cout << "\n";
    }
    std::cout << std::string(30, '-') << "\n";
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    const size_t N = 1000;            // Розмірність матриці N x N
    const size_t THREADS_COUNT = 8;   // Кількість потоків

    std::cout << "Тестування для " << N << " x " << N  << std::endl;

    std::vector<std::vector<int>> matrix_seq;
    std::vector<std::vector<int>> matrix_par;

    std::cout << "Генерація матриці" << std::endl;
    generateMatrix(matrix_seq, N);
    matrix_par = matrix_seq;

    // Тест послідовного варіанту
    auto start = std::chrono::high_resolution_clock::now();
    processSequential(matrix_seq, N);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> seq_time = end - start;
    std::cout << "Послідовний час: " << seq_time.count() << " ms\n";

    // Тест паралельного варіанту
    start = std::chrono::high_resolution_clock::now();
    processParallel(matrix_par, N, THREADS_COUNT);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> par_time = end - start;
    std::cout << "Паралельний час (" << THREADS_COUNT << " потоки): " << par_time.count() << " ms\n";

    // Розрахунок прискорення
    std::cout << "Прискорення: " << seq_time.count() / par_time.count() << "x\n";

    // printMatrix(matrix_par);

    return 0;
}