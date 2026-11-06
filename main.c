#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define FILE_NAME "students.txt"

// --- ENUMS & STRUCTURES ---
typedef enum {
    SCREEN_LOGIN = 0,
    SCREEN_DASHBOARD
} CurrentScreen;

typedef struct {
    int roll;
    char name[50];
    float marks;
} Student;

// --- GLOBAL VARIABLES ---
Student students[MAX_STUDENTS];
int studentCount = 0;

// Font Variable
Font customFont;

// Form & State Variables
CurrentScreen currentScreen = SCREEN_LOGIN;
char loginUser[30] = "";
char loginPass[30] = "";
int activeLoginField = 0;
char loginErrorMsg[60] = "";

char rollText[10] = "";
char nameText[50] = "";
char marksText[10] = "";
int activeField = 0;
int selectedIndex = -1;
char statusMsg[60] = "";
Color statusColor = (Color){ 46, 196, 182, 255 };

// Cursors
int loginUserCursor = 0;
int loginPassCursor = 0;
int rollCursor = 0;
int nameCursorPos = 0;
int marksCursor = 0;

// Modern UI Color Palette
#define COLOR_BG        (Color){ 30, 30, 46, 255 }     
#define COLOR_PANEL     (Color){ 40, 40, 60, 255 }     
#define COLOR_ACCENT    (Color){ 0, 180, 216, 255 }    
#define COLOR_SUCCESS   (Color){ 46, 196, 182, 255 }   
#define COLOR_DANGER    (Color){ 230, 57, 70, 255 }    
#define COLOR_TEXT_LIGHT (Color){ 241, 250, 238, 255 } 
#define COLOR_TEXT_MUTED (Color){ 141, 153, 174, 255 } 

// --- FILE OPERATIONS ---
void SaveData() {
    FILE *fp = fopen(FILE_NAME, "w");
    if (!fp) return;
    for (int i = 0; i < studentCount; i++) {
        fprintf(fp, "%d %s %.2f\n", students[i].roll, students[i].name, students[i].marks);
    }
    fclose(fp);
}

void LoadData() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;
    studentCount = 0;
    while (fscanf(fp, "%d %s %f", &students[studentCount].roll, students[studentCount].name, &students[studentCount].marks) == 3) {
        studentCount++;
        if (studentCount >= MAX_STUDENTS) break;
    }
    fclose(fp);
}

void ResetForm() {
    rollText[0] = '\0';
    nameText[0] = '\0';
    marksText[0] = '\0';
    activeField = 0;
    selectedIndex = -1;
    rollCursor = 0;
    nameCursorPos = 0;
    marksCursor = 0;
}

int IsDuplicateRoll(int roll, int ignoreIndex) {
    for (int i = 0; i < studentCount; i++) {
        if (i == ignoreIndex) continue;
        if (students[i].roll == roll) return 1;
    }
    return 0;
}

void InsertCharAt(char *text, int *cursor, int maxLen, char c) {
    int len = strlen(text);
    if (len >= maxLen - 1) return;
    for (int i = len; i >= *cursor; i--) {
        text[i + 1] = text[i];
    }
    text[*cursor] = c;
    (*cursor)++;
}

void HandleBackspaceAt(char *text, int *cursor) {
    int len = strlen(text);
    if (*cursor > 0 && len > 0) {
        for (int i = *cursor - 1; i < len; i++) {
            text[i] = text[i + 1];
        }
        (*cursor)--;
    }
}

int GetClickedCursorPos(const char *text, Rectangle box, float fontSize, float mouseX) {
    int len = strlen(text);
    int bestPos = 0;
    float minDiff = 9999.0f;
    
    for (int i = 0; i <= len; i++) {
        char tempSub[50];
        strncpy(tempSub, text, i);
        tempSub[i] = '\0';
        float charX = box.x + 10 + MeasureTextEx(customFont, tempSub, fontSize, 1.0f).x;
        float diff = fabsf(mouseX - charX);
        if (diff < minDiff) {
            minDiff = diff;
            bestPos = i;
        }
    }
    return bestPos;
}

