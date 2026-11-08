#include <windows.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Student {
    string name;
    string roll;
    string father;
    string caste;
    string district;
    string gpa;
};

const char* DATA_FILE = "students.txt";

enum {
    ID_NAME = 101,
    ID_ROLL,
    ID_FATHER,
    ID_CASTE,
    ID_DISTRICT,
    ID_GPA,
    ID_ADD,
    ID_UPDATE,
    ID_DELETE,
    ID_NEXT
};

HWND hName, hRoll, hFather, hCaste, hDistrict, hGpa;
HWND hAdd, hUpdate, hDelete, hNext;
HWND hStatus;
vector<Student> students;
int currentIndex = -1;
bool viewingMode = false;

string readField(HWND control) {
    char buffer[512];
    GetWindowTextA(control, buffer, sizeof(buffer));
    return string(buffer);
}

void setField(HWND control, const string& value) {
    SetWindowTextA(control, value.c_str());
}

void clearFields() {
    setField(hName, "");
    setField(hRoll, "");
    setField(hFather, "");
    setField(hCaste, "");
    setField(hDistrict, "");
    setField(hGpa, "");
}

void showStudent(int index) {
    if (index < 0 || index >= (int)students.size()) return;

    setField(hName, students[index].name);
    setField(hRoll, students[index].roll);
    setField(hFather, students[index].father);
    setField(hCaste, students[index].caste);
    setField(hDistrict, students[index].district);
    setField(hGpa, students[index].gpa);
}

void setStatus(const string& message) {
    SetWindowTextA(hStatus, message.c_str());
}

void updateButtons() {
    EnableWindow(hAdd, viewingMode ? FALSE : TRUE);
    EnableWindow(hUpdate, viewingMode ? TRUE : FALSE);
    EnableWindow(hDelete, viewingMode ? TRUE : FALSE);
    EnableWindow(hNext, students.empty() ? FALSE : TRUE);
}

bool validateFields(Student& s) {
    s.name = readField(hName);
    s.roll = readField(hRoll);
    s.father = readField(hFather);
    s.caste = readField(hCaste);
    s.district = readField(hDistrict);
    s.gpa = readField(hGpa);

    if (s.name.empty() || s.roll.empty() || s.father.empty() ||
        s.caste.empty() || s.district.empty() || s.gpa.empty()) {
        MessageBoxA(NULL, "Please fill in all six fields.", "Missing Information",
                    MB_OK | MB_ICONWARNING);
        return false;
    }

    for (size_t i = 0; i < s.gpa.length(); ++i) {
        if ((s.gpa[i] < '0' || s.gpa[i] > '9') && s.gpa[i] != '.') {
            MessageBoxA(NULL, "GPA must be a number, for example 3.50.",
                        "Invalid GPA", MB_OK | MB_ICONWARNING);
            return false;
        }
    }

    double gpaValue = atof(s.gpa.c_str());
    if (gpaValue < 0.0 || gpaValue > 4.0) {
        MessageBoxA(NULL, "Please enter a GPA between 0.00 and 4.00.",
                    "Invalid GPA", MB_OK | MB_ICONWARNING);
        return false;
    }

    // Keep the comma-separated text file readable.
    string* fields[6] = { &s.name, &s.roll, &s.father, &s.caste, &s.district, &s.gpa };
    for (int i = 0; i < 6; ++i) {
        for (size_t j = 0; j < fields[i]->length(); ++j) {
            if ((*fields[i])[j] == ',') (*fields[i])[j] = ' ';
        }
    }
    return true;
}

void saveStudents() {
    ofstream file(DATA_FILE);
    for (size_t i = 0; i < students.size(); ++i) {
        file << students[i].name << ","
             << students[i].roll << ","
             << students[i].father << ","
             << students[i].caste << ","
             << students[i].district << ","
             << students[i].gpa << "\n";
    }
}

void loadStudents() {
    students.clear();
    ifstream file(DATA_FILE);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        Student s;
        if (getline(ss, s.name, ',') &&
            getline(ss, s.roll, ',') &&
            getline(ss, s.father, ',') &&
            getline(ss, s.caste, ',') &&
            getline(ss, s.district, ',') &&
            getline(ss, s.gpa)) {
            students.push_back(s);
        }
    }
}

void addStudent(HWND hwnd) {
    if (viewingMode) return;

    Student s;
    if (!validateFields(s)) return;

    for (size_t i = 0; i < students.size(); ++i) {
        if (students[i].roll == s.roll) {
            MessageBoxA(hwnd, "A student with this Roll Number already exists.",
                        "Duplicate Roll Number", MB_OK | MB_ICONWARNING);
            return;
        }
    }

    students.push_back(s);
    saveStudents();
    clearFields();
    currentIndex = -1;
    setStatus("Student record added and saved successfully.");
    updateButtons();
    MessageBoxA(hwnd, "Student record added successfully!",
                "Success", MB_OK | MB_ICONINFORMATION);
}

