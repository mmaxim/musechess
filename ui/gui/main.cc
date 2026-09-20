#include <windows.h>
#include <string>
#include <cstdio>

#include "game/game.h"
#include "movegen/board.h"
#include "movegen/movegen.h"
#include "search/search.h"
#include "eval/eval.h"

using namespace chess;

constexpr int BOARD_SIZE = 8;
constexpr int SQUARE_PX = 60;
constexpr int BOARD_PX = BOARD_SIZE * SQUARE_PX;
constexpr int WINDOW_W = BOARD_PX + 16;
constexpr int WINDOW_H = BOARD_PX + 58;

struct AppState {
  Board board;
  bool dragging = false;
  int drag_from = -1;
  POINT drag_offset{0,0};
  POINT mouse_pos{0,0};
  char status[256] = "White to move";
};

AppState g_state;

int square_from_point(POINT pt) {
  int x = pt.x - 8;
  int y = pt.y - 30;
  if (x < 0 || y < 0) return -1;
  int col = x / SQUARE_PX;
  int row = y / SQUARE_PX;
  if (col >= BOARD_SIZE || row >= BOARD_SIZE) return -1;
  int rank = BOARD_SIZE - 1 - row;
  return rank * 8 + col;
}

std::wstring piece_to_wstring(Piece p) {
  if (p == static_cast<Piece>(kNumPieces)) return L".";
  static const wchar_t* white_sym[] = {L"♙",L"♘",L"♗",L"♖",L"♕",L"♔"};
  static const wchar_t* black_sym[] = {L"♟",L"♞",L"♝",L"♜",L"♛",L"♚"};
  int idx = static_cast<int>(p) % 6;
  if (p < kBlackPawn) return white_sym[idx];
  return black_sym[idx];
}