// --- LOGIN SCREEN ---
void HandleLoginInput() {
    if (IsKeyPressed(KEY_TAB)) {
        activeLoginField = (activeLoginField == 1) ? 2 : 1;
        if (activeLoginField == 1) loginUserCursor = strlen(loginUser);
        else loginPassCursor = strlen(loginPass);
    }

    if (activeLoginField == 1) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(loginUser) < 25)) {
                InsertCharAt(loginUser, &loginUserCursor, 25, (char)key);
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_LEFT) && loginUserCursor > 0) loginUserCursor--;
        if (IsKeyPressed(KEY_RIGHT) && loginUserCursor < strlen(loginUser)) loginUserCursor++;
        if (IsKeyPressed(KEY_BACKSPACE)) HandleBackspaceAt(loginUser, &loginUserCursor);
    } 
    else if (activeLoginField == 2) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(loginPass) < 25)) {
                InsertCharAt(loginPass, &loginPassCursor, 25, (char)key);
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_LEFT) && loginPassCursor > 0) loginPassCursor--;
        if (IsKeyPressed(KEY_RIGHT) && loginPassCursor < strlen(loginPass)) loginPassCursor++;
        if (IsKeyPressed(KEY_BACKSPACE)) HandleBackspaceAt(loginPass, &loginPassCursor);
    }
}

void DrawLoginScreen() {
    Rectangle card = { 280, 100, 440, 420 };
    DrawRectangleRounded(card, 0.08f, 4, COLOR_PANEL);
    DrawRectangleRoundedLines(card, 0.08f, 4, COLOR_ACCENT);

    DrawTextEx(customFont, "PORTAL LOGIN", (Vector2){ card.x + 130, card.y + 30 }, 22, 1.0f, COLOR_TEXT_LIGHT);
    DrawLine(card.x + 40, card.y + 70, card.x + 400, card.y + 70, COLOR_TEXT_MUTED);

    // Username Box
    Rectangle userBox = { card.x + 40, card.y + 110, 360, 45 };
    DrawTextEx(customFont, "Username:", (Vector2){ userBox.x, userBox.y - 20 }, 14, 1.0f, COLOR_TEXT_MUTED);
    DrawRectangleRounded(userBox, 0.1f, 4, (activeLoginField == 1) ? (Color){ 50, 50, 75, 255 } : (Color){ 30, 30, 45, 255 });
    DrawRectangleRoundedLines(userBox, 0.1f, 4, (activeLoginField == 1) ? COLOR_ACCENT : COLOR_TEXT_MUTED);
    DrawTextEx(customFont, loginUser, (Vector2){ userBox.x + 12, userBox.y + 12 }, 18, 1.0f, COLOR_TEXT_LIGHT);

    if (activeLoginField == 1) {
        char tempSub[30];
        strncpy(tempSub, loginUser, loginUserCursor);
        tempSub[loginUserCursor] = '\0';
        float w = MeasureTextEx(customFont, tempSub, 18, 1.0f).x;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(userBox.x + 12 + w, userBox.y + 10, userBox.x + 12 + w, userBox.y + 35, COLOR_ACCENT);
        }
    }

    if (CheckCollisionPointRec(GetMousePosition(), userBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeLoginField = 1;
        loginUserCursor = GetClickedCursorPos(loginUser, userBox, 18, GetMousePosition().x);
    }

    // Password Box
    Rectangle passBox = { card.x + 40, card.y + 190, 360, 45 };
    DrawTextEx(customFont, "Password:", (Vector2){ passBox.x, passBox.y - 20 }, 14, 1.0f, COLOR_TEXT_MUTED);
    DrawRectangleRounded(passBox, 0.1f, 4, (activeLoginField == 2) ? (Color){ 50, 50, 75, 255 } : (Color){ 30, 30, 45, 255 });
    DrawRectangleRoundedLines(passBox, 0.1f, 4, (activeLoginField == 2) ? COLOR_ACCENT : COLOR_TEXT_MUTED);

    char maskedPass[30] = "";
    for (int i = 0; i < strlen(loginPass); i++) maskedPass[i] = '*';
    maskedPass[strlen(loginPass)] = '\0';
    DrawTextEx(customFont, maskedPass, (Vector2){ passBox.x + 12, passBox.y + 15 }, 20, 1.0f, COLOR_TEXT_LIGHT);

    if (activeLoginField == 2) {
        char tempSub[30];
        for (int i = 0; i < loginPassCursor; i++) tempSub[i] = '*';
        tempSub[loginPassCursor] = '\0';
        float w = MeasureTextEx(customFont, tempSub, 20, 1.0f).x;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(passBox.x + 12 + w, passBox.y + 10, passBox.x + 12 + w, passBox.y + 35, COLOR_ACCENT);
        }
    }

    if (CheckCollisionPointRec(GetMousePosition(), passBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeLoginField = 2;
        loginPassCursor = GetClickedCursorPos(loginPass, passBox, 20, GetMousePosition().x);
    }

    // Login Button
    Rectangle btnLogin = { card.x + 40, card.y + 270, 360, 45 };
    bool hoverLogin = CheckCollisionPointRec(GetMousePosition(), btnLogin);
    DrawRectangleRounded(btnLogin, 0.1f, 4, hoverLogin ? COLOR_SUCCESS : COLOR_ACCENT);
    DrawTextEx(customFont, "LOGIN", (Vector2){ btnLogin.x + 150, btnLogin.y + 12 }, 18, 1.0f, COLOR_BG);

    if (strlen(loginErrorMsg) > 0) {
        DrawTextEx(customFont, loginErrorMsg, (Vector2){ card.x + 40, card.y + 335 }, 14, 1.0f, COLOR_DANGER);
    }

    DrawTextEx(customFont, "Default: admin / 1234", (Vector2){ card.x + 100, card.y + 380 }, 13, 1.0f, COLOR_TEXT_MUTED);

    if ((hoverLogin && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ENTER)) {
        if (strcmp(loginUser, "admin") == 0 && strcmp(loginPass, "1234") == 0) {
            currentScreen = SCREEN_DASHBOARD;
            loginErrorMsg[0] = '\0';
            statusMsg[0] = '\0';
        } else {
            strcpy(loginErrorMsg, "Invalid Username or Password!");
        }
    }
}

