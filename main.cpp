// Sirocco 1.0 - single-file build (generated from src/*). Build:  g++ -O3 -std=c++17 -pthread -DNDEBUG -o sirocco sirocco_single.cpp

// ======================================== types.h
// types.h - basic types and constants

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <cassert>

typedef uint64_t U64;
typedef uint64_t Bitboard;

constexpr int MAX_PLY = 128;
constexpr int MAX_MOVES = 256;

typedef int Color;
enum : int { WHITE, BLACK, COLOR_NB };

typedef int PieceType;
enum : int { NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB = 8 };

typedef int Piece;
enum : int {
  NO_PIECE,
  W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  PIECE_NB = 16
};
constexpr Piece make_piece(Color c, PieceType pt) { return (c << 3) + pt; }
constexpr PieceType type_of(Piece p) { return p & 7; }
constexpr Color color_of(Piece p) { return p >> 3; }

typedef int Square;
enum : int {
  SQ_A1 = 0, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A8 = 56, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE = 64, SQUARE_NB = 64
};

typedef int Direction;
enum : int { NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
             NORTH_EAST = 9, NORTH_WEST = 7, SOUTH_EAST = -7, SOUTH_WEST = -9 };

// Castling rights bits
enum : int { WHITE_OO = 1, WHITE_OOO = 2, BLACK_OO = 4, BLACK_OOO = 8 };

// ---- Moves: 16 bit: from(6) to(6) flag(4) ----
typedef uint16_t Move;
constexpr Move MOVE_NONE = 0;
constexpr Move MOVE_NULL = 65;   // from=1,to=1 : never a real move

enum : int {
  QUIET = 0, DOUBLE_PUSH = 1, KING_CASTLE = 2, QUEEN_CASTLE = 3,
  CAPTURE = 4, EP_CAPTURE = 5,
  PROMO_N = 8, PROMO_B = 9, PROMO_R = 10, PROMO_Q = 11,
  PROMO_CAP_N = 12, PROMO_CAP_B = 13, PROMO_CAP_R = 14, PROMO_CAP_Q = 15
};
constexpr Move make_move(int from, int to, int flag = QUIET) { return Move(from | (to << 6) | (flag << 12)); }
constexpr Square from_sq(Move m) { return m & 63; }
constexpr Square to_sq(Move m) { return (m >> 6) & 63; }
constexpr int move_flag(Move m) { return m >> 12; }
constexpr bool is_capture(Move m) { return move_flag(m) & 4; }          // incl. ep and promo captures
constexpr bool is_promo(Move m) { return move_flag(m) & 8; }
constexpr bool is_capture_or_promo(Move m) { return move_flag(m) >= 4; }
constexpr PieceType promo_type(Move m) { return (move_flag(m) & 3) + KNIGHT; }

struct ExtMove { Move move; int score; };

// ---- Scores ----
constexpr int VALUE_ZERO = 0;
constexpr int VALUE_MATE = 30000;
constexpr int VALUE_INFINITE = 30001;
constexpr int VALUE_NONE = 32002;
constexpr int MATE_IN_MAX = VALUE_MATE - MAX_PLY;
constexpr int MATED_IN_MAX = -VALUE_MATE + MAX_PLY;
constexpr int mate_in(int ply) { return VALUE_MATE - ply; }
constexpr int mated_in(int ply) { return -VALUE_MATE + ply; }

// Piece values used by SEE / pruning / move ordering
constexpr int SeeValue[PIECE_TYPE_NB] = { 0, 100, 320, 330, 500, 900, 0, 0 };

// ======================================== bitboard.h
// bitboard.h - bit tricks, attack tables, magic bitboards

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1, FileCBB = FileABB << 2, FileDBB = FileABB << 3,
                   FileEBB = FileABB << 4, FileFBB = FileABB << 5, FileGBB = FileABB << 6,
                   FileHBB = FileABB << 7;
constexpr Bitboard Rank1BB = 0xFFULL;
constexpr Bitboard Rank2BB = Rank1BB << 8, Rank3BB = Rank1BB << 16, Rank4BB = Rank1BB << 24,
                   Rank5BB = Rank1BB << 32, Rank6BB = Rank1BB << 40, Rank7BB = Rank1BB << 48,
                   Rank8BB = Rank1BB << 56;
constexpr Bitboard LightSquares = 0x55AA55AA55AA55AAULL;
constexpr Bitboard DarkSquares  = 0xAA55AA55AA55AA55ULL;

inline int popcount(Bitboard b) { return __builtin_popcountll(b); }
inline Square lsb(Bitboard b) { return __builtin_ctzll(b); }
inline Square msb(Bitboard b) { return 63 ^ __builtin_clzll(b); }
inline Square pop_lsb(Bitboard &b) { Square s = lsb(b); b &= b - 1; return s; }
constexpr bool more_than_one(Bitboard b) { return b & (b - 1); }

constexpr Bitboard sq_bb(Square s) { return 1ULL << s; }
constexpr int file_of(Square s) { return s & 7; }
constexpr int rank_of(Square s) { return s >> 3; }
constexpr int relative_rank(Color c, Square s) { return rank_of(s) ^ (c * 7); }
constexpr Square make_square(int f, int r) { return r * 8 + f; }
constexpr int pawn_push(Color c) { return c == WHITE ? 8 : -8; }
constexpr Bitboard file_bb_f(int f) { return FileABB << f; }
constexpr Bitboard rank_bb_r(int r) { return Rank1BB << (8 * r); }
inline int distance(Square a, Square b) {
  return std::max(std::abs(file_of(a) - file_of(b)), std::abs(rank_of(a) - rank_of(b)));
}

template <int D> constexpr Bitboard shift(Bitboard b) {
  return D == NORTH ? b << 8 : D == SOUTH ? b >> 8
       : D == NORTH_EAST ? (b & ~FileHBB) << 9 : D == NORTH_WEST ? (b & ~FileABB) << 7
       : D == SOUTH_EAST ? (b & ~FileHBB) >> 7 : D == SOUTH_WEST ? (b & ~FileABB) >> 9
       : D == EAST ? (b & ~FileHBB) << 1 : D == WEST ? (b & ~FileABB) >> 1 : 0;
}

template <Color C> constexpr Bitboard pawn_attacks_bb(Bitboard b) {
  return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                    : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

struct Magic {
  Bitboard mask;
  Bitboard magic;
  Bitboard *attacks;
  unsigned shift;
};

extern Bitboard PawnAttacks[COLOR_NB][64];
extern Bitboard KnightAttacks[64];
extern Bitboard KingAttacks[64];
extern Bitboard BetweenBB[64][64];     // squares strictly between (0 if not aligned)
extern Bitboard LineBB[64][64];        // full line through both squares (0 if not aligned)
extern Bitboard AdjacentFilesBB[8];
extern Bitboard ForwardRanksBB[COLOR_NB][8];
extern Bitboard PassedMask[COLOR_NB][64];     // squares in front on own+adjacent files
extern Bitboard ForwardFileBB[COLOR_NB][64];  // squares in front on same file
extern Bitboard AdjSpan[COLOR_NB][64];        // squares in front on adjacent files
extern Magic RookMagics[64];
extern Magic BishopMagics[64];

inline Bitboard rook_attacks(Square s, Bitboard occ) {
  const Magic &m = RookMagics[s];
  return m.attacks[((occ & m.mask) * m.magic) >> m.shift];
}
inline Bitboard bishop_attacks(Square s, Bitboard occ) {
  const Magic &m = BishopMagics[s];
  return m.attacks[((occ & m.mask) * m.magic) >> m.shift];
}
inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occ) {
  switch (pt) {
    case KNIGHT: return KnightAttacks[s];
    case BISHOP: return bishop_attacks(s, occ);
    case ROOK:   return rook_attacks(s, occ);
    case QUEEN:  return bishop_attacks(s, occ) | rook_attacks(s, occ);
    case KING:   return KingAttacks[s];
    default:     return 0;
  }
}
inline bool aligned(Square a, Square b, Square c) { return LineBB[a][b] & sq_bb(c); }

void init_bitboards();

// ======================================== position.h
// position.h - board state, make/unmake, legality

#include <vector>

namespace Zobrist {
extern U64 psq[PIECE_NB][64];
extern U64 castling[16];
extern U64 ep[8];
extern U64 side;
}

struct StateInfo {
  U64 key;
  U64 pawnKey;
  int castling;
  int epSquare;
  int rule50;
  int pliesFromNull;
  int repetition;
  Piece captured;
  Bitboard checkers;
  Bitboard blockers[COLOR_NB];   // pieces (any colour) blocking enemy sliders from king of colour c
  Bitboard checkSq[PIECE_TYPE_NB];
};

class Position {
public:
  static void init();
  bool set(const std::string &fen, size_t stackSize = 8192 + MAX_PLY + 16);   // false if the FEN is not a legal position
  std::string fen() const;

  Color side_to_move() const { return stm; }
  Piece piece_on(Square s) const { return board[s]; }
  bool empty(Square s) const { return board[s] == NO_PIECE; }
  Bitboard pieces() const { return occ; }
  Bitboard pieces(Color c) const { return byColor[c]; }
  Bitboard by_type(PieceType pt) const { return byType[pt]; }
  Bitboard by_type(PieceType a, PieceType b) const { return byType[a] | byType[b]; }
  Bitboard pieces(Color c, PieceType pt) const { return byColor[c] & byType[pt]; }
  Bitboard pieces(Color c, PieceType a, PieceType b) const { return byColor[c] & (byType[a] | byType[b]); }
  int count(Color c, PieceType pt) const { return pieceCount[make_piece(c, pt)]; }
  Square king_square(Color c) const { return kingSq[c]; }

  const StateInfo &state() const { return stack[sp]; }
  int castling_rights() const { return stack[sp].castling; }
  int ep_square() const { return stack[sp].epSquare; }
  int rule50() const { return stack[sp].rule50; }
  U64 key() const { return stack[sp].key; }
  U64 pawn_key() const { return stack[sp].pawnKey; }
  Bitboard checkers() const { return stack[sp].checkers; }
  bool in_check() const { return stack[sp].checkers != 0; }
  Bitboard blockers_for_king(Color c) const { return stack[sp].blockers[c]; }
  int game_ply() const { return sp; }
  Move last_move_captured() const { return stack[sp].captured; }

  Bitboard attackers_to(Square s, Bitboard o) const;
  Bitboard attackers_to(Square s) const { return attackers_to(s, occ); }

  bool legal(Move m) const;
  bool pseudo_legal(Move m) const;
  bool gives_check(Move m) const;
  bool see_ge(Move m, int threshold) const;

  void do_move(Move m);
  void undo_move(Move m);
  void do_null_move();
  void undo_null_move();

  bool is_draw(int ply) const;
  bool has_non_pawn(Color c) const { return pieces(c) & ~(byType[PAWN] | byType[KING]); }
  bool can_castle(int right) const { return stack[sp].castling & right; }

  int psq_mg(Color c) const { return psqMg[c]; }
  int psq_eg(Color c) const { return psqEg[c]; }
  int phase() const { return gamePhase; }
  int non_pawn_material(Color c) const;

private:
  void put_piece(Piece pc, Square s);
  void remove_piece(Square s);
  void move_piece(Square from, Square to);
  Bitboard slider_blockers(Bitboard sliders, Square s) const;
  void set_check_info(StateInfo &st) const;

  Piece board[64];
  Bitboard byType[PIECE_TYPE_NB];
  Bitboard byColor[COLOR_NB];
  Bitboard occ;
  int pieceCount[PIECE_NB];
  Square kingSq[COLOR_NB];
  Color stm;
  int sp;
  int fullmove;
  int psqMg[COLOR_NB], psqEg[COLOR_NB], gamePhase;
  std::vector<StateInfo> stack;
};

// ======================================== eval.h
// eval.h

class Position;

namespace Eval {
extern int PsqMg[PIECE_NB][64];
extern int PsqEg[PIECE_NB][64];
extern const int PhaseWeight[PIECE_TYPE_NB];
extern int TermMask;
extern int Param[];
extern const char *ParamName[];
extern const int NumParams;          // debug/ablation switch, all bits set by default
void init();
int evaluate(const Position &pos);   // from side-to-move perspective
void trace(const Position &pos);
}

// ======================================== tt.h
// tt.h - transposition table: 64-byte clusters of 4 entries, lock-free (key^data verification)

#include <atomic>
#include <cstddef>

enum Bound : int { BOUND_NONE = 0, BOUND_UPPER = 1, BOUND_LOWER = 2, BOUND_EXACT = 3 };

struct TTData {
  Move move;
  int value;
  int eval;
  int depth;
  int bound;
  bool pv;
};

class TranspositionTable {
public:
  TranspositionTable() : table(nullptr), count(0), gen(0) {}
  ~TranspositionTable();
  void resize(size_t mb);
  void clear();
  void new_search() { gen = (gen + 1) & 31; }
  bool probe(U64 key, TTData &out) const;
  void store(U64 key, int value, bool pv, int bound, int depth, Move m, int eval);
  int hashfull() const;
  void prefetch(U64 key) const { __builtin_prefetch(&table[index(key)]); }

private:
  struct Entry {
    std::atomic<U64> keyx;
    std::atomic<U64> data;
  };
  struct Cluster { Entry e[4]; };
  size_t index(U64 key) const { return size_t((unsigned __int128)key * count >> 64); }
  Cluster *table;
  size_t count;
  unsigned gen;
};

// ======================================== movegen.h
// movegen.h

enum GenType { CAPTURES, QUIETS, EVASIONS };

// Pseudo-legal generation. CAPTURES = captures + queen promotions (+ep),
// QUIETS = non-captures (+underpromotions, +castling), EVASIONS = all moves when in check.
template <GenType T> ExtMove *generate(const Position &pos, ExtMove *list);

// Fully legal move list (uses the above + Position::legal)
ExtMove *generate_legal(const Position &pos, ExtMove *list);

// ======================================== search.h
// search.h

#include <atomic>
#include <string>


struct SearchLimits {
  int time[2] = {0, 0};
  int inc[2] = {0, 0};
  int movestogo = 0;
  int movetime = 0;
  int depth = 0;
  U64 nodes = 0;
  bool infinite = false;
};

namespace Search {
extern TranspositionTable TT;
extern std::atomic<bool> stop;

void init();
void clear();                      // new game: clears TT + histories
void set_hash(int mb);
void set_threads(int n);
void set_move_overhead(int ms);
void go(const Position &pos, const SearchLimits &limits);   // asynchronous
void wait();                       // block until the running search has printed bestmove
void stop_search();
U64 nodes_searched();

std::string move_to_uci(Move m);
Move parse_move(const Position &pos, const std::string &s);
}

// ======================================== bitboard.cpp

