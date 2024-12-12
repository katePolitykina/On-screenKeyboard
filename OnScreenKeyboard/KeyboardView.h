#pragma once
#include <array>
#include <vector>
#include <cmath>

class KeyboardView {
public:
    // Конструктор
    KeyboardView();

    // Количество рядов
    static constexpr int numbOfRows = 6;

    // Массив с размерами клавиш (0 - увеличенный, 1 - стандартный)
    const std::array<std::vector<bool>, numbOfRows> keySizes;


    // Относительная высота элементов
    // Отрицательное значение высчитывается снизу ряда вверх
    // В каждом ряду минимум 1 элемент = высоте ряда\
    2-n элементы характеризуют высоту конкретной клавиши

    const std::vector<std::vector<int>> keyHeight;

    // Высота ряда
    int rowHeight;
    int padding;

    // Метод для вычисления rowHeight
    void CalculateRowHeight(int height);

    // Стандартная ширина клавиши
    const std::array<int, numbOfRows> normalKeyWidth;

private:
    // Метод для инициализации keySizes
    std::array<std::vector<bool>, numbOfRows> InitializeKeySizes() const;

    // Метод для инициализации keyHeight
    std::vector<std::vector<int>> InitializeKeyHeight() const;

    // Метод для инициализации normalKeyWidth
    std::array<int, numbOfRows> InitializeNormalKeyWidth() const;
};