#include "KeyboardView.h"

// Конструктор
KeyboardView::KeyboardView()
    : keySizes(InitializeKeySizes()),
    keyHeight(InitializeKeyHeight()),
    normalKeyWidth(InitializeNormalKeyWidth()),
    rowHeight(0), // Инициализация rowHeight нулем
    padding(0)
{}

// Метод для вычисления rowHeight
void KeyboardView::CalculateRowHeight(int height) {
    int maxHeight = 0;
    for (const auto& row : keyHeight) {
        maxHeight += row[0];
    }
    rowHeight = height / maxHeight; // Вычисляем rowHeight
    padding = height % maxHeight;

}

// Метод для инициализации keySizes
std::array<std::vector<bool>, KeyboardView::numbOfRows> KeyboardView::InitializeKeySizes() const {
    std::array<std::vector<bool>, numbOfRows> result;
    result[0] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // Верхний ряд (13 элементов)
    result[1] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // Второй ряд (14 элементов)
    result[2] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // Третий ряд (14 элементов)
    result[3] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // Четвертый ряд (13 элементов)
    result[4] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // Пятый ряд (13 элементов)
    result[5] = { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1 }; // Шестой ряд (10 элементов, пробел - увеличенный)
    return result;
}

// Метод для инициализации keyHeight
std::vector<std::vector<int>> KeyboardView::InitializeKeyHeight() const {
    return {
        { 1 }, // Верхний ряд (13 элементов)
        { 2 }, // Второй ряд (14 элементов)
        { 2 }, // Третий ряд (14 элементов)
        { 2 }, // Четвертый ряд (13 элементов)
        { 2 }, // Пятый ряд (13 элементов)
        { 2, 2, 2, 2, 2, 2, 2, 1, 1, -1, 1 } // Шестой ряд (10 элементов, пробел - увеличенный)
    };
}

// Метод для инициализации normalKeyWidth
std::array<int, KeyboardView::numbOfRows> KeyboardView::InitializeNormalKeyWidth() const {
    return { 13, 14, 14, 14, 14, 14 };
}