Bitboard PawnAttacks[COLOR_NB][64];
Bitboard KnightAttacks[64];
Bitboard KingAttacks[64];
Bitboard BetweenBB[64][64];
Bitboard LineBB[64][64];
Bitboard AdjacentFilesBB[8];
Bitboard ForwardRanksBB[COLOR_NB][8];
Bitboard PassedMask[COLOR_NB][64];
Bitboard ForwardFileBB[COLOR_NB][64];
Bitboard AdjSpan[COLOR_NB][64];
Magic RookMagics[64];
Magic BishopMagics[64];

namespace {
Bitboard RookTable[0x19000];
Bitboard BishopTable[0x1480];

struct PRNG {
  U64 s;
  explicit PRNG(U64 seed) : s(seed) {}
  U64 rand64() { s ^= s >> 12; s ^= s << 25; s ^= s >> 27; return s * 2685821657736338717ULL; }
  U64 sparse() { return rand64() & rand64() & rand64(); }
};

Bitboard sliding_attack(Square sq, Bitboard occ, bool rook) {
  static const int rd[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  static const int bd[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  Bitboard att = 0;
  for (int d = 0; d < 4; ++d) {
    int df = rook ? rd[d][0] : bd[d][0], dr = rook ? rd[d][1] : bd[d][1];
    int f = file_of(sq) + df, r = rank_of(sq) + dr;
    while (f >= 0 && f < 8 && r >= 0 && r < 8) {
      Bitboard b = sq_bb(make_square(f, r));
      att |= b;
      if (occ & b) break;
      f += df; r += dr;
    }
  }
  return att;
}

void init_magics(Magic magics[], Bitboard table[], bool rook) {
  static const int seeds[8] = {728, 10316, 55013, 32803, 12281, 15100, 16645, 255};
  static Bitboard occupancy[4096], reference[4096];
  static int epoch[4096];
  std::memset(epoch, 0, sizeof(epoch));
  int cnt = 0, size = 0;
  for (Square s = 0; s < 64; ++s) {
    Bitboard edges = ((Rank1BB | Rank8BB) & ~rank_bb_r(rank_of(s))) | ((FileABB | FileHBB) & ~file_bb_f(file_of(s)));
    Magic &m = magics[s];
    m.mask = sliding_attack(s, 0, rook) & ~edges;
    m.shift = 64 - popcount(m.mask);
    m.attacks = s == 0 ? table : magics[s - 1].attacks + size;
    size = 0;
    Bitboard b = 0;
    do {
      occupancy[size] = b;
      reference[size] = sliding_attack(s, b, rook);
      ++size;
      b = (b - m.mask) & m.mask;
    } while (b);
    PRNG rng(seeds[rank_of(s)]);
    for (int i = 0; i < size;) {
      for (m.magic = 0; popcount((m.magic * m.mask) >> 56) < 6;) m.magic = rng.sparse();
      ++cnt;
      for (i = 0; i < size; ++i) {
        unsigned idx = unsigned(((occupancy[i] & m.mask) * m.magic) >> m.shift);
        if (epoch[idx] < cnt) { epoch[idx] = cnt; m.attacks[idx] = reference[i]; }
        else if (m.attacks[idx] != reference[i]) break;
      }
    }
  }
}
}  // namespace

void init_bitboards() {
  init_magics(RookMagics, RookTable, true);
  init_magics(BishopMagics, BishopTable, false);

  for (Square s = 0; s < 64; ++s) {
    int f = file_of(s), r = rank_of(s);
    PawnAttacks[WHITE][s] = shift<NORTH_WEST>(sq_bb(s)) | shift<NORTH_EAST>(sq_bb(s));
    PawnAttacks[BLACK][s] = shift<SOUTH_WEST>(sq_bb(s)) | shift<SOUTH_EAST>(sq_bb(s));
    static const int kn[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
    for (auto &d : kn) {
      int nf = f + d[0], nr = r + d[1];
      if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) KnightAttacks[s] |= sq_bb(make_square(nf, nr));
    }
    for (int df = -1; df <= 1; ++df)
      for (int dr = -1; dr <= 1; ++dr) {
        if (!df && !dr) continue;
        int nf = f + df, nr = r + dr;
        if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) KingAttacks[s] |= sq_bb(make_square(nf, nr));
      }
  }

  for (int f = 0; f < 8; ++f)
    AdjacentFilesBB[f] = (f > 0 ? file_bb_f(f - 1) : 0) | (f < 7 ? file_bb_f(f + 1) : 0);
  for (int r = 0; r < 8; ++r) {
    ForwardRanksBB[WHITE][r] = 0; ForwardRanksBB[BLACK][r] = 0;
    for (int rr = r + 1; rr < 8; ++rr) ForwardRanksBB[WHITE][r] |= rank_bb_r(rr);
    for (int rr = 0; rr < r; ++rr) ForwardRanksBB[BLACK][r] |= rank_bb_r(rr);
  }
  for (Color c = 0; c < 2; ++c)
    for (Square s = 0; s < 64; ++s) {
      int f = file_of(s), r = rank_of(s);
      ForwardFileBB[c][s] = file_bb_f(f) & ForwardRanksBB[c][r];
      AdjSpan[c][s] = AdjacentFilesBB[f] & ForwardRanksBB[c][r];
      PassedMask[c][s] = ForwardFileBB[c][s] | AdjSpan[c][s];
    }

  for (Square s1 = 0; s1 < 64; ++s1)
    for (Square s2 = 0; s2 < 64; ++s2) {
      BetweenBB[s1][s2] = 0; LineBB[s1][s2] = 0;
      if (s1 == s2) continue;
      if (bishop_attacks(s1, 0) & sq_bb(s2)) {
        LineBB[s1][s2] = (bishop_attacks(s1, 0) & bishop_attacks(s2, 0)) | sq_bb(s1) | sq_bb(s2);
        BetweenBB[s1][s2] = bishop_attacks(s1, sq_bb(s2)) & bishop_attacks(s2, sq_bb(s1));
      } else if (rook_attacks(s1, 0) & sq_bb(s2)) {
        LineBB[s1][s2] = (rook_attacks(s1, 0) & rook_attacks(s2, 0)) | sq_bb(s1) | sq_bb(s2);
        BetweenBB[s1][s2] = rook_attacks(s1, sq_bb(s2)) & rook_attacks(s2, sq_bb(s1));
      }
    }
}

// ======================================== position.cpp


#include <sstream>

namespace Zobrist {
U64 psq[PIECE_NB][64];
U64 castling[16];
U64 ep[8];
U64 side;
}

static int g_castlingMask[64];

void Position::init() {
  U64 s = 1070372ULL;
  auto rnd = [&s]() { s ^= s >> 12; s ^= s << 25; s ^= s >> 27; return s * 2685821657736338717ULL; };
  for (int p = 0; p < PIECE_NB; ++p)
    for (int sq = 0; sq < 64; ++sq) Zobrist::psq[p][sq] = rnd();
  for (int i = 0; i < 16; ++i) Zobrist::castling[i] = rnd();
  for (int i = 0; i < 8; ++i) Zobrist::ep[i] = rnd();
  Zobrist::side = rnd();
  for (int i = 0; i < 64; ++i) g_castlingMask[i] = 0;
  g_castlingMask[SQ_E1] = WHITE_OO | WHITE_OOO; g_castlingMask[SQ_H1] = WHITE_OO; g_castlingMask[SQ_A1] = WHITE_OOO;
  g_castlingMask[SQ_E8] = BLACK_OO | BLACK_OOO; g_castlingMask[SQ_H8] = BLACK_OO; g_castlingMask[SQ_A8] = BLACK_OOO;
}

// ---------------------------------------------------------------- piece ops
void Position::put_piece(Piece pc, Square s) {
  Bitboard b = sq_bb(s);
  board[s] = pc;
  byType[type_of(pc)] |= b;
  byColor[color_of(pc)] |= b;
  occ |= b;
  ++pieceCount[pc];
  Color c = color_of(pc);
  psqMg[c] += Eval::PsqMg[pc][s];
  psqEg[c] += Eval::PsqEg[pc][s];
  gamePhase += Eval::PhaseWeight[type_of(pc)];
  if (type_of(pc) == KING) kingSq[c] = s;
}

void Position::remove_piece(Square s) {
  Piece pc = board[s];
  Bitboard b = sq_bb(s);
  byType[type_of(pc)] ^= b;
  byColor[color_of(pc)] ^= b;
  occ ^= b;
  board[s] = NO_PIECE;
  --pieceCount[pc];
  Color c = color_of(pc);
  psqMg[c] -= Eval::PsqMg[pc][s];
  psqEg[c] -= Eval::PsqEg[pc][s];
  gamePhase -= Eval::PhaseWeight[type_of(pc)];
}

void Position::move_piece(Square from, Square to) {
  Piece pc = board[from];
  Bitboard ft = sq_bb(from) | sq_bb(to);
  byType[type_of(pc)] ^= ft;
  byColor[color_of(pc)] ^= ft;
  occ ^= ft;
  board[from] = NO_PIECE;
  board[to] = pc;
  Color c = color_of(pc);
  psqMg[c] += Eval::PsqMg[pc][to] - Eval::PsqMg[pc][from];
  psqEg[c] += Eval::PsqEg[pc][to] - Eval::PsqEg[pc][from];
  if (type_of(pc) == KING) kingSq[c] = to;
}

// ---------------------------------------------------------------- attacks / pins
Bitboard Position::attackers_to(Square s, Bitboard o) const {
  return (PawnAttacks[BLACK][s] & pieces(WHITE, PAWN)) | (PawnAttacks[WHITE][s] & pieces(BLACK, PAWN)) |
         (KnightAttacks[s] & byType[KNIGHT]) | (rook_attacks(s, o) & (byType[ROOK] | byType[QUEEN])) |
         (bishop_attacks(s, o) & (byType[BISHOP] | byType[QUEEN])) | (KingAttacks[s] & byType[KING]);
}

Bitboard Position::slider_blockers(Bitboard sliders, Square s) const {
  Bitboard blockers = 0;
  Bitboard snipers = ((rook_attacks(s, 0) & (byType[ROOK] | byType[QUEEN])) |
                      (bishop_attacks(s, 0) & (byType[BISHOP] | byType[QUEEN]))) & sliders;
  Bitboard occupancy = occ ^ snipers;
  while (snipers) {
    Square sn = pop_lsb(snipers);
    Bitboard b = BetweenBB[s][sn] & occupancy;
    if (b && !more_than_one(b)) blockers |= b;
  }
  return blockers;
}

// blockers / check squares for the side to move (stm must already be updated)
void Position::set_check_info(StateInfo &st) const {
  st.blockers[WHITE] = slider_blockers(pieces(BLACK), kingSq[WHITE]);
  st.blockers[BLACK] = slider_blockers(pieces(WHITE), kingSq[BLACK]);
  Square eksq = kingSq[stm ^ 1];
  st.checkSq[PAWN] = PawnAttacks[stm ^ 1][eksq];
  st.checkSq[KNIGHT] = KnightAttacks[eksq];
  st.checkSq[BISHOP] = bishop_attacks(eksq, occ);
  st.checkSq[ROOK] = rook_attacks(eksq, occ);
  st.checkSq[QUEEN] = st.checkSq[BISHOP] | st.checkSq[ROOK];
  st.checkSq[KING] = 0;
  st.checkSq[0] = 0;
  st.checkSq[7] = 0;
}

// ---------------------------------------------------------------- FEN
bool Position::set(const std::string &fenStr, size_t stackSize) {
  std::memset(board, 0, sizeof(board));
  std::memset(byType, 0, sizeof(byType));
  std::memset(byColor, 0, sizeof(byColor));
  std::memset(pieceCount, 0, sizeof(pieceCount));
  occ = 0;
  psqMg[0] = psqMg[1] = psqEg[0] = psqEg[1] = 0;
  gamePhase = 0;
  kingSq[0] = kingSq[1] = 0;
  stack.assign(stackSize, StateInfo());
  sp = 0;
  StateInfo &st = stack[0];
  std::memset(&st, 0, sizeof(st));
  st.epSquare = SQ_NONE;

  std::istringstream ss(fenStr);
  std::string placement, side = "w", castle = "-", ep = "-";
  int r50 = 0, fm = 1;
  ss >> placement >> side >> castle >> ep >> r50 >> fm;

  static const std::string pcs = " PNBRQK  pnbrqk";
  int f = 0, r = 7;
  for (char ch : placement) {
    if (ch == '/') { --r; f = 0; }
    else if (ch >= '1' && ch <= '8') f += ch - '0';
    else {
      size_t idx = pcs.find(ch);
      if (idx != std::string::npos && idx != 0 && f < 8 && f >= 0 && r >= 0 && r < 8)
        put_piece(Piece(idx), make_square(f, r));
      ++f;
    }
  }
  stm = (side == "b") ? BLACK : WHITE;
  st.castling = 0;
  for (char ch : castle) {
    if (ch == 'K') st.castling |= WHITE_OO;
    else if (ch == 'Q') st.castling |= WHITE_OOO;
    else if (ch == 'k') st.castling |= BLACK_OO;
    else if (ch == 'q') st.castling |= BLACK_OOO;
  }
  if (board[SQ_E1] != W_KING || board[SQ_H1] != W_ROOK) st.castling &= ~WHITE_OO;
  if (board[SQ_E1] != W_KING || board[SQ_A1] != W_ROOK) st.castling &= ~WHITE_OOO;
  if (board[SQ_E8] != B_KING || board[SQ_H8] != B_ROOK) st.castling &= ~BLACK_OO;
  if (board[SQ_E8] != B_KING || board[SQ_A8] != B_ROOK) st.castling &= ~BLACK_OOO;

  st.epSquare = SQ_NONE;
  if (ep.size() == 2 && ep[0] >= 'a' && ep[0] <= 'h' && (ep[1] == '3' || ep[1] == '6')) {
    Square e = make_square(ep[0] - 'a', ep[1] - '1');
    if (PawnAttacks[stm ^ 1][e] & pieces(stm, PAWN)) st.epSquare = e;
  }
  st.rule50 = r50;
  st.pliesFromNull = 0;
  st.repetition = 0;
  st.captured = NO_PIECE;
  fullmove = fm;

  U64 k = 0, pk = 0;
  for (Square s = 0; s < 64; ++s)
    if (board[s]) {
      k ^= Zobrist::psq[board[s]][s];
      if (type_of(board[s]) == PAWN) pk ^= Zobrist::psq[board[s]][s];
    }
  k ^= Zobrist::castling[st.castling];
  if (st.epSquare != SQ_NONE) k ^= Zobrist::ep[file_of(st.epSquare)];
  if (stm == BLACK) k ^= Zobrist::side;
  st.key = k;
  st.pawnKey = pk;
  st.checkers = attackers_to(kingSq[stm]) & byColor[stm ^ 1];
  set_check_info(st);

  // sanity: exactly one king per side, no pawns on the back ranks, side not to move not in check
  if (pieceCount[W_KING] != 1 || pieceCount[B_KING] != 1) return false;
  if (byType[PAWN] & (Rank1BB | Rank8BB)) return false;
  if (pieceCount[W_PAWN] > 8 || pieceCount[B_PAWN] > 8) return false;
  if (attackers_to(kingSq[stm ^ 1]) & byColor[stm]) return false;
  if (popcount(st.checkers) > 2) return false;
  return true;
}

std::string Position::fen() const {
  std::string s;
  static const char *pcs = " PNBRQK  pnbrqk";
  for (int r = 7; r >= 0; --r) {
    int emp = 0;
    for (int f = 0; f < 8; ++f) {
      Piece pc = board[make_square(f, r)];
      if (!pc) { ++emp; continue; }
      if (emp) { s += char('0' + emp); emp = 0; }
      s += pcs[pc];
    }
    if (emp) s += char('0' + emp);
    if (r) s += '/';
  }
  s += stm == WHITE ? " w " : " b ";
  int c = castling_rights();
  if (!c) s += '-';
  else {
    if (c & WHITE_OO) s += 'K';
    if (c & WHITE_OOO) s += 'Q';
    if (c & BLACK_OO) s += 'k';
    if (c & BLACK_OOO) s += 'q';
  }
  s += ' ';
  if (ep_square() == SQ_NONE) s += '-';
  else { s += char('a' + file_of(ep_square())); s += char('1' + rank_of(ep_square())); }
  s += " " + std::to_string(rule50()) + " " + std::to_string(fullmove + sp / 2);
  return s;
}

int Position::non_pawn_material(Color c) const {
  return count(c, KNIGHT) * 320 + count(c, BISHOP) * 330 + count(c, ROOK) * 500 + count(c, QUEEN) * 900;
}

// ---------------------------------------------------------------- legality
bool Position::legal(Move m) const {
  const Color us = stm, them = us ^ 1;
  const Square from = from_sq(m), to = to_sq(m);
  const Square ksq = kingSq[us];
  const StateInfo &st = stack[sp];
  const int fl = move_flag(m);

  if (fl == EP_CAPTURE) {
    Square capsq = to - pawn_push(us);
    Bitboard o = (occ ^ sq_bb(from) ^ sq_bb(capsq)) | sq_bb(to);
    return (attackers_to(ksq, o) & pieces(them) & ~sq_bb(capsq)) == 0;
  }
  if (type_of(board[from]) == KING) {
    if (fl == KING_CASTLE || fl == QUEEN_CASTLE) {
      if (st.checkers) return false;
      Square mid = from + (fl == KING_CASTLE ? 1 : -1);
      return !(attackers_to(mid, occ) & pieces(them)) && !(attackers_to(to, occ) & pieces(them));
    }
    return !(attackers_to(to, occ ^ sq_bb(from)) & pieces(them));
  }
  if (st.checkers) {
    if (more_than_one(st.checkers)) return false;
    Square cs = lsb(st.checkers);
    if (!((BetweenBB[ksq][cs] | sq_bb(cs)) & sq_bb(to))) return false;
  }
  if (st.blockers[us] & sq_bb(from)) return aligned(from, to, ksq);
  return true;
}

bool Position::pseudo_legal(Move m) const {
  if (m == MOVE_NONE || m == MOVE_NULL) return false;
  const Color us = stm;
  const Square from = from_sq(m), to = to_sq(m);
  const int fl = move_flag(m);
  if (fl == 6 || fl == 7) return false;
  const Piece pc = board[from];
  if (!pc || color_of(pc) != us) return false;
  const PieceType pt = type_of(pc);
  const Piece target = board[to];
  if (target && (color_of(target) == us || type_of(target) == KING)) return false;
  if (from == to) return false;

  if (fl == KING_CASTLE || fl == QUEEN_CASTLE) {
    if (pt != KING || target) return false;
    if (from != (us == WHITE ? SQ_E1 : SQ_E8)) return false;
    if (fl == KING_CASTLE)
      return (castling_rights() & (us == WHITE ? WHITE_OO : BLACK_OO)) && to == from + 2 &&
             !(occ & (sq_bb(from + 1) | sq_bb(from + 2)));
    return (castling_rights() & (us == WHITE ? WHITE_OOO : BLACK_OOO)) && to == from - 2 &&
           !(occ & (sq_bb(from - 1) | sq_bb(from - 2) | sq_bb(from - 3)));
  }
  if (pt == PAWN) {
    const int push = pawn_push(us);
    const bool lastRank = relative_rank(us, to) == 7;
    if (fl == EP_CAPTURE)
      return to == ep_square() && !target && (PawnAttacks[us][from] & sq_bb(to));
    if (is_promo(m) != lastRank) return false;
    if (fl == DOUBLE_PUSH)
      return relative_rank(us, from) == 1 && to == from + 2 * push && !target && !board[from + push];
    if (is_capture(m)) return target && (PawnAttacks[us][from] & sq_bb(to));
    return to == from + push && !target;
  }
  if (fl != QUIET && fl != CAPTURE) return false;
  if ((fl == CAPTURE) != (target != NO_PIECE)) return false;
  return attacks_bb(pt, from, occ) & sq_bb(to);
}

bool Position::gives_check(Move m) const {
  const Color us = stm;
  const Square from = from_sq(m), to = to_sq(m);
  const Square eksq = kingSq[us ^ 1];
  const StateInfo &st = stack[sp];
  const int fl = move_flag(m);
  const PieceType pt = type_of(board[from]);

  if (!is_promo(m) && (st.checkSq[pt] & sq_bb(to))) return true;
  if ((st.blockers[us ^ 1] & sq_bb(from)) && !aligned(from, to, eksq)) return true;

  if (is_promo(m))
    return attacks_bb(promo_type(m), to, occ ^ sq_bb(from)) & sq_bb(eksq);
  if (fl == EP_CAPTURE) {
    Square capsq = to - pawn_push(us);
    Bitboard o = (occ ^ sq_bb(from) ^ sq_bb(capsq)) | sq_bb(to);
    return (rook_attacks(eksq, o) & pieces(us, QUEEN, ROOK)) | (bishop_attacks(eksq, o) & pieces(us, QUEEN, BISHOP));
  }
  if (fl == KING_CASTLE || fl == QUEEN_CASTLE) {
    Square rfrom = fl == KING_CASTLE ? to + 1 : to - 2;
    Square rto = fl == KING_CASTLE ? to - 1 : to + 1;
    Bitboard o = (occ ^ sq_bb(from) ^ sq_bb(rfrom)) | sq_bb(to) | sq_bb(rto);
    return rook_attacks(rto, o) & sq_bb(eksq);
  }
  return false;
}

// Static exchange evaluation: does the exchange starting with m gain at least `threshold`?
bool Position::see_ge(Move m, int threshold) const {
  const int fl = move_flag(m);
  if (fl == KING_CASTLE || fl == QUEEN_CASTLE || fl == EP_CAPTURE || is_promo(m)) return 0 >= threshold;
  const Square from = from_sq(m), to = to_sq(m);
  int swap = SeeValue[type_of(board[to])] - threshold;
  if (swap < 0) return false;
  swap = SeeValue[type_of(board[from])] - swap;
  if (swap <= 0) return true;

  Bitboard occupied = occ ^ sq_bb(from) ^ sq_bb(to);
  Color c = color_of(board[from]);
  Bitboard attackers = attackers_to(to, occupied), stmAttackers, bb;
  int res = 1;
  for (;;) {
    c ^= 1;
    attackers &= occupied;
    if (!(stmAttackers = attackers & pieces(c))) break;
    res ^= 1;
    if ((bb = stmAttackers & byType[PAWN])) {
      if ((swap = SeeValue[PAWN] - swap) < res) break;
      occupied ^= bb & (~bb + 1);
      attackers |= bishop_attacks(to, occupied) & (byType[BISHOP] | byType[QUEEN]);
    } else if ((bb = stmAttackers & byType[KNIGHT])) {
      if ((swap = SeeValue[KNIGHT] - swap) < res) break;
      occupied ^= bb & (~bb + 1);
    } else if ((bb = stmAttackers & byType[BISHOP])) {
      if ((swap = SeeValue[BISHOP] - swap) < res) break;
      occupied ^= bb & (~bb + 1);
      attackers |= bishop_attacks(to, occupied) & (byType[BISHOP] | byType[QUEEN]);
    } else if ((bb = stmAttackers & byType[ROOK])) {
      if ((swap = SeeValue[ROOK] - swap) < res) break;
      occupied ^= bb & (~bb + 1);
      attackers |= rook_attacks(to, occupied) & (byType[ROOK] | byType[QUEEN]);
    } else if ((bb = stmAttackers & byType[QUEEN])) {
      if ((swap = SeeValue[QUEEN] - swap) < res) break;
      occupied ^= bb & (~bb + 1);
      attackers |= (bishop_attacks(to, occupied) & (byType[BISHOP] | byType[QUEEN])) |
                   (rook_attacks(to, occupied) & (byType[ROOK] | byType[QUEEN]));
    } else  // king
      return (attackers & ~pieces(c)) ? res ^ 1 : res;
  }
  return bool(res);
}

// ---------------------------------------------------------------- make / unmake
void Position::do_move(Move m) {
  const StateInfo &p = stack[sp];
  StateInfo &n = stack[sp + 1];
  n.castling = p.castling;
  n.rule50 = p.rule50 + 1;
  n.pliesFromNull = p.pliesFromNull + 1;
  n.epSquare = SQ_NONE;
  n.captured = NO_PIECE;
  n.repetition = 0;
  ++sp;

  U64 k = p.key ^ Zobrist::side;
  U64 pk = p.pawnKey;
  if (p.epSquare != SQ_NONE) k ^= Zobrist::ep[file_of(p.epSquare)];

  const Color us = stm, them = us ^ 1;
  const Square from = from_sq(m), to = to_sq(m);
  const int fl = move_flag(m);
  const Piece pc = board[from];
  const PieceType pt = type_of(pc);
  const Piece cap = (fl == EP_CAPTURE) ? make_piece(them, PAWN) : board[to];

  if (fl == KING_CASTLE || fl == QUEEN_CASTLE) {
    Square rfrom = fl == KING_CASTLE ? to + 1 : to - 2;
    Square rto = fl == KING_CASTLE ? to - 1 : to + 1;
    Piece rook = board[rfrom];
    move_piece(rfrom, rto);
    k ^= Zobrist::psq[rook][rfrom] ^ Zobrist::psq[rook][rto];
  }
  if (cap) {
    Square capsq = (fl == EP_CAPTURE) ? to - pawn_push(us) : to;
    remove_piece(capsq);
    k ^= Zobrist::psq[cap][capsq];
    if (type_of(cap) == PAWN) pk ^= Zobrist::psq[cap][capsq];
    n.rule50 = 0;
    n.captured = cap;
  }
  int mask = g_castlingMask[from] | g_castlingMask[to];
  if (mask && (n.castling & mask)) {
    k ^= Zobrist::castling[n.castling];
    n.castling &= ~mask;
    k ^= Zobrist::castling[n.castling];
  }
  if (is_promo(m)) {
    Piece promo = make_piece(us, promo_type(m));
    remove_piece(from);
    put_piece(promo, to);
    k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[promo][to];
    pk ^= Zobrist::psq[pc][from];
  } else {
    move_piece(from, to);
    k ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
    if (pt == PAWN) pk ^= Zobrist::psq[pc][from] ^ Zobrist::psq[pc][to];
  }
  if (pt == PAWN) {
    n.rule50 = 0;
    if (fl == DOUBLE_PUSH) {
      Square e = from + pawn_push(us);
      if (PawnAttacks[us][e] & pieces(them, PAWN)) {
        n.epSquare = e;
        k ^= Zobrist::ep[file_of(e)];
      }
    }
  }
  n.key = k;
  n.pawnKey = pk;
  stm = them;
  n.checkers = attackers_to(kingSq[them]) & byColor[us];
  set_check_info(n);

  int end = std::min(std::min(n.rule50, n.pliesFromNull), sp);
  for (int i = 4; i <= end; i += 2) {
    const StateInfo &s = stack[sp - i];
    if (s.key == n.key) { n.repetition = s.repetition ? -i : i; break; }
  }
}

void Position::undo_move(Move m) {
  stm ^= 1;
  const Color us = stm;
  const Square from = from_sq(m), to = to_sq(m);
  const int fl = move_flag(m);
  const Piece cap = stack[sp].captured;
  --sp;

  if (is_promo(m)) {
    remove_piece(to);
    put_piece(make_piece(us, PAWN), to);
  }
  move_piece(to, from);
  if (fl == KING_CASTLE || fl == QUEEN_CASTLE) {
    Square rfrom = fl == KING_CASTLE ? to + 1 : to - 2;
    Square rto = fl == KING_CASTLE ? to - 1 : to + 1;
    move_piece(rto, rfrom);
  }
  if (cap) {
    Square capsq = (fl == EP_CAPTURE) ? to - pawn_push(us) : to;
    put_piece(cap, capsq);
  }
}

void Position::do_null_move() {
  const StateInfo &p = stack[sp];
  StateInfo &n = stack[sp + 1];
  n = p;
  ++sp;
  n.key ^= Zobrist::side;
  if (p.epSquare != SQ_NONE) { n.key ^= Zobrist::ep[file_of(p.epSquare)]; n.epSquare = SQ_NONE; }
  n.rule50 = p.rule50 + 1;
  n.pliesFromNull = 0;
  n.repetition = 0;
  n.captured = NO_PIECE;
  n.checkers = 0;
  stm ^= 1;
  set_check_info(n);
}

void Position::undo_null_move() {
  --sp;
  stm ^= 1;
}

bool Position::is_draw(int ply) const {
  const StateInfo &st = stack[sp];
  if (st.rule50 >= 100 && !st.checkers) return true;
  return st.repetition && st.repetition < ply;
}

// ======================================== eval.cpp
// eval.cpp - hand-crafted tapered evaluation (PeSTO-style tables + positional terms)


#include <cstdio>

namespace Eval {

int PsqMg[PIECE_NB][64];
int PsqEg[PIECE_NB][64];
const int PhaseWeight[PIECE_TYPE_NB] = {0, 0, 1, 1, 2, 4, 0, 0};
int TermMask = 0xFF;

#define PARAM_LIST(X) \
  X(MOB_N_MG, 6) X(MOB_N_EG, 7) X(MOB_B_MG, 7) X(MOB_B_EG, 7) X(MOB_R_MG, 2) X(MOB_R_EG, 6) X(MOB_Q_MG, 3) X(MOB_Q_EG, 6) \
  X(BPAIR_MG, 29) X(BPAIR_EG, 64) X(OUTN_MG, 30) X(OUTN_EG, 20) X(OUTB_MG, 30) X(OUTB_EG, 4) \
  X(ROPEN_MG, 36) X(ROPEN_EG, 8) X(RSEMI_MG, 18) X(RSEMI_EG, 15) X(R7_MG, 0) X(R7_EG, 2) \
  X(ISOO_MG, 0) X(ISOO_EG, -17) X(ISON_MG, -12) X(ISON_EG, -17) X(BACKO_MG, 0) X(BACKO_EG, 0) \
  X(BACKN_MG, -27) X(BACKN_EG, -33) X(DBL_MG, -16) X(DBL_EG, -14) \
  X(CONN2_MG, 2) X(CONN3_MG, 12) X(CONN4_MG, 28) X(CONN5_MG, 50) X(CONN6_MG, 75) \
  X(CONN2_EG, 4) X(CONN3_EG, 0) X(CONN4_EG, 6) X(CONN5_EG, 0) X(CONN6_EG, 82) \
  X(PASS1_MG, 9) X(PASS2_MG, 0) X(PASS3_MG, 0) X(PASS4_MG, 18) X(PASS5_MG, 17) X(PASS6_MG, 71) \
  X(PASS1_EG, 13) X(PASS2_EG, 16) X(PASS3_EG, 49) X(PASS4_EG, 68) X(PASS5_EG, 84) X(PASS6_EG, 137) \
  X(PKING_E, 5) X(PKING_O, 5) X(THR_MG, 40) X(THR_EG, 13) \
  X(SHLD_OPEN, -9) X(SHLD_FULL, -10) X(SHLD_D1, 16) X(SHLD_D2, 6) X(SHLD_FAR, 0) \
  X(DANGER_DIV, 2) X(DANGER_EGDIV, 23) X(TEMPO, 25) X(MOP_EDGE, 12) X(MOP_DIST, 6)

enum ParamId {
#define X(n, v) P_##n,
  PARAM_LIST(X)
#undef X
  P_COUNT
};
int Param[] = {
#define X(n, v) v,
  PARAM_LIST(X)
#undef X
};
const char *ParamName[] = {
#define X(n, v) #n,
  PARAM_LIST(X)
#undef X
};
const int NumParams = P_COUNT;

namespace {
// Tables are written a8..h1 (rank 8 first) from White's point of view.
const int MgVal[7] = {0, 82, 337, 365, 477, 1025, 0};
const int EgVal[7] = {0, 94, 281, 297, 512, 936, 0};

const int MgTab[7][64] = {
  {0},
  { 0,0,0,0,0,0,0,0, 98,134,61,95,68,126,34,-11, -6,7,26,31,65,56,25,-20, -14,13,6,21,23,12,17,-23,
    -27,-2,-5,12,17,6,10,-25, -26,-4,-4,-10,3,3,33,-12, -35,-1,-20,-23,-15,24,38,-22, 0,0,0,0,0,0,0,0 },
  { -167,-89,-34,-49,61,-97,-15,-107, -73,-41,72,36,23,62,7,-17, -47,60,37,65,84,129,73,44, -9,17,19,53,37,69,18,22,
    -13,4,16,13,28,19,21,-8, -23,-9,12,10,19,17,25,-16, -29,-53,-12,-3,-1,18,-14,-19, -105,-21,-58,-33,-17,-28,-19,-23 },
  { -29,4,-82,-37,-25,-42,7,-8, -26,16,-18,-13,30,59,18,-47, -16,37,43,40,35,50,37,-2, -4,5,19,50,37,37,7,-2,
    -6,13,13,26,34,12,10,4, 0,15,15,15,14,27,18,10, 4,15,16,0,7,21,33,1, -33,-3,-14,-21,-13,-12,-39,-21 },
  { 32,42,32,51,63,9,31,43, 27,32,58,62,80,67,26,44, -5,19,26,36,17,45,61,16, -24,-11,7,26,24,35,-8,-20,
    -36,-26,-12,-1,9,-7,6,-23, -45,-25,-16,-17,3,0,-5,-33, -44,-16,-20,-9,-1,11,-6,-71, -19,-13,1,17,16,7,-37,-26 },
  { -28,0,29,12,59,44,43,45, -24,-39,-5,1,-16,57,28,54, -13,-17,7,8,29,56,47,57, -27,-27,-16,-16,-1,17,-2,1,
    -9,-26,-9,-10,-2,-4,3,-3, -14,2,-11,-2,-5,2,14,5, -35,-8,11,2,8,15,-3,1, -1,-18,-9,10,-15,-25,-31,-50 },
  { -65,23,16,-15,-56,-34,2,13, 29,-1,-20,-7,-8,-4,-38,-29, -9,24,2,-16,-20,6,22,-22, -17,-20,-12,-27,-30,-25,-14,-36,
    -49,-1,-27,-39,-46,-44,-33,-51, -14,-14,-22,-46,-44,-30,-15,-27, 1,7,-8,-64,-43,-16,9,8, -15,36,12,-54,8,-28,24,14 }
};
const int EgTab[7][64] = {
  {0},
  { 0,0,0,0,0,0,0,0, 178,173,158,134,147,132,165,187, 94,100,85,67,56,53,82,84, 32,24,13,5,-2,4,17,17,
    13,9,-3,-7,-7,-8,3,-1, 4,7,-6,1,0,-5,-1,-8, 13,8,8,10,13,0,2,-7, 0,0,0,0,0,0,0,0 },
  { -58,-38,-13,-28,-31,-27,-63,-99, -25,-8,-25,-2,-9,-25,-24,-52, -24,-20,10,9,-1,-9,-19,-41, -17,3,22,22,22,11,8,-18,
    -18,-6,16,25,16,17,4,-18, -23,-3,-1,15,10,-3,-20,-22, -42,-20,-10,-5,-2,-20,-23,-44, -29,-51,-23,-15,-22,-18,-50,-64 },
  { -14,-21,-11,-8,-7,-9,-17,-24, -8,-4,7,-12,-3,-13,-4,-14, 2,-8,0,-1,-2,6,0,4, -3,9,12,9,14,10,3,2,
    -6,3,13,19,7,10,-3,-9, -12,-3,8,10,13,3,-7,-15, -14,-18,-7,-1,4,-9,-15,-27, -23,-9,-23,-5,-9,-16,-5,-17 },
  { 13,10,18,15,12,12,8,5, 11,13,13,11,-3,3,8,3, 7,7,7,5,4,-3,-5,-3, 4,3,13,1,2,1,-1,2,
    3,5,8,4,-5,-6,-8,-11, -4,0,-5,-1,-7,-12,-8,-16, -6,-6,0,2,-9,-9,-11,-3, -9,2,3,-1,-5,-13,4,-20 },
  { -9,22,22,27,27,19,10,20, -17,20,32,41,58,25,30,0, -20,6,9,49,47,35,19,9, 3,22,24,45,57,40,57,36,
    -18,28,19,47,31,34,39,23, -16,-27,15,6,9,17,10,5, -22,-23,-30,-16,-16,-23,-36,-32, -33,-28,-22,-43,-5,-32,-20,-41 },
  { -74,-35,-18,-18,-11,15,4,-17, -12,17,14,17,17,38,23,11, 10,17,23,15,20,45,44,13, -8,22,24,27,26,33,26,3,
    -18,-4,21,24,27,23,9,-11, -19,-3,11,21,23,16,7,-9, -27,-11,4,13,14,4,-5,-17, -53,-34,-21,-11,-28,-14,-24,-43 }
};

struct Score { int mg = 0, eg = 0; void add(int m, int e) { mg += m; eg += e; } };

struct KingAttack { int count = 0, weight = 0, squares = 0; };

inline int center_dist(Square s) {
  return (3 - std::min(file_of(s), 7 - file_of(s))) + (3 - std::min(rank_of(s), 7 - rank_of(s)));
}

template <Color Us>
Score eval_side(const Position &pos, KingAttack &atk) {
  constexpr Color Them = Us ^ 1;
  Score sc;
  const Bitboard occ = pos.pieces();
  const Bitboard ourPawns = pos.pieces(Us, PAWN), theirPawns = pos.pieces(Them, PAWN);
  const Bitboard theirPawnAtt = pawn_attacks_bb<Them>(theirPawns);
  const Bitboard ourPawnAtt = pawn_attacks_bb<Us>(ourPawns);
  const Bitboard mobArea = ~(ourPawns | pos.pieces(Us, KING) | theirPawnAtt);
  const Square ksq = pos.king_square(Us), eksq = pos.king_square(Them);
  const Bitboard zone = KingAttacks[eksq] | sq_bb(eksq);
  const bool useMob = TermMask & 1, usePawn = TermMask & 2, useMisc = TermMask & 8, useThreat = TermMask & 16;

  Bitboard bb = pos.pieces(Us, KNIGHT);
  while (bb) {
    Square s = pop_lsb(bb);
    Bitboard b = KnightAttacks[s];
    if (useMob) { int m = popcount(b & mobArea) - 4; sc.add(Param[P_MOB_N_MG] * m, Param[P_MOB_N_EG] * m); }
    if (b & zone) { atk.count++; atk.weight += 2; atk.squares += popcount(b & zone); }
    if (useMisc) {
      int r = relative_rank(Us, s);
      if (r >= 3 && r <= 5 && (ourPawnAtt & sq_bb(s)) && !(theirPawns & AdjSpan[Us][s])) sc.add(Param[P_OUTN_MG], Param[P_OUTN_EG]);
    }
  }
  bb = pos.pieces(Us, BISHOP);
  if (useMisc && more_than_one(bb)) sc.add(Param[P_BPAIR_MG], Param[P_BPAIR_EG]);
  while (bb) {
    Square s = pop_lsb(bb);
    Bitboard b = bishop_attacks(s, occ);
    if (useMob) { int m = popcount(b & mobArea) - 5; sc.add(Param[P_MOB_B_MG] * m, Param[P_MOB_B_EG] * m); }
    if (b & zone) { atk.count++; atk.weight += 2; atk.squares += popcount(b & zone); }
    if (useMisc) {
      int r = relative_rank(Us, s);
      if (r >= 3 && r <= 5 && (ourPawnAtt & sq_bb(s)) && !(theirPawns & AdjSpan[Us][s])) sc.add(Param[P_OUTB_MG], Param[P_OUTB_EG]);
    }
  }
  bb = pos.pieces(Us, ROOK);
  while (bb) {
    Square s = pop_lsb(bb);
    Bitboard b = rook_attacks(s, occ);
    if (useMob) { int m = popcount(b & mobArea) - 6; sc.add(Param[P_MOB_R_MG] * m, Param[P_MOB_R_EG] * m); }
    if (b & zone) { atk.count++; atk.weight += 3; atk.squares += popcount(b & zone); }
    if (useMisc) {
      Bitboard fb = file_bb_f(file_of(s));
      if (!(ourPawns & fb)) { if (!(theirPawns & fb)) sc.add(Param[P_ROPEN_MG], Param[P_ROPEN_EG]); else sc.add(Param[P_RSEMI_MG], Param[P_RSEMI_EG]); }
      if (relative_rank(Us, s) == 6 && ((theirPawns & rank_bb_r(rank_of(s))) || relative_rank(Us, eksq) == 7)) sc.add(Param[P_R7_MG], Param[P_R7_EG]);
    }
  }
  bb = pos.pieces(Us, QUEEN);
  while (bb) {
    Square s = pop_lsb(bb);
    Bitboard b = bishop_attacks(s, occ) | rook_attacks(s, occ);
    if (useMob) { int m = popcount(b & mobArea) - 12; sc.add(Param[P_MOB_Q_MG] * m, Param[P_MOB_Q_EG] * m); }
    if (b & zone) { atk.count++; atk.weight += 5; atk.squares += popcount(b & zone); }
  }

  if (usePawn) {
    Bitboard pb = ourPawns;
    while (pb) {
      Square s = pop_lsb(pb);
      const int f = file_of(s), r = relative_rank(Us, s);
      const Square stop = s + pawn_push(Us);
      const bool opposed = theirPawns & ForwardFileBB[Us][s];
      const Bitboard neighbors = ourPawns & AdjacentFilesBB[f];
      const Bitboard phalanx = ourPawns & (shift<EAST>(sq_bb(s)) | shift<WEST>(sq_bb(s)));
      const Bitboard supported = ourPawns & PawnAttacks[Them][s];
      const bool passed = !(theirPawns & PassedMask[Us][s]);
      const bool isolated = !neighbors;
      if (isolated) { if (opposed) sc.add(Param[P_ISOO_MG], Param[P_ISOO_EG]); else sc.add(Param[P_ISON_MG], Param[P_ISON_EG]); }
      else if (!phalanx && !(neighbors & ~ForwardRanksBB[Us][rank_of(s)]) && (theirPawnAtt & sq_bb(stop))) {
        if (opposed) sc.add(Param[P_BACKO_MG], Param[P_BACKO_EG]); else sc.add(Param[P_BACKN_MG], Param[P_BACKN_EG]);
      }
      if ((phalanx || supported) && r >= 2 && r <= 6) {
        int mult = (phalanx ? 2 : 0) + (supported ? 1 : 0);
        sc.add(Param[P_CONN2_MG + r - 2] * mult / 2, Param[P_CONN2_EG + r - 2] * mult / 2);
      }
      if (passed && r >= 1 && r <= 6) {
        int mg = Param[P_PASS1_MG + r - 1], eg = Param[P_PASS1_EG + r - 1];
        if (pos.piece_on(stop) != NO_PIECE) { mg /= 2; eg /= 2; }
        int w = std::max(0, r - 1);
        eg += (distance(eksq, stop) * Param[P_PKING_E] - distance(ksq, stop) * Param[P_PKING_O]) * w;
        sc.add(mg, eg);
      }
    }
    for (int f = 0; f < 8; ++f) {
      int n = popcount(ourPawns & file_bb_f(f));
      if (n > 1) sc.add(Param[P_DBL_MG] * (n - 1), Param[P_DBL_EG] * (n - 1));
    }
  }

  if (useThreat) {
    int n = popcount(ourPawnAtt & pos.pieces(Them) & ~theirPawns & ~pos.pieces(Them, KING));
    sc.add(Param[P_THR_MG] * n, Param[P_THR_EG] * n);
  }

  if (TermMask & 4) {
    if (relative_rank(Us, ksq) <= 1) {
      int cf = std::max(1, std::min(6, file_of(ksq)));
      for (int f = cf - 1; f <= cf + 1; ++f) {
        Bitboard ahead = ourPawns & file_bb_f(f) & ForwardRanksBB[Us][rank_of(ksq)];
        if (!ahead) sc.add(!(theirPawns & file_bb_f(f)) ? Param[P_SHLD_FULL] : Param[P_SHLD_OPEN], 0);
        else {
          Square p = Us == WHITE ? lsb(ahead) : msb(ahead);
          int d = std::abs(rank_of(p) - rank_of(ksq));
          sc.add(d == 1 ? Param[P_SHLD_D1] : d == 2 ? Param[P_SHLD_D2] : Param[P_SHLD_FAR], 0);
        }
      }
    }
  }
  return sc;
}

}  // namespace

void init() {
  for (int pt = PAWN; pt <= KING; ++pt)
    for (int s = 0; s < 64; ++s) {
      PsqMg[make_piece(WHITE, pt)][s] = MgVal[pt] + MgTab[pt][s ^ 56];
      PsqEg[make_piece(WHITE, pt)][s] = EgVal[pt] + EgTab[pt][s ^ 56];
      PsqMg[make_piece(BLACK, pt)][s] = MgVal[pt] + MgTab[pt][s];
      PsqEg[make_piece(BLACK, pt)][s] = EgVal[pt] + EgTab[pt][s];
    }
}

int evaluate(const Position &pos) {
  const Color us = pos.side_to_move();

  if (!pos.by_type(PAWN) && !pos.by_type(ROOK, QUEEN)) {
    int minors = popcount(pos.by_type(KNIGHT, BISHOP));
    if (minors <= 1) return 0;
    if (!pos.by_type(KNIGHT) && pos.count(WHITE, BISHOP) == 1 && pos.count(BLACK, BISHOP) == 1) {
      bool wl = pos.pieces(WHITE, BISHOP) & LightSquares, bl = pos.pieces(BLACK, BISHOP) & LightSquares;
      if (wl == bl) return 0;
    }
  }

  int mg = pos.psq_mg(WHITE) - pos.psq_mg(BLACK);
  int eg = pos.psq_eg(WHITE) - pos.psq_eg(BLACK);
  const int phase = std::min(pos.phase(), 24);

  KingAttack atkW, atkB;
  Score w = eval_side<WHITE>(pos, atkW);
  Score b = eval_side<BLACK>(pos, atkB);
  mg += w.mg - b.mg;
  eg += w.eg - b.eg;

  if (TermMask & 4) {
    auto danger = [&](const KingAttack &a, Color attacker) {
      if (a.count < 2) return 0;
      int units = a.weight + a.squares;
      int d = units * units / std::max(1, Param[P_DANGER_DIV]);
      if (!pos.count(attacker, QUEEN)) d /= 2;
      return std::min(d, 350);
    };
    int dw = danger(atkW, WHITE), db = danger(atkB, BLACK);
    mg += dw - db;
    eg += (dw - db) / std::max(1, Param[P_DANGER_EGDIV]);
  }

  if (TermMask & 32) {
    Color strong = eg > 0 ? WHITE : BLACK, weak = strong ^ 1;
    int npmS = pos.non_pawn_material(strong), npmW = pos.non_pawn_material(weak);
    if (!pos.count(strong, PAWN)) {
      if (npmS - npmW <= 330) eg = eg / 8;
      else if (!npmW && !pos.count(weak, PAWN) && npmS >= 500) {
        Square wk = pos.king_square(weak), sk = pos.king_square(strong);
        int bonus = Param[P_MOP_EDGE] * center_dist(wk) +
                    Param[P_MOP_DIST] * (14 - (std::abs(file_of(wk) - file_of(sk)) + std::abs(rank_of(wk) - rank_of(sk))));
        eg += strong == WHITE ? bonus : -bonus;
      }
    }
    if (pos.count(WHITE, BISHOP) == 1 && pos.count(BLACK, BISHOP) == 1 && !pos.by_type(KNIGHT, ROOK) && !pos.by_type(QUEEN)) {
      bool wl = pos.pieces(WHITE, BISHOP) & LightSquares, bl = pos.pieces(BLACK, BISHOP) & LightSquares;
      if (wl != bl) eg = eg / 2;
    }
  }

  int score = (mg * phase + eg * (24 - phase)) / 24;
  score = (us == WHITE ? score : -score) + Param[P_TEMPO];
  return std::max(-MATE_IN_MAX + 1, std::min(MATE_IN_MAX - 1, score));
}

void trace(const Position &pos) {
  std::printf("phase %d  psq-mg %d psq-eg %d  static eval (side to move) %d cp\n", pos.phase(),
              pos.psq_mg(WHITE) - pos.psq_mg(BLACK), pos.psq_eg(WHITE) - pos.psq_eg(BLACK), evaluate(pos));
}

}  // namespace Eval

// ======================================== movegen.cpp

namespace {

inline ExtMove *add(ExtMove *l, Square from, Square to, int flag) {
  l->move = make_move(from, to, flag);
  l->score = 0;
  return l + 1;
}

template <GenType T>
ExtMove *piece_moves(const Position &pos, ExtMove *l, Color us, PieceType pt, Bitboard target) {
  Bitboard bb = pos.pieces(us, pt);
  Bitboard enemies = pos.pieces(us ^ 1);
  Bitboard occ = pos.pieces();
  while (bb) {
    Square from = pop_lsb(bb);
    Bitboard b = attacks_bb(pt, from, occ) & target;
    while (b) {
      Square to = pop_lsb(b);
      l = add(l, from, to, (enemies & sq_bb(to)) ? CAPTURE : QUIET);
    }
  }
  return l;
}

template <Color Us, GenType T>
ExtMove *gen_pawns(const Position &pos, ExtMove *l, Bitboard target) {
  constexpr Color Them = Us ^ 1;
  constexpr int Up = Us == WHITE ? NORTH : SOUTH;
  constexpr int UpRight = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
  constexpr int UpLeft = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
  constexpr int dUp = Us == WHITE ? 8 : -8;
  constexpr int dUpRight = Us == WHITE ? 9 : -9;
  constexpr int dUpLeft = Us == WHITE ? 7 : -7;
  constexpr Bitboard Rank7 = Us == WHITE ? Rank7BB : Rank2BB;
  constexpr Bitboard Rank3 = Us == WHITE ? Rank3BB : Rank6BB;

  const Bitboard empty = ~pos.pieces();
  const Bitboard enemies = T == EVASIONS ? pos.checkers() : pos.pieces(Them);
  const Bitboard pawns = pos.pieces(Us, PAWN);
  const Bitboard on7 = pawns & Rank7;
  const Bitboard not7 = pawns & ~Rank7;

  if (T != CAPTURES) {
    Bitboard b1 = shift<Up>(not7) & empty;
    Bitboard b2 = shift<Up>(b1 & Rank3) & empty;
    if (T == EVASIONS) { b1 &= target; b2 &= target; }
    while (b1) { Square to = pop_lsb(b1); l = add(l, to - dUp, to, QUIET); }
    while (b2) { Square to = pop_lsb(b2); l = add(l, to - 2 * dUp, to, DOUBLE_PUSH); }
  }
  if (on7) {
    Bitboard b1 = shift<UpRight>(on7) & enemies;
    Bitboard b2 = shift<UpLeft>(on7) & enemies;
    Bitboard b3 = shift<Up>(on7) & empty;
    if (T == EVASIONS) b3 &= target;
    while (b1) {
      Square to = pop_lsb(b1);
      if (T != QUIETS) l = add(l, to - dUpRight, to, PROMO_CAP_Q);
      if (T != CAPTURES) { l = add(l, to - dUpRight, to, PROMO_CAP_R); l = add(l, to - dUpRight, to, PROMO_CAP_B); l = add(l, to - dUpRight, to, PROMO_CAP_N); }
    }
    while (b2) {
      Square to = pop_lsb(b2);
      if (T != QUIETS) l = add(l, to - dUpLeft, to, PROMO_CAP_Q);
      if (T != CAPTURES) { l = add(l, to - dUpLeft, to, PROMO_CAP_R); l = add(l, to - dUpLeft, to, PROMO_CAP_B); l = add(l, to - dUpLeft, to, PROMO_CAP_N); }
    }
    while (b3) {
      Square to = pop_lsb(b3);
      if (T != QUIETS) l = add(l, to - dUp, to, PROMO_Q);
      if (T != CAPTURES) { l = add(l, to - dUp, to, PROMO_R); l = add(l, to - dUp, to, PROMO_B); l = add(l, to - dUp, to, PROMO_N); }
    }
  }
  if (T == CAPTURES || T == EVASIONS) {
    Bitboard b1 = shift<UpRight>(not7) & enemies;
    Bitboard b2 = shift<UpLeft>(not7) & enemies;
    while (b1) { Square to = pop_lsb(b1); l = add(l, to - dUpRight, to, CAPTURE); }
    while (b2) { Square to = pop_lsb(b2); l = add(l, to - dUpLeft, to, CAPTURE); }
    if (pos.ep_square() != SQ_NONE) {
      Bitboard b = not7 & PawnAttacks[Them][pos.ep_square()];
      while (b) l = add(l, pop_lsb(b), pos.ep_square(), EP_CAPTURE);
    }
  }
  return l;
}

template <Color Us, GenType T>
ExtMove *gen_all(const Position &pos, ExtMove *l) {
  constexpr Color Them = Us ^ 1;
  const Square ksq = pos.king_square(Us);
  const Bitboard checkers = pos.checkers();
  const Bitboard occ = pos.pieces();
  Bitboard target;

  if (T == EVASIONS) {
    Bitboard kb = KingAttacks[ksq] & ~pos.pieces(Us);
    if (more_than_one(checkers)) {
      while (kb) { Square to = pop_lsb(kb); l = add(l, ksq, to, (pos.pieces(Them) & sq_bb(to)) ? CAPTURE : QUIET); }
      return l;
    }
    Square cs = lsb(checkers);
    target = BetweenBB[ksq][cs] | checkers;
  } else if (T == CAPTURES) target = pos.pieces(Them);
  else target = ~occ;

  l = gen_pawns<Us, T>(pos, l, target);
  l = piece_moves<T>(pos, l, Us, KNIGHT, target);
  l = piece_moves<T>(pos, l, Us, BISHOP, target);
  l = piece_moves<T>(pos, l, Us, ROOK, target);
  l = piece_moves<T>(pos, l, Us, QUEEN, target);

  Bitboard kb = KingAttacks[ksq] & (T == EVASIONS ? ~pos.pieces(Us) : target);
  while (kb) { Square to = pop_lsb(kb); l = add(l, ksq, to, (pos.pieces(Them) & sq_bb(to)) ? CAPTURE : QUIET); }

  if (T == QUIETS && !checkers) {
    constexpr int OO = Us == WHITE ? WHITE_OO : BLACK_OO;
    constexpr int OOO = Us == WHITE ? WHITE_OOO : BLACK_OOO;
    if (pos.can_castle(OO) && !(occ & (sq_bb(ksq + 1) | sq_bb(ksq + 2)))) l = add(l, ksq, ksq + 2, KING_CASTLE);
    if (pos.can_castle(OOO) && !(occ & (sq_bb(ksq - 1) | sq_bb(ksq - 2) | sq_bb(ksq - 3)))) l = add(l, ksq, ksq - 2, QUEEN_CASTLE);
  }
  return l;
}
}  // namespace

template <GenType T> ExtMove *generate(const Position &pos, ExtMove *list) {
  return pos.side_to_move() == WHITE ? gen_all<WHITE, T>(pos, list) : gen_all<BLACK, T>(pos, list);
}
template ExtMove *generate<CAPTURES>(const Position &, ExtMove *);
template ExtMove *generate<QUIETS>(const Position &, ExtMove *);
template ExtMove *generate<EVASIONS>(const Position &, ExtMove *);

ExtMove *generate_legal(const Position &pos, ExtMove *list) {
  ExtMove *end;
  if (pos.in_check()) end = generate<EVASIONS>(pos, list);
  else { end = generate<CAPTURES>(pos, list); end = generate<QUIETS>(pos, end); }
  ExtMove *out = list;
  for (ExtMove *m = list; m != end; ++m)
    if (pos.legal(m->move)) *out++ = *m;
  return out;
}

// ======================================== tt.cpp

#include <cstdlib>
#include <cstring>
#include <climits>

namespace {
inline U64 pack(Move m, int value, int eval, int depth, int bound, bool pv, unsigned gen) {
  return U64(m) | (U64(uint16_t(int16_t(value))) << 16) | (U64(uint16_t(int16_t(eval))) << 32) |
         (U64(uint8_t(std::max(0, std::min(255, depth)))) << 48) | (U64(bound & 3) << 56) |
         (U64(pv ? 1 : 0) << 58) | (U64(gen & 31) << 59);
}
}  // namespace

TranspositionTable::~TranspositionTable() { std::free(table); }

void TranspositionTable::resize(size_t mb) {
  std::free(table);
  count = std::max<size_t>(1, mb * 1024 * 1024 / sizeof(Cluster));
  table = static_cast<Cluster *>(std::aligned_alloc(64, count * sizeof(Cluster)));
  clear();
}

void TranspositionTable::clear() {
  if (!table) return;
  std::memset(static_cast<void *>(table), 0, count * sizeof(Cluster));
  gen = 0;
}

bool TranspositionTable::probe(U64 key, TTData &out) const {
  const Cluster &c = table[index(key)];
  for (int i = 0; i < 4; ++i) {
    U64 d = c.e[i].data.load(std::memory_order_relaxed);
    U64 kx = c.e[i].keyx.load(std::memory_order_relaxed);
    if (d && (kx ^ d) == key) {
      out.move = Move(d & 0xFFFF);
      out.value = int16_t((d >> 16) & 0xFFFF);
      out.eval = int16_t((d >> 32) & 0xFFFF);
      out.depth = int((d >> 48) & 0xFF);
      out.bound = int((d >> 56) & 3);
      out.pv = (d >> 58) & 1;
      return true;
    }
  }
  return false;
}

void TranspositionTable::store(U64 key, int value, bool pv, int bound, int depth, Move m, int eval) {
  Cluster &c = table[index(key)];
  Entry *rep = &c.e[0];
  int repScore = INT_MAX;
  for (int i = 0; i < 4; ++i) {
    U64 d = c.e[i].data.load(std::memory_order_relaxed);
    U64 kx = c.e[i].keyx.load(std::memory_order_relaxed);
    if (!d) { rep = &c.e[i]; break; }
    if ((kx ^ d) == key) {
      int odepth = int((d >> 48) & 0xFF);
      unsigned ogen = unsigned((d >> 59) & 31);
      if (m == MOVE_NONE) m = Move(d & 0xFFFF);          // keep the old best move
      if (bound != BOUND_EXACT && ogen == gen && depth + 3 < odepth) return;   // don't overwrite deeper data
      rep = &c.e[i];
      goto write;
    }
    {
      int odepth = int((d >> 48) & 0xFF);
      unsigned ogen = unsigned((d >> 59) & 31);
      int score = odepth - 8 * int((32 + gen - ogen) & 31);
      if (score < repScore) { repScore = score; rep = &c.e[i]; }
    }
  }
write:
  U64 nd = pack(m, value, eval, depth, bound, pv, gen);
  rep->data.store(nd, std::memory_order_relaxed);
  rep->keyx.store(key ^ nd, std::memory_order_relaxed);
}

int TranspositionTable::hashfull() const {
  int cnt = 0;
  size_t n = std::min<size_t>(count, 250);
  for (size_t i = 0; i < n; ++i)
    for (int j = 0; j < 4; ++j) {
      U64 d = table[i].e[j].data.load(std::memory_order_relaxed);
      if (d && unsigned((d >> 59) & 31) == gen) ++cnt;
    }
  return int(cnt * 1000 / (n * 4));
}

// ======================================== search.cpp
// search.cpp - iterative deepening PVS with Lazy SMP



#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <sstream>

namespace Search {

TranspositionTable TT;
std::atomic<bool> stop{false};

namespace {

enum { NonPV, PV, Root };

typedef int16_t PieceToHist[16][64];
typedef int16_t (*CH)[64];           // pointer to a [16][64] continuation-history table

constexpr int HIST_D = 16384;

inline void gravity(int16_t &e, int bonus) { e = int16_t(e + bonus - e * std::abs(bonus) / HIST_D); }
inline int stat_bonus(int d) { return std::min(16 * d * d + 32 * d, 2400); }

int Reductions[64][64];
int g_threads = 1;
int g_overhead = 30;
SearchLimits g_limits;
std::chrono::steady_clock::time_point g_start;
double g_optimum = 0, g_maximum = 0;     // ms; 0 = unlimited
bool g_timed = false, g_movetime = false;

inline double elapsed_ms() {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - g_start).count();
}

inline int value_to_tt(int v, int ply) { return v >= MATE_IN_MAX ? v + ply : v <= MATED_IN_MAX ? v - ply : v; }
inline int value_from_tt(int v, int ply) {
  if (v == VALUE_NONE) return VALUE_NONE;
  return v >= MATE_IN_MAX ? v - ply : v <= MATED_IN_MAX ? v + ply : v;
}

struct Stack {
  Move *pv;
  int ply;
  Move currentMove;
  Move excludedMove;
  Move killers[2];
  int staticEval;
  int statScore;
  int moveCount;
  bool inCheck;
  bool ttPv;
  CH contHist;
};

struct RootMove {
  Move move;
  int score = -VALUE_INFINITE;
  int previousScore = -VALUE_INFINITE;
  int selDepth = 0;
  std::vector<Move> pv;
};

struct SearchThread;

class MovePicker {
public:
  MovePicker(const Position &p, Move ttm, const SearchThread &t, const Stack *s, bool qs);
  Move next(bool skipQuiets);

private:
  enum Stage { MAIN_TT, CAP_INIT, GOOD_CAP, KILLER1, KILLER2, COUNTER, QUIET_INIT, QUIET, BAD_CAP, DONE,
               EV_TT, EV_INIT, EV, QS_TT, QS_INIT, QS_CAP };
  void score_captures();
  void score_quiets();
  void score_evasions();
  static ExtMove *pick_best(ExtMove *b, ExtMove *e) {
    ExtMove *best = b;
    for (ExtMove *p = b + 1; p < e; ++p) if (p->score > best->score) best = p;
    std::swap(*b, *best);
    return b;
  }
  const Position &pos;
  const SearchThread &th;
  const Stack *ss;
  Move ttMove, k1, k2, counter;
  int stage;
  ExtMove moves[MAX_MOVES];
  ExtMove *cur, *end;
  Move bad[MAX_MOVES];
  int nbad, curBad;
};

struct SearchThread {
  int id;
  bool isMain;
  Position pos;
  std::vector<RootMove> rootMoves;
  std::atomic<U64> nodes{0};
  int selDepth = 0, rootDepth = 0, completedDepth = 0, nmpMinPly = 0;
  int16_t mainHist[2][64][64];
  int16_t capHist[16][64][8];
  Move counters[16][64];
  PieceToHist *contStore;
  Stack stack[MAX_PLY + 10];
  Move pvTable[MAX_PLY + 10][MAX_PLY + 2];
  Move lastBest = MOVE_NONE;
  int stability = 0;
  int prevIterScore = 0;

