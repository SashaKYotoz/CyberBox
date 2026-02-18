#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "actions.h"

#define IDC_STATUS_LABEL 101
#define IDC_EDIT_BOX     102
#define IDC_BTN_SAVE     103
#define IDC_BTN_RUN      104

HWND hStatus, hEdit, hButton, hRunBtn;
char g_currentFile[MAX_PATH] = {0};
long g_currentOffset = -1;

HANDLE g_hPythonProcess = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void WideToAnsi(const wchar_t *wide, char *ansi, int size) {
    WideCharToMultiByte(CP_ACP, 0, wide, -1, ansi, size, NULL, NULL);
}

void ProcessDroppedFile(HWND hwnd, const wchar_t *wPath) {
    WideToAnsi(wPath, g_currentFile, MAX_PATH);
    g_currentOffset = find_payload_offset(g_currentFile);

    char statusMsg[512];
    SetWindowTextW(hEdit, L"");

    if (g_currentOffset == -1) {
        snprintf(statusMsg, sizeof(statusMsg), "Error: Not a valid PNG/JPG or file not found: %s", g_currentFile);
        SetWindowTextA(hStatus, statusMsg);
        EnableWindow(hButton, FALSE);
        return;
    }

    long size = 0;
    unsigned char *data = read_hidden_data(g_currentFile, g_currentOffset, &size);

    if (data && size > 0) {
        char *safeData = malloc(size + 1);
        memcpy(safeData, data, size);
        safeData[size] = '\0';

        SetWindowTextA(hEdit, (char *) safeData);
        snprintf(statusMsg, sizeof(statusMsg), "File Loaded. Hidden data found (%ld bytes).", size);
        free(safeData);
        free(data);
    } else {
        snprintf(statusMsg, sizeof(statusMsg), "File Loaded. No hidden data detected. Ready to add.");
    }

    SetWindowTextA(hStatus, statusMsg);
    EnableWindow(hButton, TRUE);
    EnableWindow(hRunBtn, TRUE);
}

void SaveData(HWND hwnd) {
    if (g_currentOffset == -1 || strlen(g_currentFile) == 0) return;

    int len = GetWindowTextLength(hEdit);
    wchar_t *wBuffer = (wchar_t *) malloc((len + 1) * sizeof(wchar_t));
    GetWindowTextW(hEdit, wBuffer, len + 1);

    int ansiLen = WideCharToMultiByte(CP_ACP, 0, wBuffer, -1, NULL, 0, NULL, NULL);
    char *ansiBuffer = (char *) malloc(ansiLen);
    WideCharToMultiByte(CP_ACP, 0, wBuffer, -1, ansiBuffer, ansiLen, NULL, NULL);

    modify_hidden_data(g_currentFile, g_currentOffset, (unsigned char *) ansiBuffer, ansiLen - 1);

    MessageBox(hwnd, L"Data written to image successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

    free(wBuffer);
    free(ansiBuffer);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"DataManagerClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, L"MAINICON");

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Steganography Data Manager", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hStatus = CreateWindow(L"STATIC", L"Drag and drop a JPG or PNG file here...",
                                   WS_CHILD | WS_VISIBLE | SS_LEFT,
                                   10, 10, 560, 40, hwnd, (HMENU)IDC_STATUS_LABEL, NULL, NULL);

            hEdit = CreateWindow(L"EDIT", L"",
                                 WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                                 10, 60, 560, 330, hwnd, (HMENU)IDC_EDIT_BOX, NULL, NULL);

            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     FIXED_PITCH | FF_MODERN, L"Consolas");
            SendMessage(hEdit, WM_SETFONT, (WPARAM) hFont, TRUE);

            hButton = CreateWindow(L"BUTTON", L"Save to Image",
                                   WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                   0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_SAVE, NULL, NULL);

            hRunBtn = CreateWindow(L"BUTTON", L"Run as Python",
                                   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   0, 0, 0, 0, hwnd, (HMENU)IDC_BTN_RUN, NULL, NULL);

            EnableWindow(hButton, FALSE);
            EnableWindow(hRunBtn, FALSE);
            break;
        }

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP) wParam;
            wchar_t wPath[MAX_PATH];
            if (DragQueryFile(hDrop, 0, wPath, MAX_PATH)) {
                ProcessDroppedFile(hwnd, wPath);
            }
            DragFinish(hDrop);
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BTN_SAVE) {
                SaveData(hwnd);
            }
            if (LOWORD(wParam) == IDC_BTN_RUN) {
                int len = GetWindowTextLength(hEdit);
                if (len > 0) {
                    wchar_t *wBuffer = malloc((len + 1) * sizeof(wchar_t));
                    GetWindowTextW(hEdit, wBuffer, len + 1);
                    int ansiLen = WideCharToMultiByte(CP_ACP, 0, wBuffer, -1, NULL, 0, NULL, NULL);
                    char *ansiBuffer = malloc(ansiLen);
                    WideCharToMultiByte(CP_ACP, 0, wBuffer, -1, ansiBuffer, ansiLen, NULL, NULL);

                    if (g_hPythonProcess != NULL) {
                        TerminateProcess(g_hPythonProcess, 0);
                        CloseHandle(g_hPythonProcess);
                        g_hPythonProcess = NULL;
                    }

                    SECURITY_ATTRIBUTES saAttr;
                    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                    saAttr.bInheritHandle = TRUE;
                    saAttr.lpSecurityDescriptor = NULL;

                    HANDLE hReadPipe = NULL, hWritePipe = NULL;
                    if (CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {

                        SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0);

                        STARTUPINFOA si;
                        PROCESS_INFORMATION pi;
                        ZeroMemory(&si, sizeof(si));
                        ZeroMemory(&pi, sizeof(pi));
                        si.cb = sizeof(si);

                        si.hStdInput = hReadPipe;
                        si.dwFlags |= STARTF_USESTDHANDLES;

                        char cmd[] = "py.exe";
                        BOOL bSuccess = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

                        if (bSuccess) {
                            g_hPythonProcess = pi.hProcess;
                            CloseHandle(pi.hThread);

                            DWORD bytesWritten;
                            WriteFile(hWritePipe, ansiBuffer, ansiLen - 1, &bytesWritten, NULL);
                        } else {
                            MessageBox(hwnd, L"Failed to start Python. Make sure 'py' is in your system PATH.", L"Error", MB_OK | MB_ICONERROR);
                        }

                        CloseHandle(hWritePipe);
                        CloseHandle(hReadPipe);
                    }

                    free(wBuffer);
                    free(ansiBuffer);
                }
            }
            break;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            MoveWindow(hStatus, 10, 10, width - 20, 40, TRUE);
            MoveWindow(hEdit, 10, 60, width - 20, height - 120, TRUE);
            int gap = 10;
            int totalBtnWidth = width - 20;
            int singleBtnWidth = (totalBtnWidth - gap) / 2;
            int btnY = height - 50;

            MoveWindow(hButton, 10, btnY, singleBtnWidth, 40, TRUE);
            MoveWindow(hRunBtn, 10 + singleBtnWidth + gap, btnY, singleBtnWidth, 40, TRUE);
            break;
        }

        case WM_DESTROY:
            if (g_hPythonProcess != NULL) {
                TerminateProcess(g_hPythonProcess, 0);
                CloseHandle(g_hPythonProcess);
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}