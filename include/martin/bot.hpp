#pragma once
#include <array>

namespace martin
{
    constexpr char EMPTY = '\0';

    constexpr char WHITE_PAWN ='P';
    constexpr char WHITE_KNIGHT ='N';
    constexpr char WHITE_BISHOP ='B';
    constexpr char WHITE_ROOK ='R';
    constexpr char WHITE_QUEEN ='Q';
    constexpr char WHITE_KING ='K';

    constexpr char BLACK_PAWN = 'p';
    constexpr char BLACK_KNIGHT = 'n';
    constexpr char BLACK_BISHOP = 'b';
    constexpr char BLACK_ROOK = 'r';
    constexpr char BLACK_QUEEN = 'q';
    constexpr char BLACK_KING = 'k';

    constexpr bool is_white(char piece) noexcept {
        return piece >= 'A' && piece <= 'Z';
    }

    constexpr bool is_black(char piece) noexcept {
        return piece >= 'a' && piece <= 'z';
    }

    /// @brief Move by modifying board
    /// @return int Pieces captures
    int move(std::array<std::array<char, 8>, 8> &board);
    /// @brief Returns true if the specified board is valid
    ///        if not then returns false
    bool check(const std::array<std::array<char, 8>, 8> &board);
}