  explicit SearchThread(int i) : id(i), isMain(i == 0) {
    contStore = new PieceToHist[16 * 64];
    clear();
  }
  ~SearchThread() { delete[] contStore; }
  void clear() {
    std::memset(mainHist, 0, sizeof(mainHist));
    std::memset(capHist, 0, sizeof(capHist));
    std::memset(counters, 0, sizeof(counters));
    std::memset(static_cast<void *>(contStore), 0, sizeof(PieceToHist) * 16 * 64);
  }

  void iterate();
  void report(int depth);
  template <int NT> int search(Stack *ss, int alpha, int beta, int depth, bool cutNode);
  template <int NT> int qsearch(Stack *ss, int alpha, int beta);
  void check_time();
  void update_quiet(Stack *ss, Move m, int bonus, bool setKiller);
  void update_cont(Stack *ss, Piece pc, Square to, int bonus);
  void update_stats(Stack *ss, Move best, int bestValue, int beta, int depth, const Move *quiets, int nq,
                    const Move *caps, int nc);
};

std::vector<std::unique_ptr<SearchThread>> g_pool;
std::thread g_driver;

// ------------------------------------------------------------------ MovePicker
MovePicker::MovePicker(const Position &p, Move ttm, const SearchThread &t, const Stack *s, bool qs)
    : pos(p), th(t), ss(s), ttMove(ttm), k1(MOVE_NONE), k2(MOVE_NONE), counter(MOVE_NONE), nbad(0), curBad(0) {
  const bool inCheck = pos.in_check();
  if (ttMove && (!pos.pseudo_legal(ttMove) || (qs && !inCheck && !is_capture_or_promo(ttMove)))) ttMove = MOVE_NONE;
  if (inCheck) stage = EV_TT;
  else if (qs) stage = QS_TT;
  else {
    stage = MAIN_TT;
    k1 = ss->killers[0];
    k2 = ss->killers[1];
    Move prev = (ss - 1)->currentMove;
    if (prev != MOVE_NONE && prev != MOVE_NULL) counter = th.counters[pos.piece_on(to_sq(prev))][to_sq(prev)];
  }
  cur = end = moves;
}

void MovePicker::score_captures() {
  for (ExtMove *m = moves; m != end; ++m) {
    Move mv = m->move;
    Piece pc = pos.piece_on(from_sq(mv));
    int victim = move_flag(mv) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to_sq(mv)));
    int s = 10 * SeeValue[victim] - type_of(pc);
    if (is_promo(mv)) s += SeeValue[promo_type(mv)];
    s += th.capHist[pc][to_sq(mv)][victim] / 16;
    m->score = s;
  }
}

