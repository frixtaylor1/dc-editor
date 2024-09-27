#include "../vendor/raylib/include/raylib.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

using String = std::string;

template <typename T> 
using Vector = std::vector<T>;

struct Word {
    String  content;
    Vector2 pos;
};

enum class Mode {
    NORMAL,
    INSERT,
    SELECT,
    COMMAND
};

struct Buffer {
    Vector2              pos;
    Vector<Vector<Word>> lines;
    Vector2              cursor;
    Mode                 mode;
    int                  spacing;
    float                fontSize;
    const char*          name;
    char                 lastCharPressed;
};

struct ModalProps {
    int                  width; 
    int                  height;
    int                  posX; 
    int                  posY;
    int                  contentMarginX; 
    int                  contentMarginY;
    const char*          content;
};

// @TODO: ADD MINIBUFFER
// The system should have a mini buffer
struct MiniBuffer {
    Vector2              cursor;
    String               content;
    Color                color;
    Vector2              pos;
    int                  spacing;
};

struct StatusBar {
    Vector2              pos;
};

void buffer_draw_modal(const ModalProps& modalProps) {
    while (!IsKeyPressed(KEY_ESCAPE)) {
        DrawRectangle(modalProps.posX, modalProps.posY, modalProps.width, modalProps.height, DARKGRAY);
        DrawRectangleLines(modalProps.posX, modalProps.posY, modalProps.width, modalProps.height, RAYWHITE);
        DrawText(modalProps.content, modalProps.posX + modalProps.contentMarginX, modalProps.posY + modalProps.contentMarginY, 14, RED);
    }
}

void buffer_initialize(Buffer& buffer) {
    buffer.cursor   = {0, 0};
    buffer.mode     = Mode::NORMAL;
    buffer.spacing  = 7;
    buffer.fontSize = 20;
    buffer.lines.push_back(Vector<Word>());
}

bool buffer_is_insert_mode(const Buffer& buffer) {
    return buffer.mode == Mode::INSERT;
}

bool buffer_is_normal_mode(const Buffer& buffer) {
    return buffer.mode == Mode::NORMAL;
}

bool buffer_is_command_mode(const Buffer& buffer) {
    return buffer.mode == Mode::COMMAND;
}

void buffer_enable_insert_mode(Buffer& buffer) {
    buffer.mode = Mode::INSERT;
}

void buffer_enable_normal_mode(Buffer& buffer) {
    buffer.mode = Mode::NORMAL;
}

void buffer_enable_select_mode(Buffer& buffer) {
    if (buffer_is_normal_mode(buffer)) {
        buffer.mode = Mode::SELECT;
    }
}

void buffer_enable_command_mode(Buffer& buffer) {
    if (buffer_is_normal_mode(buffer)) {
        buffer.mode = Mode::COMMAND;
    }
}

void buffer_handle_mode(Buffer& buffer) {
    if (buffer_is_normal_mode(buffer)) buffer.lastCharPressed = GetCharPressed();

    if (IsKeyPressed(KEY_I))                 buffer_enable_insert_mode(buffer);
    if (IsKeyPressed(KEY_ESCAPE))            buffer_enable_normal_mode(buffer);
    if (IsKeyPressed(KEY_V))                 buffer_enable_select_mode(buffer);
    if (buffer.lastCharPressed == KEY_COLON) buffer_enable_command_mode(buffer);
}

void cursor_move_left(Buffer& buffer) {
    if (buffer.cursor.x > 0) {
        buffer.cursor.x--;
    } else if (buffer.cursor.y > 0) {
        buffer.cursor.y--;
        buffer.cursor.x = buffer.lines[buffer.cursor.y].size();
    }
}

void cursor_move_right(Buffer& buffer) {
    if (buffer.cursor.x < buffer.lines[buffer.cursor.y].size()) {
        buffer.cursor.x++;
    } else if (buffer.cursor.y < buffer.lines.size() - 1) {
        buffer.cursor.y++;
        buffer.cursor.x = 0;
    }
}

void cursor_move_up(Buffer& buffer) {
    if (buffer.cursor.y > 0) {
        buffer.cursor.y--;
        buffer.cursor.x = std::min((int)buffer.cursor.x, (int)buffer.lines[buffer.cursor.y].size());
    }
}

void cursor_move_down(Buffer& buffer) {
    if (buffer.cursor.y < buffer.lines.size() - 1) {
        buffer.cursor.y++;
        buffer.cursor.x = std::min((int)buffer.cursor.x, (int)buffer.lines[buffer.cursor.y].size());
    }
}