void updateStudent(HWND hwnd) {
    if (!viewingMode || currentIndex < 0 || currentIndex >= (int)students.size()) return;

    Student s;
    if (!validateFields(s)) return;

    // Roll Number is the unique identifier. Do not allow it to match another record.
    for (size_t i = 0; i < students.size(); ++i) {
        if ((int)i != currentIndex && students[i].roll == s.roll) {
            MessageBoxA(hwnd, "Another student already uses this Roll Number.",
                        "Duplicate Roll Number", MB_OK | MB_ICONWARNING);
            return;
        }
    }

    students[currentIndex] = s;
    saveStudents();
    showStudent(currentIndex);
    setStatus("Student record updated successfully.");
    MessageBoxA(hwnd, "Student record updated successfully!",
                "Success", MB_OK | MB_ICONINFORMATION);
}

void deleteStudent(HWND hwnd) {
    if (!viewingMode || currentIndex < 0 || currentIndex >= (int)students.size()) return;

    int answer = MessageBoxA(hwnd, "Are you sure you want to delete this student record?",
                             "Confirm Delete", MB_YESNO | MB_ICONQUESTION);
    if (answer != IDYES) return;

    students.erase(students.begin() + currentIndex);
    saveStudents();

    if (students.empty()) {
        currentIndex = -1;
        viewingMode = false;
        clearFields();
        setStatus("All records deleted. You can add a new student.");
    } else {
        if (currentIndex >= (int)students.size()) currentIndex = (int)students.size() - 1;
        showStudent(currentIndex);
        setStatus("Student record deleted. Showing an available record.");
    }

    updateButtons();
}

void nextStudent(HWND hwnd) {
    if (students.empty()) {
        MessageBoxA(hwnd, "There are no student records yet. Add a student first.",
                    "No Records", MB_OK | MB_ICONINFORMATION);
        return;
    }

    viewingMode = true;
    if (currentIndex < 0 || currentIndex >= (int)students.size()) {
        currentIndex = 0;
    } else {
        currentIndex = (currentIndex + 1) % (int)students.size();
    }

    showStudent(currentIndex);
    setStatus("Viewing record " + to_string(currentIndex + 1) +
              " of " + to_string(students.size()) + ".");
    updateButtons();
}

void createLabel(HWND hwnd, const char* text, int x, int y) {
    CreateWindowExA(0, "STATIC", text, WS_CHILD | WS_VISIBLE,
                    x, y, 125, 24, hwnd, NULL, NULL, NULL);
}

HWND createInput(HWND hwnd, int id, int x, int y) {
    return CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                           WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                           x, y, 300, 27, hwnd, (HMENU)(INT_PTR)id, NULL, NULL);
}

void createButton(HWND hwnd, const char* text, int id, int x, int y, int width) {
    HWND button = CreateWindowExA(0, "BUTTON", text,
                                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  x, y, width, 34, hwnd, (HMENU)(INT_PTR)id, NULL, NULL);
    if (id == ID_ADD) hAdd = button;
    if (id == ID_UPDATE) hUpdate = button;
    if (id == ID_DELETE) hDelete = button;
    if (id == ID_NEXT) hNext = button;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            HWND title = CreateWindowExA(0, "STATIC", "STUDENT RECORD MANAGEMENT SYSTEM",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 25, 18, 510, 32, hwnd, NULL, NULL, NULL);

            createLabel(hwnd, "Student Name:", 35, 75);
            hName = createInput(hwnd, ID_NAME, 170, 72);

            createLabel(hwnd, "Roll Number:", 35, 115);
            hRoll = createInput(hwnd, ID_ROLL, 170, 112);

            createLabel(hwnd, "Father's Name:", 35, 155);
            hFather = createInput(hwnd, ID_FATHER, 170, 152);

            createLabel(hwnd, "Caste:", 35, 195);
            hCaste = createInput(hwnd, ID_CASTE, 170, 192);

            createLabel(hwnd, "District:", 35, 235);
            hDistrict = createInput(hwnd, ID_DISTRICT, 170, 232);

            createLabel(hwnd, "GPA (0.00 - 4.00):", 35, 275);
            hGpa = createInput(hwnd, ID_GPA, 170, 272);

            createButton(hwnd, "Add", ID_ADD, 35, 330, 105);
            createButton(hwnd, "Update", ID_UPDATE, 155, 330, 105);
            createButton(hwnd, "Delete", ID_DELETE, 275, 330, 105);
            createButton(hwnd, "Next", ID_NEXT, 395, 330, 105);

            hStatus = CreateWindowExA(0, "STATIC", "Ready. Enter student details to add a record.",
                WS_CHILD | WS_VISIBLE, 35, 385, 490, 35, hwnd, NULL, NULL, NULL);

            SendMessageA(title, WM_SETFONT, (WPARAM)font, TRUE);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)font, TRUE);

            HWND controls[] = { hName, hRoll, hFather, hCaste, hDistrict, hGpa,
                                hAdd, hUpdate, hDelete, hNext };
            for (int i = 0; i < 10; ++i)
                SendMessageA(controls[i], WM_SETFONT, (WPARAM)font, TRUE);

            loadStudents();
            viewingMode = false;
            currentIndex = -1;
            updateButtons();
            if (!students.empty())
                setStatus("Existing records loaded. Click Next to view them.");
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_ADD: addStudent(hwnd); return 0;
                case ID_UPDATE: updateStudent(hwnd); return 0;
                case ID_DELETE: deleteStudent(hwnd); return 0;
                case ID_NEXT: nextStudent(hwnd); return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "StudentRecordManagementWindow";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Could not register the application window.",
                    "Startup Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "Student Record Management System",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 570, 490,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        MessageBoxA(NULL, "Could not create the application window.",
                    "Startup Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
