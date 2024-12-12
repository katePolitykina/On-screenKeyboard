#pragma once
#include <array>
#include <vector>
#include <cmath>

class KeyboardView {
public:
    // �����������
    KeyboardView();

    // ���������� �����
    static constexpr int numbOfRows = 6;

    // ������ � ��������� ������ (0 - �����������, 1 - �����������)
    const std::array<std::vector<bool>, numbOfRows> keySizes;


    // ������������� ������ ���������
    // ������������� �������� ������������� ����� ���� �����
    // � ������ ���� ������� 1 ������� = ������ ����\
    2-n �������� ������������� ������ ���������� �������

    const std::vector<std::vector<int>> keyHeight;

    // ������ ����
    int rowHeight;
    int padding;

    // ����� ��� ���������� rowHeight
    void CalculateRowHeight(int height);

    // ����������� ������ �������
    const std::array<int, numbOfRows> normalKeyWidth;

private:
    // ����� ��� ������������� keySizes
    std::array<std::vector<bool>, numbOfRows> InitializeKeySizes() const;

    // ����� ��� ������������� keyHeight
    std::vector<std::vector<int>> InitializeKeyHeight() const;

    // ����� ��� ������������� normalKeyWidth
    std::array<int, numbOfRows> InitializeNormalKeyWidth() const;
};