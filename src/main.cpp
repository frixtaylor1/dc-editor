#include "../vendor/raylib/include/raylib.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

using String = std::string;

template <typename T> 
using Vector = std::vector<T>;

struct Word {
    String               content;
    Vector2              pos;
};

enum class Mode { NORMAL, INSERT, SELECT, COMMAND };

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
    struct LineBar {
        Vector2          pos;
        Vector2          measures;
        Color            fontColor;
        int              spacing;
    };

    Vector2              cursor;
    String               content;
    Vector2              pos;
    float                fontSize;
    LineBar              lineBar;
};

void draw_modal(const ModalProps& modalProps) {
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

void mini_buffer_initialize(MiniBuffer& minibuffer) {
    minibuffer.content           = "";
    minibuffer.lineBar.spacing   = 7;
    minibuffer.fontSize          = 20;
    minibuffer.lineBar.fontColor = RAYWHITE;
    minibuffer.lineBar.pos       = { 10, (GetScreenHeight() - minibuffer.fontSize) };
    minibuffer.cursor            = { 10, minibuffer.lineBar.pos.y };
}

float buffer_cursor_measure_text(const MiniBuffer& minibuffer) {
    return MeasureText(minibuffer.content.c_str(), minibuffer.fontSize) + minibuffer.lineBar.spacing;
}

float buffer_cursor_measure_text(const Buffer& buffer, int idx) {
    return MeasureText(buffer.lines[buffer.cursor.y][idx].content.c_str(), buffer.fontSize) + buffer.spacing;
}

void mini_buffer_insert_char(char userInput, MiniBuffer& minibuffer) {
    if (userInput >= 32 && userInput <= 126) {
        minibuffer.content.push_back(userInput);
        minibuffer.cursor.x = buffer_cursor_measure_text(minibuffer);
    }
}

void mini_buffer_delete_char(MiniBuffer& minibuffer) {
    if (!minibuffer.content.empty()) {
        minibuffer.content.pop_back();
        minibuffer.cursor.x = buffer_cursor_measure_text(minibuffer);
    }
}

void mini_buffer_handle_input(MiniBuffer& minibuffer) {
    char key = GetCharPressed();
    while (key > 0) {
        if (IsKeyPressed(KEY_BACKSPACE)) {
            mini_buffer_delete_char(minibuffer);
        } else if (key >= 32 && key <= 126) {
            mini_buffer_insert_char(key, minibuffer);
        }
        key = GetCharPressed();
    }
}

void mini_buffer_draw_cursor(MiniBuffer& minibuffer) {
    DrawRectangleLines(minibuffer.cursor.x, GetScreenHeight() - minibuffer.fontSize, 3, minibuffer.fontSize, RED);
}

void mini_buffer_draw(MiniBuffer& minibuffer) {
    DrawRectangle(0, GetScreenHeight() - minibuffer.fontSize, GetScreenWidth(), minibuffer.fontSize * 2, DARKGRAY);
    DrawText(minibuffer.content.c_str(), 10, GetScreenHeight() - minibuffer.fontSize, minibuffer.fontSize, minibuffer.lineBar.fontColor);
    mini_buffer_draw_cursor(minibuffer);
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
    if (buffer_is_normal_mode(buffer)) {
        buffer.mode = Mode::INSERT;
    }
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

void buffer_cursor_move_left(Buffer& buffer) {
    if (buffer.cursor.x > 0) {
        buffer.cursor.x--;
    } else if (buffer.cursor.y > 0) {
        buffer.cursor.y--;
        buffer.cursor.x = buffer.lines[buffer.cursor.y].size();
    }
}

void buffer_cursor_move_right(Buffer& buffer) {
    if (buffer.cursor.x < buffer.lines[buffer.cursor.y].size()) {
        buffer.cursor.x++;
    } else if (buffer.cursor.y < buffer.lines.size() - 1) {
        buffer.cursor.y++;
        buffer.cursor.x = 0;
    }
}

void buffer_cursor_move_up(Buffer& buffer) {
    if (buffer.cursor.y > 0) {
        buffer.cursor.y--;
        buffer.cursor.x = std::min((int)buffer.cursor.x, (int)buffer.lines[buffer.cursor.y].size());
    }
}

void buffer_cursor_move_down(Buffer& buffer) {
    if (buffer.cursor.y < buffer.lines.size() - 1) {
        buffer.cursor.y++;
        buffer.cursor.x = std::min((int)buffer.cursor.x, (int)buffer.lines[buffer.cursor.y].size());
    }
}

void buffer_handle_cursor_movement(Buffer& buffer) {
    if (IsKeyPressed(KEY_LEFT))  buffer_cursor_move_left(buffer);
    if (IsKeyPressed(KEY_RIGHT)) buffer_cursor_move_right(buffer);
    if (IsKeyPressed(KEY_UP))    buffer_cursor_move_up(buffer);
    if (IsKeyPressed(KEY_DOWN))  buffer_cursor_move_down(buffer);
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
        x_offset += buffer_cursor_measure_text(buffer, idx);
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

void save_modal_error() {
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
        draw_modal(modalProps);
    EndDrawing();
}

void save(Buffer& buffer) {
    buffer.name = "./first_file.txt";
    std::ofstream outputFile(buffer.name);

    if (!outputFile.is_open()) {
        return save_modal_error();
    }

    for (const auto& line : buffer.lines) {
        for (const auto& word : line) {
            outputFile << word.content;
        }
        outputFile << '\n';
    }

    outputFile.close();
}

void buffer_exit() {
    EndDrawing();
    CloseWindow();
    exit (EXIT_SUCCESS);
}

void buffer_handle_command(Buffer& buffer, MiniBuffer& minibuffer) {
    if (buffer_is_command_mode(buffer)) {
        mini_buffer_handle_input(minibuffer);
        mini_buffer_draw(minibuffer);
    
        if (IsKeyPressed(KEY_ENTER) && !minibuffer.content.empty()) {
            if (minibuffer.content.length() == 1) {
                switch (minibuffer.content[0]) {
                    case 'w': save(buffer); break;
                    case 'q': buffer_exit(); break;
                }
            }

            minibuffer.content.clear();
            buffer.mode = Mode::NORMAL;
        }
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

    MiniBuffer miniBuffer;
    mini_buffer_initialize(miniBuffer);

    initialize_graphics();
        while (!WindowShouldClose()) {
            BeginDrawing();
                buffer_handle_text_input(buffer);
                buffer_handle_mode(buffer);
                buffer_handle_cursor_movement(buffer);
                buffer_handle_command(buffer, miniBuffer);
                buffer_draw(buffer);
            EndDrawing();
        }
    close_graphics();
}

int main() {
    // @TODO: CRUD OF FILES...
    // @TODO: FILE TREE or FILE EXPLORER...
    // @TODO: MUST HAVE AN ARRAY OF EDITORS...
    run_editor();
    return 0;
}