void MovePicker::score_quiets() {
  const Color us = pos.side_to_move();
  CH c1 = (ss - 1)->contHist, c2 = (ss - 2)->contHist;
  for (ExtMove *m = moves; m != end; ++m) {
    Move mv = m->move;
    Piece pc = pos.piece_on(from_sq(mv));
    Square to = to_sq(mv);
    m->score = th.mainHist[us][from_sq(mv)][to] + 2 * c1[pc][to] + c2[pc][to];
  }
}

void MovePicker::score_evasions() {
  const Color us = pos.side_to_move();
  CH c1 = (ss - 1)->contHist;
  for (ExtMove *m = moves; m != end; ++m) {
    Move mv = m->move;
    Piece pc = pos.piece_on(from_sq(mv));
    Square to = to_sq(mv);
    if (is_capture(mv)) {
      int victim = move_flag(mv) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to));
      m->score = 100000 + 10 * SeeValue[victim] - type_of(pc);
    } else
      m->score = th.mainHist[us][from_sq(mv)][to] + c1[pc][to];
  }
}

Move MovePicker::next(bool skipQuiets) {
  for (;;) {
    switch (stage) {
    case MAIN_TT: case EV_TT: case QS_TT:
      ++stage;
      if (ttMove) return ttMove;
      break;
    case CAP_INIT:
      cur = moves;
      end = generate<CAPTURES>(pos, moves);
      score_captures();
      ++stage;
      break;
    case GOOD_CAP:
      while (cur < end) {
        pick_best(cur, end);
        Move m = cur->move;
        ++cur;
        if (m == ttMove) continue;
        if (pos.see_ge(m, 0)) return m;
        bad[nbad++] = m;
      }
      ++stage;
      break;
    case KILLER1:
      ++stage;
      if (!skipQuiets && k1 && k1 != ttMove && !is_capture_or_promo(k1) && pos.pseudo_legal(k1)) return k1;
      break;
    case KILLER2:
      ++stage;
      if (!skipQuiets && k2 && k2 != ttMove && k2 != k1 && !is_capture_or_promo(k2) && pos.pseudo_legal(k2)) return k2;
      break;
    case COUNTER:
      ++stage;
      if (!skipQuiets && counter && counter != ttMove && counter != k1 && counter != k2 &&
          !is_capture_or_promo(counter) && pos.pseudo_legal(counter)) return counter;
      break;
    case QUIET_INIT:
      cur = end = moves;
      if (!skipQuiets) {
        end = generate<QUIETS>(pos, moves);
        score_quiets();
      }
      ++stage;
      break;
    case QUIET:
      if (!skipQuiets)
        while (cur < end) {
          pick_best(cur, end);
          Move m = cur->move;
          ++cur;
          if (m == ttMove || m == k1 || m == k2 || m == counter) continue;
          return m;
        }
      ++stage;
      break;
    case BAD_CAP:
      while (curBad < nbad) {
        Move m = bad[curBad++];
        if (m != ttMove) return m;
      }
      stage = DONE;
      break;
    case DONE:
      return MOVE_NONE;
    case EV_INIT:
      cur = moves;
      end = generate<EVASIONS>(pos, moves);
      score_evasions();
      ++stage;
      break;
    case EV:
    case QS_CAP:
      while (cur < end) {
        pick_best(cur, end);
        Move m = cur->move;
        ++cur;
        if (m == ttMove) continue;
        return m;
      }
      return MOVE_NONE;
    case QS_INIT:
      cur = moves;
      end = generate<CAPTURES>(pos, moves);
      score_captures();
      ++stage;
      break;
    default:
      return MOVE_NONE;
    }
  }
}