void buffer_handle_cursor_movement(Buffer& buffer) {
    if (IsKeyPressed(KEY_LEFT))  cursor_move_left(buffer);
    if (IsKeyPressed(KEY_RIGHT)) cursor_move_right(buffer);
    if (IsKeyPressed(KEY_UP))    cursor_move_up(buffer);
    if (IsKeyPressed(KEY_DOWN))  cursor_move_down(buffer);
}

void buffer_insert_char(Buffer& buffer, char c) {
    Word word;
    word.content = String(1, c);
    word.pos = {0, 0};

    if (buffer.lines[buffer.cursor.y].empty()) {
        buffer.lines[buffer.cursor.y].push_back(word);
    } else {
        buffer.lines[buffer.cursor.y].insert(buffer.lines[buffer.cursor.y].begin() + buffer.cursor.x, word);
    }
    buffer.cursor.x++;
}

void buffer_new_line(Buffer& buffer) {
    if (IsKeyPressed(KEY_ENTER)) {
        buffer.lines.insert(buffer.lines.begin() + buffer.cursor.y + 1, Vector<Word>());
        buffer.cursor.y++;
        buffer.cursor.x = 0;
    }
}

void buffer_delete_char(Buffer& buffer) {
    if (IsKeyPressed(KEY_BACKSPACE) && buffer.cursor.x > 0) {
        buffer.lines[buffer.cursor.y].erase(buffer.lines[buffer.cursor.y].begin() + buffer.cursor.x - 1);
        buffer.cursor.x--;
    }
}

void buffer_handle_text_input(Buffer& buffer) {
    if (!buffer_is_insert_mode(buffer)) return;

    char key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126) {
            buffer_insert_char(buffer, key);
        }
        key = GetCharPressed();
    }

    buffer_new_line(buffer);
    buffer_delete_char(buffer);
}

void buffer_draw_cursor(const Buffer& buffer) {
    float x_offset = 10;
    for (int idx = 0; idx < buffer.cursor.x; idx++) {
        x_offset += MeasureText(buffer.lines[buffer.cursor.y][idx].content.c_str(), buffer.fontSize) + buffer.spacing;
    }
    
    float y_offset = 10 + buffer.cursor.y * buffer.fontSize;
    
    DrawRectangleLines((int)x_offset, (int)y_offset, 2, (int)buffer.fontSize, GetColor(0x4388c1b3));
}

void buffer_draw(const Buffer& buffer) {
    ClearBackground(GetColor(0x181818FF));

    for (size_t y = 0; y < buffer.lines.size(); y++) {
        float x_offset = 10;
        float y_offset = 10 + y * buffer.fontSize;

        for (const Word& word : buffer.lines[y]) {
            DrawText(word.content.c_str(), (int)x_offset, (int)y_offset, buffer.fontSize, RAYWHITE);
            x_offset += MeasureText(word.content.c_str(), buffer.fontSize) + buffer.spacing;
        }
    }

    buffer_draw_cursor(buffer);
}

void buffer_save_modal_error() {
    int width = 120, height = 70;
    ModalProps modalProps {
        width, 
        height,
        (GetScreenWidth() / 2) - width,
        (GetScreenHeight() / 2) - height,
        30, 
        40,
        "Error saving file!"
    };

    BeginDrawing();
        ClearBackground(BLACK);
        buffer_draw_modal(modalProps);
    EndDrawing();
}

void buffer_save(Buffer& buffer) {
    buffer.name = "./first_file.txt";
    std::ofstream outputFile(buffer.name);

    if (!outputFile.is_open()) {
        return buffer_save_modal_error();
    }

    for (const auto& line : buffer.lines) {
        for (const auto& word : line) {
            outputFile << word.content;
        }
        outputFile << '\n';
    }

    outputFile.close();
}

void buffer_handle_command(Buffer& buffer) {
    if (buffer_is_command_mode(buffer) && IsKeyPressed(KEY_W)) {
        buffer_save(buffer);
    }
}

void initialize_graphics() {
    InitWindow(1280, 720, "Editor de Texto");
    SetTargetFPS(60);
    SetExitKey(0);
}

void close_graphics() {
    CloseWindow();
}

void run_editor() {
    Buffer buffer;
    buffer_initialize(buffer);
    initialize_graphics();

    while (!WindowShouldClose()) {
        BeginDrawing();

            buffer_handle_text_input(buffer);
            buffer_handle_mode(buffer);
            buffer_handle_cursor_movement(buffer);
            buffer_handle_command(buffer);
            buffer_draw(buffer);

        EndDrawing();
    }

    close_graphics();
}

int main() {
    run_editor();
    return 0;
}
