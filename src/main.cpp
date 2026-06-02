#include <array>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <raylib.h>
#include <string>
#include <thread>
#include <unordered_map>

#include <martin/bot.hpp>

static std::uint32_t float_bits(float f)
{
    if (f == 0.0f) f = 0.0f; // normalize -0 -> 0
    return std::bit_cast<std::uint32_t>(f);
}

std::size_t hash_combine(float a, float b)
{
    std::size_t h1 = float_bits(a);
    std::size_t h2 = float_bits(b);

    return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
}

template<>
struct std::hash<Vector2> 
{
    size_t operator()(Vector2 v2) const {
        return hash_combine(v2.x, v2.y);
    }
};

std::unordered_map<Vector2, Vector2> scales = {
    { {320, 180}, {32, 32} },
    { {640, 360}, {48, 48} },
    { {1280, 720}, {64, 64} },
    { {1920, 1080}, {96, 96} },
    { {2560, 1440}, {128, 128} }
};

static float dist2(Vector2 a, Vector2 b) 
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

Vector2 get_square_size(Vector2 screen_size)
{
    Vector2 best_size = {64, 64};
    float best_dist = std::numeric_limits<float>::max();

    for (const auto& [res, size] : scales) {
        float d = dist2(screen_size, res);
        if (d < best_dist) {
            best_dist = d;
            best_size = size;
        }
    }

    return best_size;
}

// for unordered_map :sob:
bool operator==(Vector2 first, Vector2 second) noexcept
{
    return first.x == second.x && first.y == second.y;
}

// who designed one pass parsing bro
extern void (*g_current_draw_function)(void);

enum class GameMode
{
    null = 0,
    PlayerVsBot,
    BotVsBot
};

struct GameOptions
{
    GameMode mode = GameMode::null;
};

struct Player
{
    std::function<int(std::array<std::array<char, 8>, 8>&)> move = nullptr;
    int captured = 0;
};

struct GameState
{
    GameOptions game_options;
    Player bot;
    Player human;
    std::array<std::array<char, 8>, 8> board;
};

GameState g_game_state;
Sound place_sound_effect;
Sound place_sound_effect_bot;

void draw_credits()
{}

Texture2D get_piece_texture(std::string piece)
{
    static Texture2D tx_bbishop = LoadTexture("assets/bbishop.png");
    static Texture2D tx_bking = LoadTexture("assets/bking.png");
    static Texture2D tx_bknight = LoadTexture("assets/bknight.png");
    static Texture2D tx_bpawn = LoadTexture("assets/bpawn.png");
    static Texture2D tx_bqueen = LoadTexture("assets/bqueen.png");
    static Texture2D tx_brook = LoadTexture("assets/brook.png");
    static Texture2D tx_wbishop = LoadTexture("assets/wbishop.png");
    static Texture2D tx_wking = LoadTexture("assets/wking.png");
    static Texture2D tx_wknight = LoadTexture("assets/wknight.png");
    static Texture2D tx_wpawn = LoadTexture("assets/wpawn.png");
    static Texture2D tx_wqueen = LoadTexture("assets/wqueen.png");
    static Texture2D tx_wrook = LoadTexture("assets/wrook.png");

    if (piece == "bbishop")
    {
        return tx_bbishop;
    }

    else if (piece == "bking")
    {
        return tx_bking;
    }

    else if (piece == "bknight")
    {
        return tx_bknight;
    }

    else if (piece == "bpawn")
    {
        return tx_bpawn;
    }

    else if (piece == "bqueen")
    {
        return tx_bqueen;
    }

    else if (piece == "brook")
    {
        return tx_brook;
    }

    else if (piece == "wbishop")
    {
        return tx_wbishop;
    }

    else if (piece == "wking")
    {
        return tx_wking;
    }

    else if (piece == "wknight")
    {
        return tx_wknight;
    }

    else if (piece == "wpawn")
    {
        return tx_wpawn;
    }

    else if (piece == "wqueen")
    {
        return tx_wqueen;
    }

    else if (piece == "wrook")
    {
        return tx_wrook;
    }

    return {};
}