// ------------------------------------------------------------------ history helpers
void SearchThread::update_cont(Stack *ss, Piece pc, Square to, int bonus) {
  for (int i = 1; i <= 2; ++i) {
    Move m = (ss - i)->currentMove;
    if (m != MOVE_NONE && m != MOVE_NULL) gravity((ss - i)->contHist[pc][to], bonus);
  }
}

void SearchThread::update_quiet(Stack *ss, Move m, int bonus, bool setKiller) {
  const Color us = pos.side_to_move();
  if (setKiller && ss->killers[0] != m) {
    ss->killers[1] = ss->killers[0];
    ss->killers[0] = m;
  }
  gravity(mainHist[us][from_sq(m)][to_sq(m)], bonus);
  update_cont(ss, pos.piece_on(from_sq(m)), to_sq(m), bonus);
  if (setKiller) {
    Move prev = (ss - 1)->currentMove;
    if (prev != MOVE_NONE && prev != MOVE_NULL) counters[pos.piece_on(to_sq(prev))][to_sq(prev)] = m;
  }
}

void SearchThread::update_stats(Stack *ss, Move best, int bestValue, int beta, int depth, const Move *quiets, int nq,
                                const Move *caps, int nc) {
  int bonus1 = stat_bonus(depth + 1);
  int bonus2 = bestValue > beta + 150 ? bonus1 : stat_bonus(depth);
  if (!is_capture_or_promo(best)) {
    update_quiet(ss, best, bonus2, true);
    for (int i = 0; i < nq; ++i) update_quiet(ss, quiets[i], -bonus2, false);
  } else {
    Piece pc = pos.piece_on(from_sq(best));
    int victim = move_flag(best) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to_sq(best)));
    gravity(capHist[pc][to_sq(best)][victim], bonus1);
  }
  for (int i = 0; i < nc; ++i) {
    Move c = caps[i];
    Piece pc = pos.piece_on(from_sq(c));
    int victim = move_flag(c) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to_sq(c)));
    gravity(capHist[pc][to_sq(c)][victim], -bonus1);
  }
}

