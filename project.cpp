#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

// Control IDs
#define ID_EDIT_NAME     101
#define ID_EDIT_ROLL     102
#define ID_EDIT_FNAME    103
#define ID_EDIT_CASTE    104
#define ID_EDIT_DISTRICT 105
#define ID_EDIT_GPA      106

#define ID_BTN_ADD       201
#define ID_BTN_UPDATE    202
#define ID_BTN_DELETE    203
#define ID_BTN_NEXT      204

// Global handles for UI controls
HWND hEditName, hEditRoll, hEditFname, hEditCaste, hEditDistrict, hEditGpa;
HWND hBtnAdd, hBtnUpdate, hBtnDelete, hBtnNext;

struct Student {
    string name;
    string roll;
    string fname;
    string caste;
    string district;
    string gpa;
};

vector<Student> records;
int currentIndex = -1;
const string FILE_NAME = "students.txt";

// Helper: Get text from an Edit Control
string GetControlText(HWND hCtrl) {
    char buffer[256];
    GetWindowTextA(hCtrl, buffer, 256);
    return string(buffer);
}

// Helper: Set text to an Edit Control
void SetControlText(HWND hCtrl, const string& text) {
    SetWindowTextA(hCtrl, text.c_str());
}

// Helper: Clear all input fields
void ClearFields() {
    SetControlText(hEditName, "");
    SetControlText(hEditRoll, "");
    SetControlText(hEditFname, "");
    SetControlText(hEditCaste, "");
    SetControlText(hEditDistrict, "");
    SetControlText(hEditGpa, "");
}

// Load all records from comma-separated text file
void LoadRecords() {
    records.clear();
    ifstream file(FILE_NAME);
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Student s;
        getline(ss, s.name, ',');
        getline(ss, s.roll, ',');
        getline(ss, s.fname, ',');
        getline(ss, s.caste, ',');
        getline(ss, s.district, ',');
        getline(ss, s.gpa, ',');
        records.push_back(s);
    }
    file.close();
}

// Save all records back to the text file
void SaveAllRecords() {
    ofstream file(FILE_NAME, ios::trunc);
    for (const auto& s : records) {
        file << s.name << "," << s.roll << "," << s.fname << ","
             << s.caste << "," << s.district << "," << s.gpa << "\n";
    }
    file.close();
}

// Reset UI to initial startup state
void ResetToAddMode() {
    ClearFields();
    currentIndex = -1;
    EnableWindow(hBtnAdd, TRUE);
    EnableWindow(hBtnUpdate, FALSE);
    EnableWindow(hBtnDelete, FALSE);
}