// --- DASHBOARD INPUT & DRAWING ---
void HandleDashboardFormInput() {
    int key = GetCharPressed();
    while (key > 0) {
        if (activeField == 1 && strlen(rollText) < 8 && (key >= '0' && key <= '9')) {
            InsertCharAt(rollText, &rollCursor, 8, (char)key);
        } 
        else if (activeField == 2 && strlen(nameText) < 40 && ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || key == ' ')) {
            InsertCharAt(nameText, &nameCursorPos, 40, (char)key);
        } 
        else if (activeField == 3 && strlen(marksText) < 6 && ((key >= '0' && key <= '9') || key == '.')) {
            InsertCharAt(marksText, &marksCursor, 6, (char)key);
        }
        key = GetCharPressed();
    }

    if (activeField == 1) {
        if (IsKeyPressed(KEY_LEFT) && rollCursor > 0) rollCursor--;
        if (IsKeyPressed(KEY_RIGHT) && rollCursor < strlen(rollText)) rollCursor++;
        if (IsKeyPressed(KEY_BACKSPACE)) HandleBackspaceAt(rollText, &rollCursor);
    }
    else if (activeField == 2) {
        if (IsKeyPressed(KEY_LEFT) && nameCursorPos > 0) nameCursorPos--;
        if (IsKeyPressed(KEY_RIGHT) && nameCursorPos < strlen(nameText)) nameCursorPos++;
        if (IsKeyPressed(KEY_BACKSPACE)) HandleBackspaceAt(nameText, &nameCursorPos);
    }
    else if (activeField == 3) {
        if (IsKeyPressed(KEY_LEFT) && marksCursor > 0) marksCursor--;
        if (IsKeyPressed(KEY_RIGHT) && marksCursor < strlen(marksText)) marksCursor++;
        if (IsKeyPressed(KEY_BACKSPACE)) HandleBackspaceAt(marksText, &marksCursor);
    }
}

