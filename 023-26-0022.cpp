#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
constexpr int kGround = 0;
constexpr int kCeiling = 10;
constexpr int kMaxTurns = 10;
constexpr int kWindowWidth = 1000;
constexpr int kWindowHeight = 700;
constexpr UINT_PTR kAnimationTimer = 1;

enum class Screen {
    Ready,
    Flying,
    Crashed,
    Cleared
};

struct GameState {
    int altitude = 5;
    int score = 0;
    int turns = 0;
    int totalAltitude = 0;
    Screen screen = Screen::Ready;
    int animationFrame = 0;
};

GameState game;
HFONT fonts[5]{};
POINT mousePosition{};

constexpr RECT kFlapButton{48, 610, 535, 674};
constexpr RECT kWaitButton{551, 610, 952, 674};

HFONT MakeFont(int height, int weight) {
    return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

void CreateFonts() {
    fonts[0] = MakeFont(14, FW_NORMAL);
    fonts[1] = MakeFont(16, FW_NORMAL);
    fonts[2] = MakeFont(18, FW_SEMIBOLD);
    fonts[3] = MakeFont(21, FW_BOLD);
    fonts[4] = MakeFont(34, FW_BOLD);
}

void DeleteFonts() {
    for (HFONT& font : fonts) {
        if (font) {
            DeleteObject(font);
            font = nullptr;
        }
    }
}

void Fill(HDC dc, int left, int top, int right, int bottom, COLORREF color) {
    RECT rect{left, top, right, bottom};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
}

void RoundedBox(HDC dc, int left, int top, int right, int bottom,
    int radius, COLORREF fillColor, COLORREF borderColor) {
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, left, top, right, bottom, radius, radius);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void Text(HDC dc, const wchar_t* value, RECT rect, HFONT font,
    COLORREF color, UINT alignment = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HGDIOBJ oldFont = SelectObject(dc, font);
    DrawTextW(dc, value, -1, &rect, alignment);
    SelectObject(dc, oldFont);
}

void TextAt(HDC dc, const wchar_t* value, int left, int top, int right,
    int bottom, HFONT font, COLORREF color,
    UINT alignment = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    RECT rect{left, top, right, bottom};
    Text(dc, value, rect, font, color, alignment);
}

void Gradient(HDC dc, RECT rect, COLORREF topColor, COLORREF bottomColor) {
    const int height = rect.bottom - rect.top;
    for (int y = 0; y < height; ++y) {
        const double ratio = height > 1
            ? static_cast<double>(y) / (height - 1)
            : 0.0;
        const int red = static_cast<int>(GetRValue(topColor) * (1.0 - ratio)
            + GetRValue(bottomColor) * ratio);
        const int green = static_cast<int>(GetGValue(topColor) * (1.0 - ratio)
            + GetGValue(bottomColor) * ratio);
        const int blue = static_cast<int>(GetBValue(topColor) * (1.0 - ratio)
            + GetBValue(bottomColor) * ratio);
        Fill(dc, rect.left, rect.top + y, rect.right, rect.top + y + 1,
            RGB(red, green, blue));
    }
}

void Ellipse(HDC dc, int left, int top, int right, int bottom,
    COLORREF fillColor, COLORREF outlineColor) {
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, outlineColor);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    ::Ellipse(dc, left, top, right, bottom);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void Polygon(HDC dc, const POINT* points, int count, COLORREF fillColor,
    COLORREF outlineColor) {
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, outlineColor);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    ::Polygon(dc, points, count);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

bool Contains(const RECT& rect, POINT point) {
    return point.x >= rect.left && point.x < rect.right
        && point.y >= rect.top && point.y < rect.bottom;
}

bool IsTerminal() {
    return game.screen == Screen::Crashed || game.screen == Screen::Cleared;
}

void DrawCloud(HDC dc, int x, int y) {
    const COLORREF cloud = RGB(213, 237, 248);
    const COLORREF edge = RGB(213, 237, 248);
    Ellipse(dc, x, y + 10, x + 48, y + 34, cloud, edge);
    Ellipse(dc, x + 20, y, x + 59, y + 34, cloud, edge);
    Ellipse(dc, x + 42, y + 9, x + 82, y + 34, cloud, edge);
    Fill(dc, x + 12, y + 20, x + 68, y + 33, cloud);
}

void DrawBird(HDC dc, int centerX, int centerY) {
    const COLORREF gold = RGB(255, 190, 74);
    Ellipse(dc, centerX - 31, centerY - 20, centerX + 31, centerY + 20,
        gold, RGB(245, 163, 50));

    POINT wing[]{{centerX - 14, centerY + 1}, {centerX + 5, centerY + 4},
        {centerX - 11, centerY + 14}};
    Polygon(dc, wing, 3, RGB(243, 158, 51), RGB(243, 158, 51));

    POINT beak[]{{centerX + 26, centerY - 5}, {centerX + 44, centerY + 1},
        {centerX + 26, centerY + 7}};
    Polygon(dc, beak, 3, RGB(255, 131, 91), RGB(255, 131, 91));
    Ellipse(dc, centerX + 10, centerY - 12, centerX + 17, centerY - 5,
        RGB(20, 40, 57), RGB(20, 40, 57));
    Ellipse(dc, centerX + 12, centerY - 11, centerX + 14, centerY - 9,
        RGB(255, 255, 255), RGB(255, 255, 255));
}

void DrawMetric(HDC dc, RECT rect, const wchar_t* label,
    const std::wstring& value, const wchar_t* detail, COLORREF accent) {
    RoundedBox(dc, rect.left, rect.top, rect.right, rect.bottom, 14,
        RGB(17, 34, 51), RGB(34, 56, 75));
    Fill(dc, rect.left + 18, rect.top + 18, rect.left + 22,
        rect.bottom - 18, accent);
    TextAt(dc, label, rect.left + 36, rect.top + 13, rect.right - 15,
        rect.top + 36, fonts[0], RGB(151, 173, 190));
    TextAt(dc, value.c_str(), rect.left + 36, rect.top + 34,
        rect.right - 15, rect.top + 69, fonts[3], RGB(242, 247, 251));
    TextAt(dc, detail, rect.left + 36, rect.top + 69,
        rect.right - 15, rect.bottom - 8, fonts[0], RGB(126, 151, 169));
}

void DrawButton(HDC dc, RECT rect, const wchar_t* title,
    const wchar_t* hint, bool primary, bool enabled, bool hovered) {
    const COLORREF background = primary
        ? (hovered ? RGB(96, 226, 204) : RGB(64, 208, 186))
        : (hovered ? RGB(33, 56, 76) : RGB(24, 43, 61));
    const COLORREF border = primary ? background : RGB(50, 73, 91);
    const COLORREF mainText = primary ? RGB(7, 37, 43) : RGB(231, 239, 245);
    const COLORREF hintText = primary ? RGB(30, 80, 82) : RGB(150, 173, 190);
    RoundedBox(dc, rect.left, rect.top, rect.right, rect.bottom, 12,
        background, border);

    if (!enabled) {
        RoundedBox(dc, rect.left, rect.top, rect.right, rect.bottom, 12,
            RGB(28, 43, 56), RGB(43, 60, 75));
    }

    TextAt(dc, title, rect.left + 22, rect.top + 9, rect.right - 22,
        rect.top + 36, fonts[2],
        enabled ? mainText : RGB(153, 169, 182));
    TextAt(dc, hint, rect.left + 22, rect.top + 34, rect.right - 22,
        rect.bottom - 7, fonts[0],
        enabled ? hintText : RGB(110, 130, 146));
}

void DrawScene(HDC dc) {
    RECT scene{48, 132, 952, 442};
    RoundedBox(dc, scene.left, scene.top, scene.right, scene.bottom, 18,
        RGB(38, 66, 87), RGB(48, 78, 99));

    RECT sky{52, 136, 948, 438};
    Gradient(dc, sky, RGB(37, 93, 124), RGB(139, 199, 210));

    const int drift = (game.animationFrame / 2) % 1030;
    DrawCloud(dc, 110 - drift, 191);
    DrawCloud(dc, 476 - drift, 231);
    DrawCloud(dc, 855 - drift, 174);
    DrawCloud(dc, 120 - ((drift + 410) % 1030), 268);
    DrawCloud(dc, 610 - ((drift + 690) % 1030), 160);

    POINT farMountains[]{{52, 364}, {172, 286}, {260, 353}, {389, 270},
        {526, 360}, {648, 296}, {786, 365}, {884, 302}, {948, 349},
        {948, 438}, {52, 438}};
    Polygon(dc, farMountains, 11, RGB(90, 160, 169), RGB(90, 160, 169));
    POINT nearMountains[]{{52, 391}, {185, 327}, {303, 387}, {444, 319},
        {576, 390}, {730, 333}, {859, 393}, {948, 344}, {948, 438},
        {52, 438}};
    Polygon(dc, nearMountains, 10, RGB(61, 132, 143), RGB(61, 132, 143));

    HPEN guidePen = CreatePen(PS_DOT, 1, RGB(237, 249, 242));
    HGDIOBJ oldPen = SelectObject(dc, guidePen);
    MoveToEx(dc, 76, 171, nullptr);
    LineTo(dc, 920, 171);
    MoveToEx(dc, 76, 406, nullptr);
    LineTo(dc, 920, 406);
    SelectObject(dc, oldPen);
    DeleteObject(guidePen);

    TextAt(dc, L"CEILING", 77, 145, 180, 168, fonts[0],
        RGB(239, 250, 246));
    TextAt(dc, L"GROUND", 77, 407, 180, 430, fonts[0],
        RGB(239, 250, 246));

    const int birdY = 406 - (game.altitude * 235 / kCeiling);
    DrawBird(dc, 479, birdY);

    RoundedBox(dc, 899, 181, 925, 397, 10, RGB(24, 64, 79),
        RGB(220, 243, 235));
    const int meterFill = std::max(0, std::min(210,
        game.altitude * 210 / kCeiling));
    if (meterFill > 0) {
        RoundedBox(dc, 906, 187 + (210 - meterFill), 918, 397, 7,
            RGB(72, 223, 188), RGB(72, 223, 188));
    }

    if (game.screen == Screen::Ready) {
        RoundedBox(dc, 284, 251, 675, 329, 14, RGB(13, 37, 53),
            RGB(57, 106, 122));
        TextAt(dc, L"READY FOR TAKEOFF", 304, 259, 655, 292, fonts[3],
            RGB(241, 248, 250), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        TextAt(dc, L"Flap to climb. Stay between the sky lines.",
            304, 293, 655, 319, fonts[1], RGB(185, 210, 219),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else if (game.screen == Screen::Crashed) {
        RoundedBox(dc, 293, 245, 667, 336, 14, RGB(55, 32, 43),
            RGB(163, 85, 88));
        TextAt(dc, L"FLIGHT ENDED", 310, 252, 650, 285, fonts[3],
            RGB(255, 206, 195), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        TextAt(dc, L"Your bird flew outside the safe zone.",
            310, 287, 650, 319, fonts[1], RGB(225, 185, 184),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else if (game.screen == Screen::Cleared) {
        RoundedBox(dc, 293, 245, 667, 336, 14, RGB(19, 59, 57),
            RGB(67, 178, 147));
        TextAt(dc, L"FLIGHT CLEARED!", 310, 252, 650, 285, fonts[3],
            RGB(182, 255, 224), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        TextAt(dc, L"Ten turns in the air. Beautiful flying!",
            310, 287, 650, 319, fonts[1], RGB(188, 226, 213),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        std::wstringstream status;
        status << L"ALTITUDE  " << game.altitude << L" / " << kCeiling;
        TextAt(dc, status.str().c_str(), 755, 146, 914, 168, fonts[0],
            RGB(244, 250, 247), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
}

void DrawGame(HDC dc) {
    Fill(dc, 0, 0, kWindowWidth, kWindowHeight, RGB(9, 21, 34));

    Ellipse(dc, 48, 39, 78, 69, RGB(61, 208, 186), RGB(61, 208, 186));
    TextAt(dc, L"S", 48, 39, 78, 68, fonts[2], RGB(8, 35, 45),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    TextAt(dc, L"SKY DODGER", 91, 33, 293, 59, fonts[3],
        RGB(239, 246, 250));
    TextAt(dc, L"FLIGHT TRAINING  /  SURVIVE THE SKY", 92, 58, 420, 81,
        fonts[0], RGB(125, 153, 173));
    RoundedBox(dc, 781, 38, 952, 70, 16, RGB(18, 37, 54),
        RGB(42, 66, 84));
    TextAt(dc, L"GROUP  023-26-0022", 792, 40, 942, 68, fonts[0],
        RGB(168, 190, 204), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    TextAt(dc, L"Your next move matters.", 48, 91, 650, 124, fonts[4],
        RGB(243, 247, 250));
    DrawScene(dc);

    std::wstringstream scoreValue;
    scoreValue << std::setw(2) << std::setfill(L'0') << game.score;
    std::wstringstream turnValue;
    turnValue << game.turns << L" / " << kMaxTurns;
    std::wstringstream altitudeValue;
    altitudeValue << game.altitude << L" / " << kCeiling;

    DrawMetric(dc, RECT{48, 465, 336, 575}, L"SCORE",
        scoreValue.str(), L"points earned", RGB(69, 211, 188));
    DrawMetric(dc, RECT{356, 465, 644, 575}, L"TURN",
        turnValue.str(), L"safe turns survived", RGB(113, 179, 250));
    DrawMetric(dc, RECT{664, 465, 952, 575}, L"ALTITUDE",
        altitudeValue.str(), L"keep clear of both edges", RGB(255, 190, 74));

    const bool ready = game.screen == Screen::Ready;
    const bool flying = game.screen == Screen::Flying;
    const bool terminal = IsTerminal();
    if (ready) {
        DrawButton(dc, kFlapButton, L"START FLIGHT",
            L"Click here or press Enter to begin", true, true,
            Contains(kFlapButton, mousePosition));
        RoundedBox(dc, kWaitButton.left, kWaitButton.top, kWaitButton.right,
            kWaitButton.bottom, 12, RGB(17, 34, 51), RGB(34, 56, 75));
        TextAt(dc, L"HOW TO PLAY", kWaitButton.left + 20,
            kWaitButton.top + 8, kWaitButton.right - 15,
            kWaitButton.top + 32, fonts[2], RGB(223, 234, 242));
        TextAt(dc, L"W / click to flap up     N / click to wait",
            kWaitButton.left + 20, kWaitButton.top + 33,
            kWaitButton.right - 15, kWaitButton.bottom - 6, fonts[0],
            RGB(146, 171, 188));
    } else if (terminal) {
        std::wstringstream result;
        result << L"FINAL SCORE  " << game.score << L"     ";
        if (game.turns == 0) {
            result << L"AVERAGE ALTITUDE  0.00";
        } else {
            result << L"AVERAGE ALTITUDE  " << std::fixed
                << std::setprecision(2)
                << static_cast<double>(game.totalAltitude) / game.turns;
        }
        DrawButton(dc, kFlapButton, L"FLY AGAIN",
            L"Click here or press Enter to restart", true, true,
            Contains(kFlapButton, mousePosition));
        RoundedBox(dc, kWaitButton.left, kWaitButton.top, kWaitButton.right,
            kWaitButton.bottom, 12, RGB(17, 34, 51), RGB(34, 56, 75));
        TextAt(dc, result.str().c_str(), kWaitButton.left + 20,
            kWaitButton.top + 10, kWaitButton.right - 16,
            kWaitButton.bottom - 10, fonts[1], RGB(181, 203, 216));
    } else {
        DrawButton(dc, kFlapButton, L"FLAP UP",
            L"+2 altitude     W / SPACE", true, flying,
            Contains(kFlapButton, mousePosition));
        DrawButton(dc, kWaitButton, L"WAIT",
            L"-1 altitude     N / DOWN", false, flying,
            Contains(kWaitButton, mousePosition));
    }
}

void ResetGame() {
    game = GameState{};
}

void MakeMove(HWND window, bool flap) {
    if (game.screen == Screen::Ready || IsTerminal()) {
        ResetGame();
        game.screen = Screen::Flying;
        if (!flap) {
            game.altitude -= 1;
        } else {
            game.altitude += 2;
        }
    } else if (game.screen == Screen::Flying) {
        game.altitude += flap ? 2 : -1;
    } else {
        return;
    }

    if (game.altitude <= kGround || game.altitude >= kCeiling) {
        game.screen = Screen::Crashed;
    } else {
        ++game.turns;
        ++game.score;
        game.totalAltitude += game.altitude;
        if (game.turns >= kMaxTurns) {
            game.screen = Screen::Cleared;
        }
    }
    InvalidateRect(window, nullptr, FALSE);
}

void DrawFrame(HWND window) {
    PAINTSTRUCT paint{};
    HDC screenDc = BeginPaint(window, &paint);
    RECT client{};
    GetClientRect(window, &client);
    HDC bufferDc = CreateCompatibleDC(screenDc);
    HBITMAP bitmap = CreateCompatibleBitmap(screenDc,
        client.right, client.bottom);
    HGDIOBJ oldBitmap = SelectObject(bufferDc, bitmap);

    DrawGame(bufferDc);
    BitBlt(screenDc, 0, 0, client.right, client.bottom, bufferDc,
        0, 0, SRCCOPY);

    SelectObject(bufferDc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(bufferDc);
    EndPaint(window, &paint);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message,
    WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateFonts();
        SetTimer(window, kAnimationTimer, 40, nullptr);
        return 0;
    case WM_TIMER:
        ++game.animationFrame;
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_PAINT:
        DrawFrame(window);
        return 0;
    case WM_MOUSEMOVE: {
        POINT nextPosition{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (nextPosition.x != mousePosition.x
            || nextPosition.y != mousePosition.y) {
            mousePosition = nextPosition;
            InvalidateRect(window, nullptr, FALSE);
        }
        TRACKMOUSEEVENT tracking{sizeof(TRACKMOUSEEVENT), TME_LEAVE,
            window, 0};
        TrackMouseEvent(&tracking);
        return 0;
    }
    case WM_MOUSELEAVE:
        mousePosition = POINT{-1, -1};
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_LBUTTONUP: {
        POINT position{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (Contains(kFlapButton, position)) {
            if (game.screen == Screen::Ready || IsTerminal()) {
                ResetGame();
                game.screen = Screen::Flying;
                InvalidateRect(window, nullptr, FALSE);
            } else {
                MakeMove(window, true);
            }
        } else if (Contains(kWaitButton, position)
            && (game.screen == Screen::Ready || game.screen == Screen::Flying)) {
            MakeMove(window, false);
        }
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_RETURN && (game.screen == Screen::Ready
            || IsTerminal())) {
            ResetGame();
            game.screen = Screen::Flying;
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        }
        if ((wParam == 'W' || wParam == VK_SPACE) && !IsTerminal()
            && (game.screen == Screen::Ready || game.screen == Screen::Flying)) {
            MakeMove(window, true);
            return 0;
        }
        if ((wParam == 'N' || wParam == VK_DOWN)
            && (game.screen == Screen::Ready || game.screen == Screen::Flying)) {
            MakeMove(window, false);
            return 0;
        }
        break;
    case WM_DESTROY:
        KillTimer(window, kAnimationTimer);
        DeleteFonts();
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int RunGame(HINSTANCE instance, int showCommand) {
    const wchar_t className[] = L"SkyDodgerWindow";
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = nullptr;
    windowClass.lpszClassName = className;

    if (!RegisterClassExW(&windowClass)) {
        MessageBoxW(nullptr, L"Could not register the game window.",
            L"Sky Dodger", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT windowRect{0, 0, kWindowWidth, kWindowHeight};
    AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION
        | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    HWND window = CreateWindowExW(0, className, L"Sky Dodger",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, instance, nullptr);
    if (!window) {
        MessageBoxW(nullptr, L"Could not create the game window.",
            L"Sky Dodger", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

int main() {
    HWND console = GetConsoleWindow();
    if (console) {
        ShowWindow(console, SW_HIDE);
    }
    FreeConsole();
    return RunGame(GetModuleHandleW(nullptr), SW_SHOWDEFAULT);
}
