#include "KeyboardView.h"

// �����������
KeyboardView::KeyboardView()
    : keySizes(InitializeKeySizes()),
    keyHeight(InitializeKeyHeight()),
    normalKeyWidth(InitializeNormalKeyWidth()),
    rowHeight(0), // ������������� rowHeight �����
    padding(0)
{}

// ����� ��� ���������� rowHeight
void KeyboardView::CalculateRowHeight(int height) {
    int maxHeight = 0;
    for (const auto& row : keyHeight) {
        maxHeight += row[0];
    }
    rowHeight = height / maxHeight; // ��������� rowHeight
    padding = height % maxHeight;

}

// ����� ��� ������������� keySizes
std::array<std::vector<bool>, KeyboardView::numbOfRows> KeyboardView::InitializeKeySizes() const {
    std::array<std::vector<bool>, numbOfRows> result;
    result[0] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // ������� ��� (13 ���������)
    result[1] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // ������ ��� (14 ���������)
    result[2] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // ������ ��� (14 ���������)
    result[3] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // ��������� ��� (13 ���������)
    result[4] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0 }; // ����� ��� (13 ���������)
    result[5] = { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1 }; // ������ ��� (10 ���������, ������ - �����������)
    return result;
}

// ����� ��� ������������� keyHeight
std::vector<std::vector<int>> KeyboardView::InitializeKeyHeight() const {
    return {
        { 1 }, // ������� ��� (13 ���������)
        { 2 }, // ������ ��� (14 ���������)
        { 2 }, // ������ ��� (14 ���������)
        { 2 }, // ��������� ��� (13 ���������)
        { 2 }, // ����� ��� (13 ���������)
        { 2, 2, 2, 2, 2, 2, 2, 1, 1, -1, 1 } // ������ ��� (10 ���������, ������ - �����������)
    };
}

// ����� ��� ������������� normalKeyWidth
std::array<int, KeyboardView::numbOfRows> KeyboardView::InitializeNormalKeyWidth() const {
    return { 13, 14, 14, 14, 14, 14 };
}