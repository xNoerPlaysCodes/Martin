#pragma once
#include <bot.hpp>
#include <iostream>
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
    void move(std::array<std::array<int, 8>, 8> &board);
}
