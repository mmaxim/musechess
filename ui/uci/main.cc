#include "uci.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace chess::uci;

int main() {
  Engine engine;
  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "uci") {
      std::cout << "id name ChessEngine\n";
      std::cout << "id author LMStudio\n";
      std::cout << "option name Depth type spin default 12 min 1 max 20\n";
      std::cout << "uciok\n";
    } else if (cmd == "isready") {
      std::cout << "readyok\n";
    } else if (cmd == "ucinewgame") {
      engine.uci_new_game();
    } else if (cmd == "position") {
      std::string token;
      std::string fen;
      std::vector<std::string> moves;
      bool have_fen = false;
      while (iss >> token) {
        if (token == "fen") {
          have_fen = true;
          std::string f;
          for (int i=0;i<6;i++) { std::string p; iss >> p; f += p + " "; }
          fen = f;
        } else if (token == "moves") {
          std::string m;
          while (iss >> m) moves.push_back(m);
        } else if (have_fen) {
          // already consumed
        }
      }
      if (!have_fen) {
        // assume startpos
        fen = "";
      }
      engine.position(fen, moves);
    } else if (cmd == "go") {
      int depth = -1;
      std::string t;
      while (iss >> t) {
        if (t == "depth") iss >> depth;
      }
      engine.go(depth);
      std::cout << "bestmove " << engine.best_move() << "\n";
    } else if (cmd == "stop") {
      engine.stop();
    } else if (cmd == "quit") {
      break;
    }
  }
  return 0;
}