void update_pv(Move *pv, Move m, const Move *child) {
  for (*pv++ = m; child && *child != MOVE_NONE;) *pv++ = *child++;
  *pv = MOVE_NONE;
}

void SearchThread::check_time() {
  if (g_limits.nodes && nodes.load(std::memory_order_relaxed) >= g_limits.nodes) stop = true;
  if (g_maximum > 0 && elapsed_ms() >= g_maximum) stop = true;
}

// ------------------------------------------------------------------ quiescence
template <int NT> int SearchThread::qsearch(Stack *ss, int alpha, int beta) {
  constexpr bool PvNode = NT == PV;
  Move bestMove = MOVE_NONE;
  ss->inCheck = pos.in_check();
  ss->ply = (ss - 1)->ply + 1;
  U64 n = nodes.fetch_add(1, std::memory_order_relaxed) + 1;
  if (isMain && (n & 1023) == 0) check_time();
  if (PvNode) {
    ss->pv[0] = MOVE_NONE;
    if (selDepth < ss->ply + 1) selDepth = ss->ply + 1;
  }
  if (stop.load(std::memory_order_relaxed)) return 0;
  if (pos.is_draw(ss->ply)) return 0;
  if (ss->ply >= MAX_PLY - 1) return ss->inCheck ? 0 : Eval::evaluate(pos);

  const U64 key = pos.key();
  TTData tte{};
  const bool ttHit = TT.probe(key, tte);
  const int ttValue = ttHit ? value_from_tt(tte.value, ss->ply) : VALUE_NONE;
  const Move ttMove = ttHit ? tte.move : MOVE_NONE;
  if (!PvNode && ttHit && ttValue != VALUE_NONE && (tte.bound & (ttValue >= beta ? BOUND_LOWER : BOUND_UPPER)))
    return ttValue;

  int bestValue, futilityBase;
  if (ss->inCheck) {
    bestValue = futilityBase = -VALUE_INFINITE;
    ss->staticEval = VALUE_NONE;
  } else {
    bestValue = ss->staticEval = (ttHit && tte.eval != VALUE_NONE) ? tte.eval : Eval::evaluate(pos);
    if (ttHit && ttValue != VALUE_NONE && (tte.bound & (ttValue > bestValue ? BOUND_LOWER : BOUND_UPPER)))
      bestValue = ttValue;
    if (bestValue >= beta) {
      if (!ttHit) TT.store(key, value_to_tt(bestValue, ss->ply), false, BOUND_LOWER, 0, MOVE_NONE, ss->staticEval);
      return bestValue;
    }
    if (PvNode && bestValue > alpha) alpha = bestValue;
    futilityBase = bestValue + 150;
  }

  MovePicker mp(pos, ttMove, *this, ss, true);
  Move move;
  int moveCount = 0;
  while ((move = mp.next(false)) != MOVE_NONE) {
    if (!pos.legal(move)) continue;
    ++moveCount;
    const bool givesCheck = pos.gives_check(move);
    if (bestValue > MATED_IN_MAX && !ss->inCheck) {
      if (!givesCheck && !is_promo(move) && futilityBase > -MATE_IN_MAX) {
        int victim = move_flag(move) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to_sq(move)));
        int fv = futilityBase + SeeValue[victim];
        if (fv <= alpha) { bestValue = std::max(bestValue, fv); continue; }
        if (futilityBase <= alpha && !pos.see_ge(move, 1)) { bestValue = std::max(bestValue, futilityBase); continue; }
      }
      if (!pos.see_ge(move, 0)) continue;
    }
    Piece pc = pos.piece_on(from_sq(move));
    ss->currentMove = move;
    ss->contHist = contStore[pc * 64 + to_sq(move)];
    pos.do_move(move);
    TT.prefetch(pos.key());
    int value = -qsearch<NT>(ss + 1, -beta, -alpha);
    pos.undo_move(move);
    if (stop.load(std::memory_order_relaxed)) return 0;
    if (value > bestValue) {
      bestValue = value;
      if (value > alpha) {
        bestMove = move;
        if (PvNode) update_pv(ss->pv, move, (ss + 1)->pv);
        if (PvNode && value < beta) alpha = value;
        else break;
      }
    }
  }
  if (ss->inCheck && bestValue == -VALUE_INFINITE) return mated_in(ss->ply);
  TT.store(key, value_to_tt(bestValue, ss->ply), PvNode && ttHit && tte.pv,
           bestValue >= beta ? BOUND_LOWER : (PvNode && bestMove) ? BOUND_EXACT : BOUND_UPPER, 0, bestMove, ss->staticEval);
  return bestValue;
}