void draw_game()
{
    Vector2 square_size = get_square_size({GetScreenWidth() * 1.f, GetScreenHeight() * 1.f});
    float piece_width = square_size.x;
    float piece_height = square_size.y;

    static auto _ = []() -> bool {
        g_game_state.board = std::array<std::array<char, 8>, 8>{{
            {{
                martin::BLACK_ROOK,   martin::BLACK_KNIGHT, martin::BLACK_BISHOP, martin::BLACK_QUEEN,
                martin::BLACK_KING,   martin::BLACK_BISHOP, martin::BLACK_KNIGHT, martin::BLACK_ROOK
            }},
            {{
                martin::BLACK_PAWN, martin::BLACK_PAWN, martin::BLACK_PAWN, martin::BLACK_PAWN,
                martin::BLACK_PAWN, martin::BLACK_PAWN, martin::BLACK_PAWN, martin::BLACK_PAWN
            }},
            {{
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY,
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY
            }},
            {{
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY,
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY
            }},
            {{
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY,
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY
            }},
            {{
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY,
                martin::EMPTY, martin::EMPTY, martin::EMPTY, martin::EMPTY
            }},
            {{
                martin::WHITE_PAWN, martin::WHITE_PAWN, martin::WHITE_PAWN, martin::WHITE_PAWN,
                martin::WHITE_PAWN, martin::WHITE_PAWN, martin::WHITE_PAWN, martin::WHITE_PAWN
            }},
            {{
                martin::WHITE_ROOK,   martin::WHITE_KNIGHT, martin::WHITE_BISHOP, martin::WHITE_QUEEN,
                martin::WHITE_KING,   martin::WHITE_BISHOP, martin::WHITE_KNIGHT, martin::WHITE_ROOK
            }}
        }};

        return true;
    } ();

    Rectangle square = { 0, 0, square_size.x, square_size.y };
    Vector2 board_pos = { GetScreenWidth() / 2 - (square.width * 8) / 2, GetScreenHeight() / 2 - (square.height * 8) / 2 };
    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            DrawRectangle(
                square.x + (x * square.width) + board_pos.x,
                square.y + (y * square.height) + board_pos.y,
                square.width,
                square.height,
                (x + y) % 2 == 0 ? Color(115, 149, 82, 255) : Color(235, 236, 208, 255));
        }
    }

    static char selected_piece = martin::EMPTY;
    static Vector2 selected_pos = { -1, -1 };

    bool player_moved = false;

    for (size_t y = 0; y < 8; ++y)
    {
        for (size_t x = 0; x < 8; ++x)
        {
            char piece = g_game_state.board[y][x];
            Texture2D tex = {};
            Rectangle piece_rect = { board_pos.x + (x * piece_width), board_pos.y + (y * piece_height), piece_width, piece_height };

            if (
                g_game_state.game_options.mode == GameMode::PlayerVsBot &&
                selected_piece != martin::EMPTY && 
                CheckCollisionPointRec(GetMousePosition(), piece_rect) &&
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            )
            {
                char from_piece = g_game_state.board[selected_pos.y][selected_pos.x];
                char to_piece = g_game_state.board[y][x];
                do {
                    if (martin::is_white(from_piece) && martin::is_white(to_piece)) {
                        break;
                    }

                    g_game_state.board[y][x] = from_piece;
                    if (to_piece != martin::EMPTY)
                    {
                        g_game_state.human.captured++;
                    }
                    g_game_state.board[selected_pos.y][selected_pos.x] = martin::EMPTY;
                    if (martin::check(g_game_state.board))
                    {
                        selected_piece = martin::EMPTY;
                        selected_pos = { -1, -1 };

                        // Successful move
                        player_moved = true;
                        PlaySound(place_sound_effect);
                    }
                    else
                    {
                        g_game_state.human.captured--;
                        g_game_state.board[selected_pos.y][selected_pos.x] = from_piece;
                        g_game_state.board[y][x] = to_piece;
                    }
                } while (false);
            }

            if (
                CheckCollisionPointRec(GetMousePosition(), piece_rect) && 
                selected_piece != martin::EMPTY &&
                !martin::is_white(g_game_state.board[y][x])
            )
            {
                DrawRectangleRec(piece_rect, Color(0, 0, 0, 127));
            }

            switch (piece)
            {
                case martin::WHITE_KING:   tex = get_piece_texture("wking"); break;
                case martin::WHITE_QUEEN:  tex = get_piece_texture("wqueen"); break;
                case martin::WHITE_ROOK:   tex = get_piece_texture("wrook"); break;
                case martin::WHITE_BISHOP: tex = get_piece_texture("wbishop"); break;
                case martin::WHITE_KNIGHT: tex = get_piece_texture("wknight"); break;
                case martin::WHITE_PAWN:   tex = get_piece_texture("wpawn"); break;

                case martin::BLACK_KING:   tex = get_piece_texture("bking"); break;
                case martin::BLACK_QUEEN:  tex = get_piece_texture("bqueen"); break;
                case martin::BLACK_ROOK:   tex = get_piece_texture("brook"); break;
                case martin::BLACK_BISHOP: tex = get_piece_texture("bbishop"); break;
                case martin::BLACK_KNIGHT: tex = get_piece_texture("bknight"); break;
                case martin::BLACK_PAWN:   tex = get_piece_texture("bpawn"); break;

                default: continue;
            }


            Rectangle draw_rect = piece_rect;
            if (selected_pos == Vector2 { x * 1.f, y * 1.f })
            {
                Vector2 mpos = GetMousePosition();
                draw_rect.x = mpos.x - piece_rect.width / 2;
                draw_rect.y = mpos.y - piece_rect.height / 2;
            }
            DrawTexturePro(tex, {0, 0, 1024, 1024}, draw_rect, {0,0}, 0, BROWN);
            if (g_game_state.game_options.mode != GameMode::PlayerVsBot) continue;
            if (CheckCollisionPointRec(GetMousePosition(), piece_rect) && martin::is_white(piece))
            {
                if (selected_piece == martin::EMPTY)
                {
                    DrawRectangleRec(piece_rect, Color(0, 0, 0, 127));
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (selected_piece == martin::EMPTY)
                    {
                        selected_piece = piece;
                        selected_pos = { x * 1.f, y * 1.f };
                    }
                    else
                    {
                        selected_piece = martin::EMPTY;
                        selected_pos = { -1, -1 };
                    }
                }
            }
        }
    }

    std::string human = std::format("Captured: {}", g_game_state.human.captured);
    std::string bot = std::format("Captured: {}", g_game_state.bot.captured);
    DrawText(human.c_str(), 0, GetScreenHeight() - 24, 24, BLACK);

    if (!player_moved)
    {
        return;
    }

    g_game_state.bot.captured += g_game_state.bot.move(g_game_state.board);
}