void make_computer_move(HWND hwnd) {
  if (g_state.board.side_to_move != Color::Black) return;
  MaterialEvaluator eval;
  Search<MaterialEvaluator> searcher(eval);
  // Simple thinking output
  char buf[256];
  sprintf_s(buf, sizeof(buf), "Engine thinking depth 3...");
  OutputDebugStringA(buf);
  sprintf_s(buf, sizeof(buf), "Engine thinking depth 3...");
  strncpy_s(g_state.status, buf, _TRUNCATE);
  InvalidateRect(hwnd, nullptr, TRUE);
  auto result = searcher.search(g_state.board, 3);
  sprintf_s(buf, sizeof(buf), "Engine move %s score %d nodes %d",
    result.best_move.to_string().c_str(), result.score, result.nodes);
  OutputDebugStringA(buf);
  sprintf_s(buf, sizeof(buf), "Black plays %s", result.best_move.to_string().c_str());
  strncpy_s(g_state.status, buf, _TRUNCATE);
  g_state.board.make_move(result.best_move);
  InvalidateRect(hwnd, nullptr, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE: {
      g_state.board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
      strncpy_s(g_state.status, "White to move", _TRUNCATE);
      return 0;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int f = 0; f < BOARD_SIZE; ++f) {
          int x = 8 + f * SQUARE_PX;
          int y = 30 + r * SQUARE_PX;
          bool light = (r + f) % 2 == 0;
          HBRUSH br = CreateSolidBrush(light ? RGB(240,217,181) : RGB(181,136,99));
          RECT sq{ x, y, x+SQUARE_PX, y+SQUARE_PX };
          FillRect(hdc, &sq, br);
          DeleteObject(br);
        }
      }
      SetBkMode(hdc, TRANSPARENT);
      SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
      HFONT hFont = CreateFontW(36,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"Segoe UI Symbol");
      HFONT old = (HFONT)SelectObject(hdc, hFont);
      for (int sq = 0; sq < 64; ++sq) {
        Piece p = g_state.board.piece_at(sq);
        if (p == static_cast<Piece>(kNumPieces)) continue;
        int f = sq % 8;
        int r = sq / 8;
        int x = 8 + f * SQUARE_PX + SQUARE_PX/2;
        int y = 30 + (7 - r) * SQUARE_PX + SQUARE_PX/2 + 12;
        std::wstring s = piece_to_wstring(p);
        SetTextColor(hdc, color_of_piece(p) == Color::White ? RGB(240,240,240) : RGB(20,20,20));
        TextOutW(hdc, x, y, s.c_str(), (int)s.size());
      }
      if (g_state.dragging && g_state.drag_from >= 0) {
        Piece p = g_state.board.piece_at(g_state.drag_from);
        if (p != static_cast<Piece>(kNumPieces)) {
          std::wstring s = piece_to_wstring(p);
          int dx = g_state.mouse_pos.x - g_state.drag_offset.x;
          int dy = g_state.mouse_pos.y - g_state.drag_offset.y;
          SetTextColor(hdc, RGB(255,0,0));
          TextOutW(hdc, dx, dy, s.c_str(), (int)s.size());
        }
      }
      SelectObject(hdc, old);
      DeleteObject(hFont);
      // Status bar
      SetBkMode(hdc, OPAQUE);
      SetTextColor(hdc, RGB(0,0,0));
      TextOutA(hdc, 10, BOARD_PX + 40, g_state.status, (int)strlen(g_state.status));
      EndPaint(hwnd, &ps);
      return 0;
    }
    case WM_LBUTTONDOWN: {
      POINT pt{ LOWORD(lParam), HIWORD(lParam) };
      g_state.mouse_pos = pt;
      int sq = square_from_point(pt);
      if (sq >= 0) {
        Piece p = g_state.board.piece_at(sq);
        if (p != static_cast<Piece>(kNumPieces) && color_of_piece(p) == g_state.board.side_to_move) {
          g_state.dragging = true;
          g_state.drag_from = sq;
          g_state.drag_offset.x = pt.x - (8 + (sq % 8)*SQUARE_PX + SQUARE_PX/2);
          g_state.drag_offset.y = pt.y - (30 + (7 - sq/8)*SQUARE_PX + SQUARE_PX/2);
          InvalidateRect(hwnd, nullptr, TRUE);
        }
      }
      return 0;
    }
    case WM_MOUSEMOVE: {
      POINT pt{ LOWORD(lParam), HIWORD(lParam) };
      g_state.mouse_pos = pt;
      if (g_state.dragging) InvalidateRect(hwnd, nullptr, TRUE);
      return 0;
    }
    case WM_LBUTTONUP: {
      if (g_state.dragging) {
        POINT pt{ LOWORD(lParam), HIWORD(lParam) };
        int to_sq = square_from_point(pt);
        if (g_state.drag_from >= 0 && to_sq >= 0) {
          MoveList moves = generate_moves(g_state.board);
          for (int i = 0; i < moves.size(); ++i) {
            if (moves[i].from == g_state.drag_from && moves[i].to == to_sq) {
              Move m;
              m.from = g_state.drag_from;
              m.to = to_sq;
              g_state.board.make_move(m);
              char buf[256];
              sprintf_s(buf, sizeof(buf), "White played %s", m.to_string().c_str());
              OutputDebugStringA(buf);
              strncpy_s(g_state.status, buf, _TRUNCATE);
              InvalidateRect(hwnd, nullptr, TRUE);
              // Computer reply
              make_computer_move(hwnd);
              break;
            }
          }
        }
        g_state.dragging = false;
        g_state.drag_from = -1;
        InvalidateRect(hwnd, nullptr, TRUE);
      }
      return 0;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
  const char CLASS_NAME[] = "ChessGUI";
  WNDCLASSA wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInst;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&wc);

  HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Chess GUI",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_W, WINDOW_H,
    nullptr, nullptr, hInst, nullptr);
  if (!hwnd) return 0;
  ShowWindow(hwnd, nCmdShow);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}