// ------------------------------------------------------------------ main search
template <int NT> int SearchThread::search(Stack *ss, int alpha, int beta, int depth, bool cutNode) {
  constexpr bool PvNode = NT != NonPV;
  constexpr bool rootNode = NT == Root;
  if (depth <= 0) return qsearch<PvNode ? PV : NonPV>(ss, alpha, beta);

  const Color us = pos.side_to_move();
  ss->inCheck = pos.in_check();
  ss->ply = rootNode ? 0 : (ss - 1)->ply + 1;
  U64 n = nodes.fetch_add(1, std::memory_order_relaxed) + 1;
  if (isMain && (n & 1023) == 0) check_time();
  if (PvNode) {
    ss->pv[0] = MOVE_NONE;
    if (selDepth < ss->ply + 1) selDepth = ss->ply + 1;
  }
  if (stop.load(std::memory_order_relaxed)) return 0;
  if (!rootNode) {
    if (pos.is_draw(ss->ply)) return 0;
    if (ss->ply >= MAX_PLY - 1) return ss->inCheck ? 0 : Eval::evaluate(pos);
    alpha = std::max(mated_in(ss->ply), alpha);
    beta = std::min(mate_in(ss->ply + 1), beta);
    if (alpha >= beta) return alpha;
  }
  ss->moveCount = 0;
  (ss + 1)->excludedMove = MOVE_NONE;
  (ss + 2)->killers[0] = (ss + 2)->killers[1] = MOVE_NONE;
  const Move excludedMove = ss->excludedMove;

  // ---- transposition table
  const U64 key = pos.key() ^ (U64(excludedMove) * 0x9E3779B97F4A7C15ULL);
  TTData tte{};
  const bool ttHit = TT.probe(key, tte);
  const int ttValue = ttHit ? value_from_tt(tte.value, ss->ply) : VALUE_NONE;
  const Move ttMove = rootNode ? rootMoves[0].move : (ttHit ? tte.move : MOVE_NONE);
  const bool ttCapture = ttMove && is_capture_or_promo(ttMove);
  if (!excludedMove) ss->ttPv = PvNode || (ttHit && tte.pv);

  if (!PvNode && !excludedMove && ttHit && ttValue != VALUE_NONE && tte.depth >= depth && pos.rule50() < 90 &&
      (tte.bound & (ttValue >= beta ? BOUND_LOWER : BOUND_UPPER))) {
    if (ttMove && ttValue >= beta && !ttCapture && pos.pseudo_legal(ttMove)) update_quiet(ss, ttMove, stat_bonus(depth), true);
    return ttValue;
  }

  // ---- static evaluation
  int eval = VALUE_NONE;
  bool improving = false;
  if (!ss->inCheck) {
    if (excludedMove) eval = ss->staticEval;
    else {
      eval = (ttHit && tte.eval != VALUE_NONE) ? tte.eval : Eval::evaluate(pos);
      ss->staticEval = eval;
      if (!ttHit) TT.store(key, VALUE_NONE, ss->ttPv, BOUND_NONE, 0, MOVE_NONE, eval);
      if (ttHit && ttValue != VALUE_NONE && (tte.bound & (ttValue > eval ? BOUND_LOWER : BOUND_UPPER))) eval = ttValue;
    }
    improving = (ss - 2)->staticEval != VALUE_NONE ? ss->staticEval > (ss - 2)->staticEval
              : (ss - 4)->staticEval != VALUE_NONE ? ss->staticEval > (ss - 4)->staticEval : true;

    if (!PvNode && !excludedMove) {
      // reverse futility
      if (depth < 9 && eval - (90 * depth - 60 * improving) >= beta && eval < MATE_IN_MAX) return eval;
      // null move
      if ((ss - 1)->currentMove != MOVE_NULL && eval >= beta && ss->staticEval >= beta - 20 * depth + 200 &&
          pos.has_non_pawn(us) && ss->ply >= nmpMinPly && beta > MATED_IN_MAX) {
        int R = 3 + depth / 3 + std::min((eval - beta) / 150, 3);
        ss->currentMove = MOVE_NULL;
        ss->contHist = contStore[0];
        pos.do_null_move();
        TT.prefetch(pos.key());
        int nv = -search<NonPV>(ss + 1, -beta, -beta + 1, depth - R, !cutNode);
        pos.undo_null_move();
        if (stop.load(std::memory_order_relaxed)) return 0;
        if (nv >= beta) {
          if (nv >= MATE_IN_MAX) nv = beta;
          if (nmpMinPly || depth < 14) return nv;
          nmpMinPly = ss->ply + 3 * (depth - R) / 4;
          int v = search<NonPV>(ss, beta - 1, beta, depth - R, false);
          nmpMinPly = 0;
          if (v >= beta) return nv;
        }
      }
    }
    // internal iterative reduction
    if (depth >= 4 && !ttMove && (PvNode || cutNode)) --depth;
  } else {
    improving = false;
    ss->staticEval = VALUE_NONE;
  }

  // ---- move loop
  MovePicker mp(pos, ttMove, *this, ss, false);
  Move move, bestMove = MOVE_NONE;
  int bestValue = -VALUE_INFINITE, value = 0, moveCount = 0;
  Move quiets[64], caps[32];
  int nq = 0, nc = 0;
  bool moveCountPruning = false;
  CH c1 = (ss - 1)->contHist, c2 = (ss - 2)->contHist;

  while ((move = mp.next(moveCountPruning)) != MOVE_NONE) {
    if (move == excludedMove) continue;
    if (!pos.legal(move)) continue;
    ++moveCount;
    ss->moveCount = moveCount;
    const Piece pc = pos.piece_on(from_sq(move));
    const Square from = from_sq(move), to = to_sq(move);
    const bool capOrPromo = is_capture_or_promo(move);
    const bool givesCheck = pos.gives_check(move);
    int extension = 0;

    // ---- shallow-depth pruning
    if (!rootNode && pos.has_non_pawn(us) && bestValue > MATED_IN_MAX) {
      moveCountPruning = moveCount >= (improving ? 3 + depth * depth : (3 + depth * depth) / 2);
      int r = Reductions[std::min(depth, 63)][std::min(moveCount, 63)] - improving;
      int lmrDepth = std::max(0, depth - std::max(r, 0));
      if (!capOrPromo && !givesCheck) {
        if (moveCountPruning) continue;
        if (lmrDepth < 5 && c1[pc][to] + c2[pc][to] < -3000 * depth) continue;
        if (!ss->inCheck && lmrDepth < 8 && ss->staticEval + 100 + 120 * lmrDepth <= alpha) continue;
        if (!pos.see_ge(move, -30 * lmrDepth * lmrDepth)) continue;
      } else {
        if (!ss->inCheck && !givesCheck && lmrDepth < 6) {
          int victim = move_flag(move) == EP_CAPTURE ? PAWN : type_of(pos.piece_on(to));
          if (ss->staticEval + 150 + 130 * lmrDepth + SeeValue[victim] <= alpha) continue;
        }
        if (!pos.see_ge(move, -70 * depth)) continue;
      }
    }

    // ---- extensions
    if (!rootNode && depth >= 8 && move == ttMove && !excludedMove && ttValue != VALUE_NONE &&
        std::abs(ttValue) < MATE_IN_MAX && (tte.bound & BOUND_LOWER) && tte.depth >= depth - 3) {
      int sBeta = ttValue - 2 * depth;
      ss->excludedMove = move;
      int v = search<NonPV>(ss, sBeta - 1, sBeta, (depth - 1) / 2, cutNode);
      ss->excludedMove = MOVE_NONE;
      if (stop.load(std::memory_order_relaxed)) return 0;
      if (v < sBeta) extension = 1;
      else if (sBeta >= beta) return sBeta;
      else if (ttValue >= beta) extension = -1;
    } else if (givesCheck && !moveCountPruning && pos.see_ge(move, 0)) extension = 1;

    const int newDepth = depth - 1 + extension;
    const int hist = mainHist[us][from][to] + c1[pc][to] + c2[pc][to];

    ss->currentMove = move;
    ss->contHist = contStore[pc * 64 + to];
    if (PvNode) (ss + 1)->pv[0] = MOVE_NONE;   // child may not be searched as PV: never copy a stale line
    pos.do_move(move);
    TT.prefetch(pos.key());

    bool doFull;
    if (depth >= 3 && moveCount > 1 + 2 * rootNode && (!capOrPromo || moveCountPruning || cutNode)) {
      int r = Reductions[std::min(depth, 63)][std::min(moveCount, 63)];
      if (!improving && r >= 2) ++r;
      if (cutNode) ++r;
      if (PvNode) --r;
      if (ss->ttPv) --r;
      if (givesCheck) --r;
      if (ttCapture) ++r;
      if ((ss - 1)->moveCount > 12) --r;
      if (!capOrPromo) {
        ss->statScore = hist;
        r -= std::max(-2, std::min(2, ss->statScore / 6000));
      }
      int d = std::max(1, std::min(newDepth - r, newDepth + 1));
      value = -search<NonPV>(ss + 1, -(alpha + 1), -alpha, d, true);
      doFull = value > alpha && d < newDepth;
    } else doFull = !PvNode || moveCount > 1;

    if (doFull) value = -search<NonPV>(ss + 1, -(alpha + 1), -alpha, newDepth, !cutNode);
    if (PvNode && (moveCount == 1 || (value > alpha && (rootNode || value < beta))))
      value = -search<PV>(ss + 1, -beta, -alpha, newDepth, false);

    pos.undo_move(move);
    if (stop.load(std::memory_order_relaxed)) return 0;

    if (rootNode) {
      RootMove &rm = *std::find_if(rootMoves.begin(), rootMoves.end(), [&](const RootMove &r) { return r.move == move; });
      if (moveCount == 1 || value > alpha) {
        rm.score = value;
        rm.selDepth = selDepth;
        rm.pv.assign(1, move);
        for (const Move *m = (ss + 1)->pv; *m != MOVE_NONE; ++m) rm.pv.push_back(*m);
      } else rm.score = -VALUE_INFINITE;
    }

    if (value > bestValue) {
      bestValue = value;
      if (value > alpha) {
        bestMove = move;
        if (PvNode && !rootNode) update_pv(ss->pv, move, (ss + 1)->pv);
        if (PvNode && value < beta) alpha = value;
        else break;
      }
    }
    if (move != bestMove) {
      if (capOrPromo) { if (nc < 32) caps[nc++] = move; }
      else if (nq < 64) quiets[nq++] = move;
    }
  }

  if (moveCount == 0) bestValue = excludedMove ? alpha : ss->inCheck ? mated_in(ss->ply) : 0;
  else if (bestMove) update_stats(ss, bestMove, bestValue, beta, depth, quiets, nq, caps, nc);

  if (!excludedMove)
    TT.store(key, value_to_tt(bestValue, ss->ply), ss->ttPv,
             bestValue >= beta ? BOUND_LOWER : (PvNode && bestMove) ? BOUND_EXACT : BOUND_UPPER, depth, bestMove,
             ss->staticEval);
  return bestValue;
}

// ------------------------------------------------------------------ reporting / iterative deepening
std::string uci_score(int v) {
  char buf[64];
  if (v >= MATE_IN_MAX) std::snprintf(buf, sizeof buf, "mate %d", (VALUE_MATE - v + 1) / 2);
  else if (v <= MATED_IN_MAX) std::snprintf(buf, sizeof buf, "mate -%d", (VALUE_MATE + v + 1) / 2);
  else std::snprintf(buf, sizeof buf, "cp %d", v);
  return buf;
}

void SearchThread::report(int depth) {
  U64 total = 0;
  for (auto &t : g_pool) total += t->nodes.load(std::memory_order_relaxed);
  double ms = std::max(1.0, elapsed_ms());
  const RootMove &rm = rootMoves[0];
  std::ostringstream os;
  os << "info depth " << depth << " seldepth " << rm.selDepth << " score " << uci_score(rm.score) << " nodes " << total
     << " nps " << U64(total * 1000.0 / ms) << " hashfull " << TT.hashfull() << " time " << int(ms) << " pv";
  for (Move m : rm.pv) os << ' ' << move_to_uci(m);
  std::cout << os.str() << std::endl;
}

