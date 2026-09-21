#include "uci.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

using namespace chess::uci;

auto log_line = [](std::ofstream& log, const std::string& s){
  std::cout << s << "\n";
  log << s << "\n";
  std::cout.flush();
  log.flush();
};

int main() {
  std::ofstream log("D:/AI/Projects/Chess/build/uci.log", std::ios::app);
  Engine engine;
  std::string line;
  while (std::getline(std::cin, line)) {
    log_line(log, "> " + line);
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd == "uci") {
      log_line(log, "id name ChessEngine");
      log_line(log, "id author LMStudio");
      log_line(log, "option name Depth type spin default 12 min 1 max 20");
      log_line(log, "uciok");
    } else if (cmd == "isready") {
      log_line(log, "readyok");
    } else if (cmd == "ucisetoptions") {
      std::string opt;
      iss >> opt;
      if (opt == "timecontrol") {
        std::string key;
        while (iss >> key) {
          std::string val;
          if (!(iss >> val)) break;
          engine.set_time_control(key, val);
        }
      }
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
      bool infinite = false;
      std::string t;
      while (iss >> t) {
        if (t == "depth") iss >> depth;
        else if (t == "infinite") infinite = true;
      }
      engine.go(depth, infinite, [&](const std::string& s){ log_line(log, s); });
      if (!infinite) {
        log_line(log, "bestmove " + engine.best_move());
      }

    } else if (cmd == "stop") {
      engine.stop();
    } else if (cmd == "quit") {
      break;
    }
  }
  return 0;
}