void draw_ui()
{
    Vector2 screen_center = { GetScreenWidth() / 2.f, GetScreenHeight() / 2.f };
    
    Rectangle buttons[] = {
        { screen_center.x - 120, screen_center.y - 32, 240, 64 },
        { screen_center.x - 120, screen_center.y + 32 + 8, 240, 64 },
        { screen_center.x - 120, screen_center.y + 32 + 32 + 32 + 8 + 8, 240, 64 },
    };

    const char* texts[] = {
        "Play against Bot",
        "Watch Bot vs Bot",
        "Credits"
    };

    Color colors[] = {
        Color(115, 149, 82, 255),
        Color(115, 149, 82, 255),
        Color(115, 149, 82, 255)
    };

    for (size_t i = 0; i < sizeof(buttons) / sizeof(Rectangle); ++i)
    {
        auto &b = buttons[i];
        Color c = colors[i];
        if (CheckCollisionPointRec(GetMousePosition(), b))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                g_current_draw_function = draw_game;
                if (i == 0)
                {
                    g_game_state.game_options.mode = GameMode::PlayerVsBot;
                }
                else if (i == 1)
                {
                    g_game_state.game_options.mode = GameMode::BotVsBot;
                }
                else if (i == 2)
                {
                    g_current_draw_function = draw_credits;
                }
            }
            g_game_state.bot.move = martin::move;
            c = Color(235, 236, 208, 255);
        }
        DrawRectangleRec(b, c);
        DrawText(texts[i], b.x + ((b.width - MeasureText(texts[i], 24)) / 2.f), b.y + (b.height - GetFontDefault().baseSize) / 2.f, 24, BLACK);
    }

    DrawText("chess.exe", screen_center.x - MeasureText("chess.exe", 64) / 2.f, 25, 64, BLACK);
}

void (*g_current_draw_function)(void) = draw_ui;

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "chess.exe");
    InitAudioDevice();
    SetExitKey(KEY_NULL);

    place_sound_effect = LoadSound("assets/place.ogg");
    place_sound_effect_bot = LoadSound("assets/place.ogg");
    SetSoundVolume(place_sound_effect, 1.0f);
    SetSoundVolume(place_sound_effect_bot, 1.0f);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        {
            g_current_draw_function();
        }
        EndDrawing();
    }

    CloseWindow();
}