// Main Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            auto CreateLabel = [&](const char* text, int x, int y) {
                HWND hLbl = CreateWindowA("STATIC", text, WS_VISIBLE | WS_CHILD,
                                          x, y, 130, 25, hwnd, NULL, NULL, NULL);
                SendMessage(hLbl, WM_SETFONT, (WPARAM)hFont, TRUE);
            };

            auto CreateInput = [&](int id, int x, int y) {
                HWND hEdt = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                          x, y, 220, 26, hwnd, (HMENU)(INT_PTR)id, NULL, NULL);
                SendMessage(hEdt, WM_SETFONT, (WPARAM)hFont, TRUE);
                return hEdt;
            };

            auto CreateBtn = [&](const char* text, int id, int x, int y, int w = 90) {
                HWND hBtn = CreateWindowA("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                          x, y, w, 32, hwnd, (HMENU)(INT_PTR)id, NULL, NULL);
                SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
                return hBtn;
            };

            // Labels & Text Fields
            CreateLabel("Student Name:", 40, 30);
            hEditName = CreateInput(ID_EDIT_NAME, 180, 30);

            CreateLabel("Roll Number:", 40, 70);
            hEditRoll = CreateInput(ID_EDIT_ROLL, 180, 70);

            CreateLabel("Father's Name:", 40, 110);
            hEditFname = CreateInput(ID_EDIT_FNAME, 180, 110);

            CreateLabel("Student Caste:", 40, 150);
            hEditCaste = CreateInput(ID_EDIT_CASTE, 180, 150);

            CreateLabel("District:", 40, 190);
            hEditDistrict = CreateInput(ID_EDIT_DISTRICT, 180, 190);

            CreateLabel("Student GPA:", 40, 230);
            hEditGpa = CreateInput(ID_EDIT_GPA, 180, 230);

            // Buttons
            hBtnNext   = CreateBtn("Next",   ID_BTN_NEXT,   430, 30, 90);
            hBtnAdd    = CreateBtn("Add",    ID_BTN_ADD,    40,  290, 95);
            hBtnUpdate = CreateBtn("Update", ID_BTN_UPDATE, 160, 290, 95);
            hBtnDelete = CreateBtn("Delete", ID_BTN_DELETE, 280, 290, 95);

            // Initial Button States (Add enabled; Update & Delete disabled)
            EnableWindow(hBtnAdd, TRUE);
            EnableWindow(hBtnUpdate, FALSE);
            EnableWindow(hBtnDelete, FALSE);

            LoadRecords();
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);

            // ADD BUTTON
            if (wmId == ID_BTN_ADD) {
                Student s;
                s.name     = GetControlText(hEditName);
                s.roll     = GetControlText(hEditRoll);
                s.fname    = GetControlText(hEditFname);
                s.caste    = GetControlText(hEditCaste);
                s.district = GetControlText(hEditDistrict);
                s.gpa      = GetControlText(hEditGpa);

                if (s.name.empty() || s.roll.empty()) {
                    MessageBoxA(hwnd, "Name and Roll Number are required!", "Validation Error", MB_ICONWARNING);
                    break;
                }

                // Check duplicate Roll Number
                bool exists = false;
                for (const auto& rec : records) {
                    if (rec.roll == s.roll) {
                        exists = true;
                        break;
                    }
                }
                if (exists) {
                    MessageBoxA(hwnd, "A student with this Roll Number already exists!", "Duplicate Error", MB_ICONERROR);
                    break;
                }

                records.push_back(s);
                SaveAllRecords();
                ClearFields();
                MessageBoxA(hwnd, "Student record added successfully!", "Success", MB_ICONINFORMATION);
            }

            // NEXT BUTTON
            else if (wmId == ID_BTN_NEXT) {
                LoadRecords();
                if (records.empty()) {
                    MessageBoxA(hwnd, "No student records found in the file.", "Info", MB_ICONINFORMATION);
                    break;
                }

                currentIndex++;
                if (currentIndex >= (int)records.size()) {
                    // Cycle back to Add mode after reaching the end
                    MessageBoxA(hwnd, "End of records reached. Returning to Add mode.", "Navigation", MB_ICONINFORMATION);
                    ResetToAddMode();
                    break;
                }

                Student s = records[currentIndex];
                SetControlText(hEditName, s.name);
                SetControlText(hEditRoll, s.roll);
                SetControlText(hEditFname, s.fname);
                SetControlText(hEditCaste, s.caste);
                SetControlText(hEditDistrict, s.district);
                SetControlText(hEditGpa, s.gpa);

                // Disable Add; Enable Update & Delete
                EnableWindow(hBtnAdd, FALSE);
                EnableWindow(hBtnUpdate, TRUE);
                EnableWindow(hBtnDelete, TRUE);
            }

            // UPDATE BUTTON
            else if (wmId == ID_BTN_UPDATE) {
                string targetRoll = GetControlText(hEditRoll);
                bool updated = false;

                for (auto& rec : records) {
                    if (rec.roll == targetRoll) {
                        rec.name     = GetControlText(hEditName);
                        rec.fname    = GetControlText(hEditFname);
                        rec.caste    = GetControlText(hEditCaste);
                        rec.district = GetControlText(hEditDistrict);
                        rec.gpa      = GetControlText(hEditGpa);
                        updated = true;
                        break;
                    }
                }

                if (updated) {
                    SaveAllRecords();
                    MessageBoxA(hwnd, "Student record updated successfully!", "Updated", MB_ICONINFORMATION);
                    ResetToAddMode();
                } else {
                    MessageBoxA(hwnd, "Roll Number not found! Cannot update Roll Number itself.", "Error", MB_ICONERROR);
                }
            }

            // DELETE BUTTON
            else if (wmId == ID_BTN_DELETE) {
                string targetRoll = GetControlText(hEditRoll);
                bool deleted = false;

                for (size_t i = 0; i < records.size(); i++) {
                    if (records[i].roll == targetRoll) {
                        records.erase(records.begin() + i);
                        deleted = true;
                        break;
                    }
                }

                if (deleted) {
                    SaveAllRecords();
                    MessageBoxA(hwnd, "Student record deleted successfully!", "Deleted", MB_ICONINFORMATION);
                    ResetToAddMode();
                } else {
                    MessageBoxA(hwnd, "Record not found!", "Error", MB_ICONERROR);
                }
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "StudentRecordWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Student Record Management System",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 570, 390,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}