const int SkipSize[20] = {1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
const int SkipPhase[20] = {0, 1, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7};

void SearchThread::iterate() {
  for (int i = 0; i < MAX_PLY + 10; ++i) {
    Stack &s = stack[i];
    s.pv = pvTable[i];
    s.ply = i - 7;
    s.currentMove = s.excludedMove = MOVE_NONE;
    s.killers[0] = s.killers[1] = MOVE_NONE;
    s.staticEval = VALUE_NONE;
    s.statScore = 0;
    s.moveCount = 0;
    s.inCheck = false;
    s.ttPv = false;
    s.contHist = contStore[0];
  }
  Stack *ss = stack + 7;
  rootMoves.clear();
  ExtMove list[MAX_MOVES];
  ExtMove *end = generate_legal(pos, list);
  for (ExtMove *m = list; m != end; ++m) { RootMove rm; rm.move = m->move; rootMoves.push_back(rm); }
  if (rootMoves.empty()) return;
  if (rootMoves.size() == 1 && g_timed && !g_limits.infinite && !g_movetime) { /* forced move: answer fast */ }

  nodes = 0; selDepth = 0; completedDepth = 0; nmpMinPly = 0; lastBest = MOVE_NONE; stability = 0;
  const int maxDepth = g_limits.depth ? std::min(g_limits.depth, MAX_PLY - 2) : MAX_PLY - 2;

  for (rootDepth = 1; rootDepth <= maxDepth && !stop.load(); ++rootDepth) {
    if (!isMain) {
      int i = (id - 1) % 20;
      if (((rootDepth + SkipPhase[i]) / SkipSize[i]) % 2) continue;
    }
    for (auto &rm : rootMoves) rm.previousScore = rm.score;
    selDepth = 0;
    int prev = rootMoves[0].previousScore;
    int delta = 12, alpha = -VALUE_INFINITE, beta = VALUE_INFINITE;
    if (rootDepth >= 4 && std::abs(prev) < MATE_IN_MAX) {
      alpha = std::max(prev - delta, -VALUE_INFINITE);
      beta = std::min(prev + delta, VALUE_INFINITE);
    }
    int best;
    for (;;) {
      best = search<Root>(ss, alpha, beta, rootDepth, false);
      std::stable_sort(rootMoves.begin(), rootMoves.end(), [](const RootMove &a, const RootMove &b) { return a.score > b.score; });
      if (stop.load()) break;
      if (best <= alpha) { beta = (alpha + beta) / 2; alpha = std::max(best - delta, -VALUE_INFINITE); }
      else if (best >= beta) beta = std::min(best + delta, VALUE_INFINITE);
      else break;
      delta += delta / 2;
      if (isMain && elapsed_ms() > 3000) report(rootDepth);
    }
    if (stop.load()) break;
    completedDepth = rootDepth;
    if (!isMain) continue;

    report(rootDepth);
    const RootMove &rm = rootMoves[0];
    if (rm.move == lastBest) ++stability; else { stability = 0; lastBest = rm.move; }
    if (g_limits.infinite) { prevIterScore = rm.score; continue; }
    if (std::abs(rm.score) >= MATE_IN_MAX && VALUE_MATE - std::abs(rm.score) <= rootDepth && !g_limits.depth) { stop = true; break; }
    if (g_timed && !g_movetime) {
      double scale = 1.0;
      if (stability == 0 && rootDepth > 6) scale *= 1.25;
      else if (stability >= 4) scale *= 0.8;
      if (rootDepth > 6 && prevIterScore - rm.score > 25) scale *= 1.2;
      if (rootMoves.size() == 1) scale *= 0.2;
      if (elapsed_ms() >= g_optimum * scale * 0.6) { stop = true; break; }
    }
    prevIterScore = rm.score;
  }
}

// ------------------------------------------------------------------ driver
void run_search() {
  std::vector<std::thread> helpers;
  for (int i = 1; i < (int)g_pool.size(); ++i) helpers.emplace_back([i]() { g_pool[i]->iterate(); });
  g_pool[0]->iterate();
  if (g_limits.infinite) while (!stop.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
  stop = true;
  for (auto &t : helpers) t.join();
  SearchThread &m = *g_pool[0];
  if (m.rootMoves.empty()) std::cout << "bestmove 0000" << std::endl;
  else {
    // make sure the final line is reported at least once
    std::cout << "bestmove " << move_to_uci(m.rootMoves[0].move);
    if (m.rootMoves[0].pv.size() > 1) std::cout << " ponder " << move_to_uci(m.rootMoves[0].pv[1]);
    std::cout << std::endl;
  }
}

void ensure_pool() {
  if ((int)g_pool.size() != g_threads) {
    g_pool.clear();
    for (int i = 0; i < g_threads; ++i) g_pool.emplace_back(new SearchThread(i));
  }
}

}  // namespace

// ------------------------------------------------------------------ public API
void init() {
  for (int d = 1; d < 64; ++d)
    for (int m = 1; m < 64; ++m) Reductions[d][m] = int(0.75 + std::log(double(d)) * std::log(double(m)) / 2.2);
  TT.resize(64);
  ensure_pool();
}

void clear() {
  wait();
  TT.clear();
  ensure_pool();
  for (auto &t : g_pool) t->clear();
}

void set_hash(int mb) { wait(); TT.resize(std::max(1, mb)); }
void set_threads(int n) { wait(); g_threads = std::max(1, std::min(n, 256)); ensure_pool(); }
void set_move_overhead(int ms) { g_overhead = std::max(0, ms); }
void stop_search() { stop = true; }
void wait() {
  if (g_driver.joinable()) g_driver.join();
}
U64 nodes_searched() {
  U64 n = 0;
  for (auto &t : g_pool) n += t->nodes.load();
  return n;
}

void go(const Position &pos, const SearchLimits &lim) {
  wait();
  ensure_pool();
  g_limits = lim;
  g_start = std::chrono::steady_clock::now();
  const Color us = pos.side_to_move();
  g_timed = g_movetime = false;
  g_optimum = g_maximum = 0;
  if (lim.movetime > 0) {
    g_timed = g_movetime = true;
    g_maximum = std::max(1, lim.movetime - g_overhead);
    g_optimum = g_maximum;
  } else if (lim.time[us] > 0 && !lim.infinite) {
    g_timed = true;
    double time = lim.time[us], inc = lim.inc[us];
    int mtg = lim.movestogo > 0 ? std::min(lim.movestogo, 50) : 28;
    double avail = std::max(1.0, time - g_overhead);
    g_optimum = std::min(avail / mtg + inc * 0.75, time * 0.4);
    g_optimum = std::max(g_optimum, 2.0);
    g_maximum = std::min(g_optimum * 4.0, avail * 0.8);
    g_maximum = std::max(g_maximum, g_optimum);
    if (lim.movestogo == 1) { g_optimum = avail * 0.6; g_maximum = avail * 0.85; }
  }
  stop = false;
  TT.new_search();
  for (auto &t : g_pool) { t->pos = pos; t->nodes = 0; }
  g_driver = std::thread(run_search);
}

std::string move_to_uci(Move m) {
  if (m == MOVE_NONE) return "0000";
  if (m == MOVE_NULL) return "0000";
  std::string s;
  s += char('a' + file_of(from_sq(m)));
  s += char('1' + rank_of(from_sq(m)));
  s += char('a' + file_of(to_sq(m)));
  s += char('1' + rank_of(to_sq(m)));
  if (is_promo(m)) s += " nbrq"[promo_type(m) - KNIGHT + 1];
  return s;
}

Move parse_move(const Position &pos, const std::string &s) {
  ExtMove list[MAX_MOVES];
  ExtMove *end = generate_legal(pos, list);
  for (ExtMove *m = list; m != end; ++m)
    if (move_to_uci(m->move) == s) return m->move;
  return MOVE_NONE;
}

}  // namespace Search

// ======================================== main.cpp
// main.cpp - UCI front end
#include <chrono>
#include <iostream>
#include <sstream>



static const char *START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char *ENGINE_NAME = "Sirocco 1.0";

static U64 perft(Position &pos, int depth) {
  ExtMove list[MAX_MOVES];
  ExtMove *end = generate_legal(pos, list);
  if (depth == 1) return U64(end - list);
  U64 n = 0;
  for (ExtMove *m = list; m != end; ++m) { pos.do_move(m->move); n += perft(pos, depth - 1); pos.undo_move(m->move); }
  return n;
}

static void do_perft(Position &pos, int depth) {
  ExtMove list[MAX_MOVES];
  ExtMove *end = generate_legal(pos, list);
  U64 total = 0;
  auto t0 = std::chrono::steady_clock::now();
  for (ExtMove *m = list; m != end; ++m) {
    pos.do_move(m->move);
    U64 n = depth > 1 ? perft(pos, depth - 1) : 1;
    pos.undo_move(m->move);
    std::cout << Search::move_to_uci(m->move) << ": " << n << "\n";
    total += n;
  }
  double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
  std::cout << "\nNodes searched: " << total << "  (" << int(ms) << " ms, " << int(total / std::max(1.0, ms) / 1000) << " Mnps)\n";
}

static void print_board(const Position &pos) {
  static const char *pcs = ".PNBRQK..pnbrqk";
  for (int r = 7; r >= 0; --r) {
    std::cout << r + 1 << "  ";
    for (int f = 0; f < 8; ++f) std::cout << pcs[pos.piece_on(make_square(f, r))] << ' ';
    std::cout << "\n";
  }
  std::cout << "\n   a b c d e f g h\nFen: " << pos.fen() << "\nKey: " << std::hex << pos.key() << std::dec << "\n";
}

static void set_position(Position &pos, std::istringstream &is) {
  std::string tok, fen;
  is >> tok;
  if (tok == "startpos") { fen = START_FEN; is >> tok; }
  else if (tok == "fen") {
    while (is >> tok && tok != "moves") fen += tok + " ";
  } else return;
  Position tmp;
  if (!tmp.set(fen)) { std::cout << "info string invalid fen, position ignored" << std::endl; return; }
  pos = tmp;
  if (tok == "moves") {
    std::string mv;
    while (is >> mv) {
      Move m = Search::parse_move(pos, mv);
      if (m == MOVE_NONE) { std::cout << "info string illegal move " << mv << std::endl; break; }
      pos.do_move(m);
    }
  }
}

static void bench(int depth) {
  static const char *fens[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "r1bq1rk1/pp2ppbp/2np1np1/8/3NP3/2N1BP2/PPPQ2PP/R3KB1R w KQ - 0 9",
    "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1",
    "6k1/5pp1/7p/8/8/6P1/5P1P/3R2K1 w - - 0 1",
    "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NN1/PPP1BPPP/R1BQ1RK1 w - - 0 8",
    "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
  };
  Position pos;
  U64 total = 0;
  auto t0 = std::chrono::steady_clock::now();
  std::ostringstream sink;
  for (const char *f : fens) {
    pos.set(f);
    Search::clear();
    SearchLimits lim; lim.depth = depth;
    auto *old = std::cout.rdbuf(sink.rdbuf());
    Search::go(pos, lim);
    Search::wait();
    std::cout.rdbuf(old);
    total += Search::nodes_searched();
  }
  double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
  std::cout << "Nodes searched: " << total << "\nTime: " << int(ms) << " ms\nNPS: " << U64(total * 1000.0 / ms) << std::endl;
}

int main(int argc, char **argv) {
  init_bitboards();
  Eval::init();
  Position::init();
  Search::init();
  Position pos;
  pos.set(START_FEN);

  std::string line, cmd;
  auto handle = [&](const std::string &l) -> bool {
    std::istringstream is(l);
    is >> std::skipws >> cmd;
    if (cmd == "uci") {
      std::cout << "id name " << ENGINE_NAME << "\nid author Claude\n"
                << "option name Hash type spin default 64 min 1 max 65536\n"
                << "option name Threads type spin default 1 min 1 max 256\n"
                << "option name MoveOverhead type spin default 30 min 0 max 5000\n"
                << "option name Clear Hash type button\n"
                << "option name EvalTerms type spin default 255 min 0 max 255\n"
                << "uciok" << std::endl;
    } else if (cmd == "isready") std::cout << "readyok" << std::endl;
    else if (cmd == "ucinewgame") { Search::clear(); }
    else if (cmd == "position") { Search::wait(); set_position(pos, is); }
    else if (cmd == "go") {
      SearchLimits lim;
      std::string t;
      while (is >> t) {
        if (t == "wtime") is >> lim.time[WHITE];
        else if (t == "btime") is >> lim.time[BLACK];
        else if (t == "winc") is >> lim.inc[WHITE];
        else if (t == "binc") is >> lim.inc[BLACK];
        else if (t == "movestogo") is >> lim.movestogo;
        else if (t == "movetime") is >> lim.movetime;
        else if (t == "depth") is >> lim.depth;
        else if (t == "nodes") is >> lim.nodes;
        else if (t == "infinite" || t == "ponder") lim.infinite = true;
      }
      if (!lim.depth && !lim.nodes && !lim.movetime && !lim.time[WHITE] && !lim.time[BLACK]) lim.infinite = true;
      Search::go(pos, lim);
    } else if (cmd == "stop") Search::stop_search();
    else if (cmd == "ponderhit") { /* pondering not offered */ }
    else if (cmd == "setoption") {
      std::string tok, name, value;
      is >> tok;  // "name"
      while (is >> tok && tok != "value") name += (name.empty() ? "" : " ") + tok;
      while (is >> tok) value += (value.empty() ? "" : " ") + tok;
      if (name == "Hash") Search::set_hash(std::atoi(value.c_str()));
      else if (name == "Threads") Search::set_threads(std::atoi(value.c_str()));
      else if (name == "MoveOverhead") Search::set_move_overhead(std::atoi(value.c_str()));
      else if (name == "Clear Hash") Search::clear();
      else if (name == "EvalTerms") Eval::TermMask = std::atoi(value.c_str());
    } else if (cmd == "d") print_board(pos);
    else if (cmd == "eval") Eval::trace(pos);
    else if (cmd == "perft") { int d = 1; is >> d; do_perft(pos, std::max(1, d)); }
    else if (cmd == "bench") { int d = 11; is >> d; bench(d); }
    else if (cmd == "quit") { Search::stop_search(); Search::wait(); return false; }
    return true;
  };

  // allow command-line commands, e.g. ./engine bench
  if (argc > 1) {
    std::string l;
    for (int i = 1; i < argc; ++i) l += std::string(argv[i]) + " ";
    handle(l);
    Search::wait();
    return 0;
  }
  while (std::getline(std::cin, line))
    if (!handle(line)) return 0;
  Search::wait();
  return 0;
}