void DrawDashboardScreen() {
    DrawRectangle(0, 0, 1000, 60, COLOR_PANEL);
    DrawTextEx(customFont, "STUDENT MANAGEMENT SYSTEM", (Vector2){ 20, 18 }, 18, 1.0f, COLOR_TEXT_LIGHT);

    // Logout Button
    Rectangle btnLogout = { 880, 12, 100, 36 };
    bool hoverLogout = CheckCollisionPointRec(GetMousePosition(), btnLogout);
    DrawRectangleRounded(btnLogout, 0.15f, 4, hoverLogout ? COLOR_DANGER : (Color){ 60, 60, 80, 255 });
    DrawTextEx(customFont, "Logout", (Vector2){ btnLogout.x + 24, btnLogout.y + 10 }, 15, 1.0f, COLOR_TEXT_LIGHT);

    if (hoverLogout && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentScreen = SCREEN_LOGIN;
        loginUser[0] = '\0';
        loginPass[0] = '\0';
        ResetForm();
        return;
    }

    // --- LEFT PANEL: FORM ---
    Rectangle formPanel = { 20, 80, 340, 500 };
    DrawRectangleRounded(formPanel, 0.05f, 4, COLOR_PANEL);
    DrawTextEx(customFont, "Student Registration", (Vector2){ formPanel.x + 20, formPanel.y + 20 }, 18, 1.0f, COLOR_ACCENT);

    // Roll Box
    Rectangle rBox = { formPanel.x + 20, formPanel.y + 80, 300, 40 };
    DrawTextEx(customFont, "Roll Number:", (Vector2){ rBox.x, rBox.y - 18 }, 13, 1.0f, COLOR_TEXT_MUTED);
    DrawRectangleRounded(rBox, 0.1f, 4, (activeField == 1) ? (Color){ 60, 60, 85, 255 } : COLOR_BG);
    DrawRectangleRoundedLines(rBox, 0.1f, 4, (activeField == 1) ? COLOR_ACCENT : COLOR_TEXT_MUTED);
    DrawTextEx(customFont, rollText, (Vector2){ rBox.x + 10, rBox.y + 10 }, 16, 1.0f, COLOR_TEXT_LIGHT);

    if (activeField == 1) {
        char tempSub[10];
        strncpy(tempSub, rollText, rollCursor);
        tempSub[rollCursor] = '\0';
        float w = MeasureTextEx(customFont, tempSub, 16, 1.0f).x;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(rBox.x + 10 + w, rBox.y + 8, rBox.x + 10 + w, rBox.y + 32, COLOR_ACCENT);
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), rBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeField = 1;
        rollCursor = GetClickedCursorPos(rollText, rBox, 16, GetMousePosition().x);
    }

    // Name Box
    Rectangle nBox = { formPanel.x + 20, formPanel.y + 150, 300, 40 };
    DrawTextEx(customFont, "Student Name:", (Vector2){ nBox.x, nBox.y - 18 }, 13, 1.0f, COLOR_TEXT_MUTED);
    DrawRectangleRounded(nBox, 0.1f, 4, (activeField == 2) ? (Color){ 60, 60, 85, 255 } : COLOR_BG);
    DrawRectangleRoundedLines(nBox, 0.1f, 4, (activeField == 2) ? COLOR_ACCENT : COLOR_TEXT_MUTED);
    DrawTextEx(customFont, nameText, (Vector2){ nBox.x + 10, nBox.y + 10 }, 16, 1.0f, COLOR_TEXT_LIGHT);
    
    if (activeField == 2) {
        char tempSub[50];
        strncpy(tempSub, nameText, nameCursorPos);
        tempSub[nameCursorPos] = '\0';
        float w = MeasureTextEx(customFont, tempSub, 16, 1.0f).x;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(nBox.x + 10 + w, nBox.y + 8, nBox.x + 10 + w, nBox.y + 32, COLOR_ACCENT);
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), nBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeField = 2;
        nameCursorPos = GetClickedCursorPos(nameText, nBox, 16, GetMousePosition().x);
    }

    // Marks Box
    Rectangle mBox = { formPanel.x + 20, formPanel.y + 220, 300, 40 };
    DrawTextEx(customFont, "Marks (0 - 100):", (Vector2){ mBox.x, mBox.y - 18 }, 13, 1.0f, COLOR_TEXT_MUTED);
    DrawRectangleRounded(mBox, 0.1f, 4, (activeField == 3) ? (Color){ 60, 60, 85, 255 } : COLOR_BG);
    DrawRectangleRoundedLines(mBox, 0.1f, 4, (activeField == 3) ? COLOR_ACCENT : COLOR_TEXT_MUTED);
    DrawTextEx(customFont, marksText, (Vector2){ mBox.x + 10, mBox.y + 10 }, 16, 1.0f, COLOR_TEXT_LIGHT);

    if (activeField == 3) {
        char tempSub[10];
        strncpy(tempSub, marksText, marksCursor);
        tempSub[marksCursor] = '\0';
        float w = MeasureTextEx(customFont, tempSub, 16, 1.0f).x;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(mBox.x + 10 + w, mBox.y + 8, mBox.x + 10 + w, mBox.y + 32, COLOR_ACCENT);
        }
    }
    if (CheckCollisionPointRec(GetMousePosition(), mBox) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeField = 3;
        marksCursor = GetClickedCursorPos(marksText, mBox, 16, GetMousePosition().x);
    }

    // Form Action Buttons
    Rectangle btnAdd = { formPanel.x + 20, formPanel.y + 280, 140, 40 };
    Rectangle btnUpdate = { formPanel.x + 180, formPanel.y + 280, 140, 40 };
    Rectangle btnDelete = { formPanel.x + 20, formPanel.y + 335, 140, 40 };
    Rectangle btnClear = { formPanel.x + 180, formPanel.y + 335, 140, 40 };

    DrawRectangleRounded(btnAdd, 0.1f, 4, COLOR_SUCCESS);
    DrawTextEx(customFont, "Add Record", (Vector2){ btnAdd.x + 25, btnAdd.y + 12 }, 14, 1.0f, COLOR_BG);

    DrawRectangleRounded(btnUpdate, 0.1f, 4, COLOR_ACCENT);
    DrawTextEx(customFont, "Update", (Vector2){ btnUpdate.x + 45, btnUpdate.y + 12 }, 14, 1.0f, COLOR_BG);

    DrawRectangleRounded(btnDelete, 0.1f, 4, COLOR_DANGER);
    DrawTextEx(customFont, "Delete", (Vector2){ btnDelete.x + 45, btnDelete.y + 12 }, 14, 1.0f, COLOR_TEXT_LIGHT);

    DrawRectangleRounded(btnClear, 0.1f, 4, COLOR_TEXT_MUTED);
    DrawTextEx(customFont, "Clear Form", (Vector2){ btnClear.x + 30, btnClear.y + 12 }, 14, 1.0f, COLOR_BG);

    if (strlen(statusMsg) > 0) {
        DrawTextEx(customFont, statusMsg, (Vector2){ formPanel.x + 20, formPanel.y + 395 }, 13, 1.0f, statusColor);
    }

    // --- ADD BUTTON ---
    if (CheckCollisionPointRec(GetMousePosition(), btnAdd) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (strlen(rollText) == 0 || strlen(nameText) == 0 || strlen(marksText) == 0) {
            strcpy(statusMsg, "Error: Fill all fields!");
            statusColor = COLOR_DANGER;
        } else {
            int roll = atoi(rollText);
            float marks = atof(marksText);

            if (marks < 0 || marks > 100) {
                strcpy(statusMsg, "Error: Marks range (0-100)!");
                statusColor = COLOR_DANGER;
            } else if (IsDuplicateRoll(roll, -1)) {
                strcpy(statusMsg, "Error: Duplicate Roll Number!");
                statusColor = COLOR_DANGER;
            } else if (studentCount < MAX_STUDENTS) {
                students[studentCount].roll = roll;
                strcpy(students[studentCount].name, nameText);
                students[studentCount].marks = marks;
                studentCount++;
                SaveData();
                ResetForm();
                strcpy(statusMsg, "Success: Record Added!");
                statusColor = COLOR_SUCCESS;
            }
        }
    }

    // --- UPDATE BUTTON ---
    if (CheckCollisionPointRec(GetMousePosition(), btnUpdate) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (selectedIndex >= 0) {
            int roll = atoi(rollText);
            float marks = atof(marksText);

            if (marks < 0 || marks > 100) {
                strcpy(statusMsg, "Error: Marks range (0-100)!");
                statusColor = COLOR_DANGER;
            } else if (IsDuplicateRoll(roll, selectedIndex)) {
                strcpy(statusMsg, "Error: Roll Number exists!");
                statusColor = COLOR_DANGER;
            } else {
                students[selectedIndex].roll = roll;
                strcpy(students[selectedIndex].name, nameText);
                students[selectedIndex].marks = marks;
                SaveData();
                ResetForm();
                strcpy(statusMsg, "Success: Record Updated!");
                statusColor = COLOR_SUCCESS;
            }
        } else {
            strcpy(statusMsg, "Error: Select a record first!");
            statusColor = COLOR_DANGER;
        }
    }

    // --- DELETE BUTTON ---
    if (CheckCollisionPointRec(GetMousePosition(), btnDelete) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (selectedIndex >= 0) {
            for (int i = selectedIndex; i < studentCount - 1; i++) {
                students[i] = students[i + 1];
            }
            studentCount--;
            SaveData();
            ResetForm();
            strcpy(statusMsg, "Success: Record Deleted!");
            statusColor = COLOR_SUCCESS;
        } else {
            strcpy(statusMsg, "Error: Select a record first!");
            statusColor = COLOR_DANGER;
        }
    }

    // --- CLEAR BUTTON ---
    if (CheckCollisionPointRec(GetMousePosition(), btnClear) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetForm();
        statusMsg[0] = '\0';
    }

    // --- RIGHT PANEL: DATA TABLE ---
    Rectangle tablePanel = { 380, 80, 600, 500 };
    DrawRectangleRounded(tablePanel, 0.05f, 4, COLOR_PANEL);

    DrawRectangleRounded((Rectangle){ tablePanel.x, tablePanel.y, tablePanel.width, 40 }, 0.05f, 4, (Color){ 50, 50, 70, 255 });
    DrawTextEx(customFont, "Roll No", (Vector2){ tablePanel.x + 20, tablePanel.y + 12 }, 15, 1.0f, COLOR_ACCENT);
    DrawTextEx(customFont, "Name", (Vector2){ tablePanel.x + 140, tablePanel.y + 12 }, 15, 1.0f, COLOR_ACCENT);
    DrawTextEx(customFont, "Marks", (Vector2){ tablePanel.x + 420, tablePanel.y + 12 }, 15, 1.0f, COLOR_ACCENT);

    int startY = tablePanel.y + 50;
    for (int i = 0; i < studentCount; i++) {
        Rectangle rowRect = { tablePanel.x + 10, startY + (i * 38), tablePanel.width - 20, 34 };
        bool isHover = CheckCollisionPointRec(GetMousePosition(), rowRect);

        Color rowBg = (i == selectedIndex) ? (Color){ 0, 180, 216, 60 } : (isHover ? (Color){ 60, 60, 80, 255 } : COLOR_BG);
        DrawRectangleRounded(rowRect, 0.08f, 4, rowBg);

        DrawTextEx(customFont, TextFormat("%d", students[i].roll), (Vector2){ rowRect.x + 10, rowRect.y + 8 }, 15, 1.0f, COLOR_TEXT_LIGHT);
        DrawTextEx(customFont, students[i].name, (Vector2){ rowRect.x + 130, rowRect.y + 8 }, 15, 1.0f, COLOR_TEXT_LIGHT);
        DrawTextEx(customFont, TextFormat("%.2f", students[i].marks), (Vector2){ rowRect.x + 410, rowRect.y + 8 }, 15, 1.0f, COLOR_TEXT_LIGHT);

        if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedIndex = i;
            sprintf(rollText, "%d", students[i].roll);
            strcpy(nameText, students[i].name);
            sprintf(marksText, "%.2f", students[i].marks);
            rollCursor = strlen(rollText);
            nameCursorPos = strlen(nameText);
            marksCursor = strlen(marksText);
            statusMsg[0] = '\0';
        }
    }
}

// --- MAIN FUNCTION ---
int main() {
    InitWindow(1000, 600, "Student Management System - Pro Edition");
    SetTargetFPS(60);

    // Load Custom Font (Make sure font.ttf is in your project folder)
    customFont = LoadFont("font.ttf"); 

    LoadData();

    while (!WindowShouldClose()) {
        if (currentScreen == SCREEN_LOGIN) {
            HandleLoginInput();
        } else if (currentScreen == SCREEN_DASHBOARD) {
            HandleDashboardFormInput();
        }

        BeginDrawing();
        ClearBackground(COLOR_BG);

        if (currentScreen == SCREEN_LOGIN) {
            DrawLoginScreen();
        } else if (currentScreen == SCREEN_DASHBOARD) {
            DrawDashboardScreen();
        }

        EndDrawing();
    }

    // Unload font and close window
    UnloadFont(customFont);
    CloseWindow();
    return 0;
}