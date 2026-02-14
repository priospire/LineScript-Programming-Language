#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

#if defined(_MSC_VER) && !defined(__clang__)
// MSVC does not provide native __int128 in C++; keep source parseable in MSVC mode.
// Clang/clang-cl remains the high-performance path with real 128-bit folding.
#define LS_HOST_NO_INT128 1
#define __int128 int64_t
#else
#define LS_HOST_NO_INT128 0
#endif

namespace ls {

struct Span {
  std::size_t line = 1;
  std::size_t col = 1;
};

class CompileError final : public std::runtime_error {
public:
  CompileError(const Span &s, const std::string &m) : std::runtime_error(format(s, m)) {}

private:
  static std::string format(const Span &s, const std::string &m) {
    std::ostringstream o;
    o << "line " << s.line << ", col " << s.col << ": " << m;
    return o.str();
  }
};

enum class TokenKind {
  End,
  Newline,
  Id,
  Str,
  Int,
  Float,
  KwFn,
  KwInline,
  KwExtern,
  KwLet,
  KwVar,
  KwConst,
  KwDeclare,
  KwOwned,
  KwReturn,
  KwIf,
  KwUnless,
  KwElif,
  KwElse,
  KwWhile,
  KwFor,
  KwParallel,
  KwMacro,
  KwClass,
  KwIn,
  KwStep,
  KwDo,
  KwEnd,
  KwThrows,
  KwBreak,
  KwContinue,
  KwTrue,
  KwFalse,
  Arrow,
  Dot,
  DotDot,
  Pow,
  PlusPlus,
  MinusMinus,
  Colon,
  Semi,
  Comma,
  LPar,
  RPar,
  LBra,
  RBra,
  Plus,
  Minus,
  Star,
  Slash,
  Percent,
  PlusAssign,
  MinusAssign,
  StarAssign,
  SlashAssign,
  PercentAssign,
  PowAssign,
  Assign,
  Eq,
  Neq,
  AndAnd,
  OrOr,
  Bang,
  Lt,
  Lte,
  Gt,
  Gte,
};

static const char *tokenKindName(TokenKind k) {
  switch (k) {
  case TokenKind::End: return "End";
  case TokenKind::Newline: return "Newline";
  case TokenKind::Id: return "Id";
  case TokenKind::Str: return "Str";
  case TokenKind::Int: return "Int";
  case TokenKind::Float: return "Float";
  case TokenKind::KwFn: return "KwFn";
  case TokenKind::KwInline: return "KwInline";
  case TokenKind::KwExtern: return "KwExtern";
  case TokenKind::KwLet: return "KwLet";
  case TokenKind::KwVar: return "KwVar";
  case TokenKind::KwConst: return "KwConst";
  case TokenKind::KwDeclare: return "KwDeclare";
  case TokenKind::KwOwned: return "KwOwned";
  case TokenKind::KwReturn: return "KwReturn";
  case TokenKind::KwIf: return "KwIf";
  case TokenKind::KwUnless: return "KwUnless";
  case TokenKind::KwElif: return "KwElif";
  case TokenKind::KwElse: return "KwElse";
  case TokenKind::KwWhile: return "KwWhile";
  case TokenKind::KwFor: return "KwFor";
  case TokenKind::KwParallel: return "KwParallel";
  case TokenKind::KwMacro: return "KwMacro";
  case TokenKind::KwClass: return "KwClass";
  case TokenKind::KwIn: return "KwIn";
  case TokenKind::KwStep: return "KwStep";
  case TokenKind::KwDo: return "KwDo";
  case TokenKind::KwEnd: return "KwEnd";
  case TokenKind::KwThrows: return "KwThrows";
  case TokenKind::KwBreak: return "KwBreak";
  case TokenKind::KwContinue: return "KwContinue";
  case TokenKind::KwTrue: return "KwTrue";
  case TokenKind::KwFalse: return "KwFalse";
  case TokenKind::Arrow: return "Arrow";
  case TokenKind::Dot: return "Dot";
  case TokenKind::DotDot: return "DotDot";
  case TokenKind::Pow: return "Pow";
  case TokenKind::PlusPlus: return "PlusPlus";
  case TokenKind::MinusMinus: return "MinusMinus";
  case TokenKind::Colon: return "Colon";
  case TokenKind::Semi: return "Semi";
  case TokenKind::Comma: return "Comma";
  case TokenKind::LPar: return "LPar";
  case TokenKind::RPar: return "RPar";
  case TokenKind::LBra: return "LBra";
  case TokenKind::RBra: return "RBra";
  case TokenKind::Plus: return "Plus";
  case TokenKind::Minus: return "Minus";
  case TokenKind::Star: return "Star";
  case TokenKind::Slash: return "Slash";
  case TokenKind::Percent: return "Percent";
  case TokenKind::PlusAssign: return "PlusAssign";
  case TokenKind::MinusAssign: return "MinusAssign";
  case TokenKind::StarAssign: return "StarAssign";
  case TokenKind::SlashAssign: return "SlashAssign";
  case TokenKind::PercentAssign: return "PercentAssign";
  case TokenKind::PowAssign: return "PowAssign";
  case TokenKind::Assign: return "Assign";
  case TokenKind::Eq: return "Eq";
  case TokenKind::Neq: return "Neq";
  case TokenKind::AndAnd: return "AndAnd";
  case TokenKind::OrOr: return "OrOr";
  case TokenKind::Bang: return "Bang";
  case TokenKind::Lt: return "Lt";
  case TokenKind::Lte: return "Lte";
  case TokenKind::Gt: return "Gt";
  case TokenKind::Gte: return "Gte";
  }
  return "Unknown";
}

struct Token {
  TokenKind kind = TokenKind::End;
  std::string text;
  Span span;
};

class Lexer {
public:
  explicit Lexer(std::string src) : src_(std::move(src)) {}

  std::vector<Token> run() {
    std::vector<Token> out;
    while (true) {
      skipTrivia();
      if (eof()) {
        out.push_back({TokenKind::End, "", {line_, col_}});
        break;
      }
      Span s{line_, col_};
      char c = peek();
      if (c == '\n') {
        adv();
        out.push_back({TokenKind::Newline, "\\n", s});
        continue;
      }
      if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        out.push_back(readIdent());
        continue;
      }
      if (c == '"') {
        out.push_back(readString());
        continue;
      }
      if (std::isdigit(static_cast<unsigned char>(c))) {
        out.push_back(readNumber());
        continue;
      }
      if (c == '-' && peek(1) == '>') {
        adv();
        adv();
        out.push_back({TokenKind::Arrow, "->", s});
        continue;
      }
      if (c == '=' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::Eq, "==", s});
        continue;
      }
      if (c == '&' && peek(1) == '&') {
        adv();
        adv();
        out.push_back({TokenKind::AndAnd, "&&", s});
        continue;
      }
      if (c == '|' && peek(1) == '|') {
        adv();
        adv();
        out.push_back({TokenKind::OrOr, "||", s});
        continue;
      }
      if (c == '!' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::Neq, "!=", s});
        continue;
      }
      if (c == '<' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::Lte, "<=", s});
        continue;
      }
      if (c == '>' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::Gte, ">=", s});
        continue;
      }
      if (c == '.' && peek(1) == '.') {
        adv();
        adv();
        out.push_back({TokenKind::DotDot, "..", s});
        continue;
      }
      if (c == '*' && peek(1) == '*') {
        if (peek(2) == '=') {
          adv();
          adv();
          adv();
          out.push_back({TokenKind::PowAssign, "**=", s});
          continue;
        }
        adv();
        adv();
        out.push_back({TokenKind::Pow, "**", s});
        continue;
      }
      if (c == '+' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::PlusAssign, "+=", s});
        continue;
      }
      if (c == '+' && peek(1) == '+') {
        adv();
        adv();
        out.push_back({TokenKind::PlusPlus, "++", s});
        continue;
      }
      if (c == '-' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::MinusAssign, "-=", s});
        continue;
      }
      if (c == '-' && peek(1) == '-') {
        adv();
        adv();
        out.push_back({TokenKind::MinusMinus, "--", s});
        continue;
      }
      if (c == '*' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::StarAssign, "*=", s});
        continue;
      }
      if (c == '/' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::SlashAssign, "/=", s});
        continue;
      }
      if (c == '%' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::PercentAssign, "%=", s});
        continue;
      }
      if (c == '^' && peek(1) == '=') {
        adv();
        adv();
        out.push_back({TokenKind::PowAssign, "^=", s});
        continue;
      }
      adv();
      switch (c) {
      case ':': out.push_back({TokenKind::Colon, ":", s}); break;
      case ';': out.push_back({TokenKind::Semi, ";", s}); break;
      case ',': out.push_back({TokenKind::Comma, ",", s}); break;
      case '.': out.push_back({TokenKind::Dot, ".", s}); break;
      case '(': out.push_back({TokenKind::LPar, "(", s}); break;
      case ')': out.push_back({TokenKind::RPar, ")", s}); break;
      case '{': out.push_back({TokenKind::LBra, "{", s}); break;
      case '}': out.push_back({TokenKind::RBra, "}", s}); break;
      case '[': out.push_back({TokenKind::LBra, "[", s}); break;
      case ']': out.push_back({TokenKind::RBra, "]", s}); break;
      case '+': out.push_back({TokenKind::Plus, "+", s}); break;
      case '-': out.push_back({TokenKind::Minus, "-", s}); break;
      case '*': out.push_back({TokenKind::Star, "*", s}); break;
      case '^': out.push_back({TokenKind::Pow, "^", s}); break;
      case '/': out.push_back({TokenKind::Slash, "/", s}); break;
      case '%': out.push_back({TokenKind::Percent, "%", s}); break;
      case '=': out.push_back({TokenKind::Assign, "=", s}); break;
      case '!': out.push_back({TokenKind::Bang, "!", s}); break;
      case '<': out.push_back({TokenKind::Lt, "<", s}); break;
      case '>': out.push_back({TokenKind::Gt, ">", s}); break;
      default: throw CompileError(s, std::string("unexpected char '") + c + "'");
      }
    }
    return out;
  }

private:
  std::string src_;
  std::size_t pos_ = 0, line_ = 1, col_ = 1;

  bool eof() const { return pos_ >= src_.size(); }
  char peek(std::size_t off = 0) const { return pos_ + off < src_.size() ? src_[pos_ + off] : '\0'; }
  char adv() {
    if (eof()) return '\0';
    char c = src_[pos_++];
    if (c == '\n') {
      ++line_;
      col_ = 1;
    } else {
      ++col_;
    }
    return c;
  }

  void skipTrivia() {
    while (!eof()) {
      char c = peek();
      if (c == ' ' || c == '\t' || c == '\r') {
        adv();
        continue;
      }
      if (c == '/' && peek(1) == '/') {
        adv();
        adv();
        while (!eof() && peek() != '\n') adv();
        continue;
      }
      break;
    }
  }

  Token readIdent() {
    Span s{line_, col_};
    std::string t;
    while (!eof()) {
      char c = peek();
      if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
        t.push_back(adv());
      else
        break;
    }
    if (t == "fn" || t == "func") return {TokenKind::KwFn, t, s};
    if (t == "inline") return {TokenKind::KwInline, t, s};
    if (t == "extern") return {TokenKind::KwExtern, t, s};
    if (t == "let") return {TokenKind::KwLet, t, s};
    if (t == "var") return {TokenKind::KwVar, t, s};
    if (t == "const") return {TokenKind::KwConst, t, s};
    if (t == "declare") return {TokenKind::KwDeclare, t, s};
    if (t == "owned") return {TokenKind::KwOwned, t, s};
    if (t == "return") return {TokenKind::KwReturn, t, s};
    if (t == "if") return {TokenKind::KwIf, t, s};
    if (t == "unless") return {TokenKind::KwUnless, t, s};
    if (t == "elif") return {TokenKind::KwElif, t, s};
    if (t == "else") return {TokenKind::KwElse, t, s};
    if (t == "while") return {TokenKind::KwWhile, t, s};
    if (t == "for") return {TokenKind::KwFor, t, s};
    if (t == "parallel") return {TokenKind::KwParallel, t, s};
    if (t == "macro") return {TokenKind::KwMacro, t, s};
    if (t == "class") return {TokenKind::KwClass, t, s};
    if (t == "in") return {TokenKind::KwIn, t, s};
    if (t == "step") return {TokenKind::KwStep, t, s};
    if (t == "do") return {TokenKind::KwDo, t, s};
    if (t == "end") return {TokenKind::KwEnd, t, s};
    if (t == "throws") return {TokenKind::KwThrows, t, s};
    if (t == "break") return {TokenKind::KwBreak, t, s};
    if (t == "continue") return {TokenKind::KwContinue, t, s};
    if (t == "and") return {TokenKind::AndAnd, t, s};
    if (t == "or") return {TokenKind::OrOr, t, s};
    if (t == "not") return {TokenKind::Bang, t, s};
    if (t == "true") return {TokenKind::KwTrue, t, s};
    if (t == "false") return {TokenKind::KwFalse, t, s};
    return {TokenKind::Id, t, s};
  }

  Token readNumber() {
    Span s{line_, col_};
    std::string t;
    bool dot = false;
    while (!eof()) {
      char c = peek();
      if (std::isdigit(static_cast<unsigned char>(c))) {
        t.push_back(adv());
      } else if (c == '.' && !dot && std::isdigit(static_cast<unsigned char>(peek(1)))) {
        dot = true;
        t.push_back(adv());
      } else {
        break;
      }
    }
    return {dot ? TokenKind::Float : TokenKind::Int, t, s};
  }

  Token readString() {
    Span s{line_, col_};
    adv(); // opening quote
    std::string out;
    while (!eof()) {
      char c = adv();
      if (c == '"') return {TokenKind::Str, out, s};
      if (c == '\\') {
        if (eof()) throw CompileError(s, "unterminated escape sequence in string literal");
        const char esc = adv();
        switch (esc) {
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        case '\\': out.push_back('\\'); break;
        case '"': out.push_back('"'); break;
        default: throw CompileError(s, std::string("unsupported escape sequence '\\") + esc + "'");
        }
        continue;
      }
      if (c == '\n') throw CompileError(s, "unterminated string literal");
      out.push_back(c);
    }
    throw CompileError(s, "unterminated string literal");
  }
};

enum class Type { I32, I64, F32, F64, Bool, Str, Void };
static bool isInt(Type t) { return t == Type::I32 || t == Type::I64; }
static bool isFloat(Type t) { return t == Type::F32 || t == Type::F64; }
static bool isNum(Type t) { return isInt(t) || isFloat(t); }
static std::string typeName(Type t) {
  switch (t) {
  case Type::I32: return "i32";
  case Type::I64: return "i64";
  case Type::F32: return "f32";
  case Type::F64: return "f64";
  case Type::Bool: return "bool";
  case Type::Str: return "str";
  case Type::Void: return "void";
  }
  return "<?>"; // unreachable
}

enum class EK { Int, Float, Bool, Str, Var, Unary, Binary, Call };
enum class UK { Neg, Not };
enum class BK { Add, Sub, Mul, Div, Mod, Pow, Eq, Neq, Lt, Lte, Gt, Gte, And, Or };

struct Expr {
  EK k;
  Span s;
  Type inf = Type::Void;
  bool typed = false;
  Expr(EK kIn, Span sIn) : k(kIn), s(sIn) {}
  virtual ~Expr() = default;
};
using EP = std::unique_ptr<Expr>;

struct EInt : Expr {
  int64_t v;
  EInt(int64_t vIn, Span s) : Expr(EK::Int, s), v(vIn) {}
};
struct EFloat : Expr {
  double v;
  EFloat(double vIn, Span s) : Expr(EK::Float, s), v(vIn) {}
};
struct EBool : Expr {
  bool v;
  EBool(bool vIn, Span s) : Expr(EK::Bool, s), v(vIn) {}
};
struct EString : Expr {
  std::string v;
  EString(std::string vIn, Span s) : Expr(EK::Str, s), v(std::move(vIn)) {}
};
struct EVar : Expr {
  std::string n;
  EVar(std::string nIn, Span s) : Expr(EK::Var, s), n(std::move(nIn)) {}
};
struct EUnary : Expr {
  UK op;
  EP x;
  std::string overrideFn;
  EUnary(UK opIn, EP xIn, Span s, std::string overrideFnIn = {})
      : Expr(EK::Unary, s), op(opIn), x(std::move(xIn)), overrideFn(std::move(overrideFnIn)) {}
};
struct EBinary : Expr {
  BK op;
  EP l, r;
  std::string overrideFn;
  EBinary(BK opIn, EP lIn, EP rIn, Span s, std::string overrideFnIn = {})
      : Expr(EK::Binary, s), op(opIn), l(std::move(lIn)), r(std::move(rIn)), overrideFn(std::move(overrideFnIn)) {}
};
struct ECall : Expr {
  std::string f;
  std::vector<EP> a;
  ECall(std::string fIn, std::vector<EP> aIn, Span s) : Expr(EK::Call, s), f(std::move(fIn)), a(std::move(aIn)) {}
};

enum class SK { Let, Assign, Expr, Ret, If, While, For, FormatBlock, Break, Continue };
struct Stmt {
  SK k;
  Span s;
  Stmt(SK kIn, Span sIn) : k(kIn), s(sIn) {}
  virtual ~Stmt() = default;
};
using SP = std::unique_ptr<Stmt>;

struct SLet : Stmt {
  std::string n;
  std::optional<Type> decl;
  bool isConst = false;
  bool isOwned = false;
  std::string ownedFreeFn;
  Type inf = Type::I64;
  bool typed = false;
  EP v;
  SLet(std::string nIn, std::optional<Type> d, bool isConstIn, bool isOwnedIn, EP vIn, Span s)
      : Stmt(SK::Let, s), n(std::move(nIn)), decl(d), isConst(isConstIn), isOwned(isOwnedIn), v(std::move(vIn)) {}
};
struct SAssign : Stmt {
  std::string n;
  EP v;
  SAssign(std::string nIn, EP vIn, Span s) : Stmt(SK::Assign, s), n(std::move(nIn)), v(std::move(vIn)) {}
};
struct SExpr : Stmt {
  EP e;
  SExpr(EP eIn, Span s) : Stmt(SK::Expr, s), e(std::move(eIn)) {}
};
struct SRet : Stmt {
  bool has = false;
  EP v;
  SRet(bool h, EP vIn, Span s) : Stmt(SK::Ret, s), has(h), v(std::move(vIn)) {}
};
struct SIf : Stmt {
  EP c;
  std::vector<SP> t, e;
  SIf(EP cIn, std::vector<SP> tIn, std::vector<SP> eIn, Span s) : Stmt(SK::If, s), c(std::move(cIn)), t(std::move(tIn)), e(std::move(eIn)) {}
};
struct SWhile : Stmt {
  EP c;
  std::vector<SP> b;
  SWhile(EP cIn, std::vector<SP> bIn, Span s) : Stmt(SK::While, s), c(std::move(cIn)), b(std::move(bIn)) {}
};
struct SFor : Stmt {
  std::string n;
  EP start;
  EP stop;
  EP step;
  bool parallel = false;
  std::vector<SP> b;
  SFor(std::string nIn, EP startIn, EP stopIn, EP stepIn, bool parallelIn, std::vector<SP> bIn, Span s)
      : Stmt(SK::For, s), n(std::move(nIn)), start(std::move(startIn)), stop(std::move(stopIn)),
        step(std::move(stepIn)), parallel(parallelIn), b(std::move(bIn)) {}
};
struct SFormatBlock : Stmt {
  EP endArg;
  std::vector<SP> b;
  SFormatBlock(EP endArgIn, std::vector<SP> bIn, Span s)
      : Stmt(SK::FormatBlock, s), endArg(std::move(endArgIn)), b(std::move(bIn)) {}
};
struct SBreak : Stmt {
  explicit SBreak(Span s) : Stmt(SK::Break, s) {}
};
struct SContinue : Stmt {
  explicit SContinue(Span s) : Stmt(SK::Continue, s) {}
};

struct Param {
  std::string n;
  Type t = Type::I64;
};
struct Fn {
  std::string n;
  std::string sourceName;
  bool isCliFlag = false;
  std::string cliFlagName;
  bool isOperatorOverride = false;
  std::optional<BK> operatorKind;
  std::optional<UK> unaryOperatorKind;
  bool isClassMethod = false;
  std::string classOwner;
  bool methodStatic = false;
  bool methodVirtual = false;
  bool methodOverride = false;
  bool methodFinal = false;
  bool ex = false;
  bool inl = false;
  std::vector<Param> p;
  Type ret = Type::Void;
  std::vector<std::string> throws;
  std::vector<SP> b;
  Span s;
};
struct Program {
  std::vector<Fn> f;
  std::vector<SP> top;
};
static constexpr const char *kScriptEntryName = "__linescript_script_main";

enum class AccessModifier { Public, Protected, Private };

struct ParsedClassField {
  std::string n;
  Type t = Type::I64;
  AccessModifier access = AccessModifier::Public;
  EP init;
  Span s;
};

struct FieldInfo {
  Type t = Type::I64;
  AccessModifier access = AccessModifier::Public;
  std::string owner;
};

struct MethodInfo {
  std::string symbol;
  std::string owner;
  AccessModifier access = AccessModifier::Public;
  bool isStatic = false;
  bool isVirtual = false;
  bool isOverride = false;
  bool isFinal = false;
  std::vector<Type> params;
  Type ret = Type::Void;
};

struct ClassInfo {
  std::string name;
  Span s;
  std::string base;
  std::unordered_map<std::string, FieldInfo> fields;
  std::unordered_map<std::string, std::vector<MethodInfo>> methods;
};

static bool startsWith(const std::string &s, const char *prefix) {
  return s.rfind(prefix, 0) == 0;
}

static bool isOverloadableBinaryOp(BK op) {
  switch (op) {
  case BK::Add:
  case BK::Sub:
  case BK::Mul:
  case BK::Div:
  case BK::Mod:
  case BK::Pow:
  case BK::Eq:
  case BK::Neq:
  case BK::Lt:
  case BK::Lte:
  case BK::Gt:
  case BK::Gte:
  case BK::And:
  case BK::Or:
    return true;
  }
  return false;
}

static std::string operatorOverrideSymbol(BK op) {
  switch (op) {
  case BK::Add: return "__ls_op_add";
  case BK::Sub: return "__ls_op_sub";
  case BK::Mul: return "__ls_op_mul";
  case BK::Div: return "__ls_op_div";
  case BK::Mod: return "__ls_op_mod";
  case BK::Pow: return "__ls_op_pow";
  case BK::Eq: return "__ls_op_eq";
  case BK::Neq: return "__ls_op_neq";
  case BK::Lt: return "__ls_op_lt";
  case BK::Lte: return "__ls_op_lte";
  case BK::Gt: return "__ls_op_gt";
  case BK::Gte: return "__ls_op_gte";
  case BK::And: return "__ls_op_and";
  case BK::Or: return "__ls_op_or";
  }
  return "";
}

static bool isOverloadableUnaryOp(UK op) {
  switch (op) {
  case UK::Neg:
  case UK::Not:
    return true;
  }
  return false;
}

static std::string unaryOperatorOverrideSymbol(UK op) {
  switch (op) {
  case UK::Neg: return "__ls_uop_neg";
  case UK::Not: return "__ls_uop_not";
  }
  return "";
}

static std::string unaryOperatorSymbolText(UK op) {
  switch (op) {
  case UK::Neg: return "-";
  case UK::Not: return "!";
  }
  return "?";
}

static std::string operatorSymbolText(BK op) {
  switch (op) {
  case BK::Add: return "+";
  case BK::Sub: return "-";
  case BK::Mul: return "*";
  case BK::Div: return "/";
  case BK::Mod: return "%";
  case BK::Pow: return "^";
  case BK::Eq: return "==";
  case BK::Neq: return "!=";
  case BK::Lt: return "<";
  case BK::Lte: return "<=";
  case BK::Gt: return ">";
  case BK::Gte: return ">=";
  case BK::And: return "&&";
  case BK::Or: return "||";
  }
  return "?";
}

static bool isSuperuserNamespaceSymbol(const std::string &name) {
  return name == "su" || name == "superuser" || startsWith(name, "su.") || startsWith(name, "superuser.");
}

static std::string canonicalSuperuserNamespaceSymbol(const std::string &name) {
  if (name == "superuser") return "su";
  if (startsWith(name, "superuser.")) return std::string("su.") + name.substr(10);
  return name;
}

static std::string canonicalSuperuserCallName(const std::string &name) {
  if (startsWith(name, "superuser.")) return std::string("su.") + name.substr(10);
  return name;
}

class Parser {
public:
  explicit Parser(std::vector<Token> t) : t_(std::move(t)) {}
  Program run() {
    Program p;
    skipNl();
    while (!is(TokenKind::End)) {
      if (eat(TokenKind::KwMacro)) {
        parseMacroDecl();
      } else if (eat(TokenKind::KwClass)) {
        parseClass(p, t_[i_ - 1].span);
      } else if (startsFunctionDecl()) {
        p.f.push_back(fn());
      } else {
        p.top.push_back(stmt());
      }
      skipNl();
    }
    return p;
  }

private:
  enum class MacroArgKind { Expr, Stmt, Item };
  struct MacroParam {
    std::string name;
    MacroArgKind kind = MacroArgKind::Expr;
  };
  struct MacroDecl {
    std::string name;
    std::vector<MacroParam> params;
    MacroArgKind retKind = MacroArgKind::Expr;
    EP bodyExpr;
    Span s;
  };
  struct OverloadDecl {
    std::vector<Type> params;
    std::string symbol;
  };

  std::vector<Token> t_;
  std::size_t i_ = 0;
  std::unordered_map<std::string, ClassInfo> classes_;
  std::unordered_map<std::string, std::string> varClass_;
  std::unordered_map<std::string, std::string> varFreeFn_;
  std::unordered_set<std::string> cliFlagNames_;
  std::unordered_map<std::string, std::vector<OverloadDecl>> topFnOverloads_;
  std::unordered_map<std::string, MacroDecl> macros_;
  std::size_t topFnOverloadId_ = 0;
  std::string currentClass_;

  const Token &cur() const { return t_[i_]; }
  const Token &look(std::size_t off) const { return i_ + off < t_.size() ? t_[i_ + off] : t_.back(); }
  const Token &lookNonNl(std::size_t off) const {
    std::size_t j = i_;
    while (j < t_.size() && t_[j].kind == TokenKind::Newline) ++j;
    if (off == 0) return j < t_.size() ? t_[j] : t_.back();
    while (off > 0) {
      if (j + 1 >= t_.size()) return t_.back();
      ++j;
      while (j < t_.size() && t_[j].kind == TokenKind::Newline) ++j;
      --off;
    }
    return j < t_.size() ? t_[j] : t_.back();
  }
  bool is(TokenKind k) const { return cur().kind == k; }
  bool eat(TokenKind k) {
    if (!is(k)) return false;
    ++i_;
    return true;
  }
  void skipNl() {
    while (eat(TokenKind::Newline)) {
    }
  }
  std::size_t skipNlFrom(std::size_t idx) const {
    while (idx < t_.size() && t_[idx].kind == TokenKind::Newline) ++idx;
    return idx;
  }
  static bool isValidCliFlagName(const std::string &name) {
    if (name.empty()) return false;
    if (name.front() == '-' || name.back() == '-') return false;
    bool prevDash = false;
    for (char c : name) {
      const unsigned char u = static_cast<unsigned char>(c);
      if (c == '-') {
        if (prevDash) return false;
        prevDash = true;
        continue;
      }
      if (!std::isalnum(u) && c != '_') return false;
      prevDash = false;
    }
    return true;
  }
  static std::string cliFlagSymbol(const std::string &name) {
    std::string out = "__ls_flag_";
    out.reserve(out.size() + name.size());
    for (char c : name) out.push_back(c == '-' ? '_' : c);
    return out;
  }
  static bool isOverloadableOperatorTokenKind(TokenKind k) {
    switch (k) {
    case TokenKind::Plus:
    case TokenKind::Minus:
    case TokenKind::Star:
    case TokenKind::Slash:
    case TokenKind::Percent:
    case TokenKind::Pow:
    case TokenKind::Eq:
    case TokenKind::Neq:
    case TokenKind::Lt:
    case TokenKind::Lte:
    case TokenKind::Gt:
    case TokenKind::Gte:
    case TokenKind::AndAnd:
    case TokenKind::OrOr:
      return true;
    default:
      return false;
    }
  }
  static bool isOverloadableUnaryOperatorTokenKind(TokenKind k) {
    switch (k) {
    case TokenKind::Minus:
    case TokenKind::Bang:
      return true;
    default:
      return false;
    }
  }
  static BK overloadableOperatorTokenToBK(TokenKind k, const Span &s) {
    switch (k) {
    case TokenKind::Plus: return BK::Add;
    case TokenKind::Minus: return BK::Sub;
    case TokenKind::Star: return BK::Mul;
    case TokenKind::Slash: return BK::Div;
    case TokenKind::Percent: return BK::Mod;
    case TokenKind::Pow: return BK::Pow;
    case TokenKind::Eq: return BK::Eq;
    case TokenKind::Neq: return BK::Neq;
    case TokenKind::Lt: return BK::Lt;
    case TokenKind::Lte: return BK::Lte;
    case TokenKind::Gt: return BK::Gt;
    case TokenKind::Gte: return BK::Gte;
    case TokenKind::AndAnd: return BK::And;
    case TokenKind::OrOr: return BK::Or;
    default: break;
    }
    throw CompileError(s, "unsupported operator override token");
  }
  static UK overloadableUnaryOperatorTokenToUK(TokenKind k, const Span &s) {
    switch (k) {
    case TokenKind::Minus: return UK::Neg;
    case TokenKind::Bang: return UK::Not;
    default: break;
    }
    throw CompileError(s, "unsupported unary operator override token");
  }
  bool startsFunctionDecl() const {
    std::size_t j = skipNlFrom(i_);
    bool sawModifier = false;
    while (j < t_.size() && (t_[j].kind == TokenKind::KwInline || t_[j].kind == TokenKind::KwExtern)) {
      sawModifier = true;
      ++j;
      j = skipNlFrom(j);
    }
    bool hasKwFn = false;
    if (j < t_.size() && t_[j].kind == TokenKind::KwFn) {
      hasKwFn = true;
      ++j;
      j = skipNlFrom(j);
    }
    if (j < t_.size() && t_[j].kind == TokenKind::Id && t_[j].text == "flag") {
      ++j;
      j = skipNlFrom(j);
      if (j >= t_.size() || t_[j].kind != TokenKind::Id) return false;
      ++j;
      j = skipNlFrom(j);
      while (j < t_.size() && t_[j].kind == TokenKind::Minus) {
        ++j;
        j = skipNlFrom(j);
        if (j >= t_.size() || t_[j].kind != TokenKind::Id) return false;
        ++j;
        j = skipNlFrom(j);
      }
      return j < t_.size() && t_[j].kind == TokenKind::LPar;
    }
    if (j >= t_.size() || t_[j].kind != TokenKind::Id) return false;
    if (t_[j].text == "operator") {
      ++j;
      j = skipNlFrom(j);
      if (j < t_.size() && t_[j].kind == TokenKind::Id && t_[j].text == "unary") {
        ++j;
        j = skipNlFrom(j);
      }
      if (j >= t_.size() ||
          !(isOverloadableOperatorTokenKind(t_[j].kind) || isOverloadableUnaryOperatorTokenKind(t_[j].kind)))
        return false;
      ++j;
      j = skipNlFrom(j);
      return j < t_.size() && t_[j].kind == TokenKind::LPar;
    }
    ++j;
    j = skipNlFrom(j);
    if (j >= t_.size() || t_[j].kind != TokenKind::LPar) return false;

    bool sawColon = false;
    enum class ParamState { ExpectNameOrEnd, AfterName, ExpectTypeName };
    ParamState st = ParamState::ExpectNameOrEnd;
    ++j; // move to first token inside '(' ... ')'
    for (;; ++j) {
      if (j >= t_.size()) return false;
      const TokenKind k = t_[j].kind;
      if (k == TokenKind::Newline) continue;

      if (st == ParamState::ExpectNameOrEnd) {
        if (k == TokenKind::RPar) {
          ++j;
          break;
        }
        if (k == TokenKind::Id) {
          st = ParamState::AfterName;
          continue;
        }
        return false;
      }

      if (st == ParamState::AfterName) {
        if (k == TokenKind::Colon) {
          sawColon = true;
          st = ParamState::ExpectTypeName;
          continue;
        }
        if (k == TokenKind::Comma) {
          st = ParamState::ExpectNameOrEnd;
          continue;
        }
        if (k == TokenKind::RPar) {
          ++j;
          break;
        }
        return false;
      }

      if (st == ParamState::ExpectTypeName) {
        if (k == TokenKind::Id) {
          st = ParamState::AfterName;
          continue;
        }
        return false;
      }
    }
    j = skipNlFrom(j);
    if (j >= t_.size()) return false;
    const TokenKind next = t_[j].kind;
    if (next == TokenKind::Arrow || next == TokenKind::KwThrows || next == TokenKind::KwDo || next == TokenKind::LBra)
      return true;
    if (next == TokenKind::Semi && sawModifier) return true;
    return hasKwFn || sawModifier || sawColon;
  }
  Token need(TokenKind k, const std::string &msg) {
    if (!is(k)) throw CompileError(cur().span, msg);
    return t_[i_++];
  }
  bool isStmtBoundary(TokenKind k) const {
    return k == TokenKind::End || k == TokenKind::RBra || k == TokenKind::KwEnd || k == TokenKind::KwElse ||
           k == TokenKind::KwElif;
  }
  void needStmtEnd(const std::string &msg) {
    if (eat(TokenKind::Semi)) {
      while (eat(TokenKind::Semi) || eat(TokenKind::Newline)) {
      }
      return;
    }
    if (eat(TokenKind::Newline)) {
      while (eat(TokenKind::Newline)) {
      }
      return;
    }
    if (isStmtBoundary(cur().kind)) return;
    throw CompileError(cur().span, msg);
  }
  MacroArgKind parseMacroArgKindToken(const Token &tok) {
    if (tok.text == "expr") return MacroArgKind::Expr;
    if (tok.text == "stmt") return MacroArgKind::Stmt;
    if (tok.text == "item") return MacroArgKind::Item;
    throw CompileError(tok.span, "unknown macro kind '" + tok.text + "' (expected expr|stmt|item)");
  }
  void parseMacroDecl() {
    Token nameTok = need(TokenKind::Id, "expected macro name");
    if (macros_.count(nameTok.text)) {
      throw CompileError(nameTok.span, "duplicate macro '" + nameTok.text + "'");
    }
    std::vector<MacroParam> params;
    need(TokenKind::LPar, "expected '(' after macro name");
    if (!is(TokenKind::RPar)) {
      while (true) {
        MacroParam p;
        p.name = need(TokenKind::Id, "expected macro parameter name").text;
        need(TokenKind::Colon, "expected ':' in macro parameter");
        Token kindTok = need(TokenKind::Id, "expected macro parameter kind");
        p.kind = parseMacroArgKindToken(kindTok);
        params.push_back(std::move(p));
        if (!eat(TokenKind::Comma)) break;
      }
    }
    need(TokenKind::RPar, "expected ')' after macro parameters");
    need(TokenKind::Arrow, "expected '->' in macro declaration");
    Token retKindTok = need(TokenKind::Id, "expected macro return kind");
    MacroArgKind retKind = parseMacroArgKindToken(retKindTok);
    skipNl();
    bool braceStyle = false, endStyle = false;
    if (eat(TokenKind::LBra)) braceStyle = true;
    else if (eat(TokenKind::KwDo)) endStyle = true;
    else throw CompileError(cur().span, "expected '{' or 'do' after macro signature");
    if (retKind != MacroArgKind::Expr) {
      throw CompileError(retKindTok.span,
                         "macro return kinds stmt/item are not implemented yet (use -> expr for now)");
    }
    skipNl();
    EP bodyExpr = expr();
    needStmtEnd("expected statement terminator after macro body expression");
    skipNl();
    if (braceStyle) need(TokenKind::RBra, "expected '}' to close macro");
    else if (endStyle) need(TokenKind::KwEnd, "expected 'end' to close macro");
    MacroDecl decl;
    decl.name = nameTok.text;
    decl.params = std::move(params);
    decl.retKind = retKind;
    decl.bodyExpr = std::move(bodyExpr);
    decl.s = nameTok.span;
    macros_[decl.name] = std::move(decl);
  }
  EP defaultValueFor(Type t, const Span &s) {
    switch (t) {
    case Type::I32: return std::make_unique<EInt>(0, s);
    case Type::I64: return std::make_unique<EInt>(0, s);
    case Type::F32: return std::make_unique<EFloat>(0.0, s);
    case Type::F64: return std::make_unique<EFloat>(0.0, s);
    case Type::Bool: return std::make_unique<EBool>(false, s);
    case Type::Str: return std::make_unique<EString>("", s);
    case Type::Void: break;
    }
    throw CompileError(s, "cannot declare variable with type void");
  }

  static EP cloneExpr(const Expr &e) {
    switch (e.k) {
    case EK::Int: return std::make_unique<EInt>(static_cast<const EInt &>(e).v, e.s);
    case EK::Float: return std::make_unique<EFloat>(static_cast<const EFloat &>(e).v, e.s);
    case EK::Bool: return std::make_unique<EBool>(static_cast<const EBool &>(e).v, e.s);
    case EK::Str: return std::make_unique<EString>(static_cast<const EString &>(e).v, e.s);
    case EK::Var: return std::make_unique<EVar>(static_cast<const EVar &>(e).n, e.s);
    case EK::Unary: {
      const auto &n = static_cast<const EUnary &>(e);
      return std::make_unique<EUnary>(n.op, cloneExpr(*n.x), e.s, n.overrideFn);
    }
    case EK::Binary: {
      const auto &n = static_cast<const EBinary &>(e);
      return std::make_unique<EBinary>(n.op, cloneExpr(*n.l), cloneExpr(*n.r), e.s, n.overrideFn);
    }
    case EK::Call: {
      const auto &n = static_cast<const ECall &>(e);
      std::vector<EP> args;
      args.reserve(n.a.size());
      for (const auto &arg : n.a) args.push_back(cloneExpr(*arg));
      return std::make_unique<ECall>(n.f, std::move(args), e.s);
    }
    }
    throw CompileError(e.s, "internal clone expression error");
  }
  static EP substMacroExpr(const Expr &e, const std::unordered_map<std::string, const Expr *> &subs) {
    if (e.k == EK::Var) {
      const auto &n = static_cast<const EVar &>(e);
      auto it = subs.find(n.n);
      if (it != subs.end()) return cloneExpr(*it->second);
      return std::make_unique<EVar>(n.n, e.s);
    }
    switch (e.k) {
    case EK::Int: return std::make_unique<EInt>(static_cast<const EInt &>(e).v, e.s);
    case EK::Float: return std::make_unique<EFloat>(static_cast<const EFloat &>(e).v, e.s);
    case EK::Bool: return std::make_unique<EBool>(static_cast<const EBool &>(e).v, e.s);
    case EK::Str: return std::make_unique<EString>(static_cast<const EString &>(e).v, e.s);
    case EK::Unary: {
      const auto &n = static_cast<const EUnary &>(e);
      return std::make_unique<EUnary>(n.op, substMacroExpr(*n.x, subs), e.s, n.overrideFn);
    }
    case EK::Binary: {
      const auto &n = static_cast<const EBinary &>(e);
      return std::make_unique<EBinary>(n.op, substMacroExpr(*n.l, subs), substMacroExpr(*n.r, subs), e.s,
                                       n.overrideFn);
    }
    case EK::Call: {
      const auto &n = static_cast<const ECall &>(e);
      std::vector<EP> args;
      args.reserve(n.a.size());
      for (const auto &a : n.a) args.push_back(substMacroExpr(*a, subs));
      return std::make_unique<ECall>(n.f, std::move(args), e.s);
    }
    case EK::Var: break;
    }
    throw CompileError(e.s, "internal macro substitution error");
  }

  Type parseType(std::string *className = nullptr) {
    Token tok = need(TokenKind::Id, "expected type name");
    if (className) className->clear();
    auto parseGenericInner = [&](const char *kindName) -> Type {
      need(TokenKind::Lt, std::string("expected '<' in ") + kindName + " type");
      Token innerTok = need(TokenKind::Id, std::string("expected primitive type in ") + kindName + " type");
      const bool knownPrim = innerTok.text == "i32" || innerTok.text == "i64" || innerTok.text == "f32" ||
                             innerTok.text == "f64" || innerTok.text == "bool" || innerTok.text == "byte" ||
                             innerTok.text == "str";
      const bool knownClass = classes_.count(innerTok.text) != 0;
      if (!knownPrim && !knownClass) {
        throw CompileError(innerTok.span, std::string("unsupported ") + kindName + " inner type '" + innerTok.text + "'");
      }
      need(TokenKind::Gt, std::string("expected '>' in ") + kindName + " type");
      return Type::I64;
    };
    if (tok.text == "i32") return Type::I32;
    if (tok.text == "i64") return Type::I64;
    if (tok.text == "f32") return Type::F32;
    if (tok.text == "f64") return Type::F64;
    if (tok.text == "bool") return Type::Bool;
    if (tok.text == "byte") return Type::I32;
    if (tok.text == "str") return Type::Str;
    if (tok.text == "void") return Type::Void;
    if (tok.text == "ptr") return parseGenericInner("ptr");
    if (tok.text == "slice") return parseGenericInner("slice");
    if (tok.text == "array") return parseGenericInner("array");
    if (classes_.count(tok.text)) {
      if (className) *className = tok.text;
      return Type::I64;
    }
    throw CompileError(tok.span, "unknown type '" + tok.text + "'");
  }

  std::vector<Param> parseParams(std::unordered_map<std::string, std::string> &paramClasses) {
    std::vector<Param> params;
    need(TokenKind::LPar, "expected '('");
    if (!is(TokenKind::RPar)) {
      while (true) {
        Param p;
        p.n = need(TokenKind::Id, "expected parameter name").text;
        need(TokenKind::Colon, "expected ':'");
        std::string classType;
        p.t = parseType(&classType);
        if (!classType.empty()) paramClasses[p.n] = classType;
        params.push_back(std::move(p));
        if (!eat(TokenKind::Comma)) break;
      }
    }
    need(TokenKind::RPar, "expected ')'");
    return params;
  }

  static bool sameParamTypes(const std::vector<Type> &a, const std::vector<Type> &b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
      if (a[i] != b[i]) return false;
    }
    return true;
  }
  std::string mangleTopLevelOverloadSymbol(const std::string &name) {
    return "__ls_ovl_" + name + "_" + std::to_string(++topFnOverloadId_);
  }
  void registerTopLevelOverload(Fn &f, const Span &s) {
    if (f.isCliFlag || f.isOperatorOverride) {
      f.sourceName = f.n;
      return;
    }
    const std::string publicName = f.n;
    std::vector<Type> paramTypes;
    paramTypes.reserve(f.p.size());
    for (const auto &param : f.p) paramTypes.push_back(param.t);
    auto &decls = topFnOverloads_[publicName];
    for (const auto &decl : decls) {
      if (sameParamTypes(decl.params, paramTypes)) {
        throw CompileError(s, "duplicate overload for function '" + publicName + "'");
      }
    }
    f.sourceName = publicName;
    if (!decls.empty()) {
      f.n = mangleTopLevelOverloadSymbol(publicName);
    }
    decls.push_back(OverloadDecl{std::move(paramTypes), f.n});
  }

  std::string classMethodSymbol(const std::string &cls, const std::string &method, std::size_t overloadIndex = 0) const {
    if (overloadIndex == 0) return "__ls_cls_" + cls + "_" + method;
    return "__ls_cls_" + cls + "_" + method + "_" + std::to_string(overloadIndex);
  }
  std::string classOperatorMethodKey(BK op) const { return operatorOverrideSymbol(op); }
  std::string classUnaryOperatorMethodKey(UK op) const { return unaryOperatorOverrideSymbol(op); }

  std::string resolveReceiverClass(const std::string &receiver) const {
    if (receiver == "this") return currentClass_;
    auto it = varClass_.find(receiver);
    return it == varClass_.end() ? "" : it->second;
  }
  bool isSubclassOf(const std::string &child, const std::string &ancestor) const {
    if (child.empty() || ancestor.empty()) return false;
    std::string cur = child;
    while (!cur.empty()) {
      if (cur == ancestor) return true;
      auto it = classes_.find(cur);
      if (it == classes_.end()) return false;
      cur = it->second.base;
    }
    return false;
  }
  const MethodInfo *findMethodByArity(const ClassInfo &info, const std::string &methodKey, std::size_t arity) const {
    auto it = info.methods.find(methodKey);
    if (it == info.methods.end()) return nullptr;
    const MethodInfo *candidate = nullptr;
    for (const auto &m : it->second) {
      if (m.params.size() == arity) {
        if (candidate) return nullptr;
        candidate = &m;
      }
    }
    return candidate;
  }
  const MethodInfo *findMethodRecursive(const std::string &className, const std::string &methodKey, std::size_t arity) const {
    std::string cur = className;
    while (!cur.empty()) {
      auto it = classes_.find(cur);
      if (it == classes_.end()) return nullptr;
      if (const MethodInfo *m = findMethodByArity(it->second, methodKey, arity)) {
        return m;
      }
      cur = it->second.base;
    }
    return nullptr;
  }
  std::optional<Type> findFieldTypeRecursive(const std::string &className, const std::string &field) const {
    std::string cur = className;
    while (!cur.empty()) {
      auto it = classes_.find(cur);
      if (it == classes_.end()) return std::nullopt;
      auto fit = it->second.fields.find(field);
      if (fit != it->second.fields.end()) return fit->second.t;
      cur = it->second.base;
    }
    return std::nullopt;
  }
  const FieldInfo *findFieldRecursive(const std::string &className, const std::string &field) const {
    std::string cur = className;
    while (!cur.empty()) {
      auto it = classes_.find(cur);
      if (it == classes_.end()) return nullptr;
      auto fit = it->second.fields.find(field);
      if (fit != it->second.fields.end()) return &fit->second;
      cur = it->second.base;
    }
    return nullptr;
  }
  bool canAccessMethod(const MethodInfo &m, const std::string &accessorClass) const {
    if (m.access == AccessModifier::Public) return true;
    if (m.access == AccessModifier::Private) return accessorClass == m.owner;
    if (accessorClass.empty()) return false;
    return accessorClass == m.owner || isSubclassOf(accessorClass, m.owner);
  }
  bool canAccessField(const FieldInfo &f, const std::string &accessorClass) const {
    if (f.access == AccessModifier::Public) return true;
    if (f.access == AccessModifier::Private) return accessorClass == f.owner;
    if (accessorClass.empty()) return false;
    return accessorClass == f.owner || isSubclassOf(accessorClass, f.owner);
  }
  std::string resolveMemberOperatorSymbol(const Expr &lhs, BK op) const {
    if (lhs.k != EK::Var) return "";
    const auto &recv = static_cast<const EVar &>(lhs);
    const std::string className = resolveReceiverClass(recv.n);
    if (className.empty()) return "";
    const std::string opKey = classOperatorMethodKey(op);
    if (opKey.empty()) return "";
    const MethodInfo *m = findMethodRecursive(className, opKey, 1);
    if (!m || m->isStatic || !canAccessMethod(*m, currentClass_)) return "";
    return m->symbol;
  }
  std::string resolveMemberUnaryOperatorSymbol(const Expr &operand, UK op) const {
    if (operand.k != EK::Var) return "";
    const auto &recv = static_cast<const EVar &>(operand);
    const std::string className = resolveReceiverClass(recv.n);
    if (className.empty()) return "";
    const std::string opKey = classUnaryOperatorMethodKey(op);
    if (opKey.empty()) return "";
    const MethodInfo *m = findMethodRecursive(className, opKey, 0);
    if (!m || m->isStatic || !canAccessMethod(*m, currentClass_)) return "";
    return m->symbol;
  }
  EP makeBinaryExpr(BK op, EP lhs, EP rhs, const Span &s) {
    const std::string memberOp = resolveMemberOperatorSymbol(*lhs, op);
    return std::make_unique<EBinary>(op, std::move(lhs), std::move(rhs), s, memberOp);
  }
  EP makeUnaryExpr(UK op, EP operand, const Span &s) {
    const std::string memberOp = resolveMemberUnaryOperatorSymbol(*operand, op);
    return std::make_unique<EUnary>(op, std::move(operand), s, memberOp);
  }
  static bool isSuperuserNamespaceReceiver(const std::string &receiver) {
    return isSuperuserNamespaceSymbol(receiver);
  }

  EP makeFieldLoadExpr(const std::string &receiver, const std::string &field, Type fieldType, const Span &s) {
    std::vector<EP> getArgs;
    getArgs.push_back(std::make_unique<EVar>(receiver, s));
    getArgs.push_back(std::make_unique<EString>(field, s));
    EP raw = std::make_unique<ECall>("object_get", std::move(getArgs), s);
    if (fieldType == Type::Str) return raw;
    if (fieldType == Type::I32) {
      std::vector<EP> parseArgs;
      parseArgs.push_back(std::move(raw));
      EP asI64 = std::make_unique<ECall>("parse_i64", std::move(parseArgs), s);
      std::vector<EP> castArgs;
      castArgs.push_back(std::move(asI64));
      return std::make_unique<ECall>("to_i32", std::move(castArgs), s);
    }
    if (fieldType == Type::I64) {
      std::vector<EP> args;
      args.push_back(std::move(raw));
      return std::make_unique<ECall>("parse_i64", std::move(args), s);
    }
    if (fieldType == Type::F32) {
      std::vector<EP> parseArgs;
      parseArgs.push_back(std::move(raw));
      EP asF64 = std::make_unique<ECall>("parse_f64", std::move(parseArgs), s);
      std::vector<EP> castArgs;
      castArgs.push_back(std::move(asF64));
      return std::make_unique<ECall>("to_f32", std::move(castArgs), s);
    }
    if (fieldType == Type::F64) {
      std::vector<EP> args;
      args.push_back(std::move(raw));
      return std::make_unique<ECall>("parse_f64", std::move(args), s);
    }
    if (fieldType == Type::Bool) {
      std::vector<EP> args;
      args.push_back(std::move(raw));
      EP asI64 = std::make_unique<ECall>("parse_i64", std::move(args), s);
      std::vector<EP> boolArgs;
      boolArgs.push_back(std::move(asI64));
      return std::make_unique<ECall>("i64_to_bool", std::move(boolArgs), s);
    }
    throw CompileError(s, "unsupported class field type");
  }

  SP makeFieldStoreStmt(const std::string &receiver, const std::string &field, Type fieldType, EP value, const Span &s) {
    std::vector<EP> fmtArgs;
    if (fieldType == Type::Bool) {
      std::vector<EP> boolArgs;
      boolArgs.push_back(std::move(value));
      fmtArgs.push_back(std::make_unique<ECall>("bool_to_i64", std::move(boolArgs), s));
    } else {
      fmtArgs.push_back(std::move(value));
    }
    EP text = std::make_unique<ECall>("formatOutput", std::move(fmtArgs), s);
    std::vector<EP> setArgs;
    setArgs.push_back(std::make_unique<EVar>(receiver, s));
    setArgs.push_back(std::make_unique<EString>(field, s));
    setArgs.push_back(std::move(text));
    EP call = std::make_unique<ECall>("object_set", std::move(setArgs), s);
    return std::make_unique<SExpr>(std::move(call), s);
  }

  std::string constructorClassFromExpr(const Expr &e) const {
    if (e.k != EK::Call) return "";
    const auto &c = static_cast<const ECall &>(e);
    return classes_.count(c.f) ? c.f : "";
  }
  std::string constructorFreeFnFromExpr(const Expr &e) const {
    if (e.k != EK::Call) return "";
    const auto &c = static_cast<const ECall &>(e);
    if (classes_.count(c.f)) return "object_free";
    if (c.f == "array_new") return "array_free";
    if (c.f == "dict_new") return "dict_free";
    if (c.f == "map_new") return "map_free";
    if (c.f == "object_new") return "object_free";
    if (c.f == "option_some" || c.f == "option_none") return "option_free";
    if (c.f == "result_ok" || c.f == "result_err") return "result_free";
    if (c.f == "np_new") return "np_free";
    if (c.f == "gfx_new") return "gfx_free";
    if (c.f == "game_new") return "game_free";
    if (c.f == "pg_surface_new") return "pg_surface_free";
    if (c.f == "phys_new") return "phys_free";
    if (c.f == "http_server_listen") return "http_server_close";
    if (c.f == "http_client_connect") return "http_client_close";
    return "";
  }

  void parseClass(Program &p, const Span &classSpan) {
    (void)classSpan;
    Token classNameTok = need(TokenKind::Id, "expected class name");
    const std::string className = classNameTok.text;
    if (classes_.count(className)) {
      throw CompileError(classNameTok.span, "duplicate class '" + className + "'");
    }
    ClassInfo classInfo;
    classInfo.name = className;
    classInfo.s = classNameTok.span;
    skipNl();
    if (is(TokenKind::Id) && cur().text == "extends") {
      ++i_;
      Token baseTok = need(TokenKind::Id, "expected base class name after 'extends'");
      if (!classes_.count(baseTok.text)) {
        throw CompileError(baseTok.span, "unknown base class '" + baseTok.text + "'");
      }
      classInfo.base = baseTok.text;
      skipNl();
    }
    classes_[className] = std::move(classInfo);
    ClassInfo &classRef = classes_.at(className);

    bool braceStyle = false;
    bool endStyle = false;
    if (eat(TokenKind::LBra)) braceStyle = true;
    else if (eat(TokenKind::KwDo)) endStyle = true;
    else
      throw CompileError(cur().span, "expected '{' or 'do' to start class body");

    std::vector<ParsedClassField> fields;
    std::optional<Fn> ctor;
    bool seenCallable = false;
    while (!is(TokenKind::End)) {
      skipNl();
      if (braceStyle && is(TokenKind::RBra)) break;
      if (endStyle && is(TokenKind::KwEnd)) break;

      AccessModifier memberAccess = AccessModifier::Public;
      bool accessSet = false;
      bool methodStatic = false;
      bool methodVirtual = false;
      bool methodOverride = false;
      bool methodFinal = false;
      while (is(TokenKind::Id)) {
        const std::string mod = cur().text;
        if (mod == "public" || mod == "protected" || mod == "private") {
          if (accessSet) throw CompileError(cur().span, "duplicate access modifier");
          accessSet = true;
          if (mod == "public") memberAccess = AccessModifier::Public;
          else if (mod == "protected") memberAccess = AccessModifier::Protected;
          else memberAccess = AccessModifier::Private;
          ++i_;
          skipNl();
          continue;
        }
        if (mod == "static") {
          if (methodStatic) throw CompileError(cur().span, "duplicate 'static' modifier");
          methodStatic = true;
          ++i_;
          skipNl();
          continue;
        }
        if (mod == "virtual") {
          if (methodVirtual) throw CompileError(cur().span, "duplicate 'virtual' modifier");
          methodVirtual = true;
          ++i_;
          skipNl();
          continue;
        }
        if (mod == "override") {
          if (methodOverride) throw CompileError(cur().span, "duplicate 'override' modifier");
          methodOverride = true;
          ++i_;
          skipNl();
          continue;
        }
        if (mod == "final") {
          if (methodFinal) throw CompileError(cur().span, "duplicate 'final' modifier");
          methodFinal = true;
          ++i_;
          skipNl();
          continue;
        }
        break;
      }

      if (eat(TokenKind::KwDeclare)) {
        if (seenCallable) {
          throw CompileError(t_[i_ - 1].span, "class fields must be declared before methods");
        }
        if (methodStatic || methodVirtual || methodOverride || methodFinal) {
          throw CompileError(t_[i_ - 1].span, "method modifiers are not allowed on class fields");
        }
        ParsedClassField field;
        field.s = t_[i_ - 1].span;
        field.access = memberAccess;
        field.n = need(TokenKind::Id, "expected class field name").text;
        if (classRef.fields.count(field.n)) {
          throw CompileError(field.s, "duplicate class field '" + field.n + "'");
        }
        need(TokenKind::Colon, "class field requires explicit type");
        field.t = parseType();
        if (eat(TokenKind::Assign)) {
          field.init = expr();
        } else {
          field.init = defaultValueFor(field.t, field.s);
        }
        classRef.fields[field.n] = FieldInfo{field.t, memberAccess, className};
        fields.push_back(std::move(field));
        needStmtEnd("expected statement terminator after class field declaration");
        continue;
      }

      seenCallable = true;
      Fn f;
      f.isClassMethod = true;
      f.classOwner = className;
      f.methodStatic = methodStatic;
      f.methodVirtual = methodVirtual;
      f.methodOverride = methodOverride;
      f.methodFinal = methodFinal;

      skipNl();
      while (true) {
        if (eat(TokenKind::KwInline)) {
          f.inl = true;
          skipNl();
          continue;
        }
        if (eat(TokenKind::KwExtern)) {
          f.ex = true;
          skipNl();
          continue;
        }
        break;
      }
      if (f.methodVirtual && f.methodStatic) {
        throw CompileError(cur().span, "static methods cannot be virtual");
      }

      Token nameTok;
      bool methodIsOperator = false;
      bool methodIsUnaryOperator = false;
      std::string methodKey;
      std::string methodDisplay;
      const bool hasFnKeyword = eat(TokenKind::KwFn);
      if (is(TokenKind::Id) && cur().text == "operator") {
        Token opTok = cur();
        ++i_;
        skipNl();
        if (is(TokenKind::Id) && cur().text == "unary") {
          methodIsUnaryOperator = true;
          ++i_;
          skipNl();
        }
        if (methodIsUnaryOperator) {
          if (!isOverloadableUnaryOperatorTokenKind(cur().kind)) {
            throw CompileError(cur().span, "expected overloadable unary operator token after 'operator unary'");
          }
          Token symTok = cur();
          ++i_;
          const UK op = overloadableUnaryOperatorTokenToUK(symTok.kind, symTok.span);
          methodIsOperator = true;
          f.unaryOperatorKind = op;
          methodKey = classUnaryOperatorMethodKey(op);
          methodDisplay = std::string("unary ") + unaryOperatorSymbolText(op);
          nameTok = opTok;
        } else {
          if (!isOverloadableOperatorTokenKind(cur().kind)) {
            throw CompileError(cur().span, "expected overloadable operator token after 'operator'");
          }
          Token symTok = cur();
          ++i_;
          const BK op = overloadableOperatorTokenToBK(symTok.kind, symTok.span);
          methodIsOperator = true;
          f.operatorKind = op;
          methodKey = classOperatorMethodKey(op);
          methodDisplay = operatorSymbolText(op);
          nameTok = opTok;
        }
      } else if (is(TokenKind::Id) && (hasFnKeyword || lookNonNl(1).kind == TokenKind::LPar)) {
        nameTok = need(TokenKind::Id, "expected method name");
        methodKey = nameTok.text;
        methodDisplay = nameTok.text;
      } else {
        throw CompileError(cur().span, "expected class method or constructor");
      }

      const bool isCtor = (!methodIsOperator && (nameTok.text == "constructor" || nameTok.text == className));
      f.s = nameTok.span;
      std::unordered_map<std::string, std::string> paramClasses;
      f.p = parseParams(paramClasses);

      bool ctorHasBaseInit = false;
      std::string ctorBaseClass;
      std::vector<EP> ctorBaseArgs;
      if (isCtor && eat(TokenKind::Colon)) {
        Token baseTok = need(TokenKind::Id, "expected base constructor name after ':'");
        ctorBaseClass = baseTok.text;
        need(TokenKind::LPar, "expected '(' after base constructor name");
        if (!is(TokenKind::RPar)) {
          while (true) {
            ctorBaseArgs.push_back(expr());
            if (!eat(TokenKind::Comma)) break;
          }
        }
        need(TokenKind::RPar, "expected ')'");
        if (classRef.base.empty()) {
          throw CompileError(baseTok.span, "constructor init-list requires an 'extends' base class");
        }
        if (ctorBaseClass != classRef.base) {
          throw CompileError(baseTok.span, "constructor init-list must target direct base '" + classRef.base + "'");
        }
        ctorHasBaseInit = true;
      }

      if (!isCtor && eat(TokenKind::Arrow)) {
        f.ret = parseType();
      } else if (isCtor && eat(TokenKind::Arrow)) {
        throw CompileError(nameTok.span, "constructor return type is implicit");
      } else if (isCtor) {
        f.ret = Type::I64;
      }
      if (eat(TokenKind::KwThrows)) {
        while (true) {
          Token errTok = need(TokenKind::Id, "expected error type after 'throws'");
          f.throws.push_back(errTok.text);
          if (!eat(TokenKind::Comma)) break;
        }
      }
      if (isCtor && f.ex) {
        throw CompileError(nameTok.span, "constructor cannot be extern");
      }
      if (methodIsOperator && methodIsUnaryOperator) {
        if (f.p.size() != 0) {
          throw CompileError(nameTok.span, "operator method '" + methodDisplay + "' expects exactly 0 parameters");
        }
        if (f.ret == Type::Void) {
          throw CompileError(nameTok.span, "operator method '" + methodDisplay + "' must return a value");
        }
        if (!f.throws.empty()) {
          throw CompileError(nameTok.span, "operator methods do not support 'throws'");
        }
      } else if (methodIsOperator) {
        if (f.p.size() != 1) {
          throw CompileError(nameTok.span, "operator method '" + methodDisplay + "' expects exactly 1 parameter");
        }
        if (f.ret == Type::Void) {
          throw CompileError(nameTok.span, "operator method '" + methodDisplay + "' must return a value");
        }
        if (!f.throws.empty()) {
          throw CompileError(nameTok.span, "operator methods do not support 'throws'");
        }
      }

      if (f.ex) {
        needStmtEnd("expected statement terminator after extern method");
      } else {
        auto savedClass = currentClass_;
        auto savedVarClass = varClass_;
        currentClass_ = className;
        varClass_.clear();
        if (!f.methodStatic) varClass_["this"] = className;
        for (const auto &entry : paramClasses) varClass_[entry.first] = entry.second;
        f.b = block();
        currentClass_ = std::move(savedClass);
        varClass_ = std::move(savedVarClass);
      }

      if (isCtor) {
        if (ctor.has_value()) throw CompileError(nameTok.span, "duplicate constructor for class '" + className + "'");
        Fn generated;
        generated.s = f.s;
        generated.n = className;
        generated.sourceName = className;
        generated.ret = Type::I64;
        generated.p = f.p;
        generated.throws = f.throws;
        generated.inl = f.inl;
        EP objCtor;
        if (ctorHasBaseInit) {
          objCtor = std::make_unique<ECall>(ctorBaseClass, std::move(ctorBaseArgs), f.s);
        } else if (!classRef.base.empty()) {
          objCtor = std::make_unique<ECall>(classRef.base, std::vector<EP>{}, f.s);
        } else {
          objCtor = std::make_unique<ECall>("object_new", std::vector<EP>{}, f.s);
        }
        generated.b.push_back(
            std::make_unique<SLet>("this", std::optional<Type>(Type::I64), false, false, std::move(objCtor), f.s));
        for (const auto &field : fields) {
          generated.b.push_back(makeFieldStoreStmt("this", field.n, field.t, cloneExpr(*field.init), field.s));
        }
        for (auto &stmt : f.b) generated.b.push_back(std::move(stmt));
        generated.b.push_back(std::make_unique<SRet>(true, std::make_unique<EVar>("this", f.s), f.s));
        ctor = std::move(generated);
      } else {
        std::vector<Type> paramTypes;
        paramTypes.reserve(f.p.size());
        for (const auto &param : f.p) paramTypes.push_back(param.t);
        auto &overloads = classRef.methods[methodKey];
        for (const auto &existing : overloads) {
          if (sameParamTypes(existing.params, paramTypes)) {
            throw CompileError(nameTok.span, "duplicate method overload '" + methodDisplay + "' in class '" + className +
                                                 "'");
          }
        }
        const MethodInfo *baseMethod = nullptr;
        if (!classRef.base.empty()) {
          baseMethod = findMethodRecursive(classRef.base, methodKey, paramTypes.size());
        }
        if (f.methodOverride && !baseMethod) {
          throw CompileError(nameTok.span, "override method '" + methodDisplay + "' has no base method to override");
        }
        if (baseMethod && baseMethod->isFinal) {
          throw CompileError(nameTok.span, "cannot override final base method '" + methodDisplay + "'");
        }
        if (baseMethod && f.methodOverride) {
          if (baseMethod->isStatic != f.methodStatic) {
            throw CompileError(nameTok.span, "override method '" + methodDisplay +
                                                 "' must keep static/instance behavior");
          }
          if (baseMethod->ret != f.ret) {
            throw CompileError(nameTok.span, "override method '" + methodDisplay + "' must keep return type '" +
                                                 typeName(baseMethod->ret) + "'");
          }
        }
        MethodInfo methodInfo;
        const std::size_t overloadIndex = overloads.size();
        methodInfo.symbol = classMethodSymbol(className, methodKey, overloadIndex);
        methodInfo.owner = className;
        methodInfo.access = memberAccess;
        methodInfo.isStatic = f.methodStatic;
        methodInfo.isVirtual = f.methodVirtual || (baseMethod != nullptr && baseMethod->isVirtual);
        methodInfo.isOverride = f.methodOverride || (baseMethod != nullptr && baseMethod->isVirtual);
        methodInfo.isFinal = f.methodFinal;
        methodInfo.params = paramTypes;
        methodInfo.ret = f.ret;
        overloads.push_back(methodInfo);

        Fn generated;
        generated.s = f.s;
        generated.n = methodInfo.symbol;
        generated.sourceName = methodKey;
        generated.ret = f.ret;
        generated.throws = f.throws;
        generated.inl = f.inl;
        generated.isClassMethod = true;
        generated.classOwner = className;
        generated.methodStatic = methodInfo.isStatic;
        generated.methodVirtual = methodInfo.isVirtual;
        generated.methodOverride = methodInfo.isOverride;
        generated.methodFinal = methodInfo.isFinal;
        if (!methodInfo.isStatic) generated.p.push_back(Param{"this", Type::I64});
        for (auto &param : f.p) generated.p.push_back(std::move(param));
        generated.b = std::move(f.b);
        p.f.push_back(std::move(generated));
      }
    }
    if (braceStyle) need(TokenKind::RBra, "expected '}'");
    else need(TokenKind::KwEnd, "expected 'end' to close class");
    if (!ctor.has_value()) {
      Fn generated;
      generated.s = classNameTok.span;
      generated.n = className;
      generated.sourceName = className;
      generated.ret = Type::I64;
      EP objCtor;
      if (!classRef.base.empty()) {
        objCtor = std::make_unique<ECall>(classRef.base, std::vector<EP>{}, classNameTok.span);
      } else {
        objCtor = std::make_unique<ECall>("object_new", std::vector<EP>{}, classNameTok.span);
      }
      generated.b.push_back(std::make_unique<SLet>("this", std::optional<Type>(Type::I64), false, false,
                                                   std::move(objCtor), classNameTok.span));
      for (const auto &field : fields) {
        generated.b.push_back(makeFieldStoreStmt("this", field.n, field.t, cloneExpr(*field.init), field.s));
      }
      generated.b.push_back(
          std::make_unique<SRet>(true, std::make_unique<EVar>("this", classNameTok.span), classNameTok.span));
      ctor = std::move(generated);
    }
    p.f.push_back(std::move(*ctor));
  }

  Fn fn() {
    Fn f;
    skipNl();
    if (is(TokenKind::Id) && cur().text == "flag") {
      ++i_;
      Token firstSeg = need(TokenKind::Id, "expected flag name after 'flag'");
      std::string cliName = firstSeg.text;
      while (eat(TokenKind::Minus)) {
        Token seg = need(TokenKind::Id, "expected flag name segment after '-'");
        cliName += "-";
        cliName += seg.text;
      }
      if (!isValidCliFlagName(cliName)) {
        throw CompileError(firstSeg.span, "invalid flag name '" + cliName + "'");
      }
      if (!cliFlagNames_.insert(cliName).second) {
        throw CompileError(firstSeg.span, "duplicate flag '" + cliName + "'");
      }
      f.s = firstSeg.span;
      f.isCliFlag = true;
      f.cliFlagName = cliName;
      f.n = cliFlagSymbol(cliName);
      f.sourceName = f.n;
      need(TokenKind::LPar, "expected '(' after flag name");
      if (!eat(TokenKind::RPar)) {
        throw CompileError(cur().span, "flag functions must not take parameters");
      }
      if (eat(TokenKind::Arrow)) {
        Type rt = parseType();
        if (rt != Type::Void) throw CompileError(firstSeg.span, "flag functions must return void");
      }
      if (eat(TokenKind::KwThrows)) {
        throw CompileError(firstSeg.span, "flag functions do not support 'throws'");
      }
      auto savedClass = currentClass_;
      auto savedVarClass = varClass_;
      currentClass_.clear();
      varClass_.clear();
      skipNl();
      f.b = block();
      currentClass_ = std::move(savedClass);
      varClass_ = std::move(savedVarClass);
      registerTopLevelOverload(f, f.s);
      return f;
    }
    while (true) {
      if (eat(TokenKind::KwInline)) {
        f.inl = true;
        skipNl();
        continue;
      }
      if (eat(TokenKind::KwExtern)) {
        f.ex = true;
        skipNl();
        continue;
      }
      break;
    }
    Token nameTok;
    const bool hasFnKeyword = eat(TokenKind::KwFn);
    if (is(TokenKind::Id) && cur().text == "operator") {
      Token opTok = cur();
      ++i_;
      skipNl();
      f.s = opTok.span;
      bool unaryOp = false;
      if (is(TokenKind::Id) && cur().text == "unary") {
        unaryOp = true;
        ++i_;
        skipNl();
      }
      if (unaryOp) {
        if (!isOverloadableUnaryOperatorTokenKind(cur().kind)) {
          throw CompileError(cur().span, "expected overloadable unary operator token after 'operator unary'");
        }
        Token symTok = cur();
        ++i_;
        const UK op = overloadableUnaryOperatorTokenToUK(symTok.kind, symTok.span);
        f.n = unaryOperatorOverrideSymbol(op);
        f.unaryOperatorKind = op;
      } else {
        if (!isOverloadableOperatorTokenKind(cur().kind)) {
          throw CompileError(cur().span, "expected overloadable operator token after 'operator'");
        }
        Token symTok = cur();
        ++i_;
        const BK op = overloadableOperatorTokenToBK(symTok.kind, symTok.span);
        f.n = operatorOverrideSymbol(op);
        f.operatorKind = op;
      }
      f.sourceName = f.n;
      f.isOperatorOverride = true;
      nameTok = opTok;
    } else if (is(TokenKind::Id) && (hasFnKeyword || lookNonNl(1).kind == TokenKind::LPar)) {
      nameTok = need(TokenKind::Id, "expected function name");
      f.s = nameTok.span;
      f.n = nameTok.text;
      f.sourceName = f.n;
    } else {
      throw CompileError(cur().span, "expected function declaration");
    }
    std::unordered_map<std::string, std::string> paramClasses;
    f.p = parseParams(paramClasses);
    if (eat(TokenKind::Arrow)) f.ret = parseType();
    if (eat(TokenKind::KwThrows)) {
      while (true) {
        Token errTok = need(TokenKind::Id, "expected error type after 'throws'");
        f.throws.push_back(errTok.text);
        if (!eat(TokenKind::Comma)) break;
      }
    }
    if (f.isOperatorOverride) {
      if (f.unaryOperatorKind.has_value()) {
        if (!isOverloadableUnaryOp(*f.unaryOperatorKind)) {
          throw CompileError(f.s, "invalid unary operator override declaration");
        }
        if (f.p.size() != 1) {
          throw CompileError(f.s, "operator override 'unary " + unaryOperatorSymbolText(*f.unaryOperatorKind) +
                                      "' expects exactly 1 parameter");
        }
        if (f.ret == Type::Void) {
          throw CompileError(f.s, "operator override 'unary " + unaryOperatorSymbolText(*f.unaryOperatorKind) +
                                      "' must return a value");
        }
        if (!f.throws.empty()) {
          throw CompileError(f.s, "operator overrides do not support 'throws'");
        }
      } else {
        if (!f.operatorKind.has_value() || !isOverloadableBinaryOp(*f.operatorKind)) {
          throw CompileError(f.s, "invalid operator override declaration");
        }
        if (f.p.size() != 2) {
          throw CompileError(f.s, "operator override '" + operatorSymbolText(*f.operatorKind) + "' expects exactly 2 parameters");
        }
        if (f.ret == Type::Void) {
          throw CompileError(f.s, "operator override '" + operatorSymbolText(*f.operatorKind) + "' must return a value");
        }
        if (!f.throws.empty()) {
          throw CompileError(f.s, "operator overrides do not support 'throws'");
        }
      }
    }
    if (f.ex) {
      needStmtEnd("expected statement terminator after extern function");
      registerTopLevelOverload(f, f.s);
      return f;
    }
    auto savedClass = currentClass_;
    auto savedVarClass = varClass_;
    currentClass_.clear();
    varClass_ = std::move(paramClasses);
    skipNl();
    f.b = block();
    currentClass_ = std::move(savedClass);
    varClass_ = std::move(savedVarClass);
    registerTopLevelOverload(f, f.s);
    return f;
  }

  std::vector<SP> block() {
    skipNl();
    bool braceStyle = false;
    bool endStyle = false;
    if (eat(TokenKind::LBra)) braceStyle = true;
    else if (eat(TokenKind::KwDo)) endStyle = true;
    else
      throw CompileError(cur().span, "expected '{' or 'do' to start block");
    std::vector<SP> b;
    while (!is(TokenKind::End)) {
      skipNl();
      if (braceStyle && is(TokenKind::RBra)) break;
      if (endStyle && is(TokenKind::KwEnd)) break;
      b.push_back(stmt());
    }
    if (braceStyle) need(TokenKind::RBra, "expected '}'");
    else need(TokenKind::KwEnd, "expected 'end'");
    return b;
  }

  bool isAny(TokenKind a, TokenKind b, TokenKind c) const { return is(a) || is(b) || is(c); }
  static bool isAssignOp(TokenKind k) {
    return k == TokenKind::Assign || k == TokenKind::PlusAssign || k == TokenKind::MinusAssign ||
           k == TokenKind::StarAssign || k == TokenKind::SlashAssign || k == TokenKind::PercentAssign ||
           k == TokenKind::PowAssign;
  }
  bool isMemberAssignStart() const {
    return is(TokenKind::Id) && lookNonNl(1).kind == TokenKind::Dot && lookNonNl(2).kind == TokenKind::Id &&
           isAssignOp(lookNonNl(3).kind);
  }
  static BK assignOpToBinary(TokenKind k, const Span &s) {
    switch (k) {
    case TokenKind::PlusAssign: return BK::Add;
    case TokenKind::MinusAssign: return BK::Sub;
    case TokenKind::StarAssign: return BK::Mul;
    case TokenKind::SlashAssign: return BK::Div;
    case TokenKind::PercentAssign: return BK::Mod;
    case TokenKind::PowAssign: return BK::Pow;
    default: break;
    }
    throw CompileError(s, "internal assignment operator mapping error");
  }
  bool isOneOf(const std::vector<TokenKind> &kinds) const {
    for (TokenKind k : kinds)
      if (is(k)) return true;
    return false;
  }
  std::vector<SP> blockDoUntil(const std::vector<TokenKind> &stops) {
    std::vector<SP> b;
    while (!is(TokenKind::End)) {
      skipNl();
      if (isOneOf(stops)) break;
      b.push_back(stmt());
    }
    return b;
  }
  SP sIfDoTail(const Span &s, EP c, std::vector<SP> t) {
    skipNl();
    std::vector<SP> e;
    if (eat(TokenKind::KwElif)) {
      Span elifSpan = t_[i_ - 1].span;
      EP nextCond = expr();
      skipNl();
      need(TokenKind::KwDo, "expected 'do' after elif condition");
      std::vector<SP> nextBody = blockDoUntil({TokenKind::KwElif, TokenKind::KwElse, TokenKind::KwEnd});
      e.push_back(sIfDoTail(elifSpan, std::move(nextCond), std::move(nextBody)));
      return std::make_unique<SIf>(std::move(c), std::move(t), std::move(e), s);
    }
    if (eat(TokenKind::KwElse)) {
      skipNl();
      (void)eat(TokenKind::KwDo);
      e = blockDoUntil({TokenKind::KwEnd});
      need(TokenKind::KwEnd, "expected 'end' to close if");
      return std::make_unique<SIf>(std::move(c), std::move(t), std::move(e), s);
    }
    need(TokenKind::KwEnd, "expected 'end' to close if");
    return std::make_unique<SIf>(std::move(c), std::move(t), std::move(e), s);
  }

  SP stmt() {
    skipNl();
    if (is(TokenKind::Id) && cur().text == "delete") return sDelete();
    if (eat(TokenKind::KwDeclare)) return sDeclare();
    if (eat(TokenKind::KwLet) || eat(TokenKind::KwVar) || eat(TokenKind::KwConst)) {
      throw CompileError(cur().span, "use 'declare <name>' for variable declarations");
    }
    if (eat(TokenKind::KwReturn)) return sRet(t_[i_ - 1].span);
    if (eat(TokenKind::KwIf)) return sIf(t_[i_ - 1].span);
    if (eat(TokenKind::KwUnless)) return sUnless(t_[i_ - 1].span);
    if (eat(TokenKind::KwWhile)) return sWhile(t_[i_ - 1].span);
    if (eat(TokenKind::KwFor)) return sFor(t_[i_ - 1].span, false);
    if (eat(TokenKind::KwParallel)) {
      const Span s = t_[i_ - 1].span;
      skipNl();
      need(TokenKind::KwFor, "expected 'for' after 'parallel'");
      return sFor(s, true);
    }
    if (is(TokenKind::Id) && (cur().text == "formatOutput" || cur().text == "FormatOutput")) {
      const std::size_t save = i_;
      const Token nameTok = need(TokenKind::Id, "expected format block keyword");
      skipNl();
      EP endArg;
      if (eat(TokenKind::LPar)) {
        if (!is(TokenKind::RPar)) endArg = expr();
        need(TokenKind::RPar, "expected ')'");
        skipNl();
      }
      if (is(TokenKind::KwDo) || is(TokenKind::LBra)) {
        return sFormatBlock(nameTok.span, std::move(endArg));
      }
      i_ = save;
    }
    if (eat(TokenKind::KwBreak)) {
      needStmtEnd("expected statement terminator after 'break'");
      return std::make_unique<SBreak>(t_[i_ - 1].span);
    }
    if (eat(TokenKind::KwContinue)) {
      needStmtEnd("expected statement terminator after 'continue'");
      return std::make_unique<SContinue>(t_[i_ - 1].span);
    }
    if (is(TokenKind::Id) && lookNonNl(1).kind == TokenKind::PlusPlus) return sPostfixIncDec(true);
    if (is(TokenKind::Id) && lookNonNl(1).kind == TokenKind::MinusMinus) return sPostfixIncDec(false);
    if (isMemberAssignStart()) return sAssignMember();
    if (is(TokenKind::Id) && isAssignOp(lookNonNl(1).kind)) return sAssign();
    EP e = expr();
    Span s = e->s;
    needStmtEnd("expected statement terminator after expression");
    return std::make_unique<SExpr>(std::move(e), s);
  }

  SP sDeclare() {
    bool isConst = false;
    bool isOwned = false;
    while (true) {
      if (eat(TokenKind::KwConst)) {
        if (isConst) throw CompileError(cur().span, "duplicate 'const' in declaration");
        isConst = true;
        continue;
      }
      if (eat(TokenKind::KwOwned)) {
        if (isOwned) throw CompileError(cur().span, "duplicate 'owned' in declaration");
        isOwned = true;
        continue;
      }
      break;
    }
    Token n = need(TokenKind::Id, "expected variable name");
    std::optional<Type> d;
    std::string declaredClass;
    if (eat(TokenKind::Colon)) d = parseType(&declaredClass);
    EP v;
    if (eat(TokenKind::Assign)) {
      v = expr();
    } else {
      if (isConst) throw CompileError(n.span, "declare const requires an initializer");
      if (d.has_value()) {
        v = defaultValueFor(*d, n.span);
      } else {
        d = Type::I64;
        v = std::make_unique<EInt>(0, n.span);
      }
    }
    const std::string ctorClass = constructorClassFromExpr(*v);
    const std::string ctorFreeFn = constructorFreeFnFromExpr(*v);
    if (!declaredClass.empty()) {
      varClass_[n.text] = declaredClass;
    } else if (!ctorClass.empty()) {
      varClass_[n.text] = ctorClass;
    } else {
      varClass_.erase(n.text);
    }
    if (!ctorFreeFn.empty()) {
      varFreeFn_[n.text] = ctorFreeFn;
    } else {
      varFreeFn_.erase(n.text);
    }
    needStmtEnd("expected statement terminator after variable declaration");
    return std::make_unique<SLet>(n.text, d, isConst, isOwned, std::move(v), n.span);
  }

  SP sAssign() {
    Token n = need(TokenKind::Id, "expected variable name");
    TokenKind op = cur().kind;
    if (!isAssignOp(op)) {
      throw CompileError(cur().span, "expected assignment operator");
    }
    ++i_;
    EP v;
    if (op == TokenKind::Assign) {
      v = expr();
    } else {
      EP l = std::make_unique<EVar>(n.text, n.span);
      EP r = expr();
      v = makeBinaryExpr(assignOpToBinary(op, n.span), std::move(l), std::move(r), n.span);
    }
    if (op == TokenKind::Assign) {
      std::string assignedClass = constructorClassFromExpr(*v);
      std::string assignedFreeFn = constructorFreeFnFromExpr(*v);
      if (assignedClass.empty() && v->k == EK::Var) {
        const auto &rhsVar = static_cast<const EVar &>(*v);
        auto it = varClass_.find(rhsVar.n);
        if (it != varClass_.end()) assignedClass = it->second;
        auto itFree = varFreeFn_.find(rhsVar.n);
        if (itFree != varFreeFn_.end()) assignedFreeFn = itFree->second;
      }
      if (!assignedClass.empty())
        varClass_[n.text] = assignedClass;
      else
        varClass_.erase(n.text);
      if (!assignedFreeFn.empty())
        varFreeFn_[n.text] = assignedFreeFn;
      else
        varFreeFn_.erase(n.text);
    }
    needStmtEnd("expected statement terminator after assignment");
    return std::make_unique<SAssign>(n.text, std::move(v), n.span);
  }

  SP sAssignMember() {
    Token receiverTok = need(TokenKind::Id, "expected object receiver");
    need(TokenKind::Dot, "expected '.'");
    Token fieldTok = need(TokenKind::Id, "expected field name");
    TokenKind op = cur().kind;
    if (!isAssignOp(op)) throw CompileError(cur().span, "expected assignment operator");
    ++i_;

    const std::string className = resolveReceiverClass(receiverTok.text);
    if (className.empty()) {
      throw CompileError(receiverTok.span, "member assignment requires a class-typed receiver");
    }
    const FieldInfo *fieldInfo = findFieldRecursive(className, fieldTok.text);
    if (!fieldInfo) {
      throw CompileError(fieldTok.span, "class '" + className + "' has no field '" + fieldTok.text + "'");
    }
    if (!canAccessField(*fieldInfo, currentClass_)) {
      throw CompileError(fieldTok.span, "field '" + fieldTok.text + "' is not accessible in this context");
    }

    Type fieldType = fieldInfo->t;
    EP rhs;
    if (op == TokenKind::Assign) {
      rhs = expr();
    } else {
      EP lhs = makeFieldLoadExpr(receiverTok.text, fieldTok.text, fieldType, fieldTok.span);
      EP r = expr();
      rhs = makeBinaryExpr(assignOpToBinary(op, fieldTok.span), std::move(lhs), std::move(r), fieldTok.span);
    }
    SP out = makeFieldStoreStmt(receiverTok.text, fieldTok.text, fieldType, std::move(rhs), fieldTok.span);
    needStmtEnd("expected statement terminator after member assignment");
    return out;
  }

  SP sPostfixIncDec(bool isInc) {
    Token n = need(TokenKind::Id, "expected variable name");
    if (isInc)
      need(TokenKind::PlusPlus, "expected '++'");
    else
      need(TokenKind::MinusMinus, "expected '--'");
    EP l = std::make_unique<EVar>(n.text, n.span);
    EP r = std::make_unique<EInt>(1, n.span);
    EP v = makeBinaryExpr(isInc ? BK::Add : BK::Sub, std::move(l), std::move(r), n.span);
    needStmtEnd("expected statement terminator after increment/decrement");
    return std::make_unique<SAssign>(n.text, std::move(v), n.span);
  }

  SP sRet(const Span &s) {
    if (is(TokenKind::Semi) || is(TokenKind::Newline) || isStmtBoundary(cur().kind)) {
      needStmtEnd("expected statement terminator after return");
      return std::make_unique<SRet>(false, nullptr, s);
    }
    EP v = expr();
    needStmtEnd("expected statement terminator after return value");
    return std::make_unique<SRet>(true, std::move(v), s);
  }

  SP sIf(const Span &s) {
    EP c = expr();
    skipNl();
    if (eat(TokenKind::KwDo)) {
      std::vector<SP> t = blockDoUntil({TokenKind::KwElif, TokenKind::KwElse, TokenKind::KwEnd});
      return sIfDoTail(s, std::move(c), std::move(t));
    }
    auto t = block();
    std::vector<SP> e;
    skipNl();
    if (eat(TokenKind::KwElif)) {
      e.push_back(sIf(t_[i_ - 1].span));
    } else if (eat(TokenKind::KwElse)) {
      skipNl();
      e = block();
    }
    return std::make_unique<SIf>(std::move(c), std::move(t), std::move(e), s);
  }

  SP sUnless(const Span &s) {
    EP c = expr();
    EP neg = makeUnaryExpr(UK::Not, std::move(c), s);
    skipNl();
    if (eat(TokenKind::KwDo)) {
      std::vector<SP> t = blockDoUntil({TokenKind::KwElif, TokenKind::KwElse, TokenKind::KwEnd});
      return sIfDoTail(s, std::move(neg), std::move(t));
    }
    auto t = block();
    std::vector<SP> e;
    skipNl();
    if (eat(TokenKind::KwElif)) {
      e.push_back(sIf(t_[i_ - 1].span));
    } else if (eat(TokenKind::KwElse)) {
      skipNl();
      e = block();
    }
    return std::make_unique<SIf>(std::move(neg), std::move(t), std::move(e), s);
  }

  SP sWhile(const Span &s) {
    EP c = expr();
    skipNl();
    auto b = block();
    return std::make_unique<SWhile>(std::move(c), std::move(b), s);
  }

  SP sFor(const Span &s, bool isParallel) {
    Token n = need(TokenKind::Id, "expected loop variable name after 'for'");
    need(TokenKind::KwIn, "expected 'in' after loop variable");
    EP start = expr();
    need(TokenKind::DotDot, "expected '..' in for-range");
    EP stop = expr();
    EP step = std::make_unique<EInt>(1, n.span);
    if (eat(TokenKind::KwStep)) step = expr();
    skipNl();
    auto b = block();
    return std::make_unique<SFor>(n.text, std::move(start), std::move(stop), std::move(step), isParallel,
                                  std::move(b), s);
  }

  SP sFormatBlock(const Span &s, EP endArg) {
    auto b = block();
    return std::make_unique<SFormatBlock>(std::move(endArg), std::move(b), s);
  }

  SP sDelete() {
    Token deleteTok = need(TokenKind::Id, "expected 'delete'");
    bool arrayDelete = false;
    if (eat(TokenKind::LBra)) {
      need(TokenKind::RBra, "expected ']' after 'delete['");
      arrayDelete = true;
    }
    Token nameTok = need(TokenKind::Id, "expected variable name after delete/delete[]");
    std::string freeFn;
    auto itFree = varFreeFn_.find(nameTok.text);
    if (itFree != varFreeFn_.end()) freeFn = itFree->second;
    if (freeFn.empty()) {
      freeFn = varClass_.count(nameTok.text) ? "object_free" : "mem_free";
    }
    std::vector<EP> args;
    args.push_back(std::make_unique<EVar>(nameTok.text, nameTok.span));
    EP call = std::make_unique<ECall>(freeFn, std::move(args), deleteTok.span);
    needStmtEnd("expected statement terminator after delete/delete[]");
    (void)arrayDelete;
    return std::make_unique<SExpr>(std::move(call), deleteTok.span);
  }

  EP expr() { return logicOr(); }

  EP logicOr() {
    EP l = logicAnd();
    while (true) {
      if (eat(TokenKind::OrOr)) {
        EP r = logicAnd();
        l = makeBinaryExpr(BK::Or, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP logicAnd() {
    EP l = eq();
    while (true) {
      if (eat(TokenKind::AndAnd)) {
        EP r = eq();
        l = makeBinaryExpr(BK::And, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP eq() {
    EP l = cmp();
    while (true) {
      if (eat(TokenKind::Eq)) {
        EP r = cmp();
        l = makeBinaryExpr(BK::Eq, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Neq)) {
        EP r = cmp();
        l = makeBinaryExpr(BK::Neq, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP cmp() {
    EP l = term();
    while (true) {
      if (eat(TokenKind::Lt)) {
        EP r = term();
        l = makeBinaryExpr(BK::Lt, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Lte)) {
        EP r = term();
        l = makeBinaryExpr(BK::Lte, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Gt)) {
        EP r = term();
        l = makeBinaryExpr(BK::Gt, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Gte)) {
        EP r = term();
        l = makeBinaryExpr(BK::Gte, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP term() {
    EP l = fac();
    while (true) {
      if (eat(TokenKind::Plus)) {
        EP r = fac();
        l = makeBinaryExpr(BK::Add, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Minus)) {
        EP r = fac();
        l = makeBinaryExpr(BK::Sub, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP fac() {
    EP l = unary();
    while (true) {
      if (eat(TokenKind::Star)) {
        EP r = unary();
        l = makeBinaryExpr(BK::Mul, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Slash)) {
        EP r = unary();
        l = makeBinaryExpr(BK::Div, std::move(l), std::move(r), l->s);
        continue;
      }
      if (eat(TokenKind::Percent)) {
        EP r = unary();
        l = makeBinaryExpr(BK::Mod, std::move(l), std::move(r), l->s);
        continue;
      }
      break;
    }
    return l;
  }

  EP power() {
    EP l = call();
    if (eat(TokenKind::Pow)) {
      EP r = unary();
      l = makeBinaryExpr(BK::Pow, std::move(l), std::move(r), l->s);
    }
    return l;
  }

  EP unary() {
    if (eat(TokenKind::Minus)) {
      Token tok = t_[i_ - 1];
      return makeUnaryExpr(UK::Neg, unary(), tok.span);
    }
    if (eat(TokenKind::Bang)) {
      Token tok = t_[i_ - 1];
      return makeUnaryExpr(UK::Not, unary(), tok.span);
    }
    return power();
  }

  EP call() {
    EP e = primary();
    while (true) {
      if (eat(TokenKind::LPar)) {
        std::vector<EP> a;
        if (!is(TokenKind::RPar)) {
          while (true) {
            a.push_back(expr());
            if (!eat(TokenKind::Comma)) break;
          }
        }
        need(TokenKind::RPar, "expected ')'");
        if (e->k != EK::Var) throw CompileError(e->s, "callee must be function identifier");
        std::string fnName = canonicalSuperuserCallName(static_cast<EVar &>(*e).n);
        if (fnName == "expand") {
          if (a.size() != 1 || a[0]->k != EK::Call) {
            throw CompileError(e->s, "expand expects exactly one macro call argument: expand(my_macro(...))");
          }
          auto &macroCall = static_cast<ECall &>(*a[0]);
          const std::string macroName = macroCall.f;
          auto itMacro = macros_.find(macroName);
          if (itMacro == macros_.end()) {
            throw CompileError(macroCall.s, "unknown macro '" + macroName + "'");
          }
          const MacroDecl &md = itMacro->second;
          if (md.retKind != MacroArgKind::Expr || !md.bodyExpr) {
            throw CompileError(macroCall.s, "macro '" + macroName + "' is not an expression macro");
          }
          if (macroCall.a.size() != md.params.size()) {
            throw CompileError(
                macroCall.s, "macro '" + macroName + "' expects " + std::to_string(md.params.size()) + " args");
          }
          std::unordered_map<std::string, const Expr *> subs;
          for (std::size_t mi = 0; mi < md.params.size(); ++mi) {
            if (md.params[mi].kind != MacroArgKind::Expr) {
              throw CompileError(macroCall.s, "macro '" + macroName + "' currently supports expr params only");
            }
            subs[md.params[mi].name] = macroCall.a[mi].get();
          }
          e = substMacroExpr(*md.bodyExpr, subs);
          continue;
        }
        e = std::make_unique<ECall>(fnName, std::move(a), e->s);
        continue;
      }
      if (eat(TokenKind::Dot)) {
        Token memberTok = need(TokenKind::Id, "expected member name after '.'");
        if (eat(TokenKind::LPar)) {
          std::vector<EP> args;
          if (!is(TokenKind::RPar)) {
            while (true) {
              args.push_back(expr());
              if (!eat(TokenKind::Comma)) break;
            }
          }
          need(TokenKind::RPar, "expected ')'");
          if (e->k != EK::Var) {
            throw CompileError(memberTok.span, "method receiver must be an identifier");
          }
          const std::string receiver = static_cast<EVar &>(*e).n;
          bool classQualifier = false;
          std::string className;
          if (receiver != "this" && !varClass_.count(receiver) && classes_.count(receiver)) {
            classQualifier = true;
            className = receiver;
          } else {
            className = resolveReceiverClass(receiver);
          }
          if (className.empty()) {
            if (!isSuperuserNamespaceReceiver(receiver)) {
              throw CompileError(memberTok.span, "method call requires a class-typed receiver");
            }
            e = std::make_unique<ECall>(canonicalSuperuserNamespaceSymbol(receiver) + "." + memberTok.text,
                                        std::move(args), memberTok.span);
          } else {
            const MethodInfo *method = findMethodRecursive(className, memberTok.text, args.size());
            if (!method) {
              throw CompileError(memberTok.span,
                                 "class '" + className + "' has no matching method '" + memberTok.text + "'");
            }
            if (!canAccessMethod(*method, currentClass_)) {
              throw CompileError(memberTok.span, "method '" + memberTok.text + "' is not accessible in this context");
            }
            if (classQualifier && !method->isStatic) {
              throw CompileError(memberTok.span, "instance method '" + memberTok.text + "' requires an object receiver");
            }
            if (!classQualifier && method->isStatic) {
              throw CompileError(memberTok.span,
                                 "static method '" + memberTok.text + "' must be called via class name");
            }
            std::vector<EP> callArgs;
            callArgs.reserve(args.size() + (classQualifier ? 0 : 1));
            if (!classQualifier) {
              callArgs.push_back(std::make_unique<EVar>(receiver, memberTok.span));
            }
            for (auto &arg : args) callArgs.push_back(std::move(arg));
            e = std::make_unique<ECall>(method->symbol, std::move(callArgs), memberTok.span);
          }
        } else {
          if (e->k != EK::Var) {
            throw CompileError(memberTok.span, "field receiver must be an identifier");
          }
          const std::string receiver = static_cast<EVar &>(*e).n;
          const std::string className = resolveReceiverClass(receiver);
          if (className.empty()) {
            if (!isSuperuserNamespaceReceiver(receiver)) {
              throw CompileError(memberTok.span, "field access requires a class-typed receiver");
            }
            e = std::make_unique<EVar>(canonicalSuperuserNamespaceSymbol(receiver) + "." + memberTok.text,
                                       memberTok.span);
          } else {
            const FieldInfo *fieldInfo = findFieldRecursive(className, memberTok.text);
            if (!fieldInfo) {
              throw CompileError(memberTok.span, "class '" + className + "' has no field '" + memberTok.text + "'");
            }
            if (!canAccessField(*fieldInfo, currentClass_)) {
              throw CompileError(memberTok.span, "field '" + memberTok.text + "' is not accessible in this context");
            }
            e = makeFieldLoadExpr(receiver, memberTok.text, fieldInfo->t, memberTok.span);
          }
        }
        continue;
      }
      break;
    }
    return e;
  }

  EP primary() {
    if (eat(TokenKind::Str)) {
      Token tok = t_[i_ - 1];
      return std::make_unique<EString>(tok.text, tok.span);
    }
    if (eat(TokenKind::Int)) {
      Token tok = t_[i_ - 1];
      return std::make_unique<EInt>(std::stoll(tok.text), tok.span);
    }
    if (eat(TokenKind::Float)) {
      Token tok = t_[i_ - 1];
      return std::make_unique<EFloat>(std::stod(tok.text), tok.span);
    }
    if (eat(TokenKind::KwTrue)) {
      Token tok = t_[i_ - 1];
      return std::make_unique<EBool>(true, tok.span);
    }
    if (eat(TokenKind::KwFalse)) {
      Token tok = t_[i_ - 1];
      return std::make_unique<EBool>(false, tok.span);
    }
    if (eat(TokenKind::Id)) {
      Token tok = t_[i_ - 1];
      if (tok.text == "quote") {
        need(TokenKind::LBra, "expected '{' after quote");
        EP quoted = expr();
        need(TokenKind::RBra, "expected '}' after quote expression");
        return quoted;
      }
      return std::make_unique<EVar>(tok.text, tok.span);
    }
    if (eat(TokenKind::Dot)) {
      Token memberTok = need(TokenKind::Id, "expected identifier after '.'");
      return std::make_unique<EVar>("." + memberTok.text, memberTok.span);
    }
    if (eat(TokenKind::LPar)) {
      EP e = expr();
      need(TokenKind::RPar, "expected ')'");
      return e;
    }
    throw CompileError(cur().span, "expected expression");
  }
};

struct Sig {
  std::vector<Type> p;
  Type r = Type::Void;
  std::vector<std::string> throws;
};

struct OverloadCandidate {
  std::string symbol;
  Sig sig;
};

class TypeCheck {
public:
  explicit TypeCheck(Program &p, bool superuserMode = false) : p_(p), superuserMode_(superuserMode) {}
  void run() {
    collect();
    for (Fn &f : p_.f) {
      if (!f.ex) fn(f);
    }
  }
  const std::unordered_map<std::string, Sig> &sigs() const { return sig_; }
  const std::vector<std::string> &warnings() const { return warnings_; }
  bool superuserMode() const { return superuserMode_; }

private:
  Program &p_;
  bool superuserMode_ = false;
  std::unordered_map<std::string, Sig> sig_;
  std::unordered_map<std::string, std::vector<OverloadCandidate>> overloads_;
  std::vector<std::string> warnings_;
  struct Local {
    Type t = Type::I64;
    bool isConst = false;
    bool isOwned = false;
    std::string ownedFreeFn;
  };

  static bool can(Type a, Type b) {
    if (a == b) return true;
    if (isNum(a) && isNum(b)) return true;
    return false;
  }
  static bool canSafeWiden(Type from, Type to) {
    if (from == to) return true;
    if (from == Type::I32 && (to == Type::I64 || to == Type::F32 || to == Type::F64)) return true;
    if (from == Type::I64 && to == Type::F64) return true;
    if (from == Type::F32 && to == Type::F64) return true;
    return false;
  }
  static int conversionCost(Type from, Type to) {
    if (from == to) return 0;
    if (canSafeWiden(from, to)) return 1;
    return -1;
  }
  static Type promote(Type a, Type b) {
    if (isFloat(a) || isFloat(b)) {
      return (a == Type::F64 || b == Type::F64) ? Type::F64 : Type::F32;
    }
    return (a == Type::I64 || b == Type::I64) ? Type::I64 : Type::I32;
  }
  static bool isPrintable(Type t) {
    return t == Type::I32 || t == Type::I64 || t == Type::F32 || t == Type::F64 || t == Type::Bool ||
           t == Type::Str;
  }
  static bool isRawMemFunction(const std::string &name) {
    return name == "mem_alloc" || name == "mem_realloc" || name == "mem_free" || name == "mem_set" ||
           name == "mem_copy" || name == "mem_read_i64" || name == "mem_write_i64" || name == "mem_read_f64" ||
           name == "mem_write_f64";
  }
  static bool isLiteralZero(const Expr &e) {
    if (e.k == EK::Int) return static_cast<const EInt &>(e).v == 0;
    if (e.k == EK::Float) return static_cast<const EFloat &>(e).v == 0.0;
    return false;
  }
  static bool hasLoopControl(const std::vector<SP> &b) {
    for (const SP &stmt : b) {
      switch (stmt->k) {
      case SK::Break:
      case SK::Continue:
        return true;
      case SK::If: {
        const auto &n = static_cast<const SIf &>(*stmt);
        if (hasLoopControl(n.t) || hasLoopControl(n.e)) return true;
        break;
      }
      case SK::While: {
        const auto &n = static_cast<const SWhile &>(*stmt);
        if (hasLoopControl(n.b)) return true;
        break;
      }
      case SK::For: {
        const auto &n = static_cast<const SFor &>(*stmt);
        if (hasLoopControl(n.b)) return true;
        break;
      }
      case SK::FormatBlock: {
        const auto &n = static_cast<const SFormatBlock &>(*stmt);
        if (hasLoopControl(n.b)) return true;
        break;
      }
      default:
        break;
      }
    }
    return false;
  }
  static bool hasForbiddenAssign(const std::vector<SP> &b, const std::unordered_set<std::string> &forbidden) {
    for (const SP &stmt : b) {
      switch (stmt->k) {
      case SK::Assign: {
        const auto &n = static_cast<const SAssign &>(*stmt);
        if (forbidden.count(n.n)) return true;
        break;
      }
      case SK::If: {
        const auto &n = static_cast<const SIf &>(*stmt);
        if (hasForbiddenAssign(n.t, forbidden) || hasForbiddenAssign(n.e, forbidden)) return true;
        break;
      }
      case SK::While: {
        const auto &n = static_cast<const SWhile &>(*stmt);
        if (hasForbiddenAssign(n.b, forbidden)) return true;
        break;
      }
    case SK::For: {
      const auto &n = static_cast<const SFor &>(*stmt);
      if (hasForbiddenAssign(n.b, forbidden)) return true;
      break;
    }
    case SK::FormatBlock: {
      const auto &n = static_cast<const SFormatBlock &>(*stmt);
      if (hasForbiddenAssign(n.b, forbidden)) return true;
      break;
    }
    default:
      break;
    }
    }
    return false;
  }

  void req(bool ok, const Span &s, const std::string &m) {
    if (ok) return;
    if (superuserMode_) {
      warn(s, "superuser mode: " + m + " (continuing)");
      return;
    }
    throw CompileError(s, m);
  }
  void warn(const Span &s, const std::string &m) {
    std::ostringstream o;
    o << "line " << s.line << ", col " << s.col << ": warning: " << m;
    warnings_.push_back(o.str());
  }

  void addSig(const std::string &name, const std::vector<Type> &params, Type ret, const Span &s,
              std::vector<std::string> throws = {}, const std::string &alias = "") {
    if (sig_.count(name)) {
      if (superuserMode_) {
        warn(s, "superuser mode: duplicate function '" + name + "' (latest signature wins)");
      } else {
        throw CompileError(s, "duplicate function '" + name + "'");
      }
    }
    Sig sg;
    sg.p = params;
    sg.r = ret;
    sg.throws = std::move(throws);
    sig_[name] = std::move(sg);
    const std::string group = alias.empty() ? name : alias;
    overloads_[group].push_back(OverloadCandidate{name, sig_[name]});
  }

  void addBuiltins() {
    const Span s{};
    addSig("print_i64", {Type::I64}, Type::Void, s);
    addSig("print_f64", {Type::F64}, Type::Void, s);
    addSig("print_bool", {Type::Bool}, Type::Void, s);
    addSig("print_str", {Type::Str}, Type::Void, s);
    addSig("println_i64", {Type::I64}, Type::Void, s);
    addSig("println_f64", {Type::F64}, Type::Void, s);
    addSig("println_bool", {Type::Bool}, Type::Void, s);
    addSig("println_str", {Type::Str}, Type::Void, s);
    addSig("await", {Type::I64}, Type::Void, s);
    addSig("await_all", {}, Type::Void, s);
    addSig("clock_ms", {}, Type::I64, s);
    addSig("clock_us", {}, Type::I64, s);
    addSig("stateSpeed", {}, Type::Void, s);
    addSig(".stateSpeed", {}, Type::Void, s);
    addSig("superuser", {}, Type::Void, s);
    addSig(".format", {}, Type::Void, s);
    addSig(".freeConsole", {}, Type::Void, s);
    addSig("FreeConsole", {}, Type::Void, s);
    addSig("su.trace.on", {}, Type::Void, s);
    addSig("su.trace.off", {}, Type::Void, s);
    addSig("su.capabilities", {}, Type::Str, s);
    addSig("su.memory.inspect", {}, Type::Str, s);
    addSig("su.limit.set", {Type::I64, Type::I64}, Type::Void, s);
    addSig("su.compiler.inspect", {}, Type::Str, s);
    addSig("su.ir.dump", {}, Type::Void, s);
    addSig("su.debug.hook", {Type::Str}, Type::Void, s);
    addSig("http_server_listen", {Type::I64}, Type::I64, s);
    addSig("http_server_accept", {Type::I64}, Type::I64, s);
    addSig("http_server_read", {Type::I64}, Type::Str, s);
    addSig("http_server_respond_text", {Type::I64, Type::I64, Type::Str}, Type::Void, s);
    addSig("http_server_close", {Type::I64}, Type::Void, s);
    addSig("http_client_connect", {Type::Str, Type::I64}, Type::I64, s);
    addSig("http_client_send", {Type::I64, Type::Str}, Type::Void, s);
    addSig("http_client_read", {Type::I64}, Type::Str, s);
    addSig("http_client_close", {Type::I64}, Type::Void, s);
    addSig("input_i64", {Type::Str}, Type::I64, s);
    addSig("input_f64", {Type::Str}, Type::F64, s);
    addSig("cli_token_count", {}, Type::I64, s);
    addSig("cli_token", {Type::I64}, Type::Str, s);
    addSig("cli_has", {Type::Str}, Type::Bool, s);
    addSig("cli_value", {Type::Str}, Type::Str, s);
    addSig("includes", {Type::Str, Type::Str}, Type::Bool, s);
    addSig("len", {Type::Str}, Type::I64, s);
    addSig("bytes_len", {Type::Str}, Type::I64, s);
    addSig("is_empty", {Type::Str}, Type::Bool, s);
    addSig("contains", {Type::Str, Type::Str}, Type::Bool, s);
    addSig("starts_with", {Type::Str, Type::Str}, Type::Bool, s);
    addSig("ends_with", {Type::Str, Type::Str}, Type::Bool, s);
    addSig("find", {Type::Str, Type::Str}, Type::I64, s);
    addSig("replace", {Type::Str, Type::Str, Type::Str}, Type::Str, s);
    addSig("trim", {Type::Str}, Type::Str, s);
    addSig("lower", {Type::Str}, Type::Str, s);
    addSig("upper", {Type::Str}, Type::Str, s);
    addSig("substring", {Type::Str, Type::I64, Type::I64}, Type::Str, s);
    addSig("repeat", {Type::Str, Type::I64}, Type::Str, s);
    addSig("reverse", {Type::Str}, Type::Str, s);
    addSig("byte_at", {Type::Str, Type::I64}, Type::I64, s);
    addSig("ord", {Type::Str}, Type::I64, s);
    addSig("chr", {Type::I64}, Type::Str, s);
    addSig("mem_alloc", {Type::I64}, Type::I64, s);
    addSig("mem_realloc", {Type::I64, Type::I64}, Type::I64, s);
    addSig("mem_free", {Type::I64}, Type::Void, s);
    addSig("mem_set", {Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("mem_copy", {Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("mem_read_i64", {Type::I64}, Type::I64, s);
    addSig("mem_write_i64", {Type::I64, Type::I64}, Type::Void, s);
    addSig("mem_read_f64", {Type::I64}, Type::F64, s);
    addSig("mem_write_f64", {Type::I64, Type::F64}, Type::Void, s);
    addSig("array_new", {}, Type::I64, s);
    addSig("array_len", {Type::I64}, Type::I64, s);
    addSig("array_free", {Type::I64}, Type::Void, s);
    addSig("array_push", {Type::I64, Type::Str}, Type::Void, s);
    addSig("array_get", {Type::I64, Type::I64}, Type::Str, s);
    addSig("array_set", {Type::I64, Type::I64, Type::Str}, Type::Void, s);
    addSig("array_pop", {Type::I64}, Type::Str, s);
    addSig("array_join", {Type::I64, Type::Str}, Type::Str, s);
    addSig("array_includes", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("dict_new", {}, Type::I64, s);
    addSig("dict_len", {Type::I64}, Type::I64, s);
    addSig("dict_free", {Type::I64}, Type::Void, s);
    addSig("dict_set", {Type::I64, Type::Str, Type::Str}, Type::Void, s);
    addSig("dict_get", {Type::I64, Type::Str}, Type::Str, s);
    addSig("dict_has", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("dict_remove", {Type::I64, Type::Str}, Type::Void, s);
    addSig("map_new", {}, Type::I64, s);
    addSig("map_len", {Type::I64}, Type::I64, s);
    addSig("map_free", {Type::I64}, Type::Void, s);
    addSig("map_set", {Type::I64, Type::Str, Type::Str}, Type::Void, s);
    addSig("map_get", {Type::I64, Type::Str}, Type::Str, s);
    addSig("map_has", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("map_remove", {Type::I64, Type::Str}, Type::Void, s);
    addSig("object_new", {}, Type::I64, s);
    addSig("object_len", {Type::I64}, Type::I64, s);
    addSig("object_free", {Type::I64}, Type::Void, s);
    addSig("object_set", {Type::I64, Type::Str, Type::Str}, Type::Void, s);
    addSig("object_get", {Type::I64, Type::Str}, Type::Str, s);
    addSig("object_has", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("object_remove", {Type::I64, Type::Str}, Type::Void, s);
    addSig("option_some", {Type::Str}, Type::I64, s);
    addSig("option_none", {}, Type::I64, s);
    addSig("option_is_some", {Type::I64}, Type::Bool, s);
    addSig("option_is_none", {Type::I64}, Type::Bool, s);
    addSig("option_unwrap", {Type::I64}, Type::Str, s);
    addSig("option_unwrap_or", {Type::I64, Type::Str}, Type::Str, s);
    addSig("option_free", {Type::I64}, Type::Void, s);
    addSig("result_ok", {Type::Str}, Type::I64, s);
    addSig("result_err", {Type::Str, Type::Str}, Type::I64, s);
    addSig("result_is_ok", {Type::I64}, Type::Bool, s);
    addSig("result_is_err", {Type::I64}, Type::Bool, s);
    addSig("result_value", {Type::I64}, Type::Str, s);
    addSig("result_error_type", {Type::I64}, Type::Str, s);
    addSig("result_error_message", {Type::I64}, Type::Str, s);
    addSig("result_unwrap_or", {Type::I64, Type::Str}, Type::Str, s);
    addSig("result_free", {Type::I64}, Type::Void, s);
    addSig("gfx_new", {Type::I64, Type::I64}, Type::I64, s);
    addSig("gfx_free", {Type::I64}, Type::Void, s);
    addSig("gfx_width", {Type::I64}, Type::I64, s);
    addSig("gfx_height", {Type::I64}, Type::I64, s);
    addSig("gfx_clear", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("gfx_set", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("gfx_get", {Type::I64, Type::I64, Type::I64}, Type::I64, s);
    addSig("gfx_line", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64},
           Type::Void, s);
    addSig("gfx_rect",
           {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::Bool},
           Type::Void, s);
    addSig("gfx_save_ppm", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("game_new", {Type::I64, Type::I64, Type::Str, Type::Bool}, Type::I64, s);
    addSig("game_free", {Type::I64}, Type::Void, s);
    addSig("game_width", {Type::I64}, Type::I64, s);
    addSig("game_height", {Type::I64}, Type::I64, s);
    addSig("game_set_target_fps", {Type::I64, Type::I64}, Type::Void, s);
    addSig("game_set_fixed_dt", {Type::I64, Type::F64}, Type::Void, s);
    addSig("game_should_close", {Type::I64}, Type::Bool, s);
    addSig("game_begin", {Type::I64}, Type::Void, s);
    addSig("game_end", {Type::I64}, Type::Void, s);
    addSig("game_poll", {Type::I64}, Type::Void, s);
    addSig("game_present", {Type::I64}, Type::Void, s);
    addSig("game_delta", {Type::I64}, Type::F64, s);
    addSig("game_frame", {Type::I64}, Type::I64, s);
    addSig("game_mouse_x", {Type::I64}, Type::F64, s);
    addSig("game_mouse_y", {Type::I64}, Type::F64, s);
    addSig("game_mouse_norm_x", {Type::I64}, Type::F64, s);
    addSig("game_mouse_norm_y", {Type::I64}, Type::F64, s);
    addSig("game_scroll_x", {Type::I64}, Type::F64, s);
    addSig("game_scroll_y", {Type::I64}, Type::F64, s);
    addSig("game_mouse_down", {Type::I64, Type::I64}, Type::Bool, s);
    addSig("game_mouse_down_name", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("game_clear", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("game_set", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("game_get", {Type::I64, Type::I64, Type::I64}, Type::I64, s);
    addSig("game_line", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64},
           Type::Void, s);
    addSig("game_rect",
           {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::Bool},
           Type::Void, s);
    addSig("game_draw_gfx", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("game_save_ppm", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("game_checksum", {Type::I64}, Type::I64, s);
    addSig("pg_init", {Type::I64, Type::I64, Type::Str, Type::Bool}, Type::I64, s);
    addSig("pg_quit", {Type::I64}, Type::Void, s);
    addSig("pg_should_quit", {Type::I64}, Type::Bool, s);
    addSig("pg_begin", {Type::I64}, Type::Void, s);
    addSig("pg_end", {Type::I64}, Type::Void, s);
    addSig("pg_set_target_fps", {Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_set_fixed_dt", {Type::I64, Type::F64}, Type::Void, s);
    addSig("pg_clear", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_draw_pixel", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_draw_line", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64},
           Type::Void, s);
    addSig("pg_draw_rect",
           {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::Bool},
           Type::Void, s);
    addSig("pg_blit", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_get_pixel", {Type::I64, Type::I64, Type::I64}, Type::I64, s);
    addSig("pg_save_ppm", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("pg_checksum", {Type::I64}, Type::I64, s);
    addSig("pg_mouse_x", {Type::I64}, Type::F64, s);
    addSig("pg_mouse_y", {Type::I64}, Type::F64, s);
    addSig("pg_mouse_norm_x", {Type::I64}, Type::F64, s);
    addSig("pg_mouse_norm_y", {Type::I64}, Type::F64, s);
    addSig("pg_scroll_x", {Type::I64}, Type::F64, s);
    addSig("pg_scroll_y", {Type::I64}, Type::F64, s);
    addSig("pg_delta", {Type::I64}, Type::F64, s);
    addSig("pg_frame", {Type::I64}, Type::I64, s);
    addSig("pg_key_down", {Type::I64}, Type::Bool, s);
    addSig("pg_key_down_name", {Type::Str}, Type::Bool, s);
    addSig("pg_mouse_down", {Type::I64, Type::I64}, Type::Bool, s);
    addSig("pg_mouse_down_name", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("pg_surface_new", {Type::I64, Type::I64}, Type::I64, s);
    addSig("pg_surface_free", {Type::I64}, Type::Void, s);
    addSig("pg_surface_clear", {Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_surface_set", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64}, Type::Void, s);
    addSig("pg_surface_get", {Type::I64, Type::I64, Type::I64}, Type::I64, s);
    addSig("pg_surface_line", {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64},
           Type::Void, s);
    addSig("pg_surface_rect",
           {Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::I64, Type::Bool},
           Type::Void, s);
    addSig("pg_surface_save_ppm", {Type::I64, Type::Str}, Type::Bool, s);
    addSig("np_new", {Type::I64}, Type::I64, s);
    addSig("np_free", {Type::I64}, Type::Void, s);
    addSig("np_len", {Type::I64}, Type::I64, s);
    addSig("np_get", {Type::I64, Type::I64}, Type::F64, s);
    addSig("np_set", {Type::I64, Type::I64, Type::F64}, Type::Void, s);
    addSig("np_copy", {Type::I64}, Type::I64, s);
    addSig("np_fill", {Type::I64, Type::F64}, Type::Void, s);
    addSig("np_from_range", {Type::F64, Type::F64, Type::F64}, Type::I64, s);
    addSig("np_linspace", {Type::F64, Type::F64, Type::I64}, Type::I64, s);
    addSig("np_sum", {Type::I64}, Type::F64, s);
    addSig("np_mean", {Type::I64}, Type::F64, s);
    addSig("np_min", {Type::I64}, Type::F64, s);
    addSig("np_max", {Type::I64}, Type::F64, s);
    addSig("np_dot", {Type::I64, Type::I64}, Type::F64, s);
    addSig("np_add", {Type::I64, Type::I64}, Type::I64, s);
    addSig("np_sub", {Type::I64, Type::I64}, Type::I64, s);
    addSig("np_mul", {Type::I64, Type::I64}, Type::I64, s);
    addSig("np_div", {Type::I64, Type::I64}, Type::I64, s);
    addSig("np_add_scalar", {Type::I64, Type::F64}, Type::Void, s);
    addSig("np_mul_scalar", {Type::I64, Type::F64}, Type::Void, s);
    addSig("np_clip", {Type::I64, Type::F64, Type::F64}, Type::Void, s);
    addSig("np_abs", {Type::I64}, Type::Void, s);
    addSig("phys_new", {Type::F64, Type::F64, Type::F64, Type::F64, Type::Bool}, Type::I64, s);
    addSig("phys_free", {Type::I64}, Type::Void, s);
    addSig("phys_set_position", {Type::I64, Type::F64, Type::F64, Type::F64}, Type::Void, s);
    addSig("phys_set_velocity", {Type::I64, Type::F64, Type::F64, Type::F64}, Type::Void, s);
    addSig("phys_move", {Type::I64, Type::F64, Type::F64, Type::F64}, Type::Void, s);
    addSig("phys_apply_force", {Type::I64, Type::F64, Type::F64, Type::F64}, Type::Void, s);
    addSig("phys_step", {Type::F64}, Type::Void, s);
    addSig("phys_get_x", {Type::I64}, Type::F64, s);
    addSig("phys_get_y", {Type::I64}, Type::F64, s);
    addSig("phys_get_z", {Type::I64}, Type::F64, s);
    addSig("phys_get_vx", {Type::I64}, Type::F64, s);
    addSig("phys_get_vy", {Type::I64}, Type::F64, s);
    addSig("phys_get_vz", {Type::I64}, Type::F64, s);
    addSig("phys_is_soft", {Type::I64}, Type::Bool, s);
    addSig("camera_bind", {Type::I64}, Type::Void, s);
    addSig("camera_target", {}, Type::I64, s);
    addSig("camera_set_offset", {Type::F64, Type::F64, Type::F64}, Type::Void, s);
    addSig("camera_get_x", {}, Type::F64, s);
    addSig("camera_get_y", {}, Type::F64, s);
    addSig("camera_get_z", {}, Type::F64, s);
    addSig("key_down", {Type::I64}, Type::Bool, s);
    addSig("key_down_name", {Type::Str}, Type::Bool, s);
    addSig("parse_i64", {Type::Str}, Type::I64, s);
    addSig("parse_f64", {Type::Str}, Type::F64, s);
    addSig("bool_to_i64", {Type::Bool}, Type::I64, s);
    addSig("i64_to_bool", {Type::I64}, Type::Bool, s);
    addSig("to_i32", {Type::I64}, Type::I32, s);
    addSig("to_f32", {Type::F64}, Type::F32, s);
    addSig("to_i64", {Type::F64}, Type::I64, s);
    addSig("to_f64", {Type::I64}, Type::F64, s);
    addSig("gcd", {Type::I64, Type::I64}, Type::I64, s);
    addSig("lcm", {Type::I64, Type::I64}, Type::I64, s);
    addSig("max_i64", {Type::I64, Type::I64}, Type::I64, s);
    addSig("min_i64", {Type::I64, Type::I64}, Type::I64, s);
    addSig("abs_i64", {Type::I64}, Type::I64, s);
    addSig("clamp_i64", {Type::I64, Type::I64, Type::I64}, Type::I64, s);
    addSig("max_f64", {Type::F64, Type::F64}, Type::F64, s);
    addSig("min_f64", {Type::F64, Type::F64}, Type::F64, s);
    addSig("abs_f64", {Type::F64}, Type::F64, s);
    addSig("clamp_f64", {Type::F64, Type::F64, Type::F64}, Type::F64, s);
    addSig("pi", {}, Type::F64, s);
    addSig("tau", {}, Type::F64, s);
    addSig("deg_to_rad", {Type::F64}, Type::F64, s);
    addSig("rad_to_deg", {Type::F64}, Type::F64, s);
    addSig("sqrt", {Type::F64}, Type::F64, s);
    addSig("sin", {Type::F64}, Type::F64, s);
    addSig("cos", {Type::F64}, Type::F64, s);
    addSig("tan", {Type::F64}, Type::F64, s);
    addSig("asin", {Type::F64}, Type::F64, s);
    addSig("acos", {Type::F64}, Type::F64, s);
    addSig("atan", {Type::F64}, Type::F64, s);
    addSig("atan2", {Type::F64, Type::F64}, Type::F64, s);
    addSig("exp", {Type::F64}, Type::F64, s);
    addSig("log", {Type::F64}, Type::F64, s);
    addSig("log10", {Type::F64}, Type::F64, s);
    addSig("floor", {Type::F64}, Type::F64, s);
    addSig("ceil", {Type::F64}, Type::F64, s);
    addSig("round", {Type::F64}, Type::F64, s);
    addSig("pow", {Type::F64, Type::F64}, Type::F64, s);
  }

  void collect() {
    addBuiltins();
    for (const Fn &f : p_.f) {
      std::vector<Type> params;
      for (const Param &p : f.p) params.push_back(p.t);
      const std::string alias = f.sourceName.empty() ? f.n : f.sourceName;
      addSig(f.n, params, f.ret, f.s, f.throws, alias);
    }
  }

  std::optional<OverloadCandidate> resolveOverload(const std::string &name, const std::vector<Type> &argTypes,
                                                    const Span &s) {
    auto itGroup = overloads_.find(name);
    if (itGroup == overloads_.end()) return std::nullopt;
    const auto &cands = itGroup->second;
    int bestCost = std::numeric_limits<int>::max();
    const OverloadCandidate *best = nullptr;
    bool ambiguous = false;
    for (const auto &cand : cands) {
      if (cand.sig.p.size() != argTypes.size()) continue;
      int cost = 0;
      bool ok = true;
      for (std::size_t i = 0; i < argTypes.size(); ++i) {
        const int c = conversionCost(argTypes[i], cand.sig.p[i]);
        if (c < 0) {
          ok = false;
          break;
        }
        cost += c;
      }
      if (!ok) continue;
      if (cost < bestCost) {
        bestCost = cost;
        best = &cand;
        ambiguous = false;
      } else if (cost == bestCost) {
        ambiguous = true;
      }
    }
    if (!best) return std::nullopt;
    if (ambiguous) {
      if (superuserMode_) {
        warn(s, "superuser mode: ambiguous overload for '" + name + "'; keeping first candidate");
      } else {
        throw CompileError(s, "ambiguous overload for function '" + name + "'");
      }
    }
    return *best;
  }

  void fn(Fn &f) {
    std::unordered_map<std::string, Local> l;
    std::unordered_set<std::string> throwsAllowed;
    throwsAllowed.reserve(f.throws.size());
    for (const std::string &e : f.throws) throwsAllowed.insert(e);
    for (const Param &p : f.p) {
      if (l.count(p.n)) {
        if (superuserMode_) {
          warn(f.s, "superuser mode: duplicate parameter '" + p.n + "' (latest declaration wins)");
        } else {
          throw CompileError(f.s, "duplicate parameter '" + p.n + "'");
        }
      }
      l[p.n] = Local{p.t, false, false, ""};
    }
    block(f.b, l, f.ret, 0, throwsAllowed);
  }

  void block(std::vector<SP> &b, std::unordered_map<std::string, Local> l, Type ret, int loopDepth,
             const std::unordered_set<std::string> &throwsAllowed) {
    for (SP &s : b) stmt(*s, l, ret, loopDepth, throwsAllowed);
  }

  static std::string ownedFreeFnForCtor(const Expr &initExpr) {
    if (initExpr.k != EK::Call) return "";
    const auto &c = static_cast<const ECall &>(initExpr);
    if (c.f == "array_new") return "array_free";
    if (c.f == "dict_new") return "dict_free";
    if (c.f == "map_new") return "map_free";
    if (c.f == "object_new") return "object_free";
    if (c.f == "np_new" || c.f == "np_copy" || c.f == "np_from_range" || c.f == "np_linspace") return "np_free";
    if (c.f == "gfx_new" || c.f == "pg_surface_new") return "gfx_free";
    if (c.f == "game_new" || c.f == "pg_init") return "game_free";
    if (c.f == "phys_new") return "phys_free";
    if (c.f == "http_server_listen") return "http_server_close";
    if (c.f == "http_client_connect") return "http_client_close";
    if (c.f == "result_ok" || c.f == "result_err") return "result_free";
    if (c.f == "option_some" || c.f == "option_none") return "option_free";
    return "";
  }

  void stmt(Stmt &s, std::unordered_map<std::string, Local> &l, Type ret, int loopDepth,
            const std::unordered_set<std::string> &throwsAllowed) {
    switch (s.k) {
    case SK::Let: {
      auto &n = static_cast<SLet &>(s);
      req(!l.count(n.n), s.s, "variable '" + n.n + "' already declared");
      Type vt = expr(*n.v, l, throwsAllowed), ft = n.decl.has_value() ? *n.decl : vt;
      req(can(vt, ft), s.s, "cannot convert '" + typeName(vt) + "' to '" + typeName(ft) + "'");
      if (n.isOwned) {
        req(loopDepth == 0, s.s, "declare owned is not allowed inside loops");
        req(ft == Type::I64, s.s, "declare owned requires i64 handle type");
        n.ownedFreeFn = ownedFreeFnForCtor(*n.v);
        req(!n.ownedFreeFn.empty(), s.s,
            "declare owned requires constructor call that has a matching free function");
      }
      n.inf = ft;
      n.typed = true;
      l[n.n] = Local{ft, n.isConst, n.isOwned, n.ownedFreeFn};
      return;
    }
    case SK::Assign: {
      auto &n = static_cast<SAssign &>(s);
      req(l.count(n.n), s.s, "assignment to undeclared variable '" + n.n + "'");
      Type vt = expr(*n.v, l, throwsAllowed);
      req(!l[n.n].isConst, s.s, "cannot assign to const variable '" + n.n + "'");
      req(!l[n.n].isOwned, s.s,
          "cannot assign to owned handle '" + n.n + "'; release explicitly and declare a new owned handle");
      req(can(vt, l[n.n].t), s.s, "cannot assign '" + typeName(vt) + "' to '" + typeName(l[n.n].t) + "'");
      return;
    }
    case SK::Expr: {
      (void)expr(*static_cast<SExpr &>(s).e, l, throwsAllowed);
      return;
    }
    case SK::Ret: {
      auto &n = static_cast<SRet &>(s);
      if (!n.has) {
        req(ret == Type::Void, s.s, "missing return value");
        return;
      }
      if (n.v && n.v->k == EK::Var) {
        const auto &rv = static_cast<const EVar &>(*n.v);
        auto it = l.find(rv.n);
        if (it != l.end() && it->second.isOwned) {
          req(false, s.s, "cannot return owned handle variable '" + rv.n + "'");
        }
      }
      Type vt = expr(*n.v, l, throwsAllowed);
      req(can(vt, ret), s.s, "cannot return '" + typeName(vt) + "' from '" + typeName(ret) + "'");
      return;
    }
    case SK::If: {
      auto &n = static_cast<SIf &>(s);
      req(expr(*n.c, l, throwsAllowed) == Type::Bool, s.s, "if condition must be bool");
      block(n.t, l, ret, loopDepth, throwsAllowed);
      block(n.e, l, ret, loopDepth, throwsAllowed);
      return;
    }
    case SK::While: {
      auto &n = static_cast<SWhile &>(s);
      req(expr(*n.c, l, throwsAllowed) == Type::Bool, s.s, "while condition must be bool");
      block(n.b, l, ret, loopDepth + 1, throwsAllowed);
      return;
    }
    case SK::For: {
      auto &n = static_cast<SFor &>(s);
      Type st = expr(*n.start, l, throwsAllowed);
      Type en = expr(*n.stop, l, throwsAllowed);
      Type sp = expr(*n.step, l, throwsAllowed);
      req(st == Type::I64, s.s, "for range start must be i64");
      req(en == Type::I64, s.s, "for range stop must be i64");
      req(sp == Type::I64, s.s, "for range step must be i64");
      if (n.step->k == EK::Int && static_cast<EInt &>(*n.step).v == 0) {
        if (superuserMode_) {
          warn(s.s, "superuser mode: allowing for-range literal step 0 (loop becomes no-op)");
        } else {
          throw CompileError(s.s, "for range step cannot be zero");
        }
      }
      if (n.parallel && hasLoopControl(n.b)) {
        if (superuserMode_) {
          warn(s.s, "superuser mode: allowing break/continue inside parallel for");
        } else {
          throw CompileError(s.s, "parallel for does not support break/continue");
        }
      }
      if (n.parallel) {
        std::unordered_set<std::string> forbidden;
        forbidden.reserve(l.size() + 1);
        for (const auto &[name, _] : l) forbidden.insert(name);
        forbidden.insert(n.n);
        if (hasForbiddenAssign(n.b, forbidden)) {
          if (superuserMode_) {
            warn(s.s, "superuser mode: allowing outer-variable writes in parallel for");
          } else {
            throw CompileError(s.s, "parallel for cannot assign to outer variables");
          }
        }
      }
      auto inner = l;
      inner[n.n] = Local{Type::I64, false, false, ""};
      block(n.b, inner, ret, loopDepth + 1, throwsAllowed);
      return;
    }
    case SK::FormatBlock: {
      auto &n = static_cast<SFormatBlock &>(s);
      if (n.endArg) {
        Type et = expr(*n.endArg, l, throwsAllowed);
        req(et == Type::Str, s.s, "formatOutput block end argument must be str");
      }
      block(n.b, l, ret, loopDepth, throwsAllowed);
      return;
    }
    case SK::Break: {
      req(loopDepth > 0, s.s, "'break' can only be used inside loops");
      return;
    }
    case SK::Continue: {
      req(loopDepth > 0, s.s, "'continue' can only be used inside loops");
      return;
    }
    }
  }

  Type expr(Expr &e, const std::unordered_map<std::string, Local> &l,
            const std::unordered_set<std::string> &throwsAllowed) {
    auto mark = [&](Type t) -> Type {
      e.inf = t;
      e.typed = true;
      return t;
    };
    auto failType = [&](const Span &s, const std::string &m, Type fallback) -> Type {
      if (superuserMode_) {
        warn(s, "superuser mode: " + m + " (continuing)");
        return mark(fallback);
      }
      throw CompileError(s, m);
    };
    auto failVoid = [&](const Span &s, const std::string &m) {
      if (superuserMode_) {
        warn(s, "superuser mode: " + m + " (continuing)");
        return;
      }
      throw CompileError(s, m);
    };
    switch (e.k) {
    case EK::Int: return mark(Type::I64);
    case EK::Float: return mark(Type::F64);
    case EK::Bool: return mark(Type::Bool);
    case EK::Str: return mark(Type::Str);
    case EK::Var: {
      auto &n = static_cast<EVar &>(e);
      auto it = l.find(n.n);
      if (it == l.end()) return failType(e.s, "unknown variable '" + n.n + "'", Type::I64);
      return mark(it->second.t);
    }
    case EK::Unary: {
      auto &n = static_cast<EUnary &>(e);
      Type t = expr(*n.x, l, throwsAllowed);
      auto tryResolvedUnaryOperator = [&](const std::string &sym) -> std::optional<Type> {
        if (sym.empty()) return std::nullopt;
        auto itOp = sig_.find(sym);
        if (itOp == sig_.end()) return std::nullopt;
        const Sig &sg = itOp->second;
        if (sg.p.size() != 1) {
          if (superuserMode_) {
            warn(e.s, "superuser mode: unary operator override '" + unaryOperatorSymbolText(n.op) +
                           "' has invalid arity (continuing)");
          } else {
            throw CompileError(e.s, "unary operator override '" + unaryOperatorSymbolText(n.op) +
                                         "' must have 1 parameter");
          }
          return std::nullopt;
        }
        if (!can(t, sg.p[0])) return std::nullopt;
        n.overrideFn = sym;
        return sg.r;
      };

      if (!n.overrideFn.empty()) {
        if (auto memberRt = tryResolvedUnaryOperator(n.overrideFn)) {
          return mark(*memberRt);
        }
        n.overrideFn.clear();
      }
      const std::string unaryOverride = unaryOperatorOverrideSymbol(n.op);
      if (auto freeRt = tryResolvedUnaryOperator(unaryOverride)) {
        return mark(*freeRt);
      }
      if (n.op == UK::Neg) {
        if (!isNum(t)) return failType(e.s, "unary '-' requires numeric", Type::I64);
        return mark(t);
      }
      if (t != Type::Bool) return failType(e.s, "unary '!' requires bool", Type::Bool);
      return mark(Type::Bool);
    }
    case EK::Binary: {
      auto &n = static_cast<EBinary &>(e);
      Type a = expr(*n.l, l, throwsAllowed), b = expr(*n.r, l, throwsAllowed);
      auto tryResolvedOperator = [&](const std::string &sym) -> std::optional<Type> {
        if (sym.empty()) return std::nullopt;
        auto itOp = sig_.find(sym);
        if (itOp == sig_.end()) return std::nullopt;
        const Sig &sg = itOp->second;
        if (sg.p.size() != 2) {
          if (superuserMode_) {
            warn(e.s, "superuser mode: operator override '" + operatorSymbolText(n.op) +
                           "' has invalid arity (continuing)");
          } else {
            throw CompileError(e.s, "operator override '" + operatorSymbolText(n.op) + "' must have 2 parameters");
          }
          return std::nullopt;
        }
        if (!can(a, sg.p[0]) || !can(b, sg.p[1])) return std::nullopt;
        n.overrideFn = sym;
        return sg.r;
      };

      if (!n.overrideFn.empty()) {
        if (auto memberRt = tryResolvedOperator(n.overrideFn)) {
          return mark(*memberRt);
        }
        n.overrideFn.clear();
      }

      const std::string opOverride = operatorOverrideSymbol(n.op);
      if (!opOverride.empty()) {
        if (auto freeRt = tryResolvedOperator(opOverride)) {
          return mark(*freeRt);
        }
      }
      switch (n.op) {
      case BK::Add:
      case BK::Sub:
      case BK::Mul:
        if (!isNum(a) || !isNum(b)) return failType(e.s, "arithmetic requires numeric", Type::I64);
        return mark(promote(a, b));
      case BK::Div:
        if (!isNum(a) || !isNum(b)) return failType(e.s, "arithmetic requires numeric", Type::I64);
        if (isLiteralZero(*n.r)) {
          if (superuserMode_) {
            warn(n.r->s, "superuser mode: allowing division by zero expression");
          } else {
            throw CompileError(n.r->s, "division by zero");
          }
        }
        return mark(promote(a, b));
      case BK::Mod:
        if (!isInt(a) || !isInt(b)) return failType(e.s, "'%' requires integer operands", Type::I64);
        if (isLiteralZero(*n.r)) {
          if (superuserMode_) {
            warn(n.r->s, "superuser mode: allowing modulo by zero expression");
          } else {
            throw CompileError(n.r->s, "modulo by zero");
          }
        }
        return mark(promote(a, b));
      case BK::Pow:
        if (!isNum(a) || !isNum(b)) return failType(e.s, "power operator requires numeric", Type::I64);
        return mark(promote(a, b));
      case BK::Lt:
      case BK::Lte:
      case BK::Gt:
      case BK::Gte:
        if (!isNum(a) || !isNum(b)) return failType(e.s, "comparison requires numeric", Type::Bool);
        return mark(Type::Bool);
      case BK::And:
      case BK::Or:
        if (a != Type::Bool || b != Type::Bool) return failType(e.s, "logical operators require bool", Type::Bool);
        return mark(Type::Bool);
      case BK::Eq:
      case BK::Neq:
        if ((a == b) || (isNum(a) && isNum(b))) return mark(Type::Bool);
        return failType(e.s, "cannot compare '" + typeName(a) + "' with '" + typeName(b) + "'", Type::Bool);
      }
    }
    case EK::Call: {
      auto &n = static_cast<ECall &>(e);
      const std::string fnName = canonicalSuperuserCallName(n.f);
      if (isRawMemFunction(fnName)) {
        warn(e.s, "raw memory API '" + fnName +
                      "' used directly; prefer typed ptr/slice wrappers and isolate raw access to audited blocks");
      }
      if (fnName == "superuser") {
        if (!n.a.empty()) failVoid(e.s, "function 'superuser' expects 0 args");
        return mark(Type::Void);
      }
      if (fnName.rfind("su.", 0) == 0 && !superuserMode_) {
        throw CompileError(e.s, "Not privileged: call superuser() to enable developer superuser mode");
      }
      if (fnName == "input") {
        if (n.a.empty()) return mark(Type::Str);
        if (n.a.size() == 1) {
          Type at = expr(*n.a[0], l, throwsAllowed);
          if (at != Type::Str) {
            failVoid(n.a[0]->s, "arg 1 cannot convert '" + typeName(at) + "' to 'str'");
          }
          return mark(Type::Str);
        }
        failVoid(e.s, "function 'input' expects 0 or 1 args");
        return mark(Type::Str);
      }
      if (fnName == "input_i64") {
        if (n.a.empty()) return mark(Type::I64);
        if (n.a.size() == 1) {
          Type at = expr(*n.a[0], l, throwsAllowed);
          if (at != Type::Str) {
            failVoid(n.a[0]->s, "arg 1 cannot convert '" + typeName(at) + "' to 'str'");
          }
          return mark(Type::I64);
        }
        failVoid(e.s, "function 'input_i64' expects 0 or 1 args");
        return mark(Type::I64);
      }
      if (fnName == "input_f64") {
        if (n.a.empty()) return mark(Type::F64);
        if (n.a.size() == 1) {
          Type at = expr(*n.a[0], l, throwsAllowed);
          if (at != Type::Str) {
            failVoid(n.a[0]->s, "arg 1 cannot convert '" + typeName(at) + "' to 'str'");
          }
          return mark(Type::F64);
        }
        failVoid(e.s, "function 'input_f64' expects 0 or 1 args");
        return mark(Type::F64);
      }
      if (fnName == "print" || fnName == "println") {
        if (n.a.size() != 1) {
          failVoid(e.s, "function '" + n.f + "' expects 1 arg");
          return mark(Type::Void);
        }
        Type at = expr(*n.a[0], l, throwsAllowed);
        if (!isPrintable(at)) {
          failVoid(n.a[0]->s, "arg 1 cannot print type '" + typeName(at) + "'");
        }
        return mark(Type::Void);
      }
      if (fnName == "formatOutput" || fnName == "FormatOutput") {
        if (n.a.size() != 1) {
          failVoid(e.s, "function '" + n.f + "' expects 1 arg");
          return mark(Type::Str);
        }
        Type at = expr(*n.a[0], l, throwsAllowed);
        if (!isPrintable(at)) {
          failVoid(n.a[0]->s, "arg 1 cannot format type '" + typeName(at) + "'");
        }
        return mark(Type::Str);
      }
      if (fnName == "max" || fnName == "min") {
        if (n.a.size() != 2) {
          failVoid(e.s, "function '" + n.f + "' expects 2 args");
          return mark(Type::I64);
        }
        Type a0 = expr(*n.a[0], l, throwsAllowed);
        Type a1 = expr(*n.a[1], l, throwsAllowed);
        if (!isNum(a0) || !isNum(a1)) {
          failVoid(e.s, "function '" + n.f + "' requires numeric args");
          return mark(Type::I64);
        }
        return mark(promote(a0, a1));
      }
      if (fnName == "abs") {
        if (n.a.size() != 1) {
          failVoid(e.s, "function '" + n.f + "' expects 1 arg");
          return mark(Type::I64);
        }
        Type a0 = expr(*n.a[0], l, throwsAllowed);
        if (!isNum(a0)) {
          failVoid(e.s, "function '" + n.f + "' requires numeric args");
          return mark(Type::I64);
        }
        return mark(a0);
      }
      if (fnName == "clamp") {
        if (n.a.size() != 3) {
          failVoid(e.s, "function '" + n.f + "' expects 3 args");
          return mark(Type::I64);
        }
        Type a0 = expr(*n.a[0], l, throwsAllowed);
        Type a1 = expr(*n.a[1], l, throwsAllowed);
        Type a2 = expr(*n.a[2], l, throwsAllowed);
        if (!isNum(a0) || !isNum(a1) || !isNum(a2)) {
          failVoid(e.s, "function '" + n.f + "' requires numeric args");
          return mark(Type::I64);
        }
        return mark(promote(promote(a0, a1), a2));
      }
      if (fnName == ".format") {
        if (!n.a.empty()) failVoid(e.s, "function '" + n.f + "' expects 0 args");
        return mark(Type::Void);
      }
      if (fnName == "spawn") {
        if (n.a.size() != 1) {
          failVoid(e.s, "function 'spawn' expects 1 arg");
          return mark(Type::I64);
        }
        if (n.a[0]->k != EK::Call) {
          failVoid(n.a[0]->s, "spawn expects a function call like spawn(worker())");
          return mark(Type::I64);
        }
        auto &target = static_cast<ECall &>(*n.a[0]);
        const std::string targetFnName = canonicalSuperuserCallName(target.f);
        if (!target.a.empty()) {
          failVoid(target.s, "spawn target must not take arguments");
        }
        std::optional<OverloadCandidate> targetCand = resolveOverload(targetFnName, {}, target.s);
        if (!targetCand.has_value()) {
          auto itTarget = sig_.find(targetFnName);
          if (itTarget == sig_.end()) {
            failVoid(target.s, "unknown function '" + target.f + "'");
            return mark(Type::I64);
          }
          targetCand = OverloadCandidate{targetFnName, itTarget->second};
        }
        target.f = targetCand->symbol;
        if (!targetCand->sig.p.empty()) {
          failVoid(target.s, "spawn target must have zero parameters");
        }
        if (targetCand->sig.r != Type::Void) {
          failVoid(target.s, "spawn target must return void");
        }
        return mark(Type::I64);
      }
      std::vector<Type> argTypes;
      argTypes.reserve(n.a.size());
      for (auto &arg : n.a) argTypes.push_back(expr(*arg, l, throwsAllowed));
      std::optional<OverloadCandidate> cand = resolveOverload(fnName, argTypes, e.s);
      if (!cand.has_value()) {
        auto it = sig_.find(fnName);
        if (it == sig_.end()) return failType(e.s, "unknown function '" + n.f + "'", Type::I64);
        cand = OverloadCandidate{fnName, it->second};
      }
      n.f = cand->symbol;
      const Sig &sg = cand->sig;
      for (const std::string &errName : sg.throws) {
        if (!throwsAllowed.count(errName)) {
          if (superuserMode_) {
            warn(e.s, "superuser mode: bypassing throws contract for '" + n.f + "' and '" + errName + "'");
          } else {
            throw CompileError(e.s, "call to '" + n.f + "' may throw '" + errName +
                                         "'; add 'throws " + errName + "' to the current function");
          }
        }
      }
      if (sg.p.size() != n.a.size()) {
        failVoid(e.s, "function '" + n.f + "' expects " + std::to_string(sg.p.size()) + " args");
      }
      for (std::size_t i = 0; i < n.a.size(); ++i) {
        if (i >= sg.p.size()) break;
        Type at = argTypes[i], pt = sg.p[i];
        if (!can(at, pt))
          failVoid(n.a[i]->s, "arg " + std::to_string(i + 1) + " cannot convert '" + typeName(at) +
                                 "' to '" + typeName(pt) + "'");
      }
      return mark(sg.r);
    }
    }
    return failType(e.s, "internal type error", Type::I64);
  }
};

static EP cloneE(const Expr &e) {
  switch (e.k) {
  case EK::Int: return std::make_unique<EInt>(static_cast<const EInt &>(e).v, e.s);
  case EK::Float: return std::make_unique<EFloat>(static_cast<const EFloat &>(e).v, e.s);
  case EK::Bool: return std::make_unique<EBool>(static_cast<const EBool &>(e).v, e.s);
  case EK::Str: return std::make_unique<EString>(static_cast<const EString &>(e).v, e.s);
  case EK::Var: return std::make_unique<EVar>(static_cast<const EVar &>(e).n, e.s);
  case EK::Unary: {
    auto &n = static_cast<const EUnary &>(e);
    return std::make_unique<EUnary>(n.op, cloneE(*n.x), e.s, n.overrideFn);
  }
  case EK::Binary: {
    auto &n = static_cast<const EBinary &>(e);
    return std::make_unique<EBinary>(n.op, cloneE(*n.l), cloneE(*n.r), e.s, n.overrideFn);
  }
  case EK::Call: {
    auto &n = static_cast<const ECall &>(e);
    std::vector<EP> a;
    a.reserve(n.a.size());
    for (const EP &x : n.a) a.push_back(cloneE(*x));
    return std::make_unique<ECall>(n.f, std::move(a), e.s);
  }
  }
  throw CompileError(e.s, "internal clone error");
}

static EP substE(const Expr &e, const std::unordered_map<std::string, const Expr *> &subs) {
  if (e.k == EK::Var) {
    auto &n = static_cast<const EVar &>(e);
    auto it = subs.find(n.n);
    if (it != subs.end()) return cloneE(*it->second);
    return std::make_unique<EVar>(n.n, e.s);
  }
  switch (e.k) {
  case EK::Int: return std::make_unique<EInt>(static_cast<const EInt &>(e).v, e.s);
  case EK::Float: return std::make_unique<EFloat>(static_cast<const EFloat &>(e).v, e.s);
  case EK::Bool: return std::make_unique<EBool>(static_cast<const EBool &>(e).v, e.s);
  case EK::Str: return std::make_unique<EString>(static_cast<const EString &>(e).v, e.s);
  case EK::Unary: {
    auto &n = static_cast<const EUnary &>(e);
    return std::make_unique<EUnary>(n.op, substE(*n.x, subs), e.s, n.overrideFn);
  }
  case EK::Binary: {
    auto &n = static_cast<const EBinary &>(e);
    return std::make_unique<EBinary>(n.op, substE(*n.l, subs), substE(*n.r, subs), e.s, n.overrideFn);
  }
  case EK::Call: {
    auto &n = static_cast<const ECall &>(e);
    std::vector<EP> a;
    a.reserve(n.a.size());
    for (const EP &x : n.a) a.push_back(substE(*x, subs));
    return std::make_unique<ECall>(n.f, std::move(a), e.s);
  }
  case EK::Var: break;
  }
  throw CompileError(e.s, "internal substitute error");
}

static bool hasCall(const Expr &e, const std::string &name) {
  if (e.k == EK::Call) {
    auto &n = static_cast<const ECall &>(e);
    if (canonicalSuperuserCallName(n.f) == canonicalSuperuserCallName(name)) return true;
    for (const EP &a : n.a)
      if (hasCall(*a, name)) return true;
    return false;
  }
  if (e.k == EK::Unary) return hasCall(*static_cast<const EUnary &>(e).x, name);
  if (e.k == EK::Binary) {
    auto &n = static_cast<const EBinary &>(e);
    return hasCall(*n.l, name) || hasCall(*n.r, name);
  }
  return false;
}

static bool hasCallPrefix(const Expr &e, const std::string &prefix) {
  if (e.k == EK::Call) {
    auto &n = static_cast<const ECall &>(e);
    if (n.f.rfind(prefix, 0) == 0) return true;
    for (const EP &a : n.a)
      if (hasCallPrefix(*a, prefix)) return true;
    return false;
  }
  if (e.k == EK::Unary) return hasCallPrefix(*static_cast<const EUnary &>(e).x, prefix);
  if (e.k == EK::Binary) {
    auto &n = static_cast<const EBinary &>(e);
    return hasCallPrefix(*n.l, prefix) || hasCallPrefix(*n.r, prefix);
  }
  return false;
}

static bool hasFormatMarkerExpr(const Expr &e) { return hasCall(e, ".format"); }
static bool hasFormatMarkerStmt(const Stmt &s);
static bool hasParallelForStmt(const Stmt &s);
static bool hasWinGraphicsDepStmt(const Stmt &s);
static bool isMinimalRuntimeCallName(const std::string &name) {
  return name == "print" || name == "println" || name == "print_i64" || name == "print_f64" ||
         name == "print_bool" || name == "print_str" || name == "println_i64" || name == "println_f64" ||
         name == "println_bool" || name == "println_str" || name == "formatOutput" || name == "FormatOutput" ||
         name == "stateSpeed" || name == ".stateSpeed";
}
static bool isUltraMinimalRuntimeCallName(const std::string &name) {
  return name == "print" || name == "println" || name == "print_i64" || name == "print_bool" ||
         name == "print_str" || name == "println_i64" || name == "println_bool" || name == "println_str";
}
static bool hasOnlyMinimalRuntimeCallsExpr(const Expr &e) {
  switch (e.k) {
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    if (!isMinimalRuntimeCallName(n.f)) return false;
    for (const auto &arg : n.a) {
      if (!hasOnlyMinimalRuntimeCallsExpr(*arg)) return false;
    }
    return true;
  }
  case EK::Unary:
    return hasOnlyMinimalRuntimeCallsExpr(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return hasOnlyMinimalRuntimeCallsExpr(*n.l) && hasOnlyMinimalRuntimeCallsExpr(*n.r);
  }
  default:
    return true;
  }
}
static bool hasOnlyMinimalRuntimeCallsStmt(const Stmt &s);
static bool hasOnlyMinimalRuntimeCallsBlock(const std::vector<SP> &b) {
  for (const auto &stmt : b) {
    if (!hasOnlyMinimalRuntimeCallsStmt(*stmt)) return false;
  }
  return true;
}
static bool hasOnlyMinimalRuntimeCallsStmt(const Stmt &s) {
  switch (s.k) {
  case SK::Let: return hasOnlyMinimalRuntimeCallsExpr(*static_cast<const SLet &>(s).v);
  case SK::Assign: return hasOnlyMinimalRuntimeCallsExpr(*static_cast<const SAssign &>(s).v);
  case SK::Expr: return hasOnlyMinimalRuntimeCallsExpr(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return !n.has || !n.v || hasOnlyMinimalRuntimeCallsExpr(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasOnlyMinimalRuntimeCallsExpr(*n.c) && hasOnlyMinimalRuntimeCallsBlock(n.t) &&
           hasOnlyMinimalRuntimeCallsBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasOnlyMinimalRuntimeCallsExpr(*n.c) && hasOnlyMinimalRuntimeCallsBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return hasOnlyMinimalRuntimeCallsExpr(*n.start) && hasOnlyMinimalRuntimeCallsExpr(*n.stop) &&
           hasOnlyMinimalRuntimeCallsExpr(*n.step) && hasOnlyMinimalRuntimeCallsBlock(n.b);
  }
  case SK::FormatBlock: {
    return false;
  }
  case SK::Break:
  case SK::Continue:
    return true;
  }
  return true;
}
static bool hasOnlyMinimalRuntimeCallsProgram(const Program &p) {
  for (const auto &f : p.f) {
    if (f.ex) continue;
    if (!hasOnlyMinimalRuntimeCallsBlock(f.b)) return false;
  }
  return true;
}
static bool hasOnlyUltraMinimalRuntimeCallsExpr(const Expr &e) {
  switch (e.k) {
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    if (!isUltraMinimalRuntimeCallName(n.f)) return false;
    if (n.f == "print" || n.f == "println") {
      if (n.a.size() != 1) return false;
      const Expr &arg = *n.a[0];
      if (arg.inf == Type::F64 || arg.inf == Type::F32) return false;
      if (arg.inf == Type::Str && arg.k != EK::Str) return false;
    }
    for (const auto &arg : n.a) {
      if (!hasOnlyUltraMinimalRuntimeCallsExpr(*arg)) return false;
    }
    return true;
  }
  case EK::Unary:
    return hasOnlyUltraMinimalRuntimeCallsExpr(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return hasOnlyUltraMinimalRuntimeCallsExpr(*n.l) && hasOnlyUltraMinimalRuntimeCallsExpr(*n.r);
  }
  default:
    return true;
  }
}
static bool hasOnlyUltraMinimalRuntimeCallsStmt(const Stmt &s);
static bool hasOnlyUltraMinimalRuntimeCallsBlock(const std::vector<SP> &b) {
  for (const auto &stmt : b) {
    if (!hasOnlyUltraMinimalRuntimeCallsStmt(*stmt)) return false;
  }
  return true;
}
static bool hasOnlyUltraMinimalRuntimeCallsStmt(const Stmt &s) {
  switch (s.k) {
  case SK::Let: return hasOnlyUltraMinimalRuntimeCallsExpr(*static_cast<const SLet &>(s).v);
  case SK::Assign: return hasOnlyUltraMinimalRuntimeCallsExpr(*static_cast<const SAssign &>(s).v);
  case SK::Expr: return hasOnlyUltraMinimalRuntimeCallsExpr(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return !n.has || !n.v || hasOnlyUltraMinimalRuntimeCallsExpr(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasOnlyUltraMinimalRuntimeCallsExpr(*n.c) && hasOnlyUltraMinimalRuntimeCallsBlock(n.t) &&
           hasOnlyUltraMinimalRuntimeCallsBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasOnlyUltraMinimalRuntimeCallsExpr(*n.c) && hasOnlyUltraMinimalRuntimeCallsBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return hasOnlyUltraMinimalRuntimeCallsExpr(*n.start) && hasOnlyUltraMinimalRuntimeCallsExpr(*n.stop) &&
           hasOnlyUltraMinimalRuntimeCallsExpr(*n.step) && hasOnlyUltraMinimalRuntimeCallsBlock(n.b);
  }
  case SK::FormatBlock:
    return false;
  case SK::Break:
  case SK::Continue:
    return true;
  }
  return true;
}
static bool hasOnlyUltraMinimalRuntimeCallsProgram(const Program &p) {
  for (const auto &f : p.f) {
    if (f.ex) continue;
    if (!hasOnlyUltraMinimalRuntimeCallsBlock(f.b)) return false;
  }
  return true;
}
static bool exprUsesStringRuntime(const Expr &e) {
  if (e.k == EK::Str) return false;
  if (e.inf == Type::Str) return true;
  switch (e.k) {
  case EK::Unary:
    return exprUsesStringRuntime(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return exprUsesStringRuntime(*n.l) || exprUsesStringRuntime(*n.r);
  }
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    for (const auto &a : n.a) if (exprUsesStringRuntime(*a)) return true;
    return false;
  }
  default:
    return false;
  }
}
static bool stmtUsesStringRuntime(const Stmt &s);
static bool blockUsesStringRuntime(const std::vector<SP> &b) {
  for (const auto &stmt : b) if (stmtUsesStringRuntime(*stmt)) return true;
  return false;
}
static bool stmtUsesStringRuntime(const Stmt &s) {
  switch (s.k) {
  case SK::Let: {
    const auto &n = static_cast<const SLet &>(s);
    return n.inf == Type::Str || exprUsesStringRuntime(*n.v);
  }
  case SK::Assign:
    return exprUsesStringRuntime(*static_cast<const SAssign &>(s).v);
  case SK::Expr:
    return exprUsesStringRuntime(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && exprUsesStringRuntime(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return exprUsesStringRuntime(*n.c) || blockUsesStringRuntime(n.t) || blockUsesStringRuntime(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return exprUsesStringRuntime(*n.c) || blockUsesStringRuntime(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return exprUsesStringRuntime(*n.start) || exprUsesStringRuntime(*n.stop) || exprUsesStringRuntime(*n.step) ||
           blockUsesStringRuntime(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && exprUsesStringRuntime(*n.endArg)) || blockUsesStringRuntime(n.b);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool usesStringRuntimeProgram(const Program &p) {
  for (const auto &f : p.f) {
    if (f.ex) continue;
    if (f.ret == Type::Str) return true;
    for (const auto &param : f.p) if (param.t == Type::Str) return true;
    if (blockUsesStringRuntime(f.b)) return true;
  }
  return false;
}
static bool exprUsesF64(const Expr &e) {
  if (e.inf == Type::F64 || e.inf == Type::F32) return true;
  switch (e.k) {
  case EK::Unary:
    return exprUsesF64(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return exprUsesF64(*n.l) || exprUsesF64(*n.r);
  }
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    for (const auto &a : n.a) if (exprUsesF64(*a)) return true;
    return false;
  }
  default:
    return false;
  }
}
static bool stmtUsesF64(const Stmt &s);
static bool blockUsesF64(const std::vector<SP> &b) {
  for (const auto &stmt : b) if (stmtUsesF64(*stmt)) return true;
  return false;
}
static bool stmtUsesF64(const Stmt &s) {
  switch (s.k) {
  case SK::Let: {
    const auto &n = static_cast<const SLet &>(s);
    return (n.inf == Type::F64 || n.inf == Type::F32) || exprUsesF64(*n.v);
  }
  case SK::Assign:
    return exprUsesF64(*static_cast<const SAssign &>(s).v);
  case SK::Expr:
    return exprUsesF64(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && exprUsesF64(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return exprUsesF64(*n.c) || blockUsesF64(n.t) || blockUsesF64(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return exprUsesF64(*n.c) || blockUsesF64(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return exprUsesF64(*n.start) || exprUsesF64(*n.stop) || exprUsesF64(*n.step) || blockUsesF64(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && exprUsesF64(*n.endArg)) || blockUsesF64(n.b);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool usesF64Program(const Program &p) {
  for (const auto &f : p.f) {
    if (f.ex) continue;
    if (f.ret == Type::F64 || f.ret == Type::F32) return true;
    for (const auto &param : f.p) if (param.t == Type::F64 || param.t == Type::F32) return true;
    if (blockUsesF64(f.b)) return true;
  }
  return false;
}
static bool hasCallNamedStmt(const Stmt &s, const std::string &name);
static bool hasCallNamedBlock(const std::vector<SP> &b, const std::string &name) {
  for (const SP &stmt : b)
    if (hasCallNamedStmt(*stmt, name)) return true;
  return false;
}
static bool hasCallNamedStmt(const Stmt &s, const std::string &name) {
  switch (s.k) {
  case SK::Let: return hasCall(*static_cast<const SLet &>(s).v, name);
  case SK::Assign: return hasCall(*static_cast<const SAssign &>(s).v, name);
  case SK::Expr: return hasCall(*static_cast<const SExpr &>(s).e, name);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && hasCall(*n.v, name);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasCall(*n.c, name) || hasCallNamedBlock(n.t, name) || hasCallNamedBlock(n.e, name);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasCall(*n.c, name) || hasCallNamedBlock(n.b, name);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return hasCall(*n.start, name) || hasCall(*n.stop, name) || hasCall(*n.step, name) ||
           hasCallNamedBlock(n.b, name);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && hasCall(*n.endArg, name)) || hasCallNamedBlock(n.b, name);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool hasFormatMarkerBlock(const std::vector<SP> &b) {
  for (const SP &stmt : b)
    if (hasFormatMarkerStmt(*stmt)) return true;
  return false;
}
static bool hasWinGraphicsDepBlock(const std::vector<SP> &b) {
  for (const SP &stmt : b)
    if (hasWinGraphicsDepStmt(*stmt)) return true;
  return false;
}
static bool hasParallelForBlock(const std::vector<SP> &b) {
  for (const SP &stmt : b)
    if (hasParallelForStmt(*stmt)) return true;
  return false;
}
static bool hasFormatMarkerStmt(const Stmt &s) {
  switch (s.k) {
  case SK::Let: return hasFormatMarkerExpr(*static_cast<const SLet &>(s).v);
  case SK::Assign: return hasFormatMarkerExpr(*static_cast<const SAssign &>(s).v);
  case SK::Expr: return hasFormatMarkerExpr(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && hasFormatMarkerExpr(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasFormatMarkerExpr(*n.c) || hasFormatMarkerBlock(n.t) || hasFormatMarkerBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasFormatMarkerExpr(*n.c) || hasFormatMarkerBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return hasFormatMarkerExpr(*n.start) || hasFormatMarkerExpr(*n.stop) || hasFormatMarkerExpr(*n.step) ||
           hasFormatMarkerBlock(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && hasFormatMarkerExpr(*n.endArg)) || hasFormatMarkerBlock(n.b);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool hasWinGraphicsDepExpr(const Expr &e) {
  return hasCallPrefix(e, "game_") || hasCallPrefix(e, "pg_") || hasCall(e, "key_down") || hasCall(e, "key_down_name");
}
static bool hasWinGraphicsDepStmt(const Stmt &s) {
  switch (s.k) {
  case SK::Let: return hasWinGraphicsDepExpr(*static_cast<const SLet &>(s).v);
  case SK::Assign: return hasWinGraphicsDepExpr(*static_cast<const SAssign &>(s).v);
  case SK::Expr: return hasWinGraphicsDepExpr(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && hasWinGraphicsDepExpr(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasWinGraphicsDepExpr(*n.c) || hasWinGraphicsDepBlock(n.t) || hasWinGraphicsDepBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasWinGraphicsDepExpr(*n.c) || hasWinGraphicsDepBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return hasWinGraphicsDepExpr(*n.start) || hasWinGraphicsDepExpr(*n.stop) || hasWinGraphicsDepExpr(*n.step) ||
           hasWinGraphicsDepBlock(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && hasWinGraphicsDepExpr(*n.endArg)) || hasWinGraphicsDepBlock(n.b);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool hasParallelForStmt(const Stmt &s) {
  switch (s.k) {
  case SK::Let:
  case SK::Assign:
  case SK::Expr:
  case SK::Ret:
  case SK::Break:
  case SK::Continue:
    return false;
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasParallelForBlock(n.t) || hasParallelForBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasParallelForBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return n.parallel || hasParallelForBlock(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return hasParallelForBlock(n.b);
  }
  }
  return false;
}
static bool hasFormatMarkerProgram(const Program &p) {
  for (const Fn &f : p.f) {
    if (f.ex) continue;
    if (hasFormatMarkerBlock(f.b)) return true;
  }
  return false;
}
static bool hasWinGraphicsDepProgram(const Program &p) {
  for (const Fn &f : p.f) {
    if (f.ex) continue;
    if (hasWinGraphicsDepBlock(f.b)) return true;
  }
  return false;
}
static bool hasParallelForProgram(const Program &p) {
  for (const Fn &f : p.f) {
    if (f.ex) continue;
    if (hasParallelForBlock(f.b)) return true;
  }
  return false;
}
static bool hasForStmt(const Stmt &s);
static bool hasForBlock(const std::vector<SP> &b) {
  for (const SP &stmt : b)
    if (hasForStmt(*stmt)) return true;
  return false;
}
static bool hasForStmt(const Stmt &s) {
  switch (s.k) {
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return hasForBlock(n.t) || hasForBlock(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return hasForBlock(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    (void)n;
    return true;
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return hasForBlock(n.b);
  }
  default:
    return false;
  }
}
static bool hasForProgram(const Program &p) {
  for (const Fn &f : p.f) {
    if (f.ex) continue;
    if (hasForBlock(f.b)) return true;
  }
  return false;
}
static bool exprHasPow(const Expr &e) {
  switch (e.k) {
  case EK::Unary:
    return exprHasPow(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return n.op == BK::Pow || exprHasPow(*n.l) || exprHasPow(*n.r);
  }
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    for (const auto &arg : n.a)
      if (exprHasPow(*arg)) return true;
    return false;
  }
  default:
    return false;
  }
}
static bool stmtHasPow(const Stmt &s);
static bool blockHasPow(const std::vector<SP> &b) {
  for (const auto &stmt : b)
    if (stmtHasPow(*stmt)) return true;
  return false;
}
static bool stmtHasPow(const Stmt &s) {
  switch (s.k) {
  case SK::Let: return exprHasPow(*static_cast<const SLet &>(s).v);
  case SK::Assign: return exprHasPow(*static_cast<const SAssign &>(s).v);
  case SK::Expr: return exprHasPow(*static_cast<const SExpr &>(s).e);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && exprHasPow(*n.v);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return exprHasPow(*n.c) || blockHasPow(n.t) || blockHasPow(n.e);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return exprHasPow(*n.c) || blockHasPow(n.b);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return exprHasPow(*n.start) || exprHasPow(*n.stop) || exprHasPow(*n.step) || blockHasPow(n.b);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    return (n.endArg && exprHasPow(*n.endArg)) || blockHasPow(n.b);
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}
static bool hasPowProgram(const Program &p) {
  for (const auto &f : p.f) {
    if (f.ex) continue;
    if (blockHasPow(f.b)) return true;
  }
  return false;
}
static bool hasCallNamedProgram(const Program &p, const std::string &name) {
  for (const Fn &f : p.f) {
    if (f.ex) continue;
    if (hasCallNamedBlock(f.b, name)) return true;
  }
  return false;
}

static std::unordered_map<std::string, const Fn *> inlineCands(const Program &p) {
  std::unordered_map<std::string, const Fn *> m;
  for (const Fn &f : p.f) {
    if (f.ex || f.p.size() > 8 || f.b.size() != 1) continue;
    auto *r = dynamic_cast<const SRet *>(f.b[0].get());
    if (r == nullptr || !r->has || !r->v) continue;
    if (hasCall(*r->v, f.n)) continue;
    m[f.n] = &f;
  }
  return m;
}

static std::optional<int64_t> litI(const Expr &e) { return e.k == EK::Int ? std::optional<int64_t>(static_cast<const EInt &>(e).v) : std::nullopt; }
static std::optional<double> litF(const Expr &e) { return e.k == EK::Float ? std::optional<double>(static_cast<const EFloat &>(e).v) : std::nullopt; }
static std::optional<bool> litB(const Expr &e) { return e.k == EK::Bool ? std::optional<bool>(static_cast<const EBool &>(e).v) : std::nullopt; }
static bool isZero(const Expr &e) {
  if (auto i = litI(e)) return *i == 0;
  if (auto f = litF(e)) return *f == 0.0;
  return false;
}
static bool isOne(const Expr &e) {
  if (auto i = litI(e)) return *i == 1;
  if (auto f = litF(e)) return *f == 1.0;
  return false;
}
static std::optional<int64_t> tripCount(int64_t start, int64_t stop, int64_t step) {
  if (step == 0) return std::nullopt;
  if (step > 0) {
    if (start >= stop) return 0;
    return (stop - start + step - 1) / step;
  }
  if (start <= stop) return 0;
  const int64_t negStep = -step;
  return (start - stop + negStep - 1) / negStep;
}

static bool hasDeclNamed(const std::vector<SP> &b, const std::string &name) {
  for (const SP &stmt : b) {
    switch (stmt->k) {
    case SK::Let: {
      const auto &n = static_cast<const SLet &>(*stmt);
      if (n.n == name) return true;
      break;
    }
    case SK::If: {
      const auto &n = static_cast<const SIf &>(*stmt);
      if (hasDeclNamed(n.t, name) || hasDeclNamed(n.e, name)) return true;
      break;
    }
    case SK::While: {
      const auto &n = static_cast<const SWhile &>(*stmt);
      if (hasDeclNamed(n.b, name)) return true;
      break;
    }
    case SK::For: {
      const auto &n = static_cast<const SFor &>(*stmt);
      if (n.n == name || hasDeclNamed(n.b, name)) return true;
      break;
    }
    case SK::FormatBlock: {
      const auto &n = static_cast<const SFormatBlock &>(*stmt);
      if (hasDeclNamed(n.b, name)) return true;
      break;
    }
    default:
      break;
    }
  }
  return false;
}

static bool stmtAssignsNamed(const Stmt &stmt, const std::string &name) {
  switch (stmt.k) {
  case SK::Assign: {
    const auto &n = static_cast<const SAssign &>(stmt);
    return n.n == name;
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(stmt);
    for (const auto &x : n.t) if (stmtAssignsNamed(*x, name)) return true;
    for (const auto &x : n.e) if (stmtAssignsNamed(*x, name)) return true;
    return false;
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(stmt);
    for (const auto &x : n.b) if (stmtAssignsNamed(*x, name)) return true;
    return false;
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(stmt);
    if (n.n == name) return true;
    for (const auto &x : n.b) if (stmtAssignsNamed(*x, name)) return true;
    return false;
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(stmt);
    for (const auto &x : n.b) if (stmtAssignsNamed(*x, name)) return true;
    return false;
  }
  default:
    return false;
  }
}

static bool hasLoopControlStmt(const std::vector<SP> &b) {
  for (const SP &stmt : b) {
    switch (stmt->k) {
    case SK::Break:
    case SK::Continue:
      return true;
    case SK::If: {
      const auto &n = static_cast<const SIf &>(*stmt);
      if (hasLoopControlStmt(n.t) || hasLoopControlStmt(n.e)) return true;
      break;
    }
    case SK::While: {
      const auto &n = static_cast<const SWhile &>(*stmt);
      if (hasLoopControlStmt(n.b)) return true;
      break;
    }
    case SK::For: {
      const auto &n = static_cast<const SFor &>(*stmt);
      if (hasLoopControlStmt(n.b)) return true;
      break;
    }
    case SK::FormatBlock: {
      const auto &n = static_cast<const SFormatBlock &>(*stmt);
      if (hasLoopControlStmt(n.b)) return true;
      break;
    }
    default:
      break;
    }
  }
  return false;
}

static bool exprHasVarName(const Expr &e, const std::string &name) {
  switch (e.k) {
  case EK::Var:
    return static_cast<const EVar &>(e).n == name;
  case EK::Unary:
    return exprHasVarName(*static_cast<const EUnary &>(e).x, name);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    return exprHasVarName(*n.l, name) || exprHasVarName(*n.r, name);
  }
  case EK::Call: {
    const auto &n = static_cast<const ECall &>(e);
    for (const auto &arg : n.a)
      if (exprHasVarName(*arg, name)) return true;
    return false;
  }
  default:
    return false;
  }
}

static std::optional<int64_t> checkedI128ToI64(__int128 v) {
  if (v < static_cast<__int128>(std::numeric_limits<int64_t>::min())) return std::nullopt;
  if (v > static_cast<__int128>(std::numeric_limits<int64_t>::max())) return std::nullopt;
  return static_cast<int64_t>(v);
}

static bool gOptHasUnaryNegOverride = false;

static std::optional<int64_t> evalConstI64Expr(const Expr &expr) {
  switch (expr.k) {
  case EK::Int:
    return static_cast<const EInt &>(expr).v;
  case EK::Unary: {
    const auto &u = static_cast<const EUnary &>(expr);
    if (!u.overrideFn.empty()) return std::nullopt;
    if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
    if (u.op != UK::Neg) return std::nullopt;
    auto v = evalConstI64Expr(*u.x);
    if (!v) return std::nullopt;
    return checkedI128ToI64(-static_cast<__int128>(*v));
  }
  case EK::Binary: {
    const auto &b = static_cast<const EBinary &>(expr);
    if (!b.overrideFn.empty()) return std::nullopt;
    auto l = evalConstI64Expr(*b.l);
    auto r = evalConstI64Expr(*b.r);
    if (!l || !r) return std::nullopt;
    switch (b.op) {
    case BK::Add: return checkedI128ToI64(static_cast<__int128>(*l) + static_cast<__int128>(*r));
    case BK::Sub: return checkedI128ToI64(static_cast<__int128>(*l) - static_cast<__int128>(*r));
    case BK::Mul: return checkedI128ToI64(static_cast<__int128>(*l) * static_cast<__int128>(*r));
    default: return std::nullopt;
    }
  }
  default:
    return std::nullopt;
  }
}

struct AffineI64Const {
  int64_t a = 0;
  int64_t b = 0;
};

static std::optional<AffineI64Const> affineI64ConstExpr(const Expr &expr, const std::string &loopVar) {
  switch (expr.k) {
  case EK::Int:
    return AffineI64Const{0, static_cast<const EInt &>(expr).v};
  case EK::Var: {
    const auto &v = static_cast<const EVar &>(expr);
    if (v.n == loopVar) return AffineI64Const{1, 0};
    return std::nullopt;
  }
  case EK::Unary: {
    const auto &u = static_cast<const EUnary &>(expr);
    if (!u.overrideFn.empty()) return std::nullopt;
    if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
    if (u.op != UK::Neg) return std::nullopt;
    auto in = affineI64ConstExpr(*u.x, loopVar);
    if (!in) return std::nullopt;
    auto a = checkedI128ToI64(-static_cast<__int128>(in->a));
    auto b = checkedI128ToI64(-static_cast<__int128>(in->b));
    if (!a || !b) return std::nullopt;
    return AffineI64Const{*a, *b};
  }
  case EK::Binary: {
    const auto &bin = static_cast<const EBinary &>(expr);
    if (!bin.overrideFn.empty()) return std::nullopt;
    if (bin.op == BK::Add || bin.op == BK::Sub) {
      auto l = affineI64ConstExpr(*bin.l, loopVar);
      auto r = affineI64ConstExpr(*bin.r, loopVar);
      if (!l || !r) return std::nullopt;
      const int sign = (bin.op == BK::Add) ? 1 : -1;
      auto a = checkedI128ToI64(static_cast<__int128>(l->a) + static_cast<__int128>(sign) * static_cast<__int128>(r->a));
      auto b = checkedI128ToI64(static_cast<__int128>(l->b) + static_cast<__int128>(sign) * static_cast<__int128>(r->b));
      if (!a || !b) return std::nullopt;
      return AffineI64Const{*a, *b};
    }
    if (bin.op == BK::Mul) {
      auto lc = evalConstI64Expr(*bin.l);
      auto rc = evalConstI64Expr(*bin.r);
      if (lc) {
        auto r = affineI64ConstExpr(*bin.r, loopVar);
        if (!r) return std::nullopt;
        auto a = checkedI128ToI64(static_cast<__int128>(*lc) * static_cast<__int128>(r->a));
        auto b = checkedI128ToI64(static_cast<__int128>(*lc) * static_cast<__int128>(r->b));
        if (!a || !b) return std::nullopt;
        return AffineI64Const{*a, *b};
      }
      if (rc) {
        auto l = affineI64ConstExpr(*bin.l, loopVar);
        if (!l) return std::nullopt;
        auto a = checkedI128ToI64(static_cast<__int128>(*rc) * static_cast<__int128>(l->a));
        auto b = checkedI128ToI64(static_cast<__int128>(*rc) * static_cast<__int128>(l->b));
        if (!a || !b) return std::nullopt;
        return AffineI64Const{*a, *b};
      }
      return std::nullopt;
    }
    return std::nullopt;
  }
  default:
    return std::nullopt;
  }
}

struct Linear2I64Const {
  int64_t ax = 0;
  int64_t ay = 0;
  int64_t c = 0;
};

static std::optional<Linear2I64Const> linear2I64ConstExpr(const Expr &expr, const std::string &xVar, const std::string &yVar) {
  switch (expr.k) {
  case EK::Int:
    return Linear2I64Const{0, 0, static_cast<const EInt &>(expr).v};
  case EK::Var: {
    const auto &v = static_cast<const EVar &>(expr);
    if (v.n == xVar) return Linear2I64Const{1, 0, 0};
    if (v.n == yVar) return Linear2I64Const{0, 1, 0};
    return std::nullopt;
  }
  case EK::Unary: {
    const auto &u = static_cast<const EUnary &>(expr);
    if (!u.overrideFn.empty()) return std::nullopt;
    if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
    if (u.op != UK::Neg) return std::nullopt;
    auto in = linear2I64ConstExpr(*u.x, xVar, yVar);
    if (!in) return std::nullopt;
    auto ax = checkedI128ToI64(-static_cast<__int128>(in->ax));
    auto ay = checkedI128ToI64(-static_cast<__int128>(in->ay));
    auto c = checkedI128ToI64(-static_cast<__int128>(in->c));
    if (!ax || !ay || !c) return std::nullopt;
    return Linear2I64Const{*ax, *ay, *c};
  }
  case EK::Binary: {
    const auto &b = static_cast<const EBinary &>(expr);
    if (!b.overrideFn.empty()) return std::nullopt;
    if (b.op == BK::Add || b.op == BK::Sub) {
      auto l = linear2I64ConstExpr(*b.l, xVar, yVar);
      auto r = linear2I64ConstExpr(*b.r, xVar, yVar);
      if (!l || !r) return std::nullopt;
      const int sign = (b.op == BK::Add) ? 1 : -1;
      auto ax = checkedI128ToI64(static_cast<__int128>(l->ax) + static_cast<__int128>(sign) * static_cast<__int128>(r->ax));
      auto ay = checkedI128ToI64(static_cast<__int128>(l->ay) + static_cast<__int128>(sign) * static_cast<__int128>(r->ay));
      auto c = checkedI128ToI64(static_cast<__int128>(l->c) + static_cast<__int128>(sign) * static_cast<__int128>(r->c));
      if (!ax || !ay || !c) return std::nullopt;
      return Linear2I64Const{*ax, *ay, *c};
    }
    if (b.op == BK::Mul) {
      auto lc = evalConstI64Expr(*b.l);
      auto rc = evalConstI64Expr(*b.r);
      if (lc) {
        auto r = linear2I64ConstExpr(*b.r, xVar, yVar);
        if (!r) return std::nullopt;
        auto ax = checkedI128ToI64(static_cast<__int128>(*lc) * static_cast<__int128>(r->ax));
        auto ay = checkedI128ToI64(static_cast<__int128>(*lc) * static_cast<__int128>(r->ay));
        auto c = checkedI128ToI64(static_cast<__int128>(*lc) * static_cast<__int128>(r->c));
        if (!ax || !ay || !c) return std::nullopt;
        return Linear2I64Const{*ax, *ay, *c};
      }
      if (rc) {
        auto l = linear2I64ConstExpr(*b.l, xVar, yVar);
        if (!l) return std::nullopt;
        auto ax = checkedI128ToI64(static_cast<__int128>(*rc) * static_cast<__int128>(l->ax));
        auto ay = checkedI128ToI64(static_cast<__int128>(*rc) * static_cast<__int128>(l->ay));
        auto c = checkedI128ToI64(static_cast<__int128>(*rc) * static_cast<__int128>(l->c));
        if (!ax || !ay || !c) return std::nullopt;
        return Linear2I64Const{*ax, *ay, *c};
      }
      return std::nullopt;
    }
    return std::nullopt;
  }
  default:
    return std::nullopt;
  }
}

struct Poly2I64Const {
  int64_t c2 = 0;
  int64_t c1 = 0;
  int64_t c0 = 0;
};

static int poly2Degree(const Poly2I64Const &p) {
  if (p.c2 != 0) return 2;
  if (p.c1 != 0) return 1;
  return 0;
}

static std::optional<Poly2I64Const> poly2Add(const Poly2I64Const &a, const Poly2I64Const &b, int sign) {
  auto c2 = checkedI128ToI64(static_cast<__int128>(a.c2) + static_cast<__int128>(sign) * static_cast<__int128>(b.c2));
  auto c1 = checkedI128ToI64(static_cast<__int128>(a.c1) + static_cast<__int128>(sign) * static_cast<__int128>(b.c1));
  auto c0 = checkedI128ToI64(static_cast<__int128>(a.c0) + static_cast<__int128>(sign) * static_cast<__int128>(b.c0));
  if (!c2 || !c1 || !c0) return std::nullopt;
  return Poly2I64Const{*c2, *c1, *c0};
}

static std::optional<Poly2I64Const> poly2Mul(const Poly2I64Const &a, const Poly2I64Const &b) {
  const __int128 c4 = static_cast<__int128>(a.c2) * static_cast<__int128>(b.c2);
  const __int128 c3 = static_cast<__int128>(a.c2) * static_cast<__int128>(b.c1) +
                      static_cast<__int128>(a.c1) * static_cast<__int128>(b.c2);
  if (c4 != 0 || c3 != 0) return std::nullopt;
  auto c2 = checkedI128ToI64(static_cast<__int128>(a.c2) * static_cast<__int128>(b.c0) +
                             static_cast<__int128>(a.c1) * static_cast<__int128>(b.c1) +
                             static_cast<__int128>(a.c0) * static_cast<__int128>(b.c2));
  auto c1 = checkedI128ToI64(static_cast<__int128>(a.c1) * static_cast<__int128>(b.c0) +
                             static_cast<__int128>(a.c0) * static_cast<__int128>(b.c1));
  auto c0 = checkedI128ToI64(static_cast<__int128>(a.c0) * static_cast<__int128>(b.c0));
  if (!c2 || !c1 || !c0) return std::nullopt;
  return Poly2I64Const{*c2, *c1, *c0};
}

static std::optional<Poly2I64Const> poly2ConstI64Expr(const Expr &expr, const std::string &loopVar) {
  switch (expr.k) {
  case EK::Int:
    return Poly2I64Const{0, 0, static_cast<const EInt &>(expr).v};
  case EK::Var: {
    const auto &v = static_cast<const EVar &>(expr);
    if (v.n == loopVar) return Poly2I64Const{0, 1, 0};
    return std::nullopt;
  }
  case EK::Unary: {
    const auto &u = static_cast<const EUnary &>(expr);
    if (!u.overrideFn.empty()) return std::nullopt;
    if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
    if (u.op != UK::Neg) return std::nullopt;
    auto in = poly2ConstI64Expr(*u.x, loopVar);
    if (!in) return std::nullopt;
    auto c2 = checkedI128ToI64(-static_cast<__int128>(in->c2));
    auto c1 = checkedI128ToI64(-static_cast<__int128>(in->c1));
    auto c0 = checkedI128ToI64(-static_cast<__int128>(in->c0));
    if (!c2 || !c1 || !c0) return std::nullopt;
    return Poly2I64Const{*c2, *c1, *c0};
  }
  case EK::Binary: {
    const auto &b = static_cast<const EBinary &>(expr);
    if (b.op == BK::Add || b.op == BK::Sub) {
      auto l = poly2ConstI64Expr(*b.l, loopVar);
      auto r = poly2ConstI64Expr(*b.r, loopVar);
      if (!l || !r) return std::nullopt;
      return poly2Add(*l, *r, b.op == BK::Add ? 1 : -1);
    }
    if (b.op == BK::Mul) {
      auto l = poly2ConstI64Expr(*b.l, loopVar);
      auto r = poly2ConstI64Expr(*b.r, loopVar);
      if (!l || !r) return std::nullopt;
      if (poly2Degree(*l) + poly2Degree(*r) > 2) return std::nullopt;
      return poly2Mul(*l, *r);
    }
    return std::nullopt;
  }
  default:
    return std::nullopt;
  }
}

static std::optional<int64_t> checkedAddI64(int64_t a, int64_t b) {
  return checkedI128ToI64(static_cast<__int128>(a) + static_cast<__int128>(b));
}

static std::optional<int64_t> checkedMulI64(int64_t a, int64_t b) {
  return checkedI128ToI64(static_cast<__int128>(a) * static_cast<__int128>(b));
}

static uint64_t absU64I64(int64_t v) {
  return v < 0 ? (uint64_t)(-(v + 1LL)) + 1ULL : (uint64_t)v;
}

static int64_t gcdSmallI64(int64_t a, int64_t b) {
  uint64_t ua = absU64I64(a);
  uint64_t ub = absU64I64(b);
  while (ub != 0ULL) {
    uint64_t t = ua % ub;
    ua = ub;
    ub = t;
  }
  return (int64_t)ua;
}

static void reduceDivisor(int64_t &x, int64_t &den) {
  if (den <= 1) return;
  int64_t g = gcdSmallI64(x, den);
  if (g > 1) {
    x /= g;
    den /= g;
  }
}

static std::optional<int64_t> mul2Div2ExactI64(int64_t a, int64_t b) {
  int64_t den = 2;
  int64_t x = a;
  int64_t y = b;
  reduceDivisor(x, den);
  reduceDivisor(y, den);
  if (den != 1) return std::nullopt;
  return checkedMulI64(x, y);
}

static std::optional<int64_t> mul3Div6ExactI64(int64_t a, int64_t b, int64_t c) {
  int64_t den = 6;
  int64_t x = a;
  int64_t y = b;
  int64_t z = c;
  reduceDivisor(x, den);
  reduceDivisor(y, den);
  reduceDivisor(z, den);
  if (den != 1) return std::nullopt;
  auto xy = checkedMulI64(x, y);
  if (!xy) return std::nullopt;
  return checkedMulI64(*xy, z);
}

static std::optional<int64_t> arithmeticSeriesSumI64(int64_t count, int64_t start, int64_t step) {
  if (count <= 0) return std::optional<int64_t>{0};
  auto nMinus1 = checkedAddI64(count, -1);
  if (!nMinus1) return std::nullopt;
  auto span = checkedMulI64(*nMinus1, step);
  if (!span) return std::nullopt;
  auto twoStart = checkedMulI64(2, start);
  if (!twoStart) return std::nullopt;
  auto term = checkedAddI64(*twoStart, *span);
  if (!term) return std::nullopt;
  return mul2Div2ExactI64(count, *term);
}

static std::optional<int64_t> modPosI64Const(int64_t x, int64_t m) {
  if (m <= 0) return std::nullopt;
  int64_t r = x % m;
  if (r < 0) r += m;
  return r;
}

static std::optional<int64_t> modLinearSeriesSumI64(
    int64_t iters,
    int64_t start,
    int64_t step,
  int64_t a,
  int64_t b,
  int64_t m) {
  if (iters <= 0 || m <= 0) return std::optional<int64_t>{0};
  auto aStart = checkedMulI64(a, start);
  auto xRaw = aStart ? checkedAddI64(*aStart, b) : std::nullopt;
  auto dRaw = checkedMulI64(a, step);
  auto x0 = xRaw ? modPosI64Const(*xRaw, m) : std::nullopt;
  auto d = dRaw ? modPosI64Const(*dRaw, m) : std::nullopt;
  if (!x0 || !d) return std::nullopt;
  if (*d == 0) {
    return checkedI128ToI64(static_cast<__int128>(*x0) * static_cast<__int128>(iters));
  }
  const int64_t g = gcdSmallI64(*d, m);
  if (g <= 0) return std::nullopt;
  const int64_t period = m / g;
  if (period <= 0) return std::nullopt;
  // Avoid pathological optimizer compile-time on very large moduli.
  if (period > 8192) return std::nullopt;
  const int64_t full = iters / period;
  const int64_t rem = iters - (full * period);
  __int128 cycle = 0;
  int64_t cur = *x0;
  for (int64_t i = 0; i < period; ++i) {
    cycle += static_cast<__int128>(cur);
    cur += *d;
    if (cur >= m) cur -= m;
  }
  __int128 tail = 0;
  cur = *x0;
  for (int64_t i = 0; i < rem; ++i) {
    tail += static_cast<__int128>(cur);
    cur += *d;
    if (cur >= m) cur -= m;
  }
  return checkedI128ToI64(static_cast<__int128>(full) * cycle + tail);
}

static EP substVarWithI64(const Expr &e, const std::string &name, int64_t value, const Span &s) {
  auto lit = std::make_unique<EInt>(value, s);
  std::unordered_map<std::string, const Expr *> subs;
  subs[name] = lit.get();
  return substE(e, subs);
}

static SP substStmtVar(const Stmt &s, const std::string &var, int64_t value);
static std::vector<SP> substBlockVar(const std::vector<SP> &b, const std::string &var, int64_t value) {
  std::vector<SP> out;
  out.reserve(b.size());
  for (const SP &stmt : b) out.push_back(substStmtVar(*stmt, var, value));
  return out;
}
static SP substStmtVar(const Stmt &s, const std::string &var, int64_t value) {
  switch (s.k) {
  case SK::Let: {
    const auto &n = static_cast<const SLet &>(s);
    auto out =
        std::make_unique<SLet>(n.n, n.decl, n.isConst, n.isOwned, substVarWithI64(*n.v, var, value, s.s), s.s);
    out->ownedFreeFn = n.ownedFreeFn;
    out->inf = n.inf;
    out->typed = n.typed;
    return out;
  }
  case SK::Assign: {
    const auto &n = static_cast<const SAssign &>(s);
    return std::make_unique<SAssign>(n.n, substVarWithI64(*n.v, var, value, s.s), s.s);
  }
  case SK::Expr: {
    const auto &n = static_cast<const SExpr &>(s);
    return std::make_unique<SExpr>(substVarWithI64(*n.e, var, value, s.s), s.s);
  }
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    if (!n.has || !n.v) return std::make_unique<SRet>(false, nullptr, s.s);
    return std::make_unique<SRet>(true, substVarWithI64(*n.v, var, value, s.s), s.s);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    return std::make_unique<SIf>(substVarWithI64(*n.c, var, value, s.s), substBlockVar(n.t, var, value),
                                 substBlockVar(n.e, var, value), s.s);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    return std::make_unique<SWhile>(substVarWithI64(*n.c, var, value, s.s), substBlockVar(n.b, var, value), s.s);
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    return std::make_unique<SFor>(n.n, substVarWithI64(*n.start, var, value, s.s),
                                  substVarWithI64(*n.stop, var, value, s.s), substVarWithI64(*n.step, var, value, s.s),
                                  n.parallel, substBlockVar(n.b, var, value), s.s);
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    EP endArg;
    if (n.endArg) endArg = substVarWithI64(*n.endArg, var, value, s.s);
    return std::make_unique<SFormatBlock>(std::move(endArg), substBlockVar(n.b, var, value), s.s);
  }
  case SK::Break:
    return std::make_unique<SBreak>(s.s);
  case SK::Continue:
    return std::make_unique<SContinue>(s.s);
  }
  throw CompileError(s.s, "internal statement substitution error");
}

static bool optE(EP &e, const std::unordered_map<std::string, const Fn *> &cand);
static bool optBlock(std::vector<SP> &b, const std::unordered_map<std::string, const Fn *> &cand,
                     bool inLoopBody = false);

static std::optional<int64_t> evalConstI64WithEnv(
    const Expr &expr,
    const std::unordered_map<std::string, int64_t> &env) {
  switch (expr.k) {
  case EK::Int:
    return static_cast<const EInt &>(expr).v;
  case EK::Var: {
    const auto &v = static_cast<const EVar &>(expr);
    auto it = env.find(v.n);
    if (it == env.end()) return std::nullopt;
    return it->second;
  }
  case EK::Unary: {
    const auto &u = static_cast<const EUnary &>(expr);
    if (!u.overrideFn.empty()) return std::nullopt;
    if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
    if (u.op != UK::Neg) return std::nullopt;
    auto x = evalConstI64WithEnv(*u.x, env);
    if (!x) return std::nullopt;
    return checkedI128ToI64(-static_cast<__int128>(*x));
  }
  case EK::Binary: {
    const auto &b = static_cast<const EBinary &>(expr);
    if (!b.overrideFn.empty()) return std::nullopt;
    auto l = evalConstI64WithEnv(*b.l, env);
    auto r = evalConstI64WithEnv(*b.r, env);
    if (!l || !r) return std::nullopt;
    switch (b.op) {
    case BK::Add: return checkedI128ToI64(static_cast<__int128>(*l) + static_cast<__int128>(*r));
    case BK::Sub: return checkedI128ToI64(static_cast<__int128>(*l) - static_cast<__int128>(*r));
    case BK::Mul: return checkedI128ToI64(static_cast<__int128>(*l) * static_cast<__int128>(*r));
    case BK::Div:
      if (*r == 0) return std::nullopt;
      return *l / *r;
    case BK::Mod:
      if (*r == 0) return std::nullopt;
      return *l % *r;
    default:
      return std::nullopt;
    }
  }
  default:
    return std::nullopt;
  }
}

static bool exprTrivialNoSideEffects(const Expr &e) {
  switch (e.k) {
  case EK::Int:
  case EK::Float:
  case EK::Bool:
  case EK::Str:
  case EK::Var:
    return true;
  case EK::Unary:
    return exprTrivialNoSideEffects(*static_cast<const EUnary &>(e).x);
  case EK::Binary: {
    const auto &n = static_cast<const EBinary &>(e);
    if (!n.overrideFn.empty()) return false;
    if (n.op == BK::Div || n.op == BK::Mod || n.op == BK::Pow) return false;
    return exprTrivialNoSideEffects(*n.l) && exprTrivialNoSideEffects(*n.r);
  }
  case EK::Call:
    return false;
  }
  return false;
}

static bool stmtReadsNamed(const Stmt &s, const std::string &name);
static bool blockReadsNamed(const std::vector<SP> &b, std::size_t from, const std::string &name) {
  for (std::size_t i = from; i < b.size(); ++i) {
    if (stmtReadsNamed(*b[i], name)) return true;
  }
  return false;
}
static bool stmtReadsNamed(const Stmt &s, const std::string &name) {
  switch (s.k) {
  case SK::Let:
    return exprHasVarName(*static_cast<const SLet &>(s).v, name);
  case SK::Assign:
    return exprHasVarName(*static_cast<const SAssign &>(s).v, name);
  case SK::Expr:
    return exprHasVarName(*static_cast<const SExpr &>(s).e, name);
  case SK::Ret: {
    const auto &n = static_cast<const SRet &>(s);
    return n.has && n.v && exprHasVarName(*n.v, name);
  }
  case SK::If: {
    const auto &n = static_cast<const SIf &>(s);
    for (const auto &x : n.t)
      if (stmtReadsNamed(*x, name)) return true;
    for (const auto &x : n.e)
      if (stmtReadsNamed(*x, name)) return true;
    return exprHasVarName(*n.c, name);
  }
  case SK::While: {
    const auto &n = static_cast<const SWhile &>(s);
    if (exprHasVarName(*n.c, name)) return true;
    for (const auto &x : n.b)
      if (stmtReadsNamed(*x, name)) return true;
    return false;
  }
  case SK::For: {
    const auto &n = static_cast<const SFor &>(s);
    if (exprHasVarName(*n.start, name) || exprHasVarName(*n.stop, name) || exprHasVarName(*n.step, name)) {
      return true;
    }
    for (const auto &x : n.b)
      if (stmtReadsNamed(*x, name)) return true;
    return false;
  }
  case SK::FormatBlock: {
    const auto &n = static_cast<const SFormatBlock &>(s);
    if (n.endArg && exprHasVarName(*n.endArg, name)) return true;
    for (const auto &x : n.b)
      if (stmtReadsNamed(*x, name)) return true;
    return false;
  }
  case SK::Break:
  case SK::Continue:
    return false;
  }
  return false;
}

static bool pruneDeadLocalStores(std::vector<SP> &b) {
  bool changed = false;
  for (std::size_t i = 0; i < b.size();) {
    Stmt &s = *b[i];
    bool erased = false;
    switch (s.k) {
    case SK::Let: {
      auto &n = static_cast<SLet &>(s);
      if (exprTrivialNoSideEffects(*n.v) && !blockReadsNamed(b, i + 1, n.n)) {
        b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
        changed = true;
        erased = true;
      }
      break;
    }
    case SK::Assign: {
      auto &n = static_cast<SAssign &>(s);
      if (exprTrivialNoSideEffects(*n.v) && !blockReadsNamed(b, i + 1, n.n)) {
        b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
        changed = true;
        erased = true;
      }
      break;
    }
    case SK::Expr: {
      auto &n = static_cast<SExpr &>(s);
      if (exprTrivialNoSideEffects(*n.e)) {
        b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
        changed = true;
        erased = true;
      }
      break;
    }
    case SK::If: {
      auto &n = static_cast<SIf &>(s);
      changed |= pruneDeadLocalStores(n.t);
      changed |= pruneDeadLocalStores(n.e);
      break;
    }
    case SK::While:
    case SK::For:
    case SK::FormatBlock:
    case SK::Ret:
    case SK::Break:
    case SK::Continue:
      break;
    }
    if (!erased) ++i;
  }
  return changed;
}

static bool propagateLocalI64Consts(std::vector<SP> &b) {
  bool changed = false;
  std::unordered_map<std::string, int64_t> known;
  for (std::size_t i = 0; i < b.size(); ++i) {
    Stmt &s = *b[i];
    switch (s.k) {
    case SK::Let: {
      auto &n = static_cast<SLet &>(s);
      if (auto v = evalConstI64WithEnv(*n.v, known)) {
        if (!litI(*n.v) || *litI(*n.v) != *v) {
          n.v = std::make_unique<EInt>(*v, n.s);
          changed = true;
        }
        known[n.n] = *v;
      } else {
        known.erase(n.n);
      }
      break;
    }
    case SK::Assign: {
      auto &n = static_cast<SAssign &>(s);
      if (auto v = evalConstI64WithEnv(*n.v, known)) {
        if (!litI(*n.v) || *litI(*n.v) != *v) {
          n.v = std::make_unique<EInt>(*v, n.s);
          changed = true;
        }
        known[n.n] = *v;
      } else {
        known.erase(n.n);
      }
      break;
    }
    case SK::Expr: {
      auto &n = static_cast<SExpr &>(s);
      if (n.e->k == EK::Call) {
        auto &c = static_cast<ECall &>(*n.e);
        if ((c.f == "print" || c.f == "println") && c.a.size() == 1) {
          if (auto v = evalConstI64WithEnv(*c.a[0], known)) {
            c.f = (c.f == "println") ? "println_str" : "print_str";
            c.a[0] = std::make_unique<EString>(std::to_string(*v), c.s);
            changed = true;
          }
        }
      }
      break;
    }
    case SK::Ret: {
      auto &n = static_cast<SRet &>(s);
      if (n.has && n.v) {
        if (auto v = evalConstI64WithEnv(*n.v, known)) {
          if (!litI(*n.v) || *litI(*n.v) != *v) {
            n.v = std::make_unique<EInt>(*v, n.s);
            changed = true;
          }
        }
      }
      break;
    }
    case SK::If: {
      auto &n = static_cast<SIf &>(s);
      changed |= propagateLocalI64Consts(n.t);
      changed |= propagateLocalI64Consts(n.e);
      known.clear();
      break;
    }
    case SK::While: {
      auto &n = static_cast<SWhile &>(s);
      changed |= propagateLocalI64Consts(n.b);
      known.clear();
      break;
    }
    case SK::For: {
      auto &n = static_cast<SFor &>(s);
      changed |= propagateLocalI64Consts(n.b);
      known.clear();
      break;
    }
    case SK::FormatBlock: {
      auto &n = static_cast<SFormatBlock &>(s);
      changed |= propagateLocalI64Consts(n.b);
      known.clear();
      break;
    }
    case SK::Break:
    case SK::Continue:
      known.clear();
      break;
    }
  }
  return changed;
}

static bool foldBin(EBinary &n, EP &e) {
  if (!n.overrideFn.empty()) return false;
  auto li = litI(*n.l);
  auto ri = litI(*n.r);
  auto lf = litF(*n.l);
  auto rf = litF(*n.r);
  auto lb = litB(*n.l);
  auto rb = litB(*n.r);
  if (li && ri) {
    int64_t l = *li, r = *ri;
    switch (n.op) {
    case BK::Add: e = std::make_unique<EInt>(l + r, n.s); return true;
    case BK::Sub: e = std::make_unique<EInt>(l - r, n.s); return true;
    case BK::Mul: e = std::make_unique<EInt>(l * r, n.s); return true;
    case BK::Div: if (r != 0) { e = std::make_unique<EInt>(l / r, n.s); return true; } return false;
    case BK::Mod: if (r != 0) { e = std::make_unique<EInt>(l % r, n.s); return true; } return false;
    case BK::Pow:
      if (r < 0) return false;
      {
        int64_t out = 1;
        int64_t base = l;
        int64_t exp = r;
        while (exp > 0) {
          if (exp & 1LL) out *= base;
          exp >>= 1LL;
          if (exp > 0) base *= base;
        }
        e = std::make_unique<EInt>(out, n.s);
      }
      return true;
    case BK::Eq: e = std::make_unique<EBool>(l == r, n.s); return true;
    case BK::Neq: e = std::make_unique<EBool>(l != r, n.s); return true;
    case BK::Lt: e = std::make_unique<EBool>(l < r, n.s); return true;
    case BK::Lte: e = std::make_unique<EBool>(l <= r, n.s); return true;
    case BK::Gt: e = std::make_unique<EBool>(l > r, n.s); return true;
    case BK::Gte: e = std::make_unique<EBool>(l >= r, n.s); return true;
    case BK::And:
    case BK::Or: return false;
    }
  }
  if (lb && rb) {
    switch (n.op) {
    case BK::Eq: e = std::make_unique<EBool>(*lb == *rb, n.s); return true;
    case BK::Neq: e = std::make_unique<EBool>(*lb != *rb, n.s); return true;
    case BK::And: e = std::make_unique<EBool>(*lb && *rb, n.s); return true;
    case BK::Or: e = std::make_unique<EBool>(*lb || *rb, n.s); return true;
    default: break;
    }
  }
  if ((li || lf) && (ri || rf)) {
    double l = lf ? *lf : static_cast<double>(*li), r = rf ? *rf : static_cast<double>(*ri);
    switch (n.op) {
    case BK::Add: e = std::make_unique<EFloat>(l + r, n.s); return true;
    case BK::Sub: e = std::make_unique<EFloat>(l - r, n.s); return true;
    case BK::Mul: e = std::make_unique<EFloat>(l * r, n.s); return true;
    case BK::Div: if (r != 0.0) { e = std::make_unique<EFloat>(l / r, n.s); return true; } return false;
    case BK::Pow: e = std::make_unique<EFloat>(std::pow(l, r), n.s); return true;
    case BK::Eq: e = std::make_unique<EBool>(l == r, n.s); return true;
    case BK::Neq: e = std::make_unique<EBool>(l != r, n.s); return true;
    case BK::Lt: e = std::make_unique<EBool>(l < r, n.s); return true;
    case BK::Lte: e = std::make_unique<EBool>(l <= r, n.s); return true;
    case BK::Gt: e = std::make_unique<EBool>(l > r, n.s); return true;
    case BK::Gte: e = std::make_unique<EBool>(l >= r, n.s); return true;
    case BK::Mod: return false;
    case BK::And:
    case BK::Or: return false;
    }
  }
  return false;
}

static bool optE(EP &e, const std::unordered_map<std::string, const Fn *> &cand) {
  bool ch = false;
  switch (e->k) {
  case EK::Unary: {
    auto &n = static_cast<EUnary &>(*e);
    ch |= optE(n.x, cand);
    if (!n.overrideFn.empty()) return ch;
    if (n.op == UK::Neg) {
      if (auto i = litI(*n.x)) { e = std::make_unique<EInt>(-*i, n.s); return true; }
      if (auto f = litF(*n.x)) { e = std::make_unique<EFloat>(-*f, n.s); return true; }
    } else if (auto b = litB(*n.x)) {
      e = std::make_unique<EBool>(!*b, n.s);
      return true;
    }
    return ch;
  }
  case EK::Binary: {
    auto &n = static_cast<EBinary &>(*e);
    ch |= optE(n.l, cand);
    ch |= optE(n.r, cand);
    if (foldBin(n, e)) return true;
    switch (n.op) {
    case BK::Add:
      if (isZero(*n.r)) { e = std::move(n.l); return true; }
      if (isZero(*n.l)) { e = std::move(n.r); return true; }
      break;
    case BK::Sub:
      if (isZero(*n.r)) { e = std::move(n.l); return true; }
      if (n.l->k == EK::Var && n.r->k == EK::Var) {
        const auto &lv = static_cast<const EVar &>(*n.l);
        const auto &rv = static_cast<const EVar &>(*n.r);
        if (lv.n == rv.n && e->typed && e->inf == Type::I64) {
          e = std::make_unique<EInt>(0, n.s);
          return true;
        }
      }
      break;
    case BK::Mul:
      if (isOne(*n.r)) { e = std::move(n.l); return true; }
      if (isOne(*n.l)) { e = std::move(n.r); return true; }
      break;
    case BK::Div:
      if (isOne(*n.r)) { e = std::move(n.l); return true; }
      break;
    case BK::And:
      if (auto lb = litB(*n.l)) {
        e = *lb ? std::move(n.r) : std::make_unique<EBool>(false, n.s);
        return true;
      }
      if (auto rb = litB(*n.r)) {
        e = *rb ? std::move(n.l) : std::make_unique<EBool>(false, n.s);
        return true;
      }
      break;
    case BK::Or:
      if (auto lb = litB(*n.l)) {
        e = *lb ? std::make_unique<EBool>(true, n.s) : std::move(n.r);
        return true;
      }
      if (auto rb = litB(*n.r)) {
        e = *rb ? std::make_unique<EBool>(true, n.s) : std::move(n.l);
        return true;
      }
      break;
    case BK::Pow:
      if (isOne(*n.r)) { e = std::move(n.l); return true; }
      break;
    default: break;
    }
    return ch;
  }
  case EK::Call: {
    auto &n = static_cast<ECall &>(*e);
    for (EP &a : n.a) ch |= optE(a, cand);
    auto it = cand.find(n.f);
    if (it == cand.end()) return ch;
    const Fn &f = *it->second;
    auto *r = dynamic_cast<const SRet *>(f.b[0].get());
    std::unordered_map<std::string, const Expr *> m;
    for (std::size_t i = 0; i < f.p.size(); ++i) m[f.p[i].n] = n.a[i].get();
    e = substE(*r->v, m);
    ch = true;
    ch |= optE(e, cand);
    return ch;
  }
  default: return ch;
  }
}

static bool optBlock(std::vector<SP> &b, const std::unordered_map<std::string, const Fn *> &cand, bool inLoopBody) {
  bool ch = false;
  for (std::size_t i = 0; i < b.size(); ++i) {
    Stmt &s = *b[i];
    switch (s.k) {
    case SK::Let: ch |= optE(static_cast<SLet &>(s).v, cand); break;
    case SK::Assign: ch |= optE(static_cast<SAssign &>(s).v, cand); break;
    case SK::Expr: ch |= optE(static_cast<SExpr &>(s).e, cand); break;
    case SK::Ret: {
      auto &n = static_cast<SRet &>(s);
      if (n.has && n.v) ch |= optE(n.v, cand);
      break;
    }
    case SK::If: {
      auto &n = static_cast<SIf &>(s);
      ch |= optE(n.c, cand);
      ch |= optBlock(n.t, cand, inLoopBody);
      ch |= optBlock(n.e, cand, inLoopBody);
      if (auto bv = litB(*n.c)) {
        std::vector<SP> rep = std::move(*bv ? n.t : n.e);
        b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
        b.insert(b.begin() + static_cast<std::ptrdiff_t>(i), std::make_move_iterator(rep.begin()), std::make_move_iterator(rep.end()));
        ch = true;
        if (i > 0) --i;
      }
      break;
    }
    case SK::While: {
      auto &n = static_cast<SWhile &>(s);
      ch |= optE(n.c, cand);
      ch |= optBlock(n.b, cand, true);
      if (auto bv = litB(*n.c); bv && !*bv) {
        b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
        ch = true;
        if (i > 0) --i;
      }
      break;
    }
    case SK::For: {
      auto &n = static_cast<SFor &>(s);
      ch |= optE(n.start, cand);
      ch |= optE(n.stop, cand);
      ch |= optE(n.step, cand);
      ch |= optBlock(n.b, cand, true);
      auto resolveLocalI64 = [&](const EP &expr) -> std::optional<int64_t> {
        if (auto v = litI(*expr)) return v;
        if (expr->k != EK::Var) return std::nullopt;
        const std::string name = static_cast<const EVar &>(*expr).n;
        std::optional<std::size_t> declIdx;
        std::optional<int64_t> declVal;
        for (std::size_t j = i; j-- > 0;) {
          if (b[j]->k == SK::Let) {
            const auto &let = static_cast<const SLet &>(*b[j]);
            if (let.n == name) {
              declIdx = j;
              declVal = litI(*let.v);
              break;
            }
          }
          if (b[j]->k == SK::Assign && static_cast<const SAssign &>(*b[j]).n == name) {
            return std::nullopt;
          }
        }
        if (!declIdx || !declVal) return std::nullopt;
        for (std::size_t k = *declIdx + 1; k < i; ++k) {
          if (stmtAssignsNamed(*b[k], name)) return std::nullopt;
        }
        return declVal;
      };
      auto st = resolveLocalI64(n.start);
      auto en = resolveLocalI64(n.stop);
      auto sp = resolveLocalI64(n.step);
      if (st && en && sp) {
        auto tc = tripCount(*st, *en, *sp);
        if (tc && *tc == 0) {
          b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
          ch = true;
          if (i > 0) --i;
          break;
        }
        if (tc && *tc > 0 && !n.parallel && n.b.size() == 1 && n.b[0]->k == SK::For) {
          const auto &inner = static_cast<const SFor &>(*n.b[0]);
          auto ist = litI(*inner.start);
          auto ien = litI(*inner.stop);
          auto isp = litI(*inner.step);
          auto itc = (ist && ien && isp) ? tripCount(*ist, *ien, *isp) : std::nullopt;
          if (!inner.parallel && itc && *itc > 0 &&
              ((inner.b.size() == 1 && inner.b[0]->k == SK::Assign) ||
               (inner.b.size() == 2 && inner.b[0]->k == SK::Let && inner.b[1]->k == SK::Assign))) {
            const SAssign *asPtr = nullptr;
            const Expr *otherExpr = nullptr;
            auto extractOther = [](const SAssign &as, const std::string *requiredVar) -> const Expr * {
              if (as.v->k != EK::Binary) return nullptr;
              const auto &bin = static_cast<const EBinary &>(*as.v);
              if (bin.op != BK::Add) return nullptr;
              const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
              const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
              if (!leftSelf && !rightSelf) return nullptr;
              const Expr &other = leftSelf ? *bin.r : *bin.l;
              if (requiredVar) {
                if (other.k != EK::Var) return nullptr;
                if (static_cast<const EVar &>(other).n != *requiredVar) return nullptr;
              }
              return &other;
            };
            if (inner.b.size() == 1) {
              const auto &as = static_cast<const SAssign &>(*inner.b[0]);
              asPtr = &as;
              otherExpr = extractOther(as, nullptr);
            } else {
              const auto &tmp = static_cast<const SLet &>(*inner.b[0]);
              const auto &as = static_cast<const SAssign &>(*inner.b[1]);
              if (tmp.n != inner.n && !exprHasVarName(*tmp.v, tmp.n)) {
                asPtr = &as;
                otherExpr = extractOther(as, &tmp.n);
                if (otherExpr) otherExpr = tmp.v.get();
              }
            }
            if (asPtr && otherExpr && !exprHasVarName(*otherExpr, asPtr->n)) {
              auto lin = linear2I64ConstExpr(*otherExpr, n.n, inner.n);
              if (lin) {
                const int64_t ni = *tc;
                const int64_t nj = *itc;
                auto sumI = arithmeticSeriesSumI64(ni, *st, *sp);
                auto sumJ = arithmeticSeriesSumI64(nj, *ist, *isp);
                auto axNj = checkedMulI64(lin->ax, nj);
                auto part1 = (axNj && sumI) ? checkedMulI64(*axNj, *sumI) : std::nullopt;
                auto ayNi = checkedMulI64(lin->ay, ni);
                auto part2 = (ayNi && sumJ) ? checkedMulI64(*ayNi, *sumJ) : std::nullopt;
                auto niNj = checkedMulI64(ni, nj);
                auto part3 = niNj ? checkedMulI64(lin->c, *niNj) : std::nullopt;
                auto p12 = (part1 && part2) ? checkedAddI64(*part1, *part2) : std::nullopt;
                auto delta = (p12 && part3) ? checkedAddI64(*p12, *part3) : std::nullopt;
                if (delta) {
                  EP lhs = std::make_unique<EVar>(asPtr->n, asPtr->s);
                  EP rhs = std::make_unique<EInt>(*delta, asPtr->s);
                  EP add = std::make_unique<EBinary>(BK::Add, std::move(lhs), std::move(rhs), asPtr->s);
                  auto replacement = std::make_unique<SAssign>(asPtr->n, std::move(add), n.s);
                  b[i] = std::move(replacement);
                  ch = true;
                  if (i > 0) --i;
                  break;
                }
              }
            }
          }
        }
        if (tc && *tc > 0 && !n.parallel && n.b.size() == 2 &&
            n.b[0]->k == SK::Assign && n.b[1]->k == SK::Assign) {
          const auto &as0 = static_cast<const SAssign &>(*n.b[0]);
          const auto &as1 = static_cast<const SAssign &>(*n.b[1]);
          struct PlusRed {
            std::string var;
            const Expr *other = nullptr;
            Span s;
          };
          auto parsePlusRed = [](const SAssign &as) -> std::optional<PlusRed> {
            if (as.v->k != EK::Binary) return std::nullopt;
            const auto &bin = static_cast<const EBinary &>(*as.v);
            if (bin.op != BK::Add) return std::nullopt;
            const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
            const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
            if (!leftSelf && !rightSelf) return std::nullopt;
            const Expr *other = leftSelf ? bin.r.get() : bin.l.get();
            if (exprHasVarName(*other, as.n)) return std::nullopt;
            return PlusRed{as.n, other, as.s};
          };
          auto r0 = parsePlusRed(as0);
          auto r1 = parsePlusRed(as1);
          auto tryFoldPair = [&](const PlusRed &acc, const PlusRed &state) -> bool {
            if (!acc.other || !state.other) return false;
            if (acc.other->k != EK::Var) return false;
            if (static_cast<const EVar &>(*acc.other).n != state.var) return false;
            if (exprHasVarName(*state.other, acc.var)) return false;
            if (exprHasVarName(*state.other, state.var)) return false;
            auto aff = affineI64ConstExpr(*state.other, n.n);
            if (!aff) return false;
            const int64_t N = *tc;
            auto sumI = arithmeticSeriesSumI64(N, *st, *sp);
            if (!sumI) return false;
            auto dY1 = checkedMulI64(aff->a, *sumI);
            auto dY2 = checkedMulI64(aff->b, N);
            auto deltaY = (dY1 && dY2) ? checkedAddI64(*dY1, *dY2) : std::nullopt;
            auto nMinus1 = checkedAddI64(N, -1);
            auto nMinus2 = checkedAddI64(N, -2);
            auto k1 = nMinus1 ? mul2Div2ExactI64(N, *nMinus1) : std::nullopt;
            auto k2 = (nMinus1 && nMinus2) ? mul3Div6ExactI64(N, *nMinus1, *nMinus2) : std::nullopt;
            auto aStart = checkedMulI64(aff->a, *st);
            auto c0 = aStart ? checkedAddI64(*aStart, aff->b) : std::nullopt;
            auto c1 = checkedMulI64(aff->a, *sp);
            auto gw0 = (c0 && k1) ? checkedMulI64(*c0, *k1) : std::nullopt;
            auto gw1 = (c1 && k2) ? checkedMulI64(*c1, *k2) : std::nullopt;
            auto gw = (gw0 && gw1) ? checkedAddI64(*gw0, *gw1) : std::nullopt;
            if (!deltaY || !gw) return false;

            std::vector<SP> rep;
            rep.reserve(2);
            EP accBase = std::make_unique<EVar>(acc.var, acc.s);
            EP stateVarForMul = std::make_unique<EVar>(state.var, acc.s);
            EP nLit = std::make_unique<EInt>(N, acc.s);
            EP nState = std::make_unique<EBinary>(BK::Mul, std::move(stateVarForMul), std::move(nLit), acc.s);
            EP gwLit = std::make_unique<EInt>(*gw, acc.s);
            EP extra = std::make_unique<EBinary>(BK::Add, std::move(nState), std::move(gwLit), acc.s);
            EP accNew = std::make_unique<EBinary>(BK::Add, std::move(accBase), std::move(extra), acc.s);
            rep.push_back(std::make_unique<SAssign>(acc.var, std::move(accNew), acc.s));

            EP stateBase = std::make_unique<EVar>(state.var, state.s);
            EP dYLit = std::make_unique<EInt>(*deltaY, state.s);
            EP stateNew = std::make_unique<EBinary>(BK::Add, std::move(stateBase), std::move(dYLit), state.s);
            rep.push_back(std::make_unique<SAssign>(state.var, std::move(stateNew), state.s));

            b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
            b.insert(b.begin() + static_cast<std::ptrdiff_t>(i),
                     std::make_move_iterator(rep.begin()), std::make_move_iterator(rep.end()));
            ch = true;
            if (i > 0) --i;
            return true;
          };
          if (r0 && r1) {
            if (tryFoldPair(*r0, *r1)) break;
            if (tryFoldPair(*r1, *r0)) break;
          }
        }
        if (tc && *tc > 0 && !n.parallel &&
            ((n.b.size() == 1 && n.b[0]->k == SK::Assign) ||
             (n.b.size() == 2 && n.b[0]->k == SK::Let && n.b[1]->k == SK::Assign))) {
          const SAssign *asPtr = nullptr;
          const Expr *otherExpr = nullptr;
          auto extractOther = [](const SAssign &as, const std::string *requiredVar) -> const Expr * {
            if (as.v->k != EK::Binary) return nullptr;
            const auto &bin = static_cast<const EBinary &>(*as.v);
            if (bin.op != BK::Add) return nullptr;
            const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
            const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
            if (!leftSelf && !rightSelf) return nullptr;
            const Expr &other = leftSelf ? *bin.r : *bin.l;
            if (requiredVar) {
              if (other.k != EK::Var) return nullptr;
              if (static_cast<const EVar &>(other).n != *requiredVar) return nullptr;
            }
            return &other;
          };
          if (n.b.size() == 1) {
            const auto &as = static_cast<const SAssign &>(*n.b[0]);
            asPtr = &as;
            otherExpr = extractOther(as, nullptr);
          } else {
            const auto &tmp = static_cast<const SLet &>(*n.b[0]);
            const auto &as = static_cast<const SAssign &>(*n.b[1]);
            if (tmp.n != n.n && !exprHasVarName(*tmp.v, tmp.n)) {
              asPtr = &as;
              otherExpr = extractOther(as, &tmp.n);
              if (otherExpr) otherExpr = tmp.v.get();
            }
          }
          if (asPtr && otherExpr && !exprHasVarName(*otherExpr, asPtr->n)) {
            std::optional<int64_t> deltaI;
            if (otherExpr->k == EK::Binary) {
              const auto &mod = static_cast<const EBinary &>(*otherExpr);
              if (mod.op == BK::Mod) {
                auto m = evalConstI64Expr(*mod.r);
                auto affMod = affineI64ConstExpr(*mod.l, n.n);
                if (m && *m > 0 && affMod) {
                  deltaI = modLinearSeriesSumI64(*tc, *st, *sp, affMod->a, affMod->b, *m);
                }
              }
            }
            if (!deltaI) {
              if (auto aff = affineI64ConstExpr(*otherExpr, n.n)) {
                const __int128 iters = static_cast<__int128>(*tc);
                const __int128 start128 = static_cast<__int128>(*st);
                const __int128 step128 = static_cast<__int128>(*sp);
                const __int128 sumIdx = (iters * ((2 * start128) + ((iters - 1) * step128))) / 2;
                const __int128 delta =
                    static_cast<__int128>(aff->a) * sumIdx + static_cast<__int128>(aff->b) * iters;
                deltaI = checkedI128ToI64(delta);
              }
            }
            if (deltaI) {
              EP lhs = std::make_unique<EVar>(asPtr->n, asPtr->s);
              EP rhs = std::make_unique<EInt>(*deltaI, asPtr->s);
              EP add = std::make_unique<EBinary>(BK::Add, std::move(lhs), std::move(rhs), asPtr->s);
              auto replacement = std::make_unique<SAssign>(asPtr->n, std::move(add), n.s);
              b[i] = std::move(replacement);
              ch = true;
              if (i > 0) --i;
              break;
            }
          }
        }
        if (tc && *tc > 0 && !n.parallel && n.b.size() >= 1 && n.b.size() <= 4) {
          bool ok = true;
          std::vector<const SAssign *> assigns;
          assigns.reserve(n.b.size());
          std::unordered_set<std::string> assignedVars;
          for (const auto &stmt : n.b) {
            if (stmt->k != SK::Assign) {
              ok = false;
              break;
            }
            const auto &as = static_cast<const SAssign &>(*stmt);
            assigns.push_back(&as);
            if (!assignedVars.insert(as.n).second) {
              ok = false;
              break;
            }
          }
          if (ok) {
            struct PolyTerm {
              std::string var;
              Poly2I64Const poly;
              Span s;
            };
            std::vector<PolyTerm> terms;
            terms.reserve(assigns.size());
            for (const SAssign *asPtr : assigns) {
              const auto &as = *asPtr;
              if (as.v->k != EK::Binary) {
                ok = false;
                break;
              }
              const auto &bin = static_cast<const EBinary &>(*as.v);
              if (bin.op != BK::Add) {
                ok = false;
                break;
              }
              const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
              const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
              if (!leftSelf && !rightSelf) {
                ok = false;
                break;
              }
              const Expr &other = leftSelf ? *bin.r : *bin.l;
              if (exprHasVarName(other, as.n)) {
                ok = false;
                break;
              }
              for (const std::string &v : assignedVars) {
                if (v != as.n && exprHasVarName(other, v)) {
                  ok = false;
                  break;
                }
              }
              if (!ok) break;
              auto poly = poly2ConstI64Expr(other, n.n);
              if (!poly) {
                ok = false;
                break;
              }
              terms.push_back(PolyTerm{as.n, *poly, as.s});
            }
            if (ok) {
              const int64_t N = *tc;
              const int64_t S = *st;
              const int64_t D = *sp;
              auto nMinus1 = checkedAddI64(N, -1);
              if (!nMinus1) ok = false;
              auto twoN = nMinus1 ? checkedMulI64(2, N) : std::nullopt;
              auto twoNMinus1 = twoN ? checkedAddI64(*twoN, -1) : std::nullopt;
              auto sumK = nMinus1 ? mul2Div2ExactI64(N, *nMinus1) : std::nullopt;
              auto sumK2 = (nMinus1 && twoNMinus1) ? mul3Div6ExactI64(N, *nMinus1, *twoNMinus1) : std::nullopt;
              auto nMulS = checkedMulI64(N, S);
              auto dMulSumK = sumK ? checkedMulI64(D, *sumK) : std::nullopt;
              auto sumI = (nMulS && dMulSumK) ? checkedAddI64(*nMulS, *dMulSumK) : std::nullopt;
              auto sSq = checkedMulI64(S, S);
              auto dSq = checkedMulI64(D, D);
              auto term1 = (sSq ? checkedMulI64(N, *sSq) : std::nullopt);
              auto sMulD = checkedMulI64(S, D);
              auto twoSMulD = (sMulD ? checkedMulI64(2, *sMulD) : std::nullopt);
              auto term2 = (twoSMulD && sumK) ? checkedMulI64(*twoSMulD, *sumK) : std::nullopt;
              auto term3 = (dSq && sumK2) ? checkedMulI64(*dSq, *sumK2) : std::nullopt;
              auto sumI2a = (term1 && term2) ? checkedAddI64(*term1, *term2) : std::nullopt;
              auto sumI2 = (sumI2a && term3) ? checkedAddI64(*sumI2a, *term3) : std::nullopt;
              if (!nMinus1 || !twoNMinus1 || !sumK || !sumK2 || !sumI || !sumI2) ok = false;
              std::vector<SP> rep;
              rep.reserve(terms.size());
              if (ok) {
                for (const auto &t : terms) {
                  const __int128 delta = static_cast<__int128>(t.poly.c2) * static_cast<__int128>(*sumI2) +
                                         static_cast<__int128>(t.poly.c1) * static_cast<__int128>(*sumI) +
                                         static_cast<__int128>(t.poly.c0) * static_cast<__int128>(N);
                  auto deltaI = checkedI128ToI64(delta);
                  if (!deltaI) {
                    ok = false;
                    break;
                  }
                  EP lhs = std::make_unique<EVar>(t.var, t.s);
                  EP rhs = std::make_unique<EInt>(*deltaI, t.s);
                  EP add = std::make_unique<EBinary>(BK::Add, std::move(lhs), std::move(rhs), t.s);
                  rep.push_back(std::make_unique<SAssign>(t.var, std::move(add), t.s));
                }
              }
              if (ok) {
                b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
                b.insert(b.begin() + static_cast<std::ptrdiff_t>(i), std::make_move_iterator(rep.begin()),
                         std::make_move_iterator(rep.end()));
                ch = true;
                if (i > 0) --i;
                break;
              }
            }
          }
        }
        if (tc && *tc > 0 && !n.parallel && n.b.size() >= 2 && n.b.size() <= 4) {
          bool ok = true;
          std::vector<const SAssign *> assigns;
          assigns.reserve(n.b.size());
          std::unordered_set<std::string> assignedVars;
          for (const auto &stmt : n.b) {
            if (stmt->k != SK::Assign) {
              ok = false;
              break;
            }
            const auto &as = static_cast<const SAssign &>(*stmt);
            assigns.push_back(&as);
            if (!assignedVars.insert(as.n).second) {
              ok = false;
              break;
            }
          }
          if (ok) {
            struct RedTerm {
              std::string var;
              AffineI64Const aff;
              Span s;
            };
            std::vector<RedTerm> terms;
            terms.reserve(assigns.size());
            for (const SAssign *asPtr : assigns) {
              const auto &as = *asPtr;
              if (as.v->k != EK::Binary) {
                ok = false;
                break;
              }
              const auto &bin = static_cast<const EBinary &>(*as.v);
              if (bin.op != BK::Add) {
                ok = false;
                break;
              }
              const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
              const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
              if (!leftSelf && !rightSelf) {
                ok = false;
                break;
              }
              const Expr &other = leftSelf ? *bin.r : *bin.l;
              if (exprHasVarName(other, as.n)) {
                ok = false;
                break;
              }
              for (const std::string &v : assignedVars) {
                if (v != as.n && exprHasVarName(other, v)) {
                  ok = false;
                  break;
                }
              }
              if (!ok) break;
              auto aff = affineI64ConstExpr(other, n.n);
              if (!aff) {
                ok = false;
                break;
              }
              terms.push_back(RedTerm{as.n, *aff, as.s});
            }
            if (ok) {
              const __int128 iters = static_cast<__int128>(*tc);
              const __int128 start128 = static_cast<__int128>(*st);
              const __int128 step128 = static_cast<__int128>(*sp);
              const __int128 sumIdx = (iters * ((2 * start128) + ((iters - 1) * step128))) / 2;
              std::vector<SP> rep;
              rep.reserve(terms.size());
              for (const auto &t : terms) {
                const __int128 delta =
                    static_cast<__int128>(t.aff.a) * sumIdx + static_cast<__int128>(t.aff.b) * iters;
                auto deltaI = checkedI128ToI64(delta);
                if (!deltaI) {
                  ok = false;
                  break;
                }
                EP lhs = std::make_unique<EVar>(t.var, t.s);
                EP rhs = std::make_unique<EInt>(*deltaI, t.s);
                EP add = std::make_unique<EBinary>(BK::Add, std::move(lhs), std::move(rhs), t.s);
                rep.push_back(std::make_unique<SAssign>(t.var, std::move(add), t.s));
              }
              if (ok) {
                b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
                b.insert(b.begin() + static_cast<std::ptrdiff_t>(i), std::make_move_iterator(rep.begin()),
                         std::make_move_iterator(rep.end()));
                ch = true;
                if (i > 0) --i;
                break;
              }
            }
          }
        }
        if (tc && *tc > 0 && *tc <= 8 && !n.parallel && !hasLoopControlStmt(n.b) && !hasDeclNamed(n.b, n.n)) {
          std::vector<SP> rep;
          rep.reserve(static_cast<std::size_t>(*tc) * n.b.size());
          int64_t cur = *st;
          for (int64_t iter = 0; iter < *tc; ++iter) {
            std::vector<SP> chunk = substBlockVar(n.b, n.n, cur);
            rep.insert(rep.end(), std::make_move_iterator(chunk.begin()), std::make_move_iterator(chunk.end()));
            cur += *sp;
          }
          b.erase(b.begin() + static_cast<std::ptrdiff_t>(i));
          b.insert(b.begin() + static_cast<std::ptrdiff_t>(i), std::make_move_iterator(rep.begin()),
                   std::make_move_iterator(rep.end()));
          ch = true;
          if (i > 0) --i;
        }
      }
      break;
    }
    case SK::FormatBlock: {
      auto &n = static_cast<SFormatBlock &>(s);
      if (n.endArg) ch |= optE(n.endArg, cand);
      ch |= optBlock(n.b, cand, inLoopBody);
      break;
    }
    case SK::Break:
    case SK::Continue:
      break;
    }

    if (i < b.size()) {
      Stmt &tail = *b[i];
      if (tail.k == SK::Ret || tail.k == SK::Break || tail.k == SK::Continue) {
        if (i + 1 < b.size()) {
          b.erase(b.begin() + static_cast<std::ptrdiff_t>(i + 1), b.end());
          ch = true;
        }
        break;
      }
    }
  }
  if (!inLoopBody) ch |= pruneDeadLocalStores(b);
  ch |= propagateLocalI64Consts(b);
  return ch;
}

static void optimize(Program &p, int passes) {
#if LS_HOST_NO_INT128
  (void)p;
  (void)passes;
  return;
#else
  gOptHasUnaryNegOverride = false;
  for (const Fn &f : p.f) {
    if (f.n == unaryOperatorOverrideSymbol(UK::Neg)) {
      gOptHasUnaryNegOverride = true;
      break;
    }
  }
  for (int k = 0; k < passes; ++k) {
    auto c = inlineCands(p);
    bool ch = false;
    for (Fn &f : p.f)
      if (!f.ex) ch |= optBlock(f.b, c, false);
    if (!ch) break;
  }
  gOptHasUnaryNegOverride = false;
#endif
}

class EmitC {
public:
  EmitC(const Program &p, std::unordered_set<std::string> inl, bool superuserMode = false,
        bool superuserStartEnabled = false,
        std::vector<std::string> activeCliFlags = {},
        std::vector<std::string> cliCustomTokens = {})
      : p_(p), inl_(std::move(inl)), superuserMode_(superuserMode), superuserStartEnabled_(superuserStartEnabled),
        cliCustomTokens_(std::move(cliCustomTokens)) {
    std::unordered_map<std::string, std::string> flagSymbols;
    for (const Fn &f : p_.f) {
      if (!f.isCliFlag) continue;
      flagSymbols[f.cliFlagName] = f.n;
    }
    std::unordered_set<std::string> seenFlags;
    for (const std::string &flagName : activeCliFlags) {
      auto it = flagSymbols.find(flagName);
      if (it == flagSymbols.end()) continue;
      if (!seenFlags.insert(flagName).second) continue;
      activeCliFlagCalls_.push_back(it->second);
    }
    minimalRuntime_ = hasOnlyMinimalRuntimeCallsProgram(p_) && !usesStringRuntimeProgram(p_);
    ultraMinimalRuntime_ =
#if defined(_WIN32)
        minimalRuntime_ && hasOnlyUltraMinimalRuntimeCallsProgram(p_) && !usesStringRuntimeProgram(p_) &&
        !usesF64Program(p_) && !hasCallNamedProgram(p_, "stateSpeed") && !hasCallNamedProgram(p_, ".stateSpeed");
#else
        false;
#endif
    needsForRuntime_ = hasForProgram(p_);
    needsPowRuntime_ = hasPowProgram(p_);
    needsStateSpeedRuntime_ = hasCallNamedProgram(p_, "stateSpeed") || hasCallNamedProgram(p_, ".stateSpeed");
    needsFormatOutputRuntime_ = hasCallNamedProgram(p_, "formatOutput") || hasCallNamedProgram(p_, "FormatOutput");
    needsHttpRuntime_ = hasCallNamedProgram(p_, "http_server_listen") || hasCallNamedProgram(p_, "http_server_accept") ||
                        hasCallNamedProgram(p_, "http_server_read") ||
                        hasCallNamedProgram(p_, "http_server_respond_text") ||
                        hasCallNamedProgram(p_, "http_server_close") ||
                        hasCallNamedProgram(p_, "http_client_connect") ||
                        hasCallNamedProgram(p_, "http_client_send") ||
                        hasCallNamedProgram(p_, "http_client_read") ||
                        hasCallNamedProgram(p_, "http_client_close");
    superuserDebugToStderr_ = superuserMode_ && hasFormatMarkerProgram(p_);
    superuserIrDumpRequested_ = superuserMode_ && hasCallNamedProgram(p_, "su.ir.dump");
    const Fn *mainEntry = nullptr;
    const Fn *scriptEntry = nullptr;
    const Fn *singleZeroArg = nullptr;
    int zeroArgCount = 0;
    for (const Fn &f : p_.f) {
      if (f.ex || f.isCliFlag) continue;
      if (f.n == "main") mainEntry = &f;
      if (f.n == kScriptEntryName) scriptEntry = &f;
      if (f.p.empty()) {
        ++zeroArgCount;
        if (!singleZeroArg) singleZeroArg = &f;
      }
    }
    if (scriptEntry) {
      entry_ = scriptEntry;
    } else if (mainEntry) {
      entry_ = mainEntry;
    } else if (zeroArgCount == 1) {
      entry_ = singleZeroArg;
    } else if (zeroArgCount == 0) {
      entryError_ = "no runnable entry point found; add top-level statements, define main(), or define one zero-argument function";
    } else {
      entryError_ =
          "ambiguous entry point: multiple zero-argument functions found; define main(), add top-level statements, or keep only one zero-argument function";
    }
  }
  bool ultraMinimalRuntime() const { return ultraMinimalRuntime_; }
  bool hasEntry() const { return entry_ != nullptr; }
  const std::string &entryError() const { return entryError_; }
  bool superuserMode() const { return superuserMode_; }
  bool superuserDebugToStderr() const { return superuserDebugToStderr_; }
  bool superuserIrDumpRequested() const { return superuserIrDumpRequested_; }
  std::string run() {
    o_ << "#include <stdint.h>\n";
    o_ << "#include <stddef.h>\n\n";
    o_ << "#include <stdio.h>\n";
    o_ << "#include <stdlib.h>\n";
    o_ << "#include <time.h>\n\n";
    o_ << "#include <math.h>\n\n";
    o_ << "#include <string.h>\n\n";
    o_ << "#include <ctype.h>\n\n";
#if defined(_WIN32)
    if (needsHttpRuntime_) {
      o_ << "#include <winsock2.h>\n";
      o_ << "#include <ws2tcpip.h>\n\n";
    }
    o_ << "#include <windows.h>\n\n";
    o_ << "#ifdef max\n";
    o_ << "#undef max\n";
    o_ << "#endif\n";
    o_ << "#ifdef min\n";
    o_ << "#undef min\n";
    o_ << "#endif\n\n";
#else
    if (needsHttpRuntime_) {
      o_ << "#include <sys/types.h>\n";
      o_ << "#include <sys/socket.h>\n";
      o_ << "#include <netinet/in.h>\n";
      o_ << "#include <netinet/tcp.h>\n";
      o_ << "#include <arpa/inet.h>\n";
      o_ << "#include <unistd.h>\n\n";
    }
    o_ << "#include <pthread.h>\n\n";
#endif
    o_ << "typedef uint8_t ls_bool;\n\n";
    o_ << "static ls_bool ls_su_enabled = " << (superuserStartEnabled_ ? "1" : "0") << ";\n";
    o_ << "static ls_bool ls_su_trace = 0;\n";
    o_ << "static int64_t ls_su_step_limit = 0;\n";
    o_ << "static int64_t ls_su_step_count = 0;\n";
    o_ << "static int64_t ls_su_mem_limit = 0;\n";
    o_ << "static int64_t ls_su_mem_in_use = 0;\n";
    o_ << "static ls_bool ls_su_debug_to_stderr = "
       << (superuserDebugToStderr_ ? "1" : "0") << ";\n\n";
    o_ << "#if defined(_MSC_VER) && !defined(__clang__)\n";
    o_ << "typedef int64_t LS_I128;\n";
    o_ << "#else\n";
    o_ << "typedef __int128 LS_I128;\n";
    o_ << "#endif\n\n";
    o_ << "#define LS_DO_PRAGMA(...) _Pragma(#__VA_ARGS__)\n";
    o_ << "#if defined(_OPENMP)\n";
    o_ << "#define LS_PAR_FOR _Pragma(\"omp parallel for simd schedule(static)\")\n";
    o_ << "#define LS_PAR_FOR_IF(c) LS_DO_PRAGMA(omp parallel for simd schedule(static) if(c))\n";
    o_ << "#define LS_PAR_MIN_ITERS 8000000LL\n";
    o_ << "#define LS_OMP_SIMD LS_DO_PRAGMA(omp simd)\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS(v) LS_DO_PRAGMA(omp simd reduction(+:v))\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS2(a,b) LS_DO_PRAGMA(omp simd reduction(+:a,b))\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS3(a,b,c) LS_DO_PRAGMA(omp simd reduction(+:a,b,c))\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS4(a,b,c,d) LS_DO_PRAGMA(omp simd reduction(+:a,b,c,d))\n";
    o_ << "#else\n";
    o_ << "#define LS_PAR_FOR\n";
    o_ << "#define LS_PAR_FOR_IF(c)\n";
    o_ << "#define LS_PAR_MIN_ITERS 8000000LL\n";
    o_ << "#define LS_OMP_SIMD\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS(v)\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS2(a,b)\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS3(a,b,c)\n";
    o_ << "#define LS_OMP_SIMD_REDUCTION_PLUS4(a,b,c,d)\n";
    o_ << "#endif\n\n";
    o_ << "#if defined(__clang__)\n";
    o_ << "#define LS_VEC_HINT _Pragma(\"clang loop vectorize(enable) interleave(enable)\")\n";
    o_ << "#elif defined(__GNUC__)\n";
    o_ << "#define LS_VEC_HINT _Pragma(\"GCC ivdep\")\n";
    o_ << "#else\n";
    o_ << "#define LS_VEC_HINT\n";
    o_ << "#endif\n\n";
    o_ << "#if defined(_MSC_VER)\n";
    o_ << "#define LS_ALWAYS_INLINE __forceinline\n";
    o_ << "#elif defined(__clang__) || defined(__GNUC__)\n";
    o_ << "#define LS_ALWAYS_INLINE __attribute__((always_inline)) inline\n";
    o_ << "#else\n";
    o_ << "#define LS_ALWAYS_INLINE inline\n";
    o_ << "#endif\n\n";
    if (ultraMinimalRuntime_)
      emitBuiltinsUltraMinimal();
    else if (minimalRuntime_)
      emitBuiltinsMinimal();
    else
      emitBuiltins();
    for (const Fn &f : p_.f) proto(f);
    o_ << '\n';
    for (const Fn &f : p_.f)
      if (!f.ex) fn(f);
    if (entry_) emitEntryWrapper();
    return o_.str();
  }

private:
  const Program &p_;
  std::unordered_set<std::string> inl_;
  bool minimalRuntime_ = false;
  bool ultraMinimalRuntime_ = false;
  bool needsForRuntime_ = false;
  bool needsPowRuntime_ = false;
  bool needsStateSpeedRuntime_ = false;
  bool needsFormatOutputRuntime_ = false;
  bool needsHttpRuntime_ = false;
  bool superuserMode_ = false;
  bool superuserStartEnabled_ = false;
  bool superuserDebugToStderr_ = false;
  bool superuserIrDumpRequested_ = false;
  std::vector<std::string> activeCliFlagCalls_;
  std::vector<std::string> cliCustomTokens_;
  std::ostringstream o_;
  const Fn *entry_ = nullptr;
  std::string entryError_;
  int loopSerial_ = 0;
  std::string stateSpeedVar_;
  std::string activeFnName_;
  Type activeFnRet_ = Type::Void;
  struct CleanupItem {
    std::string var;
    std::string freeFn;
  };
  struct CleanupScope {
    std::vector<CleanupItem> items;
    bool loopBoundary = false;
  };
  std::vector<CleanupScope> cleanupScopes_;
  static constexpr const char *kEntryName = "__linescript_main";

  void pushCleanupScope(bool loopBoundary = false) { cleanupScopes_.push_back(CleanupScope{{}, loopBoundary}); }

  void popCleanupScope() {
    if (!cleanupScopes_.empty()) cleanupScopes_.pop_back();
  }

  void registerOwned(const std::string &var, const std::string &freeFn) {
    if (cleanupScopes_.empty() || freeFn.empty()) return;
    cleanupScopes_.back().items.push_back(CleanupItem{var, freeFn});
  }

  void emitScopeCleanup(std::size_t scopeIndex, int indentLevel) {
    if (scopeIndex >= cleanupScopes_.size()) return;
    auto &items = cleanupScopes_[scopeIndex].items;
    for (std::size_t i = items.size(); i-- > 0;) {
      ind(o_, indentLevel);
      o_ << items[i].freeFn << "(" << items[i].var << ");\n";
    }
  }

  void emitCurrentScopeCleanup(int indentLevel) {
    if (cleanupScopes_.empty()) return;
    emitScopeCleanup(cleanupScopes_.size() - 1, indentLevel);
  }

  void emitCleanupForReturn(int indentLevel) {
    for (std::size_t i = cleanupScopes_.size(); i-- > 0;) emitScopeCleanup(i, indentLevel);
  }

  void emitCleanupForLoopTransfer(int indentLevel) {
    for (std::size_t i = cleanupScopes_.size(); i-- > 0;) {
      emitScopeCleanup(i, indentLevel);
      if (cleanupScopes_[i].loopBoundary) break;
    }
  }

  std::string cFnName(const std::string &n) const {
    if (entry_ && n == entry_->n) return kEntryName;
    return n;
  }

  static std::string cType(Type t) {
    switch (t) {
    case Type::I32: return "int32_t";
    case Type::I64: return "int64_t";
    case Type::F32: return "float";
    case Type::F64: return "double";
    case Type::Bool: return "ls_bool";
    case Type::Str: return "const char *";
    case Type::Void: return "void";
    }
    return "void";
  }
  static std::string uop(UK o) { return o == UK::Neg ? "-" : "!"; }
  static std::string bop(BK o) {
    switch (o) {
    case BK::Add: return "+";
    case BK::Sub: return "-";
    case BK::Mul: return "*";
    case BK::Div: return "/";
    case BK::Mod: return "%";
    case BK::Pow: return "^";
    case BK::Eq: return "==";
    case BK::Neq: return "!=";
    case BK::Lt: return "<";
    case BK::Lte: return "<=";
    case BK::Gt: return ">";
    case BK::Gte: return ">=";
    case BK::And: return "&&";
    case BK::Or: return "||";
    }
    return "?";
  }
  static const char *stmtKindName(SK k) {
    switch (k) {
    case SK::Let: return "declare";
    case SK::Assign: return "assign";
    case SK::Expr: return "expr";
    case SK::Ret: return "return";
    case SK::If: return "if";
    case SK::While: return "while";
    case SK::For: return "for";
    case SK::FormatBlock: return "format_block";
    case SK::Break: return "break";
    case SK::Continue: return "continue";
    }
    return "stmt";
  }
  static std::string dLit(double v) {
    std::ostringstream s;
    s << std::setprecision(17) << v;
    std::string t = s.str();
    if (t.find('.') == std::string::npos && t.find('e') == std::string::npos && t.find('E') == std::string::npos)
      t += ".0";
    return t;
  }
  static bool exprHasVar(const Expr &e, const std::string &name) {
    switch (e.k) {
    case EK::Var:
      return static_cast<const EVar &>(e).n == name;
    case EK::Unary:
      return exprHasVar(*static_cast<const EUnary &>(e).x, name);
    case EK::Binary: {
      const auto &n = static_cast<const EBinary &>(e);
      return exprHasVar(*n.l, name) || exprHasVar(*n.r, name);
    }
    case EK::Call: {
      const auto &n = static_cast<const ECall &>(e);
      for (const auto &arg : n.a)
        if (exprHasVar(*arg, name)) return true;
      return false;
    }
    default:
      return false;
    }
  }
  static bool exprHasAnyCall(const Expr &e) {
    switch (e.k) {
    case EK::Call:
      return true;
    case EK::Unary:
      return exprHasAnyCall(*static_cast<const EUnary &>(e).x);
    case EK::Binary: {
      const auto &n = static_cast<const EBinary &>(e);
      return exprHasAnyCall(*n.l) || exprHasAnyCall(*n.r);
    }
    default:
      return false;
    }
  }
  static bool vectorHintEligible(const SFor &loop) {
    if (loop.b.empty()) return false;
    std::unordered_set<std::string> locals;
    for (const SP &stmt : loop.b) {
      if (stmt->k == SK::Let) {
        locals.insert(static_cast<const SLet &>(*stmt).n);
      } else if (stmt->k != SK::Assign && stmt->k != SK::Expr) {
        return false;
      }
    }
    for (const SP &stmt : loop.b) {
      switch (stmt->k) {
      case SK::Let: {
        const auto &n = static_cast<const SLet &>(*stmt);
        if (exprHasAnyCall(*n.v)) return false;
        break;
      }
      case SK::Assign: {
        const auto &n = static_cast<const SAssign &>(*stmt);
        if (!locals.count(n.n)) return false;
        if (exprHasAnyCall(*n.v)) return false;
        break;
      }
      case SK::Expr: {
        const auto &n = static_cast<const SExpr &>(*stmt);
        if (exprHasAnyCall(*n.e)) return false;
        break;
      }
      default:
        return false;
      }
    }
    return true;
  }
  struct PlusReductionInfo {
    std::string var;
    const Expr *other = nullptr;
  };
  static std::optional<PlusReductionInfo> plusReductionFromAssign(
      const SAssign &as,
      const std::string &loopVar,
      const Expr *substituteOther = nullptr,
      const std::string *substituteName = nullptr) {
    if (as.n == loopVar) return std::nullopt;
    if (as.v->k != EK::Binary) return std::nullopt;
    const auto &bin = static_cast<const EBinary &>(*as.v);
    if (bin.op != BK::Add) return std::nullopt;
    const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
    const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
    if (!leftSelf && !rightSelf) return std::nullopt;
    const Expr &other = leftSelf ? *bin.r : *bin.l;
    const Expr *useOther = &other;
    if (substituteOther != nullptr) {
      if (!substituteName || other.k != EK::Var) return std::nullopt;
      const auto &v = static_cast<const EVar &>(other);
      if (v.n != *substituteName) return std::nullopt;
      useOther = substituteOther;
    }
    if (exprHasVar(*useOther, as.n)) return std::nullopt;
    return PlusReductionInfo{as.n, useOther};
  }
  static std::optional<PlusReductionInfo> plusReductionInfo(const SFor &loop) {
    if (loop.b.size() == 1 && loop.b[0]->k == SK::Assign) {
      return plusReductionFromAssign(static_cast<const SAssign &>(*loop.b[0]), loop.n);
    }
    if (loop.b.size() == 2 && loop.b[0]->k == SK::Let && loop.b[1]->k == SK::Assign) {
      const auto &tmp = static_cast<const SLet &>(*loop.b[0]);
      if (tmp.n == loop.n) return std::nullopt;
      if (exprHasVar(*tmp.v, tmp.n)) return std::nullopt;
      return plusReductionFromAssign(static_cast<const SAssign &>(*loop.b[1]), loop.n, tmp.v.get(), &tmp.n);
    }
    return std::nullopt;
  }
  struct MultiPlusReductionInfo {
    std::vector<PlusReductionInfo> terms;
  };
  std::optional<MultiPlusReductionInfo> multiPlusReductionInfo(const SFor &loop) const {
    if (loop.b.size() < 2 || loop.b.size() > 4) return std::nullopt;
    MultiPlusReductionInfo out;
    out.terms.reserve(loop.b.size());
    std::unordered_set<std::string> vars;
    for (const SP &stmt : loop.b) {
      if (stmt->k != SK::Assign) return std::nullopt;
      auto red = plusReductionFromAssign(static_cast<const SAssign &>(*stmt), loop.n);
      if (!red || !red->other) return std::nullopt;
      if (exprHasAnyCall(*red->other)) return std::nullopt;
      if (!vars.insert(red->var).second) return std::nullopt;
      out.terms.push_back(*red);
    }
    for (const PlusReductionInfo &t : out.terms) {
      for (const std::string &v : vars) {
        if (exprHasVar(*t.other, v)) return std::nullopt;
      }
    }
    return out;
  }
  struct AffineI64Expr {
    std::string a;
    std::string b;
  };
  std::optional<AffineI64Expr> affineI64Expr(const Expr &expr, const std::string &loopVar) const {
    if (expr.inf != Type::I64) return std::nullopt;
    const bool hasLoopVar = exprHasVar(expr, loopVar);
    if (!hasLoopVar) {
      if (exprHasAnyCall(expr)) return std::nullopt;
      return AffineI64Expr{"0LL", "(" + e(expr) + ")"};
    }
    switch (expr.k) {
    case EK::Var: {
      const auto &v = static_cast<const EVar &>(expr);
      if (v.n == loopVar) return AffineI64Expr{"1LL", "0LL"};
      return std::nullopt;
    }
    case EK::Unary: {
      const auto &u = static_cast<const EUnary &>(expr);
      if (!u.overrideFn.empty()) return std::nullopt;
      if (u.op == UK::Neg && gOptHasUnaryNegOverride) return std::nullopt;
      if (u.op != UK::Neg) return std::nullopt;
      auto inner = affineI64Expr(*u.x, loopVar);
      if (!inner) return std::nullopt;
      return AffineI64Expr{"(-(" + inner->a + "))", "(-(" + inner->b + "))"};
    }
    case EK::Binary: {
      const auto &bin = static_cast<const EBinary &>(expr);
      if (!bin.overrideFn.empty()) return std::nullopt;
      if (bin.op == BK::Add || bin.op == BK::Sub) {
        auto l = affineI64Expr(*bin.l, loopVar);
        auto r = affineI64Expr(*bin.r, loopVar);
        if (!l || !r) return std::nullopt;
        const std::string op = (bin.op == BK::Add) ? "+" : "-";
        return AffineI64Expr{"((" + l->a + ")" + op + "(" + r->a + "))", "((" + l->b + ")" + op + "(" + r->b + "))"};
      }
      if (bin.op == BK::Mul) {
        const bool leftHas = exprHasVar(*bin.l, loopVar);
        const bool rightHas = exprHasVar(*bin.r, loopVar);
        if (leftHas && rightHas) return std::nullopt;
        if (leftHas) {
          if (exprHasAnyCall(*bin.r)) return std::nullopt;
          auto l = affineI64Expr(*bin.l, loopVar);
          if (!l) return std::nullopt;
          const std::string c = "(" + e(*bin.r) + ")";
          return AffineI64Expr{"((" + l->a + ")*" + c + ")", "((" + l->b + ")*" + c + ")"};
        }
        if (rightHas) {
          if (exprHasAnyCall(*bin.l)) return std::nullopt;
          auto r = affineI64Expr(*bin.r, loopVar);
          if (!r) return std::nullopt;
          const std::string c = "(" + e(*bin.l) + ")";
          return AffineI64Expr{"((" + r->a + ")*" + c + ")", "((" + r->b + ")*" + c + ")"};
        }
        return std::nullopt;
      }
      return std::nullopt;
    }
    default:
      return std::nullopt;
    }
  }
  struct AffineReductionInfo {
    std::string var;
    std::string a;
    std::string b;
  };
  std::optional<AffineReductionInfo> affineReductionInfo(const SFor &loop) const {
    auto red = plusReductionInfo(loop);
    if (!red || !red->other) return std::nullopt;
    if (exprHasAnyCall(*red->other)) return std::nullopt;
    auto aff = affineI64Expr(*red->other, loop.n);
    if (!aff) return std::nullopt;
    return AffineReductionInfo{red->var, aff->a, aff->b};
  }
  struct ModLinearReductionInfo {
    std::string var;
    int64_t a = 0;
    int64_t b = 0;
    int64_t m = 0;
  };
  static std::optional<ModLinearReductionInfo> modLinearReductionInfo(const SFor &loop) {
    auto red = plusReductionInfo(loop);
    if (!red || !red->other) return std::nullopt;
    if (red->other->k != EK::Binary) return std::nullopt;
    const auto &mod = static_cast<const EBinary &>(*red->other);
    if (mod.op != BK::Mod) return std::nullopt;
    if (exprHasAnyCall(*mod.l) || exprHasAnyCall(*mod.r)) return std::nullopt;
    auto m = evalConstI64Expr(*mod.r);
    if (!m || *m <= 0) return std::nullopt;
    auto aff = affineI64ConstExpr(*mod.l, loop.n);
    if (!aff) return std::nullopt;
    return ModLinearReductionInfo{red->var, aff->a, aff->b, *m};
  }
  struct MultiAffineReductionInfo {
    std::vector<AffineReductionInfo> terms;
  };
  std::optional<MultiAffineReductionInfo> multiAffineReductionInfo(const SFor &loop) const {
    auto multi = multiPlusReductionInfo(loop);
    if (!multi) return std::nullopt;
    MultiAffineReductionInfo out;
    out.terms.reserve(multi->terms.size());
    for (const PlusReductionInfo &t : multi->terms) {
      if (!t.other) return std::nullopt;
      auto aff = affineI64Expr(*t.other, loop.n);
      if (!aff) return std::nullopt;
      out.terms.push_back(AffineReductionInfo{t.var, aff->a, aff->b});
    }
    return out;
  }
  struct PairCoupledInfo {
    std::string accVar;
    std::string stateVar;
    std::string a;
    std::string b;
  };
  std::optional<PairCoupledInfo> pairCoupledInfo(const SFor &loop) const {
    if (loop.b.size() != 2) return std::nullopt;
    if (loop.b[0]->k != SK::Assign || loop.b[1]->k != SK::Assign) return std::nullopt;
    const auto &as0 = static_cast<const SAssign &>(*loop.b[0]);
    const auto &as1 = static_cast<const SAssign &>(*loop.b[1]);
    auto r0 = plusReductionFromAssign(as0, loop.n);
    auto r1 = plusReductionFromAssign(as1, loop.n);
    if (!r0 || !r1 || !r0->other || !r1->other) return std::nullopt;

    auto tryBuild = [&](const PlusReductionInfo &acc, const PlusReductionInfo &state)
        -> std::optional<PairCoupledInfo> {
      if (acc.other->k != EK::Var) return std::nullopt;
      if (static_cast<const EVar &>(*acc.other).n != state.var) return std::nullopt;
      if (exprHasVar(*state.other, acc.var)) return std::nullopt;
      if (exprHasVar(*state.other, state.var)) return std::nullopt;
      if (exprHasAnyCall(*state.other)) return std::nullopt;
      auto aff = affineI64Expr(*state.other, loop.n);
      if (!aff) return std::nullopt;
      return PairCoupledInfo{acc.var, state.var, aff->a, aff->b};
    };

    if (auto out = tryBuild(*r0, *r1)) return out;
    if (auto out = tryBuild(*r1, *r0)) return out;
    return std::nullopt;
  }
  struct AlternatingSignReductionInfo {
    std::string var;
    bool evenPositive = false;
  };
  static std::optional<bool> condTrueMeansEven(const Expr &cond, const std::string &loopVar) {
    if (cond.k != EK::Binary) return std::nullopt;
    const auto &cmp = static_cast<const EBinary &>(cond);
    if (cmp.op != BK::Eq && cmp.op != BK::Neq) return std::nullopt;
    const Expr *modExpr = nullptr;
    const Expr *litExpr = nullptr;
    if (cmp.l->k == EK::Binary && cmp.r->k == EK::Int) {
      modExpr = cmp.l.get();
      litExpr = cmp.r.get();
    } else if (cmp.r->k == EK::Binary && cmp.l->k == EK::Int) {
      modExpr = cmp.r.get();
      litExpr = cmp.l.get();
    } else {
      return std::nullopt;
    }
    const auto &mod = static_cast<const EBinary &>(*modExpr);
    if (mod.op != BK::Mod) return std::nullopt;
    if (mod.l->k != EK::Var || static_cast<const EVar &>(*mod.l).n != loopVar) return std::nullopt;
    if (mod.r->k != EK::Int || static_cast<const EInt &>(*mod.r).v != 2) return std::nullopt;
    const int64_t lit = static_cast<const EInt &>(*litExpr).v;
    if (lit != 0 && lit != 1) return std::nullopt;
    const bool eq = (cmp.op == BK::Eq);
    const bool trueEven = (lit == 0) ? eq : (!eq);
    return trueEven;
  }
  struct LoopVarSignedAssignInfo {
    std::string var;
    int sign = 0;
  };
  static std::optional<LoopVarSignedAssignInfo> loopVarSignedAssignInfo(const Stmt &stmt, const std::string &loopVar) {
    if (stmt.k != SK::Assign) return std::nullopt;
    const auto &as = static_cast<const SAssign &>(stmt);
    if (as.n == loopVar) return std::nullopt;
    if (as.v->k != EK::Binary) return std::nullopt;
    const auto &bin = static_cast<const EBinary &>(*as.v);
    const bool leftSelf = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == as.n);
    const bool rightSelf = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == as.n);
    const bool leftLoop = (bin.l->k == EK::Var && static_cast<const EVar &>(*bin.l).n == loopVar);
    const bool rightLoop = (bin.r->k == EK::Var && static_cast<const EVar &>(*bin.r).n == loopVar);
    if (bin.op == BK::Add) {
      if (leftSelf && rightLoop) return LoopVarSignedAssignInfo{as.n, +1};
      if (rightSelf && leftLoop) return LoopVarSignedAssignInfo{as.n, +1};
    }
    if (bin.op == BK::Sub) {
      if (leftSelf && rightLoop) return LoopVarSignedAssignInfo{as.n, -1};
    }
    return std::nullopt;
  }
  static std::optional<AlternatingSignReductionInfo> alternatingSignReductionInfo(const SFor &loop) {
    if (loop.b.size() != 1 || loop.b[0]->k != SK::If) return std::nullopt;
    if (loop.start->k != EK::Int || static_cast<const EInt &>(*loop.start).v != 0) return std::nullopt;
    if (loop.step->k != EK::Int || static_cast<const EInt &>(*loop.step).v != 1) return std::nullopt;
    const auto &ifs = static_cast<const SIf &>(*loop.b[0]);
    if (ifs.t.size() != 1 || ifs.e.size() != 1) return std::nullopt;
    auto trueEven = condTrueMeansEven(*ifs.c, loop.n);
    if (!trueEven) return std::nullopt;
    auto t = loopVarSignedAssignInfo(*ifs.t[0], loop.n);
    auto e = loopVarSignedAssignInfo(*ifs.e[0], loop.n);
    if (!t || !e) return std::nullopt;
    if (t->var != e->var) return std::nullopt;
    if (!((t->sign == 1 && e->sign == -1) || (t->sign == -1 && e->sign == 1))) return std::nullopt;
    const int evenSign = *trueEven ? t->sign : e->sign;
    return AlternatingSignReductionInfo{t->var, evenSign > 0};
  }
  static std::string cStrLit(const std::string &v) {
    std::string out;
    out.reserve(v.size() + 2);
    out.push_back('"');
    for (char c : v) {
      switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out.push_back(c); break;
      }
    }
    out.push_back('"');
    return out;
  }
  static void ind(std::ostringstream &o, int k) {
    for (int i = 0; i < k; ++i) o << "  ";
  }
  void emitBuiltinsUltraMinimal() {
    o_ << "static inline void *ls_stdout_handle(void) {\n";
    o_ << "  static void *h = (void *)(intptr_t)(-2);\n";
    o_ << "  if (h == (void *)(intptr_t)(-2)) h = GetStdHandle((unsigned long)-11);\n";
    o_ << "  return h;\n";
    o_ << "}\n";
    o_ << "static inline size_t ls_cstr_len(const char *s) {\n";
    o_ << "  if (!s) return 0;\n";
    o_ << "  const char *p = s;\n";
    o_ << "  while (*p) ++p;\n";
    o_ << "  return (size_t)(p - s);\n";
    o_ << "}\n";
    o_ << "static inline void ls_emit_bytes(const char *s, unsigned long n) {\n";
    o_ << "  if (!s || n == 0u) return;\n";
    o_ << "  void *h = ls_stdout_handle();\n";
    o_ << "  if (!h || h == (void *)(intptr_t)(-1)) return;\n";
    o_ << "  unsigned long wrote = 0;\n";
    o_ << "  (void)WriteFile(h, s, n, &wrote, NULL);\n";
    o_ << "}\n";
    o_ << "static inline void ls_emit_text(const char *s) {\n";
    o_ << "  const size_t n = ls_cstr_len(s);\n";
    o_ << "  if (n == 0) return;\n";
    o_ << "  ls_emit_bytes(s, (unsigned long)n);\n";
    o_ << "}\n";
    o_ << "static inline void ls_emit_newline(void) {\n";
    o_ << "  static const char nl = '\\n';\n";
    o_ << "  ls_emit_bytes(&nl, 1u);\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_i64_to_cstr(int64_t v, char *buf) {\n";
    o_ << "  uint64_t x;\n";
    o_ << "  int neg = 0;\n";
    o_ << "  if (v < 0) { neg = 1; x = (uint64_t)(-(v + 1LL)) + 1ULL; }\n";
    o_ << "  else { x = (uint64_t)v; }\n";
    o_ << "  char tmp[32];\n";
    o_ << "  int n = 0;\n";
    o_ << "  do { tmp[n++] = (char)('0' + (x % 10ULL)); x /= 10ULL; } while (x != 0ULL);\n";
    o_ << "  if (neg) tmp[n++] = '-';\n";
    o_ << "  int j = 0;\n";
    o_ << "  while (n > 0) buf[j++] = tmp[--n];\n";
    o_ << "  buf[j] = '\\0';\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline void print_i64(int64_t v) { char b[32]; ls_i64_to_cstr(v, b); ls_emit_text(b); }\n";
    o_ << "static inline void print_f64(double v) { (void)v; print_i64((int64_t)v); }\n";
    o_ << "static inline void print_bool(ls_bool v) { ls_emit_text(v ? \"true\" : \"false\"); }\n";
    o_ << "static inline void print_str(const char *v) { ls_emit_text(v ? v : \"\"); }\n";
    o_ << "static inline void println_i64(int64_t v) { print_i64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_f64(double v) { print_f64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_bool(ls_bool v) { print_bool(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_str(const char *v) { print_str(v); ls_emit_newline(); }\n";
    if (needsForRuntime_) {
      o_ << "static inline int64_t ls_trip_count_runtime(int64_t start, int64_t stop, int64_t step) {\n";
      o_ << "  if (step > 0) { if (start >= stop) return 0; return (int64_t)((((LS_I128)stop - (LS_I128)start) + (LS_I128)step - 1) / (LS_I128)step); }\n";
      o_ << "  if (step < 0) { if (start <= stop) return 0; const int64_t pos = -step; return (int64_t)((((LS_I128)start - (LS_I128)stop) + (LS_I128)pos - 1) / (LS_I128)pos); }\n";
      o_ << "  return 0;\n";
      o_ << "}\n";
      o_ << "static inline int64_t ls_mod_pos_i128(LS_I128 x, int64_t m) { if (m <= 0) return 0; LS_I128 r = x % (LS_I128)m; if (r < 0) r += (LS_I128)m; return (int64_t)r; }\n";
      o_ << "static inline int64_t ls_gcd_nonneg_i64(int64_t a, int64_t b) { if (a < 0) a = -a; if (b < 0) b = -b; while (b != 0) { const int64_t t = a % b; a = b; b = t; } return a; }\n";
      o_ << "static inline int64_t ls_mod_linear_period_i64(int64_t delta, int64_t m) { if (m <= 0) return 0; const int64_t d = ls_mod_pos_i128((LS_I128)delta, m); if (d == 0) return 1; const int64_t g = ls_gcd_nonneg_i64(d, m); if (g <= 0) return 0; return m / g; }\n";
      o_ << "static inline ls_bool ls_mul_u64_overflow(uint64_t a, uint64_t b, uint64_t *out) {\n";
      o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
      o_ << "  if (!out) return 1;\n";
      o_ << "  if (a == 0 || b == 0) { *out = 0; return 0; }\n";
      o_ << "  if (a > (umax / b)) return 1;\n";
      o_ << "  *out = a * b;\n";
      o_ << "  return 0;\n";
      o_ << "}\n";
      o_ << "static inline ls_bool ls_floor_sum_u64_checked(uint64_t n, uint64_t m, uint64_t a, uint64_t b, uint64_t *out) {\n";
      o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
      o_ << "  if (!out || m == 0) return 0;\n";
      o_ << "  uint64_t ans = 0;\n";
      o_ << "  while (1) {\n";
      o_ << "    if (a >= m) {\n";
      o_ << "      const uint64_t q = a / m;\n";
      o_ << "      a %= m;\n";
      o_ << "      uint64_t x = n;\n";
      o_ << "      uint64_t y = (n == 0) ? 0 : (n - 1);\n";
      o_ << "      if ((x & 1ULL) == 0ULL) x >>= 1ULL; else y >>= 1ULL;\n";
      o_ << "      uint64_t tri = 0, addv = 0;\n";
      o_ << "      if (ls_mul_u64_overflow(x, y, &tri) || ls_mul_u64_overflow(tri, q, &addv) || (ans > umax - addv)) return 0;\n";
      o_ << "      ans += addv;\n";
      o_ << "    }\n";
      o_ << "    if (b >= m) {\n";
      o_ << "      const uint64_t q = b / m;\n";
      o_ << "      b %= m;\n";
      o_ << "      uint64_t addv = 0;\n";
      o_ << "      if (ls_mul_u64_overflow(n, q, &addv) || (ans > umax - addv)) return 0;\n";
      o_ << "      ans += addv;\n";
      o_ << "    }\n";
      o_ << "    uint64_t an = 0;\n";
      o_ << "    if (ls_mul_u64_overflow(a, n, &an) || an > umax - b) return 0;\n";
      o_ << "    const uint64_t y = an + b;\n";
      o_ << "    if (y < m) break;\n";
      o_ << "    n = y / m;\n";
      o_ << "    b = y % m;\n";
      o_ << "    const uint64_t t = m; m = a; a = t;\n";
      o_ << "  }\n";
      o_ << "  *out = ans;\n";
      o_ << "  return 1;\n";
      o_ << "}\n";
      o_ << "static inline LS_I128 ls_sum_mod_linear_i128(int64_t n, int64_t m, int64_t a, int64_t b) {\n";
      o_ << "  if (n <= 0 || m <= 0) return 0;\n";
      o_ << "  const int64_t aa = ls_mod_pos_i128((LS_I128)a, m);\n";
      o_ << "  const int64_t bb = ls_mod_pos_i128((LS_I128)b, m);\n";
      o_ << "  uint64_t qsum = 0;\n";
      o_ << "  if (ls_floor_sum_u64_checked((uint64_t)n, (uint64_t)m, (uint64_t)aa, (uint64_t)bb, &qsum)) {\n";
      o_ << "    const LS_I128 nn = (LS_I128)n;\n";
      o_ << "    const LS_I128 tri = (((LS_I128)aa * nn * (nn - 1)) >> 1);\n";
      o_ << "    const LS_I128 total = tri + ((LS_I128)bb * nn);\n";
      o_ << "    return total - ((LS_I128)m * (LS_I128)qsum);\n";
      o_ << "  }\n";
      o_ << "  LS_I128 acc = 0;\n";
      o_ << "  int64_t cur = bb;\n";
      o_ << "  const int64_t delta = aa;\n";
      o_ << "  for (int64_t k = 0; k < n; ++k) {\n";
      o_ << "    acc += (LS_I128)cur;\n";
      o_ << "    cur += delta;\n";
      o_ << "    if (cur >= m) cur -= m;\n";
      o_ << "  }\n";
      o_ << "  return acc;\n";
      o_ << "}\n";
    }
    if (needsPowRuntime_) {
      o_ << "static inline int64_t ls_pow_i64(int64_t base, int64_t exp) { if (exp < 0) return 0; int64_t out = 1; while (exp > 0) { if (exp & 1LL) out *= base; exp >>= 1LL; if (exp > 0) base *= base; } return out; }\n";
      o_ << "#define ls_pow(a,b) ls_pow_i64((int64_t)(a),(int64_t)(b))\n";
    }
    if (needsFormatOutputRuntime_) {
      o_ << "static inline const char *formatOutput_i64(int64_t v) { static char b[32]; ls_i64_to_cstr(v, b); return b; }\n";
      o_ << "static inline const char *formatOutput_f64(double v) { (void)v; return \"0\"; }\n";
      o_ << "static inline const char *formatOutput_bool(ls_bool v) { return v ? \"true\" : \"false\"; }\n";
      o_ << "static inline const char *formatOutput_str(const char *v) { return v ? v : \"\"; }\n";
    }
    o_ << "#define __ls_print_dispatch(x) _Generic((x), int64_t: print_i64, int: print_i64, float: print_f64, double: print_f64, ls_bool: print_bool, const char *: print_str, char *: print_str, default: print_i64)(x)\n";
    o_ << "#define __ls_println_dispatch(x) _Generic((x), int64_t: println_i64, int: println_i64, float: println_f64, double: println_f64, ls_bool: println_bool, const char *: println_str, char *: println_str, default: println_i64)(x)\n";
    o_ << "#define print(x) __ls_print_dispatch(x)\n";
    o_ << "#define println(x) __ls_println_dispatch(x)\n\n";
    if (needsFormatOutputRuntime_) {
      o_ << "#define __ls_format_dispatch(x) _Generic((x), int64_t: formatOutput_i64, int: formatOutput_i64, float: formatOutput_f64, double: formatOutput_f64, ls_bool: formatOutput_bool, const char *: formatOutput_str, char *: formatOutput_str, default: formatOutput_i64)(x)\n";
      o_ << "#define formatOutput(x) __ls_format_dispatch(x)\n";
      o_ << "#define FormatOutput(x) __ls_format_dispatch(x)\n\n";
    }
  }
  void emitBuiltinsMinimal() {
    o_ << "static inline void ls_emit_text(const char *s) { fputs(s ? s : \"\", stdout); }\n";
    o_ << "static inline void ls_emit_newline(void) { fputc('\\n', stdout); }\n";
    o_ << "static inline const char *ls_i64_to_cstr(int64_t v, char *buf) {\n";
    o_ << "  uint64_t x;\n";
    o_ << "  int neg = 0;\n";
    o_ << "  if (v < 0) { neg = 1; x = (uint64_t)(-(v + 1LL)) + 1ULL; }\n";
    o_ << "  else { x = (uint64_t)v; }\n";
    o_ << "  char tmp[32];\n";
    o_ << "  int n = 0;\n";
    o_ << "  do { tmp[n++] = (char)('0' + (x % 10ULL)); x /= 10ULL; } while (x != 0ULL);\n";
    o_ << "  if (neg) tmp[n++] = '-';\n";
    o_ << "  int j = 0;\n";
    o_ << "  while (n > 0) buf[j++] = tmp[--n];\n";
    o_ << "  buf[j] = '\\0';\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline void print_i64(int64_t v) { char b[32]; ls_i64_to_cstr(v, b); ls_emit_text(b); }\n";
    o_ << "static inline void print_f64(double v) { char b[64]; (void)snprintf(b, sizeof(b), \"%.17g\", v); ls_emit_text(b); }\n";
    o_ << "static inline void print_bool(ls_bool v) { ls_emit_text(v ? \"true\" : \"false\"); }\n";
    o_ << "static inline void print_str(const char *v) { ls_emit_text(v ? v : \"\"); }\n";
    o_ << "static inline void println_i64(int64_t v) { print_i64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_f64(double v) { print_f64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_bool(ls_bool v) { print_bool(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_str(const char *v) { puts(v ? v : \"\"); }\n";
    if (needsStateSpeedRuntime_) {
      o_ << "static inline int64_t clock_us(void) { struct timespec ts; timespec_get(&ts, TIME_UTC); return (int64_t)ts.tv_sec * 1000000LL + (int64_t)(ts.tv_nsec / 1000LL); }\n";
      o_ << "static inline void ls_state_speed(int64_t start_us) { const int64_t now_us = clock_us(); const int64_t elapsed_us = now_us >= start_us ? (now_us - start_us) : 0; print_str(\"speed_us=\"); println_i64(elapsed_us); }\n";
    }
    if (needsForRuntime_) {
      o_ << "static inline int64_t ls_trip_count_runtime(int64_t start, int64_t stop, int64_t step) {\n";
      o_ << "  if (step > 0) { if (start >= stop) return 0; return (int64_t)((((LS_I128)stop - (LS_I128)start) + (LS_I128)step - 1) / (LS_I128)step); }\n";
      o_ << "  if (step < 0) { if (start <= stop) return 0; const int64_t pos = -step; return (int64_t)((((LS_I128)start - (LS_I128)stop) + (LS_I128)pos - 1) / (LS_I128)pos); }\n";
      o_ << "  return 0;\n";
      o_ << "}\n";
      o_ << "static inline int64_t ls_mod_pos_i128(LS_I128 x, int64_t m) { if (m <= 0) return 0; LS_I128 r = x % (LS_I128)m; if (r < 0) r += (LS_I128)m; return (int64_t)r; }\n";
      o_ << "static inline int64_t ls_gcd_nonneg_i64(int64_t a, int64_t b) { if (a < 0) a = -a; if (b < 0) b = -b; while (b != 0) { const int64_t t = a % b; a = b; b = t; } return a; }\n";
      o_ << "static inline int64_t ls_mod_linear_period_i64(int64_t delta, int64_t m) { if (m <= 0) return 0; const int64_t d = ls_mod_pos_i128((LS_I128)delta, m); if (d == 0) return 1; const int64_t g = ls_gcd_nonneg_i64(d, m); if (g <= 0) return 0; return m / g; }\n";
      o_ << "static inline ls_bool ls_mul_u64_overflow(uint64_t a, uint64_t b, uint64_t *out) {\n";
      o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
      o_ << "  if (!out) return 1;\n";
      o_ << "  if (a == 0 || b == 0) { *out = 0; return 0; }\n";
      o_ << "  if (a > (umax / b)) return 1;\n";
      o_ << "  *out = a * b;\n";
      o_ << "  return 0;\n";
      o_ << "}\n";
      o_ << "static inline ls_bool ls_floor_sum_u64_checked(uint64_t n, uint64_t m, uint64_t a, uint64_t b, uint64_t *out) {\n";
      o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
      o_ << "  if (!out || m == 0) return 0;\n";
      o_ << "  uint64_t ans = 0;\n";
      o_ << "  while (1) {\n";
      o_ << "    if (a >= m) {\n";
      o_ << "      const uint64_t q = a / m;\n";
      o_ << "      a %= m;\n";
      o_ << "      uint64_t x = n;\n";
      o_ << "      uint64_t y = (n == 0) ? 0 : (n - 1);\n";
      o_ << "      if ((x & 1ULL) == 0ULL) x >>= 1ULL; else y >>= 1ULL;\n";
      o_ << "      uint64_t tri = 0, addv = 0;\n";
      o_ << "      if (ls_mul_u64_overflow(x, y, &tri) || ls_mul_u64_overflow(tri, q, &addv) || (ans > umax - addv)) return 0;\n";
      o_ << "      ans += addv;\n";
      o_ << "    }\n";
      o_ << "    if (b >= m) {\n";
      o_ << "      const uint64_t q = b / m;\n";
      o_ << "      b %= m;\n";
      o_ << "      uint64_t addv = 0;\n";
      o_ << "      if (ls_mul_u64_overflow(n, q, &addv) || (ans > umax - addv)) return 0;\n";
      o_ << "      ans += addv;\n";
      o_ << "    }\n";
      o_ << "    uint64_t an = 0;\n";
      o_ << "    if (ls_mul_u64_overflow(a, n, &an) || an > umax - b) return 0;\n";
      o_ << "    const uint64_t y = an + b;\n";
      o_ << "    if (y < m) break;\n";
      o_ << "    n = y / m;\n";
      o_ << "    b = y % m;\n";
      o_ << "    const uint64_t t = m; m = a; a = t;\n";
      o_ << "  }\n";
      o_ << "  *out = ans;\n";
      o_ << "  return 1;\n";
      o_ << "}\n";
      o_ << "static inline LS_I128 ls_sum_mod_linear_i128(int64_t n, int64_t m, int64_t a, int64_t b) {\n";
      o_ << "  if (n <= 0 || m <= 0) return 0;\n";
      o_ << "  const int64_t aa = ls_mod_pos_i128((LS_I128)a, m);\n";
      o_ << "  const int64_t bb = ls_mod_pos_i128((LS_I128)b, m);\n";
      o_ << "  uint64_t qsum = 0;\n";
      o_ << "  if (ls_floor_sum_u64_checked((uint64_t)n, (uint64_t)m, (uint64_t)aa, (uint64_t)bb, &qsum)) {\n";
      o_ << "    const LS_I128 nn = (LS_I128)n;\n";
      o_ << "    const LS_I128 tri = (((LS_I128)aa * nn * (nn - 1)) >> 1);\n";
      o_ << "    const LS_I128 total = tri + ((LS_I128)bb * nn);\n";
      o_ << "    return total - ((LS_I128)m * (LS_I128)qsum);\n";
      o_ << "  }\n";
      o_ << "  LS_I128 acc = 0;\n";
      o_ << "  int64_t cur = bb;\n";
      o_ << "  const int64_t delta = aa;\n";
      o_ << "  for (int64_t k = 0; k < n; ++k) {\n";
      o_ << "    acc += (LS_I128)cur;\n";
      o_ << "    cur += delta;\n";
      o_ << "    if (cur >= m) cur -= m;\n";
      o_ << "  }\n";
      o_ << "  return acc;\n";
      o_ << "}\n";
    }
    if (needsPowRuntime_) {
      o_ << "static inline int64_t ls_pow_i64(int64_t base, int64_t exp) { if (exp < 0) return 0; int64_t out = 1; while (exp > 0) { if (exp & 1LL) out *= base; exp >>= 1LL; if (exp > 0) base *= base; } return out; }\n";
      o_ << "static inline double ls_pow_f64(double a, double b) { return pow(a, b); }\n";
      o_ << "#define ls_pow(a,b) _Generic(((a)+(b)), double: ls_pow_f64, float: ls_pow_f64, default: ls_pow_i64)((a),(b))\n";
    }
    if (needsFormatOutputRuntime_) {
      o_ << "static inline const char *formatOutput_i64(int64_t v) { static char b[32]; ls_i64_to_cstr(v, b); return b; }\n";
      o_ << "static inline const char *formatOutput_f64(double v) { static char b[64]; (void)snprintf(b, sizeof(b), \"%.17g\", v); return b; }\n";
      o_ << "static inline const char *formatOutput_bool(ls_bool v) { return v ? \"true\" : \"false\"; }\n";
      o_ << "static inline const char *formatOutput_str(const char *v) { return v ? v : \"\"; }\n";
    }
    o_ << "#define __ls_print_dispatch(x) _Generic((x), int64_t: print_i64, int: print_i64, float: print_f64, double: print_f64, ls_bool: print_bool, const char *: print_str, char *: print_str, default: print_i64)(x)\n";
    o_ << "#define __ls_println_dispatch(x) _Generic((x), int64_t: println_i64, int: println_i64, float: println_f64, double: println_f64, ls_bool: println_bool, const char *: println_str, char *: println_str, default: println_i64)(x)\n";
    o_ << "#define print(x) __ls_print_dispatch(x)\n";
    o_ << "#define println(x) __ls_println_dispatch(x)\n\n";
    if (needsFormatOutputRuntime_) {
      o_ << "#define __ls_format_dispatch(x) _Generic((x), int64_t: formatOutput_i64, int: formatOutput_i64, float: formatOutput_f64, double: formatOutput_f64, ls_bool: formatOutput_bool, const char *: formatOutput_str, char *: formatOutput_str, default: formatOutput_i64)(x)\n";
      o_ << "#define formatOutput(x) __ls_format_dispatch(x)\n";
      o_ << "#define FormatOutput(x) __ls_format_dispatch(x)\n\n";
    }
  }
  void emitBuiltins() {
    o_ << "#if defined(_MSC_VER)\n";
    o_ << "#define LS_THREAD_LOCAL __declspec(thread)\n";
    o_ << "#else\n";
    o_ << "#define LS_THREAD_LOCAL _Thread_local\n";
    o_ << "#endif\n";
    o_ << "typedef struct {\n";
    o_ << "  char *buf;\n";
    o_ << "  size_t len;\n";
    o_ << "  size_t cap;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_format_state;\n";
    o_ << "static LS_THREAD_LOCAL ls_format_state ls_fmt = {NULL, 0, 0, 0};\n";
    o_ << "static inline void ls_emit_text(const char *s) {\n";
    o_ << "  const char *txt = s ? s : \"\";\n";
    o_ << "  if (!ls_fmt.active) {\n";
    o_ << "    fputs(txt, stdout);\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  const size_t n = strlen(txt);\n";
    o_ << "  if (n == 0) return;\n";
    o_ << "  const size_t need = ls_fmt.len + n + 1;\n";
    o_ << "  if (need > ls_fmt.cap) {\n";
    o_ << "    size_t nextCap = ls_fmt.cap ? ls_fmt.cap : 64;\n";
    o_ << "    while (nextCap < need) nextCap <<= 1;\n";
    o_ << "    char *next = (char *)realloc(ls_fmt.buf, nextCap);\n";
    o_ << "    if (!next) return;\n";
    o_ << "    ls_fmt.buf = next;\n";
    o_ << "    ls_fmt.cap = nextCap;\n";
    o_ << "  }\n";
    o_ << "  memcpy(ls_fmt.buf + ls_fmt.len, txt, n);\n";
    o_ << "  ls_fmt.len += n;\n";
    o_ << "  ls_fmt.buf[ls_fmt.len] = '\\0';\n";
    o_ << "}\n";
    o_ << "static inline void ls_emit_newline(void) {\n";
    o_ << "  if (!ls_fmt.active) {\n";
    o_ << "    fputc('\\n', stdout);\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  ls_emit_text(\"\\n\");\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_i64_to_cstr(int64_t v, char *buf) {\n";
    o_ << "  uint64_t x;\n";
    o_ << "  int neg = 0;\n";
    o_ << "  if (v < 0) {\n";
    o_ << "    neg = 1;\n";
    o_ << "    x = (uint64_t)(-(v + 1LL)) + 1ULL;\n";
    o_ << "  } else {\n";
    o_ << "    x = (uint64_t)v;\n";
    o_ << "  }\n";
    o_ << "  char tmp[32];\n";
    o_ << "  int n = 0;\n";
    o_ << "  do {\n";
    o_ << "    tmp[n++] = (char)('0' + (x % 10ULL));\n";
    o_ << "    x /= 10ULL;\n";
    o_ << "  } while (x != 0ULL);\n";
    o_ << "  if (neg) tmp[n++] = '-';\n";
    o_ << "  int j = 0;\n";
    o_ << "  while (n > 0) buf[j++] = tmp[--n];\n";
    o_ << "  buf[j] = '\\0';\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline void print_i64(int64_t v) {\n";
    o_ << "  char b[32];\n";
    o_ << "  ls_i64_to_cstr(v, b);\n";
    o_ << "  ls_emit_text(b);\n";
    o_ << "}\n";
    o_ << "static inline void print_f64(double v) {\n";
    o_ << "  char b[64];\n";
    o_ << "  (void)snprintf(b, sizeof(b), \"%.17g\", v);\n";
    o_ << "  ls_emit_text(b);\n";
    o_ << "}\n";
    o_ << "static inline void print_bool(ls_bool v) { ls_emit_text(v ? \"true\" : \"false\"); }\n";
    o_ << "static inline void print_str(const char *v) { ls_emit_text(v ? v : \"\"); }\n";
    o_ << "static inline void println_i64(int64_t v) { print_i64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_f64(double v) { print_f64(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_bool(ls_bool v) { print_bool(v); ls_emit_newline(); }\n";
    o_ << "static inline void println_str(const char *v) { print_str(v); ls_emit_newline(); }\n";
    o_ << "static inline int64_t clock_ms(void) { return (int64_t)((clock() * 1000LL) / CLOCKS_PER_SEC); }\n";
    o_ << "static inline int64_t clock_us(void) {\n";
    o_ << "  struct timespec ts;\n";
    o_ << "  timespec_get(&ts, TIME_UTC);\n";
    o_ << "  return (int64_t)ts.tv_sec * 1000000LL + (int64_t)(ts.tv_nsec / 1000LL);\n";
    o_ << "}\n";
    o_ << "static inline void ls_detach_console(void) {\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  (void)FreeConsole();\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_trip_count_runtime(int64_t start, int64_t stop, int64_t step) {\n";
    o_ << "  if (step > 0) {\n";
    o_ << "    if (start >= stop) return 0;\n";
    o_ << "    return (int64_t)((((LS_I128)stop - (LS_I128)start) + (LS_I128)step - 1) / (LS_I128)step);\n";
    o_ << "  }\n";
    o_ << "  if (step < 0) {\n";
    o_ << "    if (start <= stop) return 0;\n";
    o_ << "    const int64_t pos = -step;\n";
    o_ << "    return (int64_t)((((LS_I128)start - (LS_I128)stop) + (LS_I128)pos - 1) / (LS_I128)pos);\n";
    o_ << "  }\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_mod_pos_i128(LS_I128 x, int64_t m) {\n";
    o_ << "  if (m <= 0) return 0;\n";
    o_ << "  LS_I128 r = x % (LS_I128)m;\n";
    o_ << "  if (r < 0) r += (LS_I128)m;\n";
    o_ << "  return (int64_t)r;\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_gcd_nonneg_i64(int64_t a, int64_t b) {\n";
    o_ << "  if (a < 0) a = -a;\n";
    o_ << "  if (b < 0) b = -b;\n";
    o_ << "  while (b != 0) {\n";
    o_ << "    const int64_t t = a % b;\n";
    o_ << "    a = b;\n";
    o_ << "    b = t;\n";
    o_ << "  }\n";
    o_ << "  return a;\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_mod_linear_period_i64(int64_t delta, int64_t m) {\n";
    o_ << "  if (m <= 0) return 0;\n";
    o_ << "  const int64_t d = ls_mod_pos_i128((LS_I128)delta, m);\n";
    o_ << "  if (d == 0) return 1;\n";
    o_ << "  const int64_t g = ls_gcd_nonneg_i64(d, m);\n";
    o_ << "  if (g <= 0) return 0;\n";
    o_ << "  return m / g;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_mul_u64_overflow(uint64_t a, uint64_t b, uint64_t *out) {\n";
    o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
    o_ << "  if (!out) return 1;\n";
    o_ << "  if (a == 0 || b == 0) { *out = 0; return 0; }\n";
    o_ << "  if (a > (umax / b)) return 1;\n";
    o_ << "  *out = a * b;\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_floor_sum_u64_checked(uint64_t n, uint64_t m, uint64_t a, uint64_t b, uint64_t *out) {\n";
    o_ << "  const uint64_t umax = (uint64_t)(~(uint64_t)0);\n";
    o_ << "  if (!out || m == 0) return 0;\n";
    o_ << "  uint64_t ans = 0;\n";
    o_ << "  while (1) {\n";
    o_ << "    if (a >= m) {\n";
    o_ << "      const uint64_t q = a / m;\n";
    o_ << "      a %= m;\n";
    o_ << "      uint64_t x = n;\n";
    o_ << "      uint64_t y = (n == 0) ? 0 : (n - 1);\n";
    o_ << "      if ((x & 1ULL) == 0ULL) x >>= 1ULL; else y >>= 1ULL;\n";
    o_ << "      uint64_t tri = 0, addv = 0;\n";
    o_ << "      if (ls_mul_u64_overflow(x, y, &tri) || ls_mul_u64_overflow(tri, q, &addv) || (ans > umax - addv)) return 0;\n";
    o_ << "      ans += addv;\n";
    o_ << "    }\n";
    o_ << "    if (b >= m) {\n";
    o_ << "      const uint64_t q = b / m;\n";
    o_ << "      b %= m;\n";
    o_ << "      uint64_t addv = 0;\n";
    o_ << "      if (ls_mul_u64_overflow(n, q, &addv) || (ans > umax - addv)) return 0;\n";
    o_ << "      ans += addv;\n";
    o_ << "    }\n";
    o_ << "    uint64_t an = 0;\n";
    o_ << "    if (ls_mul_u64_overflow(a, n, &an) || an > umax - b) return 0;\n";
    o_ << "    const uint64_t y = an + b;\n";
    o_ << "    if (y < m) break;\n";
    o_ << "    n = y / m;\n";
    o_ << "    b = y % m;\n";
    o_ << "    const uint64_t t = m; m = a; a = t;\n";
    o_ << "  }\n";
    o_ << "  *out = ans;\n";
    o_ << "  return 1;\n";
    o_ << "}\n";
    o_ << "static inline LS_I128 ls_sum_mod_linear_i128(int64_t n, int64_t m, int64_t a, int64_t b) {\n";
    o_ << "  if (n <= 0 || m <= 0) return 0;\n";
    o_ << "  const int64_t aa = ls_mod_pos_i128((LS_I128)a, m);\n";
    o_ << "  const int64_t bb = ls_mod_pos_i128((LS_I128)b, m);\n";
    o_ << "  uint64_t qsum = 0;\n";
    o_ << "  if (ls_floor_sum_u64_checked((uint64_t)n, (uint64_t)m, (uint64_t)aa, (uint64_t)bb, &qsum)) {\n";
    o_ << "    const LS_I128 nn = (LS_I128)n;\n";
    o_ << "    const LS_I128 tri = (((LS_I128)aa * nn * (nn - 1)) >> 1);\n";
    o_ << "    const LS_I128 total = tri + ((LS_I128)bb * nn);\n";
    o_ << "    return total - ((LS_I128)m * (LS_I128)qsum);\n";
    o_ << "  }\n";
    o_ << "  LS_I128 acc = 0;\n";
    o_ << "  int64_t cur = bb;\n";
    o_ << "  const int64_t delta = aa;\n";
    o_ << "  for (int64_t k = 0; k < n; ++k) {\n";
    o_ << "    acc += (LS_I128)cur;\n";
    o_ << "    cur += delta;\n";
    o_ << "    if (cur >= m) cur -= m;\n";
    o_ << "  }\n";
    o_ << "  return acc;\n";
    o_ << "}\n";
    o_ << "static inline void ls_state_speed(int64_t start_us) {\n";
    o_ << "  const int64_t now_us = clock_us();\n";
    o_ << "  const int64_t elapsed_us = now_us >= start_us ? (now_us - start_us) : 0;\n";
    o_ << "  print_str(\"speed_us=\");\n";
    o_ << "  println_i64(elapsed_us);\n";
    o_ << "}\n";
    o_ << "static LS_THREAD_LOCAL ls_bool ls_format_cli_marker = 0;\n";
    o_ << "static inline void ls_mark_format_mode(void) { ls_format_cli_marker = 1; }\n";
    if (cliCustomTokens_.empty()) {
      o_ << "static const char *ls_cli_tokens[1] = {\"\"};\n";
      o_ << "static const int64_t ls_cli_token_count = 0;\n";
    } else {
      o_ << "static const char *ls_cli_tokens[" << static_cast<unsigned long long>(cliCustomTokens_.size()) << "] = {";
      for (std::size_t i = 0; i < cliCustomTokens_.size(); ++i) {
        if (i) o_ << ", ";
        o_ << cStrLit(cliCustomTokens_[i]);
      }
      o_ << "};\n";
      o_ << "static const int64_t ls_cli_token_count = "
         << static_cast<long long>(cliCustomTokens_.size()) << "LL;\n";
    }
    o_ << "static inline int64_t cli_token_count(void) { return ls_cli_token_count; }\n";
    o_ << "static inline const char *cli_token(int64_t idx) {\n";
    o_ << "  if (idx < 0 || idx >= ls_cli_token_count) return \"\";\n";
    o_ << "  return ls_cli_tokens[(size_t)idx];\n";
    o_ << "}\n";
    o_ << "static inline ls_bool cli_has(const char *flag) {\n";
    o_ << "  if (!flag || !*flag) return 0;\n";
    o_ << "  for (int64_t i = 0; i < ls_cli_token_count; ++i) {\n";
    o_ << "    if (strcmp(ls_cli_tokens[(size_t)i], flag) == 0) return 1;\n";
    o_ << "  }\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "static inline const char *cli_value(const char *flag) {\n";
    o_ << "  if (!flag || !*flag) return \"\";\n";
    o_ << "  const size_t n = strlen(flag);\n";
    o_ << "  for (int64_t i = 0; i < ls_cli_token_count; ++i) {\n";
    o_ << "    const char *tok = ls_cli_tokens[(size_t)i];\n";
    o_ << "    if (strcmp(tok, flag) == 0) {\n";
    o_ << "      if (i + 1 < ls_cli_token_count) {\n";
    o_ << "        const char *next = ls_cli_tokens[(size_t)(i + 1)];\n";
    o_ << "        if (strcmp(next, \"[\") != 0 && strcmp(next, \"]\") != 0 && next[0] != '-') return next;\n";
    o_ << "      }\n";
    o_ << "      return \"\";\n";
    o_ << "    }\n";
    o_ << "    if (strncmp(tok, flag, n) == 0 && tok[n] == '=') return tok + n + 1;\n";
    o_ << "  }\n";
    o_ << "  return \"\";\n";
    o_ << "}\n";
    o_ << "#define LS_SCRATCH_SLOTS 16\n";
    o_ << "static LS_THREAD_LOCAL char *ls_scratch_bufs[LS_SCRATCH_SLOTS];\n";
    o_ << "static LS_THREAD_LOCAL size_t ls_scratch_caps[LS_SCRATCH_SLOTS];\n";
    o_ << "static LS_THREAD_LOCAL int ls_scratch_pos = 0;\n";
    o_ << "static inline char *ls_scratch_take(size_t need) {\n";
    o_ << "  if (need < 1) need = 1;\n";
    o_ << "  const int idx = ls_scratch_pos;\n";
    o_ << "  ls_scratch_pos = (ls_scratch_pos + 1) % LS_SCRATCH_SLOTS;\n";
    o_ << "  if (ls_scratch_caps[idx] < need) {\n";
    o_ << "    char *next = (char *)realloc(ls_scratch_bufs[idx], need);\n";
    o_ << "    if (!next) return NULL;\n";
    o_ << "    ls_scratch_bufs[idx] = next;\n";
    o_ << "    ls_scratch_caps[idx] = need;\n";
    o_ << "  }\n";
    o_ << "  return ls_scratch_bufs[idx];\n";
    o_ << "}\n";
    o_ << "static inline char *ls_heap_dup(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t n = strlen(src);\n";
    o_ << "  char *out = (char *)malloc(n + 1);\n";
    o_ << "  if (!out) return NULL;\n";
    o_ << "  memcpy(out, src, n);\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_scratch_dup(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t n = strlen(src);\n";
    o_ << "  char *out = ls_scratch_take(n + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  memcpy(out, src, n);\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_ptr_in_buf(const char *p, const char *buf, size_t cap) {\n";
    o_ << "  if (!p || !buf || cap == 0) return 0;\n";
    o_ << "  const uintptr_t pv = (uintptr_t)(const void *)p;\n";
    o_ << "  const uintptr_t bv = (uintptr_t)(const void *)buf;\n";
    o_ << "  return (pv >= bv && pv < bv + cap) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_str_needs_hold(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  if (src[0] == '\\0') return 0;\n";
    o_ << "  if (ls_ptr_in_buf(src, ls_fmt.buf, ls_fmt.cap)) return 1;\n";
    o_ << "  for (int i = 0; i < LS_SCRATCH_SLOTS; ++i) {\n";
    o_ << "    if (ls_ptr_in_buf(src, ls_scratch_bufs[i], ls_scratch_caps[i])) return 1;\n";
    o_ << "  }\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_str_hold(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  if (!ls_str_needs_hold(src)) return src;\n";
    o_ << "  char *dup = ls_heap_dup(src);\n";
    o_ << "  return dup ? dup : \"\";\n";
    o_ << "}\n";
    o_ << "static LS_THREAD_LOCAL char *ls_input_buf = NULL;\n";
    o_ << "static LS_THREAD_LOCAL size_t ls_input_cap = 0;\n";
    o_ << "static inline const char *input(void) {\n";
    o_ << "  if (ls_input_cap < 256) {\n";
    o_ << "    char *next = (char *)realloc(ls_input_buf, 256);\n";
    o_ << "    if (!next) return \"\";\n";
    o_ << "    ls_input_buf = next;\n";
    o_ << "    ls_input_cap = 256;\n";
    o_ << "  }\n";
    o_ << "  size_t len = 0;\n";
    o_ << "  ls_input_buf[0] = '\\0';\n";
    o_ << "  while (1) {\n";
    o_ << "    if (!fgets(ls_input_buf + len, (int)(ls_input_cap - len), stdin)) {\n";
    o_ << "      if (len == 0) {\n";
    o_ << "        ls_input_buf[0] = '\\0';\n";
    o_ << "        return \"\";\n";
    o_ << "      }\n";
    o_ << "      break;\n";
    o_ << "    }\n";
    o_ << "    len += strlen(ls_input_buf + len);\n";
    o_ << "    if (len > 0 && (ls_input_buf[len - 1] == '\\n' || ls_input_buf[len - 1] == '\\r')) break;\n";
    o_ << "    if (ls_input_cap - len > 1) continue;\n";
    o_ << "    size_t nextCap = ls_input_cap << 1;\n";
    o_ << "    char *next = (char *)realloc(ls_input_buf, nextCap);\n";
    o_ << "    if (!next) break;\n";
    o_ << "    ls_input_buf = next;\n";
    o_ << "    ls_input_cap = nextCap;\n";
    o_ << "  }\n";
    o_ << "  while (len > 0 && (ls_input_buf[len - 1] == '\\n' || ls_input_buf[len - 1] == '\\r')) {\n";
    o_ << "    ls_input_buf[len - 1] = '\\0';\n";
    o_ << "    --len;\n";
    o_ << "  }\n";
    o_ << "  char *owned = ls_heap_dup(ls_input_buf);\n";
    o_ << "  return owned ? owned : \"\";\n";
    o_ << "}\n";
    o_ << "static inline const char *input_prompt(const char *prompt) {\n";
    o_ << "  if (prompt) print_str(prompt);\n";
    o_ << "  return input();\n";
    o_ << "}\n";
    o_ << "static inline int64_t bytes_len(const char *s) { return (int64_t)strlen(s ? s : \"\"); }\n";
    o_ << "static inline int64_t len(const char *s) { return (int64_t)strlen(s ? s : \"\"); }\n";
    o_ << "static inline ls_bool includes(const char *s, const char *part) {\n";
    o_ << "  if (!s || !part) return 0;\n";
    o_ << "  return strstr(s, part) != NULL;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool is_empty(const char *s) { return len(s) == 0; }\n";
    o_ << "static inline ls_bool contains(const char *s, const char *part) {\n";
    o_ << "  if (!s || !part) return 0;\n";
    o_ << "  return strstr(s, part) != NULL;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool starts_with(const char *s, const char *prefix) {\n";
    o_ << "  if (!s || !prefix) return 0;\n";
    o_ << "  size_t n = strlen(prefix);\n";
    o_ << "  return strncmp(s, prefix, n) == 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ends_with(const char *s, const char *suffix) {\n";
    o_ << "  if (!s || !suffix) return 0;\n";
    o_ << "  size_t ls = strlen(s);\n";
    o_ << "  size_t ss = strlen(suffix);\n";
    o_ << "  if (ss > ls) return 0;\n";
    o_ << "  return strncmp(s + ls - ss, suffix, ss) == 0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t find(const char *s, const char *part) {\n";
    o_ << "  if (!s || !part) return -1;\n";
    o_ << "  const char *p = strstr(s, part);\n";
    o_ << "  return p ? (int64_t)(p - s) : -1;\n";
    o_ << "}\n";
    o_ << "static inline const char *lower(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t n = strlen(src);\n";
    o_ << "  char *out = ls_scratch_take(n + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  for (size_t i = 0; i < n; ++i) out[i] = (char)tolower((unsigned char)src[i]);\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *upper(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t n = strlen(src);\n";
    o_ << "  char *out = ls_scratch_take(n + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  for (size_t i = 0; i < n; ++i) out[i] = (char)toupper((unsigned char)src[i]);\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *trim(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  size_t start = 0;\n";
    o_ << "  size_t end = strlen(src);\n";
    o_ << "  while (start < end && isspace((unsigned char)src[start])) ++start;\n";
    o_ << "  while (end > start && isspace((unsigned char)src[end - 1])) --end;\n";
    o_ << "  const size_t n = end - start;\n";
    o_ << "  char *out = ls_scratch_take(n + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  memcpy(out, src + start, n);\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *substring(const char *s, int64_t start, int64_t count) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const int64_t n = (int64_t)strlen(src);\n";
    o_ << "  if (start < 0) start = 0;\n";
    o_ << "  if (start > n) start = n;\n";
    o_ << "  if (count < 0) count = 0;\n";
    o_ << "  if (count > n - start) count = n - start;\n";
    o_ << "  char *out = ls_scratch_take((size_t)count + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  memcpy(out, src + start, (size_t)count);\n";
    o_ << "  out[count] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *repeat(const char *s, int64_t n) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t slen = strlen(src);\n";
    o_ << "  if (n <= 0 || slen == 0) return \"\";\n";
    o_ << "  if ((size_t)n > (SIZE_MAX - 1) / slen) return \"\";\n";
    o_ << "  const size_t outLen = (size_t)n * slen;\n";
    o_ << "  char *out = ls_scratch_take(outLen + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  size_t pos = 0;\n";
    o_ << "  for (int64_t i = 0; i < n; ++i) {\n";
    o_ << "    memcpy(out + pos, src, slen);\n";
    o_ << "    pos += slen;\n";
    o_ << "  }\n";
    o_ << "  out[outLen] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *reverse(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const size_t n = strlen(src);\n";
    o_ << "  char *out = ls_scratch_take(n + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  for (size_t i = 0; i < n; ++i) out[i] = src[n - 1 - i];\n";
    o_ << "  out[n] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline const char *replace(const char *s, const char *from, const char *to) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const char *needle = from ? from : \"\";\n";
    o_ << "  const char *rep = to ? to : \"\";\n";
    o_ << "  const size_t srcLen = strlen(src);\n";
    o_ << "  const size_t nLen = strlen(needle);\n";
    o_ << "  const size_t rLen = strlen(rep);\n";
    o_ << "  if (nLen == 0) return ls_scratch_dup(src);\n";
    o_ << "  size_t count = 0;\n";
    o_ << "  const char *cur = src;\n";
    o_ << "  while (1) {\n";
    o_ << "    const char *p = strstr(cur, needle);\n";
    o_ << "    if (!p) break;\n";
    o_ << "    ++count;\n";
    o_ << "    cur = p + nLen;\n";
    o_ << "  }\n";
    o_ << "  if (count == 0) return ls_scratch_dup(src);\n";
    o_ << "  size_t outLen = srcLen;\n";
    o_ << "  if (rLen >= nLen) {\n";
    o_ << "    const size_t grow = rLen - nLen;\n";
    o_ << "    if (grow > 0 && count > (SIZE_MAX - srcLen) / grow) return \"\";\n";
    o_ << "    outLen = srcLen + count * grow;\n";
    o_ << "  } else {\n";
    o_ << "    const size_t shrink = nLen - rLen;\n";
    o_ << "    if (shrink > 0 && count > srcLen / shrink) return \"\";\n";
    o_ << "    outLen = srcLen - count * shrink;\n";
    o_ << "  }\n";
    o_ << "  char *out = ls_scratch_take(outLen + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  size_t pos = 0;\n";
    o_ << "  cur = src;\n";
    o_ << "  while (1) {\n";
    o_ << "    const char *p = strstr(cur, needle);\n";
    o_ << "    if (!p) break;\n";
    o_ << "    const size_t prefix = (size_t)(p - cur);\n";
    o_ << "    memcpy(out + pos, cur, prefix);\n";
    o_ << "    pos += prefix;\n";
    o_ << "    memcpy(out + pos, rep, rLen);\n";
    o_ << "    pos += rLen;\n";
    o_ << "    cur = p + nLen;\n";
    o_ << "  }\n";
    o_ << "  const size_t tail = strlen(cur);\n";
    o_ << "  memcpy(out + pos, cur, tail);\n";
    o_ << "  pos += tail;\n";
    o_ << "  out[pos] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline int64_t byte_at(const char *s, int64_t idx) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  const int64_t n = (int64_t)strlen(src);\n";
    o_ << "  if (idx < 0 || idx >= n) return -1;\n";
    o_ << "  return (int64_t)(unsigned char)src[idx];\n";
    o_ << "}\n";
    o_ << "static inline int64_t ord(const char *s) {\n";
    o_ << "  const char *src = s ? s : \"\";\n";
    o_ << "  if (src[0] == '\\0') return -1;\n";
    o_ << "  return (int64_t)(unsigned char)src[0];\n";
    o_ << "}\n";
    o_ << "static inline const char *chr(int64_t code) {\n";
    o_ << "  if (code < 0 || code > 255) return \"\";\n";
    o_ << "  char *out = ls_scratch_take(2);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  out[0] = (char)code;\n";
    o_ << "  out[1] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline int64_t mem_alloc(int64_t bytes) {\n";
    o_ << "  if (bytes <= 0) return 0;\n";
    o_ << "  if ((uint64_t)bytes > (uint64_t)(SIZE_MAX - sizeof(int64_t))) return 0;\n";
    o_ << "  if (ls_su_enabled && ls_su_mem_limit > 0 && (ls_su_mem_in_use + bytes) > ls_su_mem_limit) return 0;\n";
    o_ << "  uint8_t *raw = (uint8_t *)malloc((size_t)bytes + sizeof(int64_t));\n";
    o_ << "  if (!raw) return 0;\n";
    o_ << "  *((int64_t *)raw) = bytes;\n";
    o_ << "  ls_su_mem_in_use += bytes;\n";
    o_ << "  return (int64_t)(intptr_t)(raw + sizeof(int64_t));\n";
    o_ << "}\n";
    o_ << "static inline int64_t mem_realloc(int64_t ptr, int64_t bytes) {\n";
    o_ << "  if (ptr == 0) return mem_alloc(bytes);\n";
    o_ << "  uint8_t *old_payload = (uint8_t *)(intptr_t)ptr;\n";
    o_ << "  uint8_t *old_raw = old_payload - sizeof(int64_t);\n";
    o_ << "  int64_t old_bytes = *((int64_t *)old_raw);\n";
    o_ << "  if (bytes <= 0) {\n";
    o_ << "    if (old_bytes > 0 && ls_su_mem_in_use >= old_bytes) ls_su_mem_in_use -= old_bytes;\n";
    o_ << "    free(old_raw);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if ((uint64_t)bytes > (uint64_t)(SIZE_MAX - sizeof(int64_t))) return 0;\n";
    o_ << "  const int64_t delta = bytes - old_bytes;\n";
    o_ << "  if (ls_su_enabled && ls_su_mem_limit > 0 && delta > 0 && (ls_su_mem_in_use + delta) > ls_su_mem_limit) return 0;\n";
    o_ << "  uint8_t *raw = (uint8_t *)realloc(old_raw, (size_t)bytes + sizeof(int64_t));\n";
    o_ << "  if (!raw) return 0;\n";
    o_ << "  *((int64_t *)raw) = bytes;\n";
    o_ << "  ls_su_mem_in_use += delta;\n";
    o_ << "  return (int64_t)(intptr_t)(raw + sizeof(int64_t));\n";
    o_ << "}\n";
    o_ << "static inline void mem_free(int64_t ptr) {\n";
    o_ << "  if (ptr == 0) return;\n";
    o_ << "  uint8_t *payload = (uint8_t *)(intptr_t)ptr;\n";
    o_ << "  uint8_t *raw = payload - sizeof(int64_t);\n";
    o_ << "  int64_t bytes = *((int64_t *)raw);\n";
    o_ << "  if (bytes > 0 && ls_su_mem_in_use >= bytes) ls_su_mem_in_use -= bytes;\n";
    o_ << "  free(raw);\n";
    o_ << "}\n";
    o_ << "static inline void mem_set(int64_t ptr, int64_t byte_val, int64_t bytes) {\n";
    o_ << "  if (ptr == 0 || bytes <= 0) return;\n";
    o_ << "  if ((uint64_t)bytes > (uint64_t)SIZE_MAX) return;\n";
    o_ << "  memset((void *)(intptr_t)ptr, (int)(byte_val & 0xFF), (size_t)bytes);\n";
    o_ << "}\n";
    o_ << "static inline void mem_copy(int64_t dst, int64_t src, int64_t bytes) {\n";
    o_ << "  if (dst == 0 || src == 0 || bytes <= 0) return;\n";
    o_ << "  if ((uint64_t)bytes > (uint64_t)SIZE_MAX) return;\n";
    o_ << "  memmove((void *)(intptr_t)dst, (const void *)(intptr_t)src, (size_t)bytes);\n";
    o_ << "}\n";
    o_ << "static inline int64_t mem_read_i64(int64_t ptr) {\n";
    o_ << "  if (ptr == 0) return 0;\n";
    o_ << "  int64_t out = 0;\n";
    o_ << "  memcpy(&out, (const void *)(intptr_t)ptr, sizeof(out));\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline void mem_write_i64(int64_t ptr, int64_t v) {\n";
    o_ << "  if (ptr == 0) return;\n";
    o_ << "  memcpy((void *)(intptr_t)ptr, &v, sizeof(v));\n";
    o_ << "}\n";
    o_ << "static inline double mem_read_f64(int64_t ptr) {\n";
    o_ << "  if (ptr == 0) return 0.0;\n";
    o_ << "  double out = 0.0;\n";
    o_ << "  memcpy(&out, (const void *)(intptr_t)ptr, sizeof(out));\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline void mem_write_f64(int64_t ptr, double v) {\n";
    o_ << "  if (ptr == 0) return;\n";
    o_ << "  memcpy((void *)(intptr_t)ptr, &v, sizeof(v));\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_ARRAYS 2048\n";
    o_ << "typedef struct {\n";
    o_ << "  char **items;\n";
    o_ << "  int64_t len;\n";
    o_ << "  int64_t cap;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_array;\n";
    o_ << "static ls_array ls_arrays[LS_MAX_ARRAYS];\n";
    o_ << "static int64_t ls_array_count = 0;\n";
    o_ << "static int64_t ls_array_free_ids[LS_MAX_ARRAYS];\n";
    o_ << "static int64_t ls_array_free_top = 0;\n";
    o_ << "static inline int64_t ls_array_alloc_id(void) {\n";
    o_ << "  if (ls_array_free_top > 0) return ls_array_free_ids[--ls_array_free_top];\n";
    o_ << "  if (ls_array_count >= LS_MAX_ARRAYS) return -1;\n";
    o_ << "  return ls_array_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_array_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_ARRAYS) return;\n";
    o_ << "  if (ls_array_free_top < LS_MAX_ARRAYS) ls_array_free_ids[ls_array_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline ls_array *ls_get_array(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_array_count) return NULL;\n";
    o_ << "  if (!ls_arrays[id].active) return NULL;\n";
    o_ << "  return &ls_arrays[id];\n";
    o_ << "}\n";
    o_ << "static inline void ls_array_reserve(ls_array *a, int64_t need) {\n";
    o_ << "  if (!a || need <= a->cap) return;\n";
    o_ << "  int64_t nextCap = a->cap > 0 ? a->cap : 8;\n";
    o_ << "  while (nextCap < need) nextCap <<= 1;\n";
    o_ << "  char **next = (char **)realloc(a->items, (size_t)nextCap * sizeof(char *));\n";
    o_ << "  if (!next) return;\n";
    o_ << "  for (int64_t i = a->cap; i < nextCap; ++i) next[i] = NULL;\n";
    o_ << "  a->items = next;\n";
    o_ << "  a->cap = nextCap;\n";
    o_ << "}\n";
    o_ << "static inline int64_t array_new(void) {\n";
    o_ << "  const int64_t id = ls_array_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_arrays[id].items = NULL;\n";
    o_ << "  ls_arrays[id].len = 0;\n";
    o_ << "  ls_arrays[id].cap = 0;\n";
    o_ << "  ls_arrays[id].active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline int64_t array_len(int64_t id) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  return a ? a->len : 0;\n";
    o_ << "}\n";
    o_ << "static inline void array_free(int64_t id) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a) return;\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) {\n";
    o_ << "    if (a->items[i]) free(a->items[i]);\n";
    o_ << "  }\n";
    o_ << "  if (a->items) free(a->items);\n";
    o_ << "  a->items = NULL;\n";
    o_ << "  a->len = 0;\n";
    o_ << "  a->cap = 0;\n";
    o_ << "  a->active = 0;\n";
    o_ << "  ls_array_release_id(id);\n";
    o_ << "}\n";
    o_ << "static inline void array_push(int64_t id, const char *value) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a) return;\n";
    o_ << "  ls_array_reserve(a, a->len + 1);\n";
    o_ << "  if (a->len >= a->cap) return;\n";
    o_ << "  a->items[a->len] = ls_heap_dup(value);\n";
    o_ << "  if (!a->items[a->len]) a->items[a->len] = ls_heap_dup(\"\");\n";
    o_ << "  if (!a->items[a->len]) return;\n";
    o_ << "  ++a->len;\n";
    o_ << "}\n";
    o_ << "static inline const char *array_get(int64_t id, int64_t idx) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a || idx < 0 || idx >= a->len) return \"\";\n";
    o_ << "  return a->items[idx] ? a->items[idx] : \"\";\n";
    o_ << "}\n";
    o_ << "static inline void array_set(int64_t id, int64_t idx, const char *value) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a || idx < 0) return;\n";
    o_ << "  ls_array_reserve(a, idx + 1);\n";
    o_ << "  if (idx >= a->cap) return;\n";
    o_ << "  while (a->len <= idx) {\n";
    o_ << "    a->items[a->len] = ls_heap_dup(\"\");\n";
    o_ << "    if (!a->items[a->len]) return;\n";
    o_ << "    ++a->len;\n";
    o_ << "  }\n";
    o_ << "  if (a->items[idx]) free(a->items[idx]);\n";
    o_ << "  a->items[idx] = ls_heap_dup(value);\n";
    o_ << "  if (!a->items[idx]) a->items[idx] = ls_heap_dup(\"\");\n";
    o_ << "}\n";
    o_ << "static inline const char *array_pop(int64_t id) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a || a->len <= 0) return \"\";\n";
    o_ << "  --a->len;\n";
    o_ << "  const char *v = a->items[a->len] ? a->items[a->len] : \"\";\n";
    o_ << "  const char *out = ls_scratch_dup(v);\n";
    o_ << "  if (a->items[a->len]) free(a->items[a->len]);\n";
    o_ << "  a->items[a->len] = NULL;\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool array_includes(int64_t id, const char *value) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a) return 0;\n";
    o_ << "  const char *needle = value ? value : \"\";\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) {\n";
    o_ << "    const char *x = a->items[i] ? a->items[i] : \"\";\n";
    o_ << "    if (strcmp(x, needle) == 0) return 1;\n";
    o_ << "  }\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "static inline const char *array_join(int64_t id, const char *sep) {\n";
    o_ << "  ls_array *a = ls_get_array(id);\n";
    o_ << "  if (!a || a->len == 0) return \"\";\n";
    o_ << "  const char *glue = sep ? sep : \"\";\n";
    o_ << "  const size_t glueLen = strlen(glue);\n";
    o_ << "  size_t outLen = 0;\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) {\n";
    o_ << "    outLen += strlen(a->items[i] ? a->items[i] : \"\");\n";
    o_ << "    if (i + 1 < a->len) outLen += glueLen;\n";
    o_ << "  }\n";
    o_ << "  char *out = ls_scratch_take(outLen + 1);\n";
    o_ << "  if (!out) return \"\";\n";
    o_ << "  size_t pos = 0;\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) {\n";
    o_ << "    const char *x = a->items[i] ? a->items[i] : \"\";\n";
    o_ << "    const size_t n = strlen(x);\n";
    o_ << "    memcpy(out + pos, x, n);\n";
    o_ << "    pos += n;\n";
    o_ << "    if (i + 1 < a->len && glueLen > 0) {\n";
    o_ << "      memcpy(out + pos, glue, glueLen);\n";
    o_ << "      pos += glueLen;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  out[pos] = '\\0';\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_DICTS 2048\n";
    o_ << "typedef struct {\n";
    o_ << "  char *key;\n";
    o_ << "  char *value;\n";
    o_ << "  uint32_t key_len;\n";
    o_ << "  uint64_t hash;\n";
    o_ << "  ls_bool used;\n";
    o_ << "  ls_bool tomb;\n";
    o_ << "} ls_dict_entry;\n";
    o_ << "typedef struct {\n";
    o_ << "  ls_dict_entry *items;\n";
    o_ << "  int64_t len;\n";
    o_ << "  int64_t cap;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_dict;\n";
    o_ << "static ls_dict ls_dicts[LS_MAX_DICTS];\n";
    o_ << "static int64_t ls_dict_count = 0;\n";
    o_ << "static int64_t ls_dict_free_ids[LS_MAX_DICTS];\n";
    o_ << "static int64_t ls_dict_free_top = 0;\n";
    o_ << "static inline int64_t ls_dict_alloc_id(void) {\n";
    o_ << "  if (ls_dict_free_top > 0) return ls_dict_free_ids[--ls_dict_free_top];\n";
    o_ << "  if (ls_dict_count >= LS_MAX_DICTS) return -1;\n";
    o_ << "  return ls_dict_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_dict_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_DICTS) return;\n";
    o_ << "  if (ls_dict_free_top < LS_MAX_DICTS) ls_dict_free_ids[ls_dict_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline uint64_t ls_hash_str(const char *s) {\n";
    o_ << "  const unsigned char *p = (const unsigned char *)(s ? s : \"\");\n";
    o_ << "  uint64_t h = 1469598103934665603ULL;\n";
    o_ << "  while (*p) {\n";
    o_ << "    h ^= (uint64_t)(*p++);\n";
    o_ << "    h *= 1099511628211ULL;\n";
    o_ << "  }\n";
    o_ << "  return h;\n";
    o_ << "}\n";
    o_ << "static inline ls_dict *ls_get_dict(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_dict_count) return NULL;\n";
    o_ << "  if (!ls_dicts[id].active) return NULL;\n";
    o_ << "  return &ls_dicts[id];\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_next_pow2_i64(int64_t x) {\n";
    o_ << "  int64_t out = 1;\n";
    o_ << "  while (out < x) out <<= 1;\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_dict_find_slot(const ls_dict *d, const char *key, uint32_t key_len, uint64_t hash, ls_bool for_insert) {\n";
    o_ << "  if (!d || d->cap <= 0 || !d->items) return -1;\n";
    o_ << "  const char *k = key ? key : \"\";\n";
    o_ << "  int64_t idx = (int64_t)(hash & (uint64_t)(d->cap - 1));\n";
    o_ << "  int64_t first_tomb = -1;\n";
    o_ << "  for (int64_t probe = 0; probe < d->cap; ++probe) {\n";
    o_ << "    const ls_dict_entry *e = &d->items[idx];\n";
    o_ << "    if (e->used) {\n";
    o_ << "      if (e->hash == hash && e->key_len == key_len && e->key && memcmp(e->key, k, key_len) == 0 && e->key[key_len] == '\\0') return idx;\n";
    o_ << "    } else {\n";
    o_ << "      if (e->tomb) {\n";
    o_ << "        if (for_insert && first_tomb < 0) first_tomb = idx;\n";
    o_ << "      } else {\n";
    o_ << "        return for_insert ? (first_tomb >= 0 ? first_tomb : idx) : -1;\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "    idx = (idx + 1) & (d->cap - 1);\n";
    o_ << "  }\n";
    o_ << "  return for_insert ? first_tomb : -1;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_dict_rehash(ls_dict *d, int64_t want_cap) {\n";
    o_ << "  if (!d) return 0;\n";
    o_ << "  if (want_cap < 16) want_cap = 16;\n";
    o_ << "  const int64_t new_cap = ls_next_pow2_i64(want_cap);\n";
    o_ << "  ls_dict_entry *next = (ls_dict_entry *)calloc((size_t)new_cap, sizeof(ls_dict_entry));\n";
    o_ << "  if (!next) return 0;\n";
    o_ << "  ls_dict_entry *old_items = d->items;\n";
    o_ << "  const int64_t old_cap = d->cap;\n";
    o_ << "  d->items = next;\n";
    o_ << "  d->cap = new_cap;\n";
    o_ << "  d->len = 0;\n";
    o_ << "  for (int64_t i = 0; i < old_cap; ++i) {\n";
    o_ << "    if (!old_items || !old_items[i].used) continue;\n";
    o_ << "    const int64_t slot = ls_dict_find_slot(d, old_items[i].key, old_items[i].key_len, old_items[i].hash, 1);\n";
    o_ << "    if (slot >= 0) {\n";
    o_ << "      d->items[slot] = old_items[i];\n";
    o_ << "      d->items[slot].used = 1;\n";
    o_ << "      d->items[slot].tomb = 0;\n";
    o_ << "      ++d->len;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  if (old_items) free(old_items);\n";
    o_ << "  return 1;\n";
    o_ << "}\n";
    o_ << "static inline int64_t dict_new(void) {\n";
    o_ << "  const int64_t id = ls_dict_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_dicts[id].items = NULL;\n";
    o_ << "  ls_dicts[id].len = 0;\n";
    o_ << "  ls_dicts[id].cap = 0;\n";
    o_ << "  ls_dicts[id].active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline int64_t dict_len(int64_t id) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  return d ? d->len : 0;\n";
    o_ << "}\n";
    o_ << "static inline void dict_free(int64_t id) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  if (!d) return;\n";
    o_ << "  if (d->items) {\n";
    o_ << "    for (int64_t i = 0; i < d->cap; ++i) {\n";
    o_ << "      if (d->items[i].used) {\n";
    o_ << "        if (d->items[i].key) free(d->items[i].key);\n";
    o_ << "        if (d->items[i].value) free(d->items[i].value);\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "    free(d->items);\n";
    o_ << "  }\n";
    o_ << "  d->items = NULL;\n";
    o_ << "  d->len = 0;\n";
    o_ << "  d->cap = 0;\n";
    o_ << "  d->active = 0;\n";
    o_ << "  ls_dict_release_id(id);\n";
    o_ << "}\n";
    o_ << "static inline void dict_set(int64_t id, const char *key, const char *value) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  if (!d) return;\n";
    o_ << "  if (d->cap == 0) {\n";
    o_ << "    if (!ls_dict_rehash(d, 16)) return;\n";
    o_ << "  }\n";
    o_ << "  if ((d->len + 1) * 10 >= d->cap * 7) {\n";
    o_ << "    if (!ls_dict_rehash(d, d->cap << 1)) return;\n";
    o_ << "  }\n";
    o_ << "  const char *k = key ? key : \"\";\n";
    o_ << "  const char *v = value ? value : \"\";\n";
    o_ << "  const uint32_t key_len = (uint32_t)strlen(k);\n";
    o_ << "  const uint64_t h = ls_hash_str(k);\n";
    o_ << "  const int64_t slot = ls_dict_find_slot(d, k, key_len, h, 1);\n";
    o_ << "  if (slot < 0) return;\n";
    o_ << "  ls_dict_entry *e = &d->items[slot];\n";
    o_ << "  if (e->used) {\n";
    o_ << "    char *nextv = ls_heap_dup(v);\n";
    o_ << "    if (!nextv) nextv = ls_heap_dup(\"\");\n";
    o_ << "    if (!nextv) return;\n";
    o_ << "    if (e->value) free(e->value);\n";
    o_ << "    e->value = nextv;\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  e->key = ls_heap_dup(k);\n";
    o_ << "  e->value = ls_heap_dup(v);\n";
    o_ << "  if (!e->key) e->key = ls_heap_dup(\"\");\n";
    o_ << "  if (!e->value) e->value = ls_heap_dup(\"\");\n";
    o_ << "  if (!e->key || !e->value) {\n";
    o_ << "    if (e->key) free(e->key);\n";
    o_ << "    if (e->value) free(e->value);\n";
    o_ << "    e->key = NULL;\n";
    o_ << "    e->value = NULL;\n";
    o_ << "    e->used = 0;\n";
    o_ << "    e->tomb = 1;\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  e->hash = h;\n";
    o_ << "  e->key_len = key_len;\n";
    o_ << "  e->used = 1;\n";
    o_ << "  e->tomb = 0;\n";
    o_ << "  ++d->len;\n";
    o_ << "}\n";
    o_ << "static inline const char *dict_get(int64_t id, const char *key) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  if (!d || d->cap == 0) return \"\";\n";
    o_ << "  const char *k = key ? key : \"\";\n";
    o_ << "  const uint32_t key_len = (uint32_t)strlen(k);\n";
    o_ << "  const int64_t slot = ls_dict_find_slot(d, k, key_len, ls_hash_str(k), 0);\n";
    o_ << "  if (slot < 0) return \"\";\n";
    o_ << "  const ls_dict_entry *e = &d->items[slot];\n";
    o_ << "  return e->value ? e->value : \"\";\n";
    o_ << "}\n";
    o_ << "static inline ls_bool dict_has(int64_t id, const char *key) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  if (!d || d->cap == 0) return 0;\n";
    o_ << "  const char *k = key ? key : \"\";\n";
    o_ << "  const uint32_t key_len = (uint32_t)strlen(k);\n";
    o_ << "  return ls_dict_find_slot(d, k, key_len, ls_hash_str(k), 0) >= 0 ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline void dict_remove(int64_t id, const char *key) {\n";
    o_ << "  ls_dict *d = ls_get_dict(id);\n";
    o_ << "  if (!d || d->cap == 0) return;\n";
    o_ << "  const char *k = key ? key : \"\";\n";
    o_ << "  const uint32_t key_len = (uint32_t)strlen(k);\n";
    o_ << "  const int64_t slot = ls_dict_find_slot(d, k, key_len, ls_hash_str(k), 0);\n";
    o_ << "  if (slot < 0) return;\n";
    o_ << "  ls_dict_entry *e = &d->items[slot];\n";
    o_ << "  if (e->key) free(e->key);\n";
    o_ << "  if (e->value) free(e->value);\n";
    o_ << "  e->key = NULL;\n";
    o_ << "  e->value = NULL;\n";
    o_ << "  e->used = 0;\n";
    o_ << "  e->tomb = 1;\n";
    o_ << "  if (d->len > 0) --d->len;\n";
    o_ << "  if (d->cap > 16 && d->len * 4 < d->cap) {\n";
    o_ << "    (void)ls_dict_rehash(d, d->cap >> 1);\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline int64_t map_new(void) { return dict_new(); }\n";
    o_ << "static inline int64_t map_len(int64_t id) { return dict_len(id); }\n";
    o_ << "static inline void map_set(int64_t id, const char *k, const char *v) { dict_set(id, k, v); }\n";
    o_ << "static inline const char *map_get(int64_t id, const char *k) { return dict_get(id, k); }\n";
    o_ << "static inline ls_bool map_has(int64_t id, const char *k) { return dict_has(id, k); }\n";
    o_ << "static inline void map_remove(int64_t id, const char *k) { dict_remove(id, k); }\n";
    o_ << "static inline void map_free(int64_t id) { dict_free(id); }\n";
    o_ << "static inline int64_t object_new(void) { return dict_new(); }\n";
    o_ << "static inline int64_t object_len(int64_t id) { return dict_len(id); }\n";
    o_ << "static inline void object_set(int64_t id, const char *k, const char *v) { dict_set(id, k, v); }\n";
    o_ << "static inline const char *object_get(int64_t id, const char *k) { return dict_get(id, k); }\n";
    o_ << "static inline ls_bool object_has(int64_t id, const char *k) { return dict_has(id, k); }\n";
    o_ << "static inline void object_remove(int64_t id, const char *k) { dict_remove(id, k); }\n";
    o_ << "static inline void object_free(int64_t id) { dict_free(id); }\n";
    o_ << "#define LS_MAX_OPTION 4096\n";
    o_ << "typedef struct {\n";
    o_ << "  char *value;\n";
    o_ << "  ls_bool has;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_option_slot;\n";
    o_ << "static ls_option_slot ls_options[LS_MAX_OPTION];\n";
    o_ << "static int64_t ls_option_count = 0;\n";
    o_ << "static int64_t ls_option_free_ids[LS_MAX_OPTION];\n";
    o_ << "static int64_t ls_option_free_top = 0;\n";
    o_ << "static inline int64_t ls_option_alloc_id(void) {\n";
    o_ << "  if (ls_option_free_top > 0) return ls_option_free_ids[--ls_option_free_top];\n";
    o_ << "  if (ls_option_count >= LS_MAX_OPTION) return -1;\n";
    o_ << "  return ls_option_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_option_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_OPTION) return;\n";
    o_ << "  if (ls_option_free_top < LS_MAX_OPTION) ls_option_free_ids[ls_option_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline ls_option_slot *ls_get_option(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_option_count) return NULL;\n";
    o_ << "  if (!ls_options[id].active) return NULL;\n";
    o_ << "  return &ls_options[id];\n";
    o_ << "}\n";
    o_ << "static inline int64_t option_some(const char *value) {\n";
    o_ << "  const int64_t id = ls_option_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_option_slot *o = &ls_options[id];\n";
    o_ << "  o->value = ls_heap_dup(value ? value : \"\");\n";
    o_ << "  o->has = 1;\n";
    o_ << "  o->active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline int64_t option_none(void) {\n";
    o_ << "  const int64_t id = ls_option_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_option_slot *o = &ls_options[id];\n";
    o_ << "  o->value = NULL;\n";
    o_ << "  o->has = 0;\n";
    o_ << "  o->active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool option_is_some(int64_t id) {\n";
    o_ << "  ls_option_slot *o = ls_get_option(id);\n";
    o_ << "  return (o && o->has) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool option_is_none(int64_t id) {\n";
    o_ << "  ls_option_slot *o = ls_get_option(id);\n";
    o_ << "  return (!o || !o->has) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline const char *option_unwrap(int64_t id) {\n";
    o_ << "  ls_option_slot *o = ls_get_option(id);\n";
    o_ << "  if (!o || !o->has || !o->value) return \"\";\n";
    o_ << "  return o->value;\n";
    o_ << "}\n";
    o_ << "static inline const char *option_unwrap_or(int64_t id, const char *fallback) {\n";
    o_ << "  ls_option_slot *o = ls_get_option(id);\n";
    o_ << "  if (!o || !o->has || !o->value) return fallback ? fallback : \"\";\n";
    o_ << "  return o->value;\n";
    o_ << "}\n";
    o_ << "static inline void option_free(int64_t id) {\n";
    o_ << "  ls_option_slot *o = ls_get_option(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  if (o->value) free(o->value);\n";
    o_ << "  o->value = NULL;\n";
    o_ << "  o->has = 0;\n";
    o_ << "  o->active = 0;\n";
    o_ << "  ls_option_release_id(id);\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_RESULT 4096\n";
    o_ << "typedef struct {\n";
    o_ << "  char *value;\n";
    o_ << "  char *err_type;\n";
    o_ << "  char *err_msg;\n";
    o_ << "  ls_bool ok;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_result_slot;\n";
    o_ << "static ls_result_slot ls_results[LS_MAX_RESULT];\n";
    o_ << "static int64_t ls_result_count = 0;\n";
    o_ << "static int64_t ls_result_free_ids[LS_MAX_RESULT];\n";
    o_ << "static int64_t ls_result_free_top = 0;\n";
    o_ << "static inline int64_t ls_result_alloc_id(void) {\n";
    o_ << "  if (ls_result_free_top > 0) return ls_result_free_ids[--ls_result_free_top];\n";
    o_ << "  if (ls_result_count >= LS_MAX_RESULT) return -1;\n";
    o_ << "  return ls_result_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_result_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_RESULT) return;\n";
    o_ << "  if (ls_result_free_top < LS_MAX_RESULT) ls_result_free_ids[ls_result_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline ls_result_slot *ls_get_result(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_result_count) return NULL;\n";
    o_ << "  if (!ls_results[id].active) return NULL;\n";
    o_ << "  return &ls_results[id];\n";
    o_ << "}\n";
    o_ << "static inline int64_t result_ok(const char *value) {\n";
    o_ << "  const int64_t id = ls_result_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_result_slot *r = &ls_results[id];\n";
    o_ << "  r->value = ls_heap_dup(value ? value : \"\");\n";
    o_ << "  r->err_type = NULL;\n";
    o_ << "  r->err_msg = NULL;\n";
    o_ << "  r->ok = 1;\n";
    o_ << "  r->active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline int64_t result_err(const char *type_name, const char *msg) {\n";
    o_ << "  const int64_t id = ls_result_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_result_slot *r = &ls_results[id];\n";
    o_ << "  r->value = NULL;\n";
    o_ << "  r->err_type = ls_heap_dup(type_name ? type_name : \"Error\");\n";
    o_ << "  r->err_msg = ls_heap_dup(msg ? msg : \"\");\n";
    o_ << "  r->ok = 0;\n";
    o_ << "  r->active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool result_is_ok(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  return (r && r->ok) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool result_is_err(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  return (!r || !r->ok) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline const char *result_value(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  if (!r || !r->ok || !r->value) return \"\";\n";
    o_ << "  return r->value;\n";
    o_ << "}\n";
    o_ << "static inline const char *result_error_type(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  if (!r || r->ok || !r->err_type) return \"\";\n";
    o_ << "  return r->err_type;\n";
    o_ << "}\n";
    o_ << "static inline const char *result_error_message(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  if (!r || r->ok || !r->err_msg) return \"\";\n";
    o_ << "  return r->err_msg;\n";
    o_ << "}\n";
    o_ << "static inline const char *result_unwrap_or(int64_t id, const char *fallback) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  if (!r || !r->ok || !r->value) return fallback ? fallback : \"\";\n";
    o_ << "  return r->value;\n";
    o_ << "}\n";
    o_ << "static inline void result_free(int64_t id) {\n";
    o_ << "  ls_result_slot *r = ls_get_result(id);\n";
    o_ << "  if (!r) return;\n";
    o_ << "  if (r->value) free(r->value);\n";
    o_ << "  if (r->err_type) free(r->err_type);\n";
    o_ << "  if (r->err_msg) free(r->err_msg);\n";
    o_ << "  r->value = NULL;\n";
    o_ << "  r->err_type = NULL;\n";
    o_ << "  r->err_msg = NULL;\n";
    o_ << "  r->ok = 0;\n";
    o_ << "  r->active = 0;\n";
    o_ << "  ls_result_release_id(id);\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_NP 1024\n";
    o_ << "#define LS_MAX_NP_ELEMS 10000000\n";
    o_ << "typedef struct {\n";
    o_ << "  double *data;\n";
    o_ << "  int64_t len;\n";
    o_ << "  int64_t cap;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_np;\n";
    o_ << "static ls_np ls_np_slots[LS_MAX_NP];\n";
    o_ << "static int64_t ls_np_count = 0;\n";
    o_ << "static int64_t ls_np_free_ids[LS_MAX_NP];\n";
    o_ << "static int64_t ls_np_free_top = 0;\n";
    o_ << "static inline int64_t ls_np_alloc_id(void) {\n";
    o_ << "  if (ls_np_free_top > 0) return ls_np_free_ids[--ls_np_free_top];\n";
    o_ << "  if (ls_np_count >= LS_MAX_NP) return -1;\n";
    o_ << "  return ls_np_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_np_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_NP) return;\n";
    o_ << "  if (ls_np_free_top < LS_MAX_NP) ls_np_free_ids[ls_np_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline ls_np *ls_get_np(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_np_count) return NULL;\n";
    o_ << "  if (!ls_np_slots[id].active) return NULL;\n";
    o_ << "  return &ls_np_slots[id];\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_new(int64_t n) {\n";
    o_ << "  if (n < 0 || n > LS_MAX_NP_ELEMS) return -1;\n";
    o_ << "  const int64_t id = ls_np_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  double *p = NULL;\n";
    o_ << "  if (n > 0) {\n";
    o_ << "    p = (double *)calloc((size_t)n, sizeof(double));\n";
    o_ << "    if (!p) {\n";
    o_ << "      ls_np_release_id(id);\n";
    o_ << "      return -1;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  ls_np_slots[id].data = p;\n";
    o_ << "  ls_np_slots[id].len = n;\n";
    o_ << "  ls_np_slots[id].cap = n;\n";
    o_ << "  ls_np_slots[id].active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline void np_free(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v) return;\n";
    o_ << "  if (v->data) free(v->data);\n";
    o_ << "  v->data = NULL;\n";
    o_ << "  v->len = 0;\n";
    o_ << "  v->cap = 0;\n";
    o_ << "  v->active = 0;\n";
    o_ << "  ls_np_release_id(id);\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_len(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  return v ? v->len : 0;\n";
    o_ << "}\n";
    o_ << "static inline double np_get(int64_t id, int64_t idx) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data || idx < 0 || idx >= v->len) return 0.0;\n";
    o_ << "  return v->data[idx];\n";
    o_ << "}\n";
    o_ << "static inline void np_set(int64_t id, int64_t idx, double val) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data || idx < 0 || idx >= v->len) return;\n";
    o_ << "  v->data[idx] = val;\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_copy(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v) return -1;\n";
    o_ << "  const int64_t out = np_new(v->len);\n";
    o_ << "  ls_np *dst = ls_get_np(out);\n";
    o_ << "  if (out < 0 || !dst) return -1;\n";
    o_ << "  if (v->len > 0 && v->data && dst->data) memcpy(dst->data, v->data, (size_t)v->len * sizeof(double));\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline void np_fill(int64_t id, double val) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data) return;\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < v->len; ++i) v->data[i] = val;\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_from_range(double start, double stop, double step) {\n";
    o_ << "  if (step == 0.0) return -1;\n";
    o_ << "  int64_t n = 0;\n";
    o_ << "  if (step > 0.0) {\n";
    o_ << "    for (double x = start; x < stop && n < LS_MAX_NP_ELEMS; x += step) ++n;\n";
    o_ << "  } else {\n";
    o_ << "    for (double x = start; x > stop && n < LS_MAX_NP_ELEMS; x += step) ++n;\n";
    o_ << "  }\n";
    o_ << "  const int64_t out = np_new(n);\n";
    o_ << "  ls_np *v = ls_get_np(out);\n";
    o_ << "  if (out < 0 || !v || !v->data) return out;\n";
    o_ << "  double x = start;\n";
    o_ << "  for (int64_t i = 0; i < n; ++i) {\n";
    o_ << "    v->data[i] = x;\n";
    o_ << "    x += step;\n";
    o_ << "  }\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_linspace(double start, double stop, int64_t count) {\n";
    o_ << "  if (count < 0 || count > LS_MAX_NP_ELEMS) return -1;\n";
    o_ << "  const int64_t out = np_new(count);\n";
    o_ << "  ls_np *v = ls_get_np(out);\n";
    o_ << "  if (out < 0 || !v) return -1;\n";
    o_ << "  if (count == 0) return out;\n";
    o_ << "  if (count == 1) {\n";
    o_ << "    if (v->data) v->data[0] = start;\n";
    o_ << "    return out;\n";
    o_ << "  }\n";
    o_ << "  const double step = (stop - start) / (double)(count - 1);\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < count; ++i) {\n";
    o_ << "    v->data[i] = start + step * (double)i;\n";
    o_ << "  }\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline double np_sum(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data || v->len <= 0) return 0.0;\n";
    o_ << "  double sum = 0.0;\n";
    o_ << "  LS_OMP_SIMD_REDUCTION_PLUS(sum)\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < v->len; ++i) sum += v->data[i];\n";
    o_ << "  return sum;\n";
    o_ << "}\n";
    o_ << "static inline double np_mean(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || v->len <= 0) return 0.0;\n";
    o_ << "  return np_sum(id) / (double)v->len;\n";
    o_ << "}\n";
    o_ << "static inline double np_min(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data || v->len <= 0) return 0.0;\n";
    o_ << "  double mn = v->data[0];\n";
    o_ << "  for (int64_t i = 1; i < v->len; ++i) if (v->data[i] < mn) mn = v->data[i];\n";
    o_ << "  return mn;\n";
    o_ << "}\n";
    o_ << "static inline double np_max(int64_t id) {\n";
    o_ << "  ls_np *v = ls_get_np(id);\n";
    o_ << "  if (!v || !v->data || v->len <= 0) return 0.0;\n";
    o_ << "  double mx = v->data[0];\n";
    o_ << "  for (int64_t i = 1; i < v->len; ++i) if (v->data[i] > mx) mx = v->data[i];\n";
    o_ << "  return mx;\n";
    o_ << "}\n";
    o_ << "static inline double np_dot(int64_t a_id, int64_t b_id) {\n";
    o_ << "  ls_np *a = ls_get_np(a_id);\n";
    o_ << "  ls_np *b = ls_get_np(b_id);\n";
    o_ << "  if (!a || !b || !a->data || !b->data) return 0.0;\n";
    o_ << "  int64_t n = a->len < b->len ? a->len : b->len;\n";
    o_ << "  if (n <= 0) return 0.0;\n";
    o_ << "  double sum = 0.0;\n";
    o_ << "  LS_OMP_SIMD_REDUCTION_PLUS(sum)\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < n; ++i) sum += a->data[i] * b->data[i];\n";
    o_ << "  return sum;\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_np_binary(int64_t a_id, int64_t b_id, int op) {\n";
    o_ << "  ls_np *a = ls_get_np(a_id);\n";
    o_ << "  ls_np *b = ls_get_np(b_id);\n";
    o_ << "  if (!a || !b || !a->data || !b->data) return -1;\n";
    o_ << "  int64_t n = a->len < b->len ? a->len : b->len;\n";
    o_ << "  if (n < 0) return -1;\n";
    o_ << "  const int64_t out = np_new(n);\n";
    o_ << "  ls_np *o = ls_get_np(out);\n";
    o_ << "  if (out < 0 || !o || !o->data) return -1;\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < n; ++i) {\n";
    o_ << "    const double x = a->data[i];\n";
    o_ << "    const double y = b->data[i];\n";
    o_ << "    if (op == 0) o->data[i] = x + y;\n";
    o_ << "    else if (op == 1) o->data[i] = x - y;\n";
    o_ << "    else if (op == 2) o->data[i] = x * y;\n";
    o_ << "    else o->data[i] = (y != 0.0) ? (x / y) : 0.0;\n";
    o_ << "  }\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline int64_t np_add(int64_t a_id, int64_t b_id) { return ls_np_binary(a_id, b_id, 0); }\n";
    o_ << "static inline int64_t np_sub(int64_t a_id, int64_t b_id) { return ls_np_binary(a_id, b_id, 1); }\n";
    o_ << "static inline int64_t np_mul(int64_t a_id, int64_t b_id) { return ls_np_binary(a_id, b_id, 2); }\n";
    o_ << "static inline int64_t np_div(int64_t a_id, int64_t b_id) { return ls_np_binary(a_id, b_id, 3); }\n";
    o_ << "static inline void np_add_scalar(int64_t id, double v) {\n";
    o_ << "  ls_np *a = ls_get_np(id);\n";
    o_ << "  if (!a || !a->data) return;\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) a->data[i] += v;\n";
    o_ << "}\n";
    o_ << "static inline void np_mul_scalar(int64_t id, double v) {\n";
    o_ << "  ls_np *a = ls_get_np(id);\n";
    o_ << "  if (!a || !a->data) return;\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) a->data[i] *= v;\n";
    o_ << "}\n";
    o_ << "static inline void np_clip(int64_t id, double lo, double hi) {\n";
    o_ << "  ls_np *a = ls_get_np(id);\n";
    o_ << "  if (!a || !a->data) return;\n";
    o_ << "  if (lo > hi) { const double t = lo; lo = hi; hi = t; }\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) {\n";
    o_ << "    double x = a->data[i];\n";
    o_ << "    if (x < lo) x = lo;\n";
    o_ << "    else if (x > hi) x = hi;\n";
    o_ << "    a->data[i] = x;\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void np_abs(int64_t id) {\n";
    o_ << "  ls_np *a = ls_get_np(id);\n";
    o_ << "  if (!a || !a->data) return;\n";
    o_ << "  LS_OMP_SIMD\n";
    o_ << "  LS_VEC_HINT\n";
    o_ << "  for (int64_t i = 0; i < a->len; ++i) a->data[i] = fabs(a->data[i]);\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_GFX 256\n";
    o_ << "typedef struct {\n";
    o_ << "  uint8_t *pix;\n";
    o_ << "  int64_t w;\n";
    o_ << "  int64_t h;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_gfx;\n";
    o_ << "static ls_gfx ls_gfx_slots[LS_MAX_GFX];\n";
    o_ << "static int64_t ls_gfx_count = 0;\n";
    o_ << "static int64_t ls_gfx_free_ids[LS_MAX_GFX];\n";
    o_ << "static int64_t ls_gfx_free_top = 0;\n";
    o_ << "static inline int64_t ls_gfx_alloc_id(void) {\n";
    o_ << "  if (ls_gfx_free_top > 0) return ls_gfx_free_ids[--ls_gfx_free_top];\n";
    o_ << "  if (ls_gfx_count >= LS_MAX_GFX) return -1;\n";
    o_ << "  return ls_gfx_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_gfx_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_GFX) return;\n";
    o_ << "  if (ls_gfx_free_top < LS_MAX_GFX) ls_gfx_free_ids[ls_gfx_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline uint8_t ls_color_u8(int64_t v) {\n";
    o_ << "  if (v <= 0) return 0;\n";
    o_ << "  if (v >= 255) return 255;\n";
    o_ << "  return (uint8_t)v;\n";
    o_ << "}\n";
    o_ << "static inline ls_gfx *ls_get_gfx(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_gfx_count) return NULL;\n";
    o_ << "  if (!ls_gfx_slots[id].active) return NULL;\n";
    o_ << "  return &ls_gfx_slots[id];\n";
    o_ << "}\n";
    o_ << "static inline int64_t gfx_new(int64_t w, int64_t h) {\n";
    o_ << "  if (w <= 0 || h <= 0 || w > 16384 || h > 16384) return -1;\n";
    o_ << "  const size_t area = (size_t)w * (size_t)h;\n";
    o_ << "  if (area == 0 || area > (((size_t)-1) / 3)) return -1;\n";
    o_ << "  const size_t bytes = area * 3;\n";
    o_ << "  uint8_t *pix = (uint8_t *)malloc(bytes);\n";
    o_ << "  if (!pix) return -1;\n";
    o_ << "  memset(pix, 0, bytes);\n";
    o_ << "  const int64_t id = ls_gfx_alloc_id();\n";
    o_ << "  if (id < 0) {\n";
    o_ << "    free(pix);\n";
    o_ << "    return -1;\n";
    o_ << "  }\n";
    o_ << "  ls_gfx_slots[id].pix = pix;\n";
    o_ << "  ls_gfx_slots[id].w = w;\n";
    o_ << "  ls_gfx_slots[id].h = h;\n";
    o_ << "  ls_gfx_slots[id].active = 1;\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline void gfx_free(int64_t id) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  if (g->pix) free(g->pix);\n";
    o_ << "  g->pix = NULL;\n";
    o_ << "  g->w = 0;\n";
    o_ << "  g->h = 0;\n";
    o_ << "  g->active = 0;\n";
    o_ << "  ls_gfx_release_id(id);\n";
    o_ << "}\n";
    o_ << "static inline int64_t gfx_width(int64_t id) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  return g ? g->w : 0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t gfx_height(int64_t id) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  return g ? g->h : 0;\n";
    o_ << "}\n";
    o_ << "static inline void ls_gfx_set_pixel_u8(ls_gfx *g, int64_t x, int64_t y, uint8_t r, uint8_t gg, uint8_t b) {\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  if ((uint64_t)x >= (uint64_t)g->w || (uint64_t)y >= (uint64_t)g->h) return;\n";
    o_ << "  const size_t idx = (((size_t)y * (size_t)g->w) + (size_t)x) * 3;\n";
    o_ << "  g->pix[idx] = r;\n";
    o_ << "  g->pix[idx + 1] = gg;\n";
    o_ << "  g->pix[idx + 2] = b;\n";
    o_ << "}\n";
    o_ << "static inline void gfx_clear(int64_t id, int64_t r, int64_t gg, int64_t b) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  const size_t area = (size_t)g->w * (size_t)g->h;\n";
    o_ << "  const size_t bytes = area * 3;\n";
    o_ << "  if (r8 == g8 && g8 == b8) {\n";
    o_ << "    memset(g->pix, r8, bytes);\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  uint8_t *p = g->pix;\n";
    o_ << "  for (size_t i = 0; i < area; ++i) {\n";
    o_ << "    p[0] = r8;\n";
    o_ << "    p[1] = g8;\n";
    o_ << "    p[2] = b8;\n";
    o_ << "    p += 3;\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void gfx_set(int64_t id, int64_t x, int64_t y, int64_t r, int64_t gg, int64_t b) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  ls_gfx_set_pixel_u8(g, x, y, ls_color_u8(r), ls_color_u8(gg), ls_color_u8(b));\n";
    o_ << "}\n";
    o_ << "static inline int64_t gfx_get(int64_t id, int64_t x, int64_t y) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g || !g->pix) return -1;\n";
    o_ << "  if ((uint64_t)x >= (uint64_t)g->w || (uint64_t)y >= (uint64_t)g->h) return -1;\n";
    o_ << "  const size_t idx = (((size_t)y * (size_t)g->w) + (size_t)x) * 3;\n";
    o_ << "  return ((int64_t)g->pix[idx] << 16) | ((int64_t)g->pix[idx + 1] << 8) | (int64_t)g->pix[idx + 2];\n";
    o_ << "}\n";
    o_ << "static inline void gfx_line(int64_t id, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int64_t r, int64_t gg, "
          "int64_t b) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  int64_t dx = x1 - x0;\n";
    o_ << "  if (dx < 0) dx = -dx;\n";
    o_ << "  int64_t sx = x0 < x1 ? 1 : -1;\n";
    o_ << "  int64_t dy = y1 - y0;\n";
    o_ << "  if (dy < 0) dy = -dy;\n";
    o_ << "  int64_t sy = y0 < y1 ? 1 : -1;\n";
    o_ << "  int64_t err = dx - dy;\n";
    o_ << "  for (;;) {\n";
    o_ << "    ls_gfx_set_pixel_u8(g, x0, y0, r8, g8, b8);\n";
    o_ << "    if (x0 == x1 && y0 == y1) break;\n";
    o_ << "    const int64_t e2 = err << 1;\n";
    o_ << "    if (e2 > -dy) {\n";
    o_ << "      err -= dy;\n";
    o_ << "      x0 += sx;\n";
    o_ << "    }\n";
    o_ << "    if (e2 < dx) {\n";
    o_ << "      err += dx;\n";
    o_ << "      y0 += sy;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void gfx_rect(int64_t id, int64_t x, int64_t y, int64_t w, int64_t h, int64_t r, int64_t gg, "
          "int64_t b, ls_bool fill) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  if (w <= 0 || h <= 0) return;\n";
    o_ << "  if (x > 9223372036854775807LL - w) return;\n";
    o_ << "  if (y > 9223372036854775807LL - h) return;\n";
    o_ << "  int64_t x0 = x;\n";
    o_ << "  int64_t y0 = y;\n";
    o_ << "  int64_t x1 = x + w;\n";
    o_ << "  int64_t y1 = y + h;\n";
    o_ << "  if (x1 <= 0 || y1 <= 0 || x0 >= g->w || y0 >= g->h) return;\n";
    o_ << "  if (x0 < 0) x0 = 0;\n";
    o_ << "  if (y0 < 0) y0 = 0;\n";
    o_ << "  if (x1 > g->w) x1 = g->w;\n";
    o_ << "  if (y1 > g->h) y1 = g->h;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  if (fill) {\n";
    o_ << "    for (int64_t yy = y0; yy < y1; ++yy) {\n";
    o_ << "      uint8_t *row = g->pix + ((((size_t)yy * (size_t)g->w) + (size_t)x0) * 3);\n";
    o_ << "      for (int64_t xx = x0; xx < x1; ++xx) {\n";
    o_ << "        row[0] = r8;\n";
    o_ << "        row[1] = g8;\n";
    o_ << "        row[2] = b8;\n";
    o_ << "        row += 3;\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  for (int64_t xx = x0; xx < x1; ++xx) {\n";
    o_ << "    ls_gfx_set_pixel_u8(g, xx, y0, r8, g8, b8);\n";
    o_ << "    ls_gfx_set_pixel_u8(g, xx, y1 - 1, r8, g8, b8);\n";
    o_ << "  }\n";
    o_ << "  for (int64_t yy = y0 + 1; yy + 1 < y1; ++yy) {\n";
    o_ << "    ls_gfx_set_pixel_u8(g, x0, yy, r8, g8, b8);\n";
    o_ << "    ls_gfx_set_pixel_u8(g, x1 - 1, yy, r8, g8, b8);\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline ls_bool gfx_save_ppm(int64_t id, const char *path) {\n";
    o_ << "  ls_gfx *g = ls_get_gfx(id);\n";
    o_ << "  if (!g || !g->pix) return 0;\n";
    o_ << "  const char *p = path ? path : \"\";\n";
    o_ << "  if (p[0] == '\\0') return 0;\n";
    o_ << "  FILE *f = NULL;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  if (fopen_s(&f, p, \"wb\") != 0 || !f) return 0;\n";
    o_ << "#else\n";
    o_ << "  f = fopen(p, \"wb\");\n";
    o_ << "  if (!f) return 0;\n";
    o_ << "#endif\n";
    o_ << "  if (fprintf(f, \"P6\\n%lld %lld\\n255\\n\", (long long)g->w, (long long)g->h) < 0) {\n";
    o_ << "    fclose(f);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  const size_t bytes = ((size_t)g->w * (size_t)g->h) * 3;\n";
    o_ << "  const size_t wrote = fwrite(g->pix, 1, bytes, f);\n";
    o_ << "  if (fclose(f) != 0) return 0;\n";
    o_ << "  return wrote == bytes ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "#define LS_MAX_GAME 32\n";
    o_ << "typedef struct {\n";
    o_ << "  uint8_t *pix;\n";
    o_ << "  int64_t w;\n";
    o_ << "  int64_t h;\n";
    o_ << "  ls_bool active;\n";
    o_ << "  ls_bool headless;\n";
    o_ << "  ls_bool should_close;\n";
    o_ << "  int64_t target_fps;\n";
    o_ << "  double fixed_dt;\n";
    o_ << "  double dt;\n";
    o_ << "  int64_t frame;\n";
    o_ << "  double mouse_x;\n";
    o_ << "  double mouse_y;\n";
    o_ << "  double mouse_norm_x;\n";
    o_ << "  double mouse_norm_y;\n";
    o_ << "  double scroll_x;\n";
    o_ << "  double scroll_y;\n";
    o_ << "  uint32_t mouse_buttons;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  HWND hwnd;\n";
    o_ << "  HDC hdc;\n";
    o_ << "  BITMAPINFO bmi;\n";
    o_ << "  LARGE_INTEGER freq;\n";
    o_ << "  LARGE_INTEGER last;\n";
    o_ << "#else\n";
    o_ << "  struct timespec last_ts;\n";
    o_ << "#endif\n";
    o_ << "} ls_game;\n";
    o_ << "static ls_game ls_games[LS_MAX_GAME];\n";
    o_ << "static int64_t ls_game_count = 0;\n";
    o_ << "static int64_t ls_game_free_ids[LS_MAX_GAME];\n";
    o_ << "static int64_t ls_game_free_top = 0;\n";
    o_ << "static inline int64_t ls_game_alloc_id(void) {\n";
    o_ << "  if (ls_game_free_top > 0) return ls_game_free_ids[--ls_game_free_top];\n";
    o_ << "  if (ls_game_count >= LS_MAX_GAME) return -1;\n";
    o_ << "  return ls_game_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_game_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_GAME) return;\n";
    o_ << "  if (ls_game_free_top < LS_MAX_GAME) ls_game_free_ids[ls_game_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline ls_game *ls_get_game(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_game_count) return NULL;\n";
    o_ << "  if (!ls_games[id].active) return NULL;\n";
    o_ << "  return &ls_games[id];\n";
    o_ << "}\n";
    o_ << "static inline void ls_game_set_pixel_u8(ls_game *g, int64_t x, int64_t y, uint8_t r, uint8_t gg, uint8_t b) {\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  if ((uint64_t)x >= (uint64_t)g->w || (uint64_t)y >= (uint64_t)g->h) return;\n";
    o_ << "  const size_t idx = (((size_t)y * (size_t)g->w) + (size_t)x) * 3;\n";
    o_ << "  g->pix[idx] = r;\n";
    o_ << "  g->pix[idx + 1] = gg;\n";
    o_ << "  g->pix[idx + 2] = b;\n";
    o_ << "}\n";
    o_ << "static inline void ls_game_sleep_ms(double ms) {\n";
    o_ << "  if (!(ms > 0.0)) return;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  DWORD wait_ms = (DWORD)(ms + 0.5);\n";
    o_ << "  if (wait_ms > 0) Sleep(wait_ms);\n";
    o_ << "#else\n";
    o_ << "  struct timespec ts;\n";
    o_ << "  ts.tv_sec = (time_t)(ms / 1000.0);\n";
    o_ << "  double rem = ms - ((double)ts.tv_sec * 1000.0);\n";
    o_ << "  ts.tv_nsec = (long)(rem * 1000000.0);\n";
    o_ << "  if (ts.tv_nsec < 0) ts.tv_nsec = 0;\n";
    o_ << "  nanosleep(&ts, NULL);\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline double ls_clamp01(double v) {\n";
    o_ << "  if (v < 0.0) return 0.0;\n";
    o_ << "  if (v > 1.0) return 1.0;\n";
    o_ << "  return v;\n";
    o_ << "}\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "static LRESULT CALLBACK ls_game_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {\n";
    o_ << "  ls_game *g = (ls_game *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);\n";
    o_ << "  if (msg == WM_NCCREATE) {\n";
    o_ << "    CREATESTRUCTA *cs = (CREATESTRUCTA *)lp;\n";
    o_ << "    if (cs && cs->lpCreateParams) {\n";
    o_ << "      SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);\n";
    o_ << "    }\n";
    o_ << "    return DefWindowProcA(hwnd, msg, wp, lp);\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_CLOSE) {\n";
    o_ << "    if (g) g->should_close = 1;\n";
    o_ << "    DestroyWindow(hwnd);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_MOUSEWHEEL) {\n";
    o_ << "    if (g) g->scroll_y += (double)((short)HIWORD(wp)) / (double)WHEEL_DELTA;\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_MOUSEHWHEEL) {\n";
    o_ << "    if (g) g->scroll_x += (double)((short)HIWORD(wp)) / (double)WHEEL_DELTA;\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_LBUTTONDOWN) {\n";
    o_ << "    if (g) g->mouse_buttons |= (uint32_t)1u;\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_LBUTTONUP) {\n";
    o_ << "    if (g) g->mouse_buttons &= ~(uint32_t)1u;\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_RBUTTONDOWN) {\n";
    o_ << "    if (g) g->mouse_buttons |= (uint32_t)(1u << 1);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_RBUTTONUP) {\n";
    o_ << "    if (g) g->mouse_buttons &= ~(uint32_t)(1u << 1);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_MBUTTONDOWN) {\n";
    o_ << "    if (g) g->mouse_buttons |= (uint32_t)(1u << 2);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_MBUTTONUP) {\n";
    o_ << "    if (g) g->mouse_buttons &= ~(uint32_t)(1u << 2);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_XBUTTONDOWN) {\n";
    o_ << "    if (g) {\n";
    o_ << "      const uint16_t xb = HIWORD(wp);\n";
    o_ << "      if (xb == XBUTTON1) g->mouse_buttons |= (uint32_t)(1u << 3);\n";
    o_ << "      else if (xb == XBUTTON2) g->mouse_buttons |= (uint32_t)(1u << 4);\n";
    o_ << "    }\n";
    o_ << "    return 1;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_XBUTTONUP) {\n";
    o_ << "    if (g) {\n";
    o_ << "      const uint16_t xb = HIWORD(wp);\n";
    o_ << "      if (xb == XBUTTON1) g->mouse_buttons &= ~(uint32_t)(1u << 3);\n";
    o_ << "      else if (xb == XBUTTON2) g->mouse_buttons &= ~(uint32_t)(1u << 4);\n";
    o_ << "    }\n";
    o_ << "    return 1;\n";
    o_ << "  }\n";
    o_ << "  if (msg == WM_DESTROY) {\n";
    o_ << "    if (g) g->should_close = 1;\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  return DefWindowProcA(hwnd, msg, wp, lp);\n";
    o_ << "}\n";
    o_ << "static inline void ls_game_register_class(void) {\n";
    o_ << "  static int ready = 0;\n";
    o_ << "  if (ready) return;\n";
    o_ << "  WNDCLASSEXA wc;\n";
    o_ << "  memset(&wc, 0, sizeof(wc));\n";
    o_ << "  wc.cbSize = sizeof(wc);\n";
    o_ << "  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;\n";
    o_ << "  wc.lpfnWndProc = ls_game_wndproc;\n";
    o_ << "  wc.hInstance = GetModuleHandleA(NULL);\n";
    o_ << "  wc.lpszClassName = \"LineScriptGameWindow\";\n";
    o_ << "  (void)RegisterClassExA(&wc);\n";
    o_ << "  ready = 1;\n";
    o_ << "}\n";
    o_ << "#endif\n";
    o_ << "static inline int64_t game_new(int64_t w, int64_t h, const char *title, ls_bool visible) {\n";
    o_ << "  if (w <= 0 || h <= 0 || w > 8192 || h > 8192) return -1;\n";
    o_ << "  const size_t area = (size_t)w * (size_t)h;\n";
    o_ << "  if (area == 0 || area > (((size_t)-1) / 3)) return -1;\n";
    o_ << "  uint8_t *pix = (uint8_t *)malloc(area * 3);\n";
    o_ << "  if (!pix) return -1;\n";
    o_ << "  memset(pix, 0, area * 3);\n";
    o_ << "  const int64_t id = ls_game_alloc_id();\n";
    o_ << "  if (id < 0) {\n";
    o_ << "    free(pix);\n";
    o_ << "    return -1;\n";
    o_ << "  }\n";
    o_ << "  ls_game *g = &ls_games[id];\n";
    o_ << "  memset(g, 0, sizeof(*g));\n";
    o_ << "  g->pix = pix;\n";
    o_ << "  g->w = w;\n";
    o_ << "  g->h = h;\n";
    o_ << "  g->active = 1;\n";
    o_ << "  g->headless = visible ? 0 : 1;\n";
    o_ << "  g->target_fps = 60;\n";
    o_ << "  g->fixed_dt = 0.0;\n";
    o_ << "  g->dt = 1.0 / 60.0;\n";
    o_ << "  g->frame = 0;\n";
    o_ << "  g->mouse_x = 0.0;\n";
    o_ << "  g->mouse_y = 0.0;\n";
    o_ << "  g->mouse_norm_x = 0.0;\n";
    o_ << "  g->mouse_norm_y = 0.0;\n";
    o_ << "  g->scroll_x = 0.0;\n";
    o_ << "  g->scroll_y = 0.0;\n";
    o_ << "  g->mouse_buttons = 0u;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  QueryPerformanceFrequency(&g->freq);\n";
    o_ << "  QueryPerformanceCounter(&g->last);\n";
    o_ << "  if (!g->headless) {\n";
    o_ << "    ls_game_register_class();\n";
    o_ << "    RECT r;\n";
    o_ << "    r.left = 0;\n";
    o_ << "    r.top = 0;\n";
    o_ << "    r.right = (LONG)w;\n";
    o_ << "    r.bottom = (LONG)h;\n";
    o_ << "    const DWORD style = WS_OVERLAPPEDWINDOW;\n";
    o_ << "    (void)AdjustWindowRect(&r, style, 0);\n";
    o_ << "    g->hwnd = CreateWindowExA(0, \"LineScriptGameWindow\", (title && title[0]) ? title : \"LineScript\", style,\n";
    o_ << "                             CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,\n";
    o_ << "                             NULL, NULL, GetModuleHandleA(NULL), g);\n";
    o_ << "    if (!g->hwnd) {\n";
    o_ << "      free(g->pix);\n";
    o_ << "      memset(g, 0, sizeof(*g));\n";
    o_ << "      ls_game_release_id(id);\n";
    o_ << "      return -1;\n";
    o_ << "    }\n";
    o_ << "    ShowWindow(g->hwnd, SW_SHOW);\n";
    o_ << "    UpdateWindow(g->hwnd);\n";
    o_ << "    g->hdc = GetDC(g->hwnd);\n";
    o_ << "    memset(&g->bmi, 0, sizeof(g->bmi));\n";
    o_ << "    g->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);\n";
    o_ << "    g->bmi.bmiHeader.biWidth = (LONG)w;\n";
    o_ << "    g->bmi.bmiHeader.biHeight = -(LONG)h;\n";
    o_ << "    g->bmi.bmiHeader.biPlanes = 1;\n";
    o_ << "    g->bmi.bmiHeader.biBitCount = 24;\n";
    o_ << "    g->bmi.bmiHeader.biCompression = BI_RGB;\n";
    o_ << "  }\n";
    o_ << "#else\n";
    o_ << "  (void)title;\n";
    o_ << "  g->headless = 1;\n";
    o_ << "  timespec_get(&g->last_ts, TIME_UTC);\n";
    o_ << "#endif\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline void game_free(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  g->scroll_x = 0.0;\n";
    o_ << "  g->scroll_y = 0.0;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  if (g->hdc && g->hwnd) {\n";
    o_ << "    ReleaseDC(g->hwnd, g->hdc);\n";
    o_ << "  }\n";
    o_ << "  g->hdc = NULL;\n";
    o_ << "  if (g->hwnd) {\n";
    o_ << "    DestroyWindow(g->hwnd);\n";
    o_ << "  }\n";
    o_ << "  g->hwnd = NULL;\n";
    o_ << "#endif\n";
    o_ << "  if (g->pix) free(g->pix);\n";
    o_ << "  memset(g, 0, sizeof(*g));\n";
    o_ << "  ls_game_release_id(id);\n";
    o_ << "}\n";
    o_ << "static inline int64_t game_width(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->w : 0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t game_height(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->h : 0;\n";
    o_ << "}\n";
    o_ << "static inline void game_set_target_fps(int64_t id, int64_t fps) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  if (fps < 0) fps = 0;\n";
    o_ << "  if (fps > 1000) fps = 1000;\n";
    o_ << "  g->target_fps = fps;\n";
    o_ << "}\n";
    o_ << "static inline void game_set_fixed_dt(int64_t id, double dt) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  if (!isfinite(dt) || dt <= 0.0) {\n";
    o_ << "    g->fixed_dt = 0.0;\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  if (dt > 0.25) dt = 0.25;\n";
    o_ << "  g->fixed_dt = dt;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool game_should_close(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->should_close : 1;\n";
    o_ << "}\n";
    o_ << "static inline void game_poll(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  MSG msg;\n";
    o_ << "  while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {\n";
    o_ << "    if (msg.message == WM_QUIT) g->should_close = 1;\n";
    o_ << "    TranslateMessage(&msg);\n";
    o_ << "    DispatchMessageA(&msg);\n";
    o_ << "  }\n";
    o_ << "  if (g->hwnd && !IsWindow(g->hwnd)) g->should_close = 1;\n";
    o_ << "  if (g->hwnd && IsWindow(g->hwnd)) {\n";
    o_ << "    POINT pt;\n";
    o_ << "    if (GetCursorPos(&pt) && ScreenToClient(g->hwnd, &pt)) {\n";
    o_ << "      RECT rc;\n";
    o_ << "      if (GetClientRect(g->hwnd, &rc)) {\n";
    o_ << "        const double cw = (double)(rc.right - rc.left);\n";
    o_ << "        const double ch = (double)(rc.bottom - rc.top);\n";
    o_ << "        double nx = 0.0;\n";
    o_ << "        double ny = 0.0;\n";
    o_ << "        if (cw > 1.0) nx = ((double)pt.x) / (cw - 1.0);\n";
    o_ << "        if (ch > 1.0) ny = ((double)pt.y) / (ch - 1.0);\n";
    o_ << "        nx = ls_clamp01(nx);\n";
    o_ << "        ny = ls_clamp01(ny);\n";
    o_ << "        g->mouse_norm_x = nx;\n";
    o_ << "        g->mouse_norm_y = ny;\n";
    o_ << "        if (g->w > 1) g->mouse_x = nx * (double)(g->w - 1);\n";
    o_ << "        else g->mouse_x = 0.0;\n";
    o_ << "        if (g->h > 1) g->mouse_y = ny * (double)(g->h - 1);\n";
    o_ << "        else g->mouse_y = 0.0;\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "#else\n";
    o_ << "  (void)id;\n";
    o_ << "  g->mouse_x = 0.0;\n";
    o_ << "  g->mouse_y = 0.0;\n";
    o_ << "  g->mouse_norm_x = 0.0;\n";
    o_ << "  g->mouse_norm_y = 0.0;\n";
    o_ << "  g->scroll_x = 0.0;\n";
    o_ << "  g->scroll_y = 0.0;\n";
    o_ << "  g->mouse_buttons = 0u;\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline void game_begin(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  game_poll(id);\n";
    o_ << "  if (g->fixed_dt > 0.0) {\n";
    o_ << "    g->dt = g->fixed_dt;\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  LARGE_INTEGER now;\n";
    o_ << "  QueryPerformanceCounter(&now);\n";
    o_ << "  if (g->freq.QuadPart > 0) {\n";
    o_ << "    g->dt = (double)(now.QuadPart - g->last.QuadPart) / (double)g->freq.QuadPart;\n";
    o_ << "  } else {\n";
    o_ << "    g->dt = 1.0 / 60.0;\n";
    o_ << "  }\n";
    o_ << "  g->last = now;\n";
    o_ << "#else\n";
    o_ << "  struct timespec now;\n";
    o_ << "  timespec_get(&now, TIME_UTC);\n";
    o_ << "  int64_t ds = (int64_t)now.tv_sec - (int64_t)g->last_ts.tv_sec;\n";
    o_ << "  int64_t dn = (int64_t)now.tv_nsec - (int64_t)g->last_ts.tv_nsec;\n";
    o_ << "  if (dn < 0) {\n";
    o_ << "    dn += 1000000000LL;\n";
    o_ << "    ds -= 1;\n";
    o_ << "  }\n";
    o_ << "  if (ds < 0) {\n";
    o_ << "    ds = 0;\n";
    o_ << "    dn = 0;\n";
    o_ << "  }\n";
    o_ << "  g->dt = (double)ds + ((double)dn / 1000000000.0);\n";
    o_ << "  g->last_ts = now;\n";
    o_ << "#endif\n";
    o_ << "  if (!isfinite(g->dt) || g->dt < 0.0) g->dt = 0.0;\n";
    o_ << "  if (g->dt > 0.25) g->dt = 0.25;\n";
    o_ << "}\n";
    o_ << "static inline void game_present(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || g->headless || g->should_close) return;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  if (!g->hwnd || !g->hdc) return;\n";
    o_ << "  StretchDIBits(g->hdc, 0, 0, (int)g->w, (int)g->h, 0, 0, (int)g->w, (int)g->h,\n";
    o_ << "               g->pix, &g->bmi, DIB_RGB_COLORS, SRCCOPY);\n";
    o_ << "#else\n";
    o_ << "  (void)id;\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline void game_end(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  game_present(id);\n";
    o_ << "  if (g->target_fps > 0 && g->fixed_dt <= 0.0 && g->dt > 0.0) {\n";
    o_ << "    const double target = 1.0 / (double)g->target_fps;\n";
    o_ << "    if (g->dt < target) {\n";
    o_ << "      ls_game_sleep_ms((target - g->dt) * 1000.0);\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  g->frame += 1;\n";
    o_ << "}\n";
    o_ << "static inline double game_delta(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->dt : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t game_frame(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->frame : 0;\n";
    o_ << "}\n";
    o_ << "static inline double game_mouse_x(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->mouse_x : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double game_mouse_y(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->mouse_y : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double game_mouse_norm_x(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->mouse_norm_x : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double game_mouse_norm_y(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->mouse_norm_y : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double game_scroll_x(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->scroll_x : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double game_scroll_y(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  return g ? g->scroll_y : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool game_mouse_down(int64_t id, int64_t button) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return 0;\n";
    o_ << "  if (button < 1 || button > 5) return 0;\n";
    o_ << "  const uint32_t bit = (uint32_t)1u << (uint32_t)(button - 1);\n";
    o_ << "  return (g->mouse_buttons & bit) ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline int ls_mouse_button_from_name(const char *name) {\n";
    o_ << "  if (!name) return -1;\n";
    o_ << "  if (strcmp(name, \"LEFT\") == 0) return 1;\n";
    o_ << "  if (strcmp(name, \"RIGHT\") == 0) return 2;\n";
    o_ << "  if (strcmp(name, \"MIDDLE\") == 0) return 3;\n";
    o_ << "  if (strcmp(name, \"X1\") == 0) return 4;\n";
    o_ << "  if (strcmp(name, \"X2\") == 0) return 5;\n";
    o_ << "  return -1;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool game_mouse_down_name(int64_t id, const char *name) {\n";
    o_ << "  const int b = ls_mouse_button_from_name(name ? name : \"\");\n";
    o_ << "  if (b < 0) return 0;\n";
    o_ << "  return game_mouse_down(id, (int64_t)b);\n";
    o_ << "}\n";
    o_ << "static inline void game_clear(int64_t id, int64_t r, int64_t gg, int64_t b) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  const size_t area = (size_t)g->w * (size_t)g->h;\n";
    o_ << "  if (r8 == g8 && g8 == b8) {\n";
    o_ << "    memset(g->pix, r8, area * 3);\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  uint8_t *p = g->pix;\n";
    o_ << "  for (size_t i = 0; i < area; ++i) {\n";
    o_ << "    p[0] = r8;\n";
    o_ << "    p[1] = g8;\n";
    o_ << "    p[2] = b8;\n";
    o_ << "    p += 3;\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void game_set(int64_t id, int64_t x, int64_t y, int64_t r, int64_t gg, int64_t b) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  ls_game_set_pixel_u8(g, x, y, ls_color_u8(r), ls_color_u8(gg), ls_color_u8(b));\n";
    o_ << "}\n";
    o_ << "static inline int64_t game_get(int64_t id, int64_t x, int64_t y) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || !g->pix) return -1;\n";
    o_ << "  if ((uint64_t)x >= (uint64_t)g->w || (uint64_t)y >= (uint64_t)g->h) return -1;\n";
    o_ << "  const size_t idx = (((size_t)y * (size_t)g->w) + (size_t)x) * 3;\n";
    o_ << "  return ((int64_t)g->pix[idx] << 16) | ((int64_t)g->pix[idx + 1] << 8) | (int64_t)g->pix[idx + 2];\n";
    o_ << "}\n";
    o_ << "static inline void game_line(int64_t id, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int64_t r, int64_t gg, "
          "int64_t b) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g) return;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  int64_t dx = x1 - x0;\n";
    o_ << "  if (dx < 0) dx = -dx;\n";
    o_ << "  int64_t sx = x0 < x1 ? 1 : -1;\n";
    o_ << "  int64_t dy = y1 - y0;\n";
    o_ << "  if (dy < 0) dy = -dy;\n";
    o_ << "  int64_t sy = y0 < y1 ? 1 : -1;\n";
    o_ << "  int64_t err = dx - dy;\n";
    o_ << "  for (;;) {\n";
    o_ << "    ls_game_set_pixel_u8(g, x0, y0, r8, g8, b8);\n";
    o_ << "    if (x0 == x1 && y0 == y1) break;\n";
    o_ << "    const int64_t e2 = err << 1;\n";
    o_ << "    if (e2 > -dy) {\n";
    o_ << "      err -= dy;\n";
    o_ << "      x0 += sx;\n";
    o_ << "    }\n";
    o_ << "    if (e2 < dx) {\n";
    o_ << "      err += dx;\n";
    o_ << "      y0 += sy;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void game_rect(int64_t id, int64_t x, int64_t y, int64_t w, int64_t h, int64_t r, int64_t gg, "
          "int64_t b, ls_bool fill) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || !g->pix) return;\n";
    o_ << "  if (w <= 0 || h <= 0) return;\n";
    o_ << "  if (x > 9223372036854775807LL - w) return;\n";
    o_ << "  if (y > 9223372036854775807LL - h) return;\n";
    o_ << "  int64_t x0 = x;\n";
    o_ << "  int64_t y0 = y;\n";
    o_ << "  int64_t x1 = x + w;\n";
    o_ << "  int64_t y1 = y + h;\n";
    o_ << "  if (x1 <= 0 || y1 <= 0 || x0 >= g->w || y0 >= g->h) return;\n";
    o_ << "  if (x0 < 0) x0 = 0;\n";
    o_ << "  if (y0 < 0) y0 = 0;\n";
    o_ << "  if (x1 > g->w) x1 = g->w;\n";
    o_ << "  if (y1 > g->h) y1 = g->h;\n";
    o_ << "  const uint8_t r8 = ls_color_u8(r);\n";
    o_ << "  const uint8_t g8 = ls_color_u8(gg);\n";
    o_ << "  const uint8_t b8 = ls_color_u8(b);\n";
    o_ << "  if (fill) {\n";
    o_ << "    for (int64_t yy = y0; yy < y1; ++yy) {\n";
    o_ << "      uint8_t *row = g->pix + ((((size_t)yy * (size_t)g->w) + (size_t)x0) * 3);\n";
    o_ << "      for (int64_t xx = x0; xx < x1; ++xx) {\n";
    o_ << "        row[0] = r8;\n";
    o_ << "        row[1] = g8;\n";
    o_ << "        row[2] = b8;\n";
    o_ << "        row += 3;\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  for (int64_t xx = x0; xx < x1; ++xx) {\n";
    o_ << "    ls_game_set_pixel_u8(g, xx, y0, r8, g8, b8);\n";
    o_ << "    ls_game_set_pixel_u8(g, xx, y1 - 1, r8, g8, b8);\n";
    o_ << "  }\n";
    o_ << "  for (int64_t yy = y0 + 1; yy + 1 < y1; ++yy) {\n";
    o_ << "    ls_game_set_pixel_u8(g, x0, yy, r8, g8, b8);\n";
    o_ << "    ls_game_set_pixel_u8(g, x1 - 1, yy, r8, g8, b8);\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void game_draw_gfx(int64_t game_id, int64_t gfx_id, int64_t dst_x, int64_t dst_y) {\n";
    o_ << "  ls_game *g = ls_get_game(game_id);\n";
    o_ << "  ls_gfx *src = ls_get_gfx(gfx_id);\n";
    o_ << "  if (!g || !src || !g->pix || !src->pix) return;\n";
    o_ << "  int64_t sx0 = 0;\n";
    o_ << "  int64_t sy0 = 0;\n";
    o_ << "  int64_t sx1 = src->w;\n";
    o_ << "  int64_t sy1 = src->h;\n";
    o_ << "  if (dst_x < 0) sx0 = -dst_x;\n";
    o_ << "  if (dst_y < 0) sy0 = -dst_y;\n";
    o_ << "  if (dst_x + sx1 > g->w) sx1 = g->w - dst_x;\n";
    o_ << "  if (dst_y + sy1 > g->h) sy1 = g->h - dst_y;\n";
    o_ << "  if (sx0 >= sx1 || sy0 >= sy1) return;\n";
    o_ << "  for (int64_t y = sy0; y < sy1; ++y) {\n";
    o_ << "    uint8_t *dst_row = g->pix + ((((size_t)(dst_y + y) * (size_t)g->w) + (size_t)(dst_x + sx0)) * 3);\n";
    o_ << "    const uint8_t *src_row = src->pix + ((((size_t)y * (size_t)src->w) + (size_t)sx0) * 3);\n";
    o_ << "    const size_t row_bytes = (size_t)(sx1 - sx0) * 3;\n";
    o_ << "    memcpy(dst_row, src_row, row_bytes);\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline ls_bool game_save_ppm(int64_t id, const char *path) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || !g->pix) return 0;\n";
    o_ << "  const char *p = path ? path : \"\";\n";
    o_ << "  if (p[0] == '\\0') return 0;\n";
    o_ << "  FILE *f = NULL;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  if (fopen_s(&f, p, \"wb\") != 0 || !f) return 0;\n";
    o_ << "#else\n";
    o_ << "  f = fopen(p, \"wb\");\n";
    o_ << "  if (!f) return 0;\n";
    o_ << "#endif\n";
    o_ << "  if (fprintf(f, \"P6\\n%lld %lld\\n255\\n\", (long long)g->w, (long long)g->h) < 0) {\n";
    o_ << "    fclose(f);\n";
    o_ << "    return 0;\n";
    o_ << "  }\n";
    o_ << "  const size_t bytes = ((size_t)g->w * (size_t)g->h) * 3;\n";
    o_ << "  const size_t wrote = fwrite(g->pix, 1, bytes, f);\n";
    o_ << "  if (fclose(f) != 0) return 0;\n";
    o_ << "  return wrote == bytes ? 1 : 0;\n";
    o_ << "}\n";
    o_ << "static inline int64_t game_checksum(int64_t id) {\n";
    o_ << "  ls_game *g = ls_get_game(id);\n";
    o_ << "  if (!g || !g->pix) return 0;\n";
    o_ << "  const size_t bytes = ((size_t)g->w * (size_t)g->h) * 3;\n";
    o_ << "  uint64_t h = 1469598103934665603ULL;\n";
    o_ << "  for (size_t i = 0; i < bytes; ++i) {\n";
    o_ << "    h ^= (uint64_t)g->pix[i];\n";
    o_ << "    h *= 1099511628211ULL;\n";
    o_ << "  }\n";
    o_ << "  return (int64_t)(h & 0x7FFFFFFFFFFFFFFFULL);\n";
    o_ << "}\n";
    o_ << "static inline ls_bool key_down(int64_t code);\n";
    o_ << "static inline ls_bool key_down_name(const char *name);\n";
    o_ << "static inline int64_t pg_init(int64_t w, int64_t h, const char *title, ls_bool visible) {\n";
    o_ << "  return game_new(w, h, title, visible);\n";
    o_ << "}\n";
    o_ << "static inline void pg_quit(int64_t game) { game_free(game); }\n";
    o_ << "static inline ls_bool pg_should_quit(int64_t game) { return game_should_close(game); }\n";
    o_ << "static inline void pg_begin(int64_t game) { game_begin(game); }\n";
    o_ << "static inline void pg_end(int64_t game) { game_end(game); }\n";
    o_ << "static inline void pg_set_target_fps(int64_t game, int64_t fps) { game_set_target_fps(game, fps); }\n";
    o_ << "static inline void pg_set_fixed_dt(int64_t game, double dt) { game_set_fixed_dt(game, dt); }\n";
    o_ << "static inline void pg_clear(int64_t game, int64_t r, int64_t g, int64_t b) { game_clear(game, r, g, b); }\n";
    o_ << "static inline void pg_draw_pixel(int64_t game, int64_t x, int64_t y, int64_t r, int64_t g, int64_t b) {\n";
    o_ << "  game_set(game, x, y, r, g, b);\n";
    o_ << "}\n";
    o_ << "static inline void pg_draw_line(int64_t game, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int64_t r, int64_t g, int64_t b) {\n";
    o_ << "  game_line(game, x0, y0, x1, y1, r, g, b);\n";
    o_ << "}\n";
    o_ << "static inline void pg_draw_rect(int64_t game, int64_t x, int64_t y, int64_t w, int64_t h, int64_t r, int64_t g, int64_t b, ls_bool fill) {\n";
    o_ << "  game_rect(game, x, y, w, h, r, g, b, fill);\n";
    o_ << "}\n";
    o_ << "static inline void pg_blit(int64_t game, int64_t surface, int64_t x, int64_t y) { game_draw_gfx(game, surface, x, y); }\n";
    o_ << "static inline int64_t pg_get_pixel(int64_t game, int64_t x, int64_t y) { return game_get(game, x, y); }\n";
    o_ << "static inline ls_bool pg_save_ppm(int64_t game, const char *path) { return game_save_ppm(game, path); }\n";
    o_ << "static inline int64_t pg_checksum(int64_t game) { return game_checksum(game); }\n";
    o_ << "static inline double pg_mouse_x(int64_t game) { return game_mouse_x(game); }\n";
    o_ << "static inline double pg_mouse_y(int64_t game) { return game_mouse_y(game); }\n";
    o_ << "static inline double pg_mouse_norm_x(int64_t game) { return game_mouse_norm_x(game); }\n";
    o_ << "static inline double pg_mouse_norm_y(int64_t game) { return game_mouse_norm_y(game); }\n";
    o_ << "static inline double pg_scroll_x(int64_t game) { return game_scroll_x(game); }\n";
    o_ << "static inline double pg_scroll_y(int64_t game) { return game_scroll_y(game); }\n";
    o_ << "static inline double pg_delta(int64_t game) { return game_delta(game); }\n";
    o_ << "static inline int64_t pg_frame(int64_t game) { return game_frame(game); }\n";
    o_ << "static inline ls_bool pg_key_down(int64_t code) { return key_down(code); }\n";
    o_ << "static inline ls_bool pg_key_down_name(const char *name) { return key_down_name(name); }\n";
    o_ << "static inline ls_bool pg_mouse_down(int64_t game, int64_t button) { return game_mouse_down(game, button); }\n";
    o_ << "static inline ls_bool pg_mouse_down_name(int64_t game, const char *name) { return game_mouse_down_name(game, name); }\n";
    o_ << "static inline int64_t pg_surface_new(int64_t w, int64_t h) { return gfx_new(w, h); }\n";
    o_ << "static inline void pg_surface_free(int64_t s) { gfx_free(s); }\n";
    o_ << "static inline void pg_surface_clear(int64_t s, int64_t r, int64_t g, int64_t b) { gfx_clear(s, r, g, b); }\n";
    o_ << "static inline void pg_surface_set(int64_t s, int64_t x, int64_t y, int64_t r, int64_t g, int64_t b) { gfx_set(s, x, y, r, g, b); }\n";
    o_ << "static inline int64_t pg_surface_get(int64_t s, int64_t x, int64_t y) { return gfx_get(s, x, y); }\n";
    o_ << "static inline void pg_surface_line(int64_t s, int64_t x0, int64_t y0, int64_t x1, int64_t y1, int64_t r, int64_t g, int64_t b) {\n";
    o_ << "  gfx_line(s, x0, y0, x1, y1, r, g, b);\n";
    o_ << "}\n";
    o_ << "static inline void pg_surface_rect(int64_t s, int64_t x, int64_t y, int64_t w, int64_t h, int64_t r, int64_t g, int64_t b, ls_bool fill) {\n";
    o_ << "  gfx_rect(s, x, y, w, h, r, g, b, fill);\n";
    o_ << "}\n";
    o_ << "static inline ls_bool pg_surface_save_ppm(int64_t s, const char *path) { return gfx_save_ppm(s, path); }\n";
    o_ << "#define LS_MAX_PHYS 4096\n";
    o_ << "typedef struct {\n";
    o_ << "  double px;\n";
    o_ << "  double py;\n";
    o_ << "  double pz;\n";
    o_ << "  double vx;\n";
    o_ << "  double vy;\n";
    o_ << "  double vz;\n";
    o_ << "  double fx;\n";
    o_ << "  double fy;\n";
    o_ << "  double fz;\n";
    o_ << "  double restx;\n";
    o_ << "  double resty;\n";
    o_ << "  double restz;\n";
    o_ << "  double mass;\n";
    o_ << "  double inv_mass;\n";
    o_ << "  double damping;\n";
    o_ << "  double bounce;\n";
    o_ << "  ls_bool soft;\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_phys_obj;\n";
    o_ << "static ls_phys_obj ls_phys_objs[LS_MAX_PHYS];\n";
    o_ << "static int64_t ls_phys_count = 0;\n";
    o_ << "static int64_t ls_phys_free_ids[LS_MAX_PHYS];\n";
    o_ << "static int64_t ls_phys_free_top = 0;\n";
    o_ << "static inline int64_t ls_phys_alloc_id(void) {\n";
    o_ << "  if (ls_phys_free_top > 0) return ls_phys_free_ids[--ls_phys_free_top];\n";
    o_ << "  if (ls_phys_count >= LS_MAX_PHYS) return -1;\n";
    o_ << "  return ls_phys_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_phys_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_PHYS) return;\n";
    o_ << "  if (ls_phys_free_top < LS_MAX_PHYS) ls_phys_free_ids[ls_phys_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static int64_t ls_camera_target = -1;\n";
    o_ << "static double ls_camera_off_x = 0.0;\n";
    o_ << "static double ls_camera_off_y = 1.8;\n";
    o_ << "static double ls_camera_off_z = -6.0;\n";
    o_ << "static double ls_camera_x = 0.0;\n";
    o_ << "static double ls_camera_y = 0.0;\n";
    o_ << "static double ls_camera_z = 0.0;\n";
    o_ << "static int64_t ls_phys_ids[LS_MAX_PHYS];\n";
    o_ << "static int64_t ls_phys_hard_idx[LS_MAX_PHYS];\n";
    o_ << "static int64_t ls_phys_soft_idx[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_px_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_py_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_pz_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_vx_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_vy_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_vz_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_fx_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_fy_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_fz_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_restx_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_resty_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_restz_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_inv_mass_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_damping_soa[LS_MAX_PHYS];\n";
    o_ << "static double ls_phys_bounce_soa[LS_MAX_PHYS];\n";
    o_ << "static ls_bool ls_phys_soft_soa[LS_MAX_PHYS];\n";
    o_ << "static inline ls_phys_obj *ls_get_phys(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= ls_phys_count) return NULL;\n";
    o_ << "  if (!ls_phys_objs[id].active) return NULL;\n";
    o_ << "  return &ls_phys_objs[id];\n";
    o_ << "}\n";
    o_ << "static inline void ls_camera_refresh(void) {\n";
    o_ << "  ls_phys_obj *t = ls_get_phys(ls_camera_target);\n";
    o_ << "  if (!t) return;\n";
    o_ << "  ls_camera_x = t->px + ls_camera_off_x;\n";
    o_ << "  ls_camera_y = t->py + ls_camera_off_y;\n";
    o_ << "  ls_camera_z = t->pz + ls_camera_off_z;\n";
    o_ << "}\n";
    o_ << "static inline void ls_camera_bind_default(void) {\n";
    o_ << "  if (ls_get_phys(ls_camera_target)) {\n";
    o_ << "    ls_camera_refresh();\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  ls_camera_target = -1;\n";
    o_ << "  for (int64_t i = 0; i < ls_phys_count; ++i) {\n";
    o_ << "    if (ls_phys_objs[i].active) {\n";
    o_ << "      ls_camera_target = i;\n";
    o_ << "      break;\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  ls_camera_refresh();\n";
    o_ << "}\n";
    o_ << "static inline int64_t phys_new(double x, double y, double z, double mass, ls_bool soft) {\n";
    o_ << "  if (!isfinite(x) || !isfinite(y) || !isfinite(z)) return -1;\n";
    o_ << "  if (!isfinite(mass) || mass <= 0.0) mass = 1.0;\n";
    o_ << "  const int64_t id = ls_phys_alloc_id();\n";
    o_ << "  if (id < 0) return -1;\n";
    o_ << "  ls_phys_obj *o = &ls_phys_objs[id];\n";
    o_ << "  o->px = x;\n";
    o_ << "  o->py = y;\n";
    o_ << "  o->pz = z;\n";
    o_ << "  o->vx = 0.0;\n";
    o_ << "  o->vy = 0.0;\n";
    o_ << "  o->vz = 0.0;\n";
    o_ << "  o->fx = 0.0;\n";
    o_ << "  o->fy = 0.0;\n";
    o_ << "  o->fz = 0.0;\n";
    o_ << "  o->restx = x;\n";
    o_ << "  o->resty = y;\n";
    o_ << "  o->restz = z;\n";
    o_ << "  o->mass = mass;\n";
    o_ << "  o->inv_mass = 1.0 / mass;\n";
    o_ << "  o->soft = soft ? 1 : 0;\n";
    o_ << "  o->damping = o->soft ? 0.88 : 0.96;\n";
    o_ << "  o->bounce = o->soft ? 0.08 : 0.32;\n";
    o_ << "  o->active = 1;\n";
    o_ << "  if (ls_camera_target < 0) ls_camera_target = id;\n";
    o_ << "  ls_camera_refresh();\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline void phys_free(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  o->active = 0;\n";
    o_ << "  ls_phys_release_id(id);\n";
    o_ << "  if (ls_camera_target == id) ls_camera_target = -1;\n";
    o_ << "  ls_camera_bind_default();\n";
    o_ << "}\n";
    o_ << "static inline void phys_set_position(int64_t id, double x, double y, double z) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  if (!isfinite(x) || !isfinite(y) || !isfinite(z)) return;\n";
    o_ << "  o->px = x;\n";
    o_ << "  o->py = y;\n";
    o_ << "  o->pz = z;\n";
    o_ << "  o->restx = x;\n";
    o_ << "  o->resty = y;\n";
    o_ << "  o->restz = z;\n";
    o_ << "  if (ls_camera_target == id) ls_camera_refresh();\n";
    o_ << "}\n";
    o_ << "static inline void phys_set_velocity(int64_t id, double vx, double vy, double vz) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  if (!isfinite(vx) || !isfinite(vy) || !isfinite(vz)) return;\n";
    o_ << "  o->vx = vx;\n";
    o_ << "  o->vy = vy;\n";
    o_ << "  o->vz = vz;\n";
    o_ << "}\n";
    o_ << "static inline void phys_move(int64_t id, double dx, double dy, double dz) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  if (!isfinite(dx) || !isfinite(dy) || !isfinite(dz)) return;\n";
    o_ << "  o->px += dx;\n";
    o_ << "  o->py += dy;\n";
    o_ << "  o->pz += dz;\n";
    o_ << "  o->restx += dx;\n";
    o_ << "  o->resty += dy;\n";
    o_ << "  o->restz += dz;\n";
    o_ << "  if (ls_camera_target == id) ls_camera_refresh();\n";
    o_ << "}\n";
    o_ << "static inline void phys_apply_force(int64_t id, double fx, double fy, double fz) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  if (!o) return;\n";
    o_ << "  if (!isfinite(fx) || !isfinite(fy) || !isfinite(fz)) return;\n";
    o_ << "  o->fx += fx;\n";
    o_ << "  o->fy += fy;\n";
    o_ << "  o->fz += fz;\n";
    o_ << "}\n";
    o_ << "static inline void phys_step(double dt) {\n";
    o_ << "  if (!isfinite(dt) || dt <= 0.0) return;\n";
    o_ << "  if (dt > 0.1) dt = 0.1;\n";
    o_ << "  const double dt60 = dt * 60.0;\n";
    o_ << "  int64_t n = 0;\n";
    o_ << "  int64_t hard_n = 0;\n";
    o_ << "  int64_t soft_n = 0;\n";
    o_ << "  for (int64_t i = 0; i < ls_phys_count; ++i) {\n";
    o_ << "    ls_phys_obj *o = &ls_phys_objs[i];\n";
    o_ << "    if (!o->active) continue;\n";
    o_ << "    const int64_t j = n++;\n";
    o_ << "    ls_phys_ids[j] = i;\n";
    o_ << "    ls_phys_px_soa[j] = o->px;\n";
    o_ << "    ls_phys_py_soa[j] = o->py;\n";
    o_ << "    ls_phys_pz_soa[j] = o->pz;\n";
    o_ << "    ls_phys_vx_soa[j] = o->vx;\n";
    o_ << "    ls_phys_vy_soa[j] = o->vy;\n";
    o_ << "    ls_phys_vz_soa[j] = o->vz;\n";
    o_ << "    ls_phys_fx_soa[j] = o->fx;\n";
    o_ << "    ls_phys_fy_soa[j] = o->fy;\n";
    o_ << "    ls_phys_fz_soa[j] = o->fz;\n";
    o_ << "    ls_phys_restx_soa[j] = o->restx;\n";
    o_ << "    ls_phys_resty_soa[j] = o->resty;\n";
    o_ << "    ls_phys_restz_soa[j] = o->restz;\n";
    o_ << "    ls_phys_inv_mass_soa[j] = o->inv_mass;\n";
    o_ << "    ls_phys_damping_soa[j] = o->damping;\n";
    o_ << "    ls_phys_bounce_soa[j] = o->bounce;\n";
    o_ << "    ls_phys_soft_soa[j] = o->soft ? 1 : 0;\n";
    o_ << "    if (o->soft) ls_phys_soft_idx[soft_n++] = j;\n";
    o_ << "    else ls_phys_hard_idx[hard_n++] = j;\n";
    o_ << "  }\n";
    o_ << "  for (int64_t h = 0; h < hard_n; ++h) {\n";
    o_ << "    const int64_t j = ls_phys_hard_idx[h];\n";
    o_ << "    double ax = ls_phys_fx_soa[j] * ls_phys_inv_mass_soa[j];\n";
    o_ << "    double ay = ls_phys_fy_soa[j] * ls_phys_inv_mass_soa[j] - 9.81;\n";
    o_ << "    double az = ls_phys_fz_soa[j] * ls_phys_inv_mass_soa[j];\n";
    o_ << "    ls_phys_vx_soa[j] += ax * dt;\n";
    o_ << "    ls_phys_vy_soa[j] += ay * dt;\n";
    o_ << "    ls_phys_vz_soa[j] += az * dt;\n";
    o_ << "    double damp = 1.0 - ((1.0 - ls_phys_damping_soa[j]) * dt60);\n";
    o_ << "    if (damp < 0.0) damp = 0.0;\n";
    o_ << "    if (damp > 1.0) damp = 1.0;\n";
    o_ << "    ls_phys_vx_soa[j] *= damp;\n";
    o_ << "    ls_phys_vy_soa[j] *= damp;\n";
    o_ << "    ls_phys_vz_soa[j] *= damp;\n";
    o_ << "    ls_phys_px_soa[j] += ls_phys_vx_soa[j] * dt;\n";
    o_ << "    ls_phys_py_soa[j] += ls_phys_vy_soa[j] * dt;\n";
    o_ << "    ls_phys_pz_soa[j] += ls_phys_vz_soa[j] * dt;\n";
    o_ << "    if (ls_phys_py_soa[j] < 0.0) {\n";
    o_ << "      ls_phys_py_soa[j] = 0.0;\n";
    o_ << "      if (ls_phys_vy_soa[j] < 0.0) ls_phys_vy_soa[j] = -ls_phys_vy_soa[j] * ls_phys_bounce_soa[j];\n";
    o_ << "      ls_phys_vx_soa[j] *= 0.95;\n";
    o_ << "      ls_phys_vz_soa[j] *= 0.95;\n";
    o_ << "    }\n";
    o_ << "    ls_phys_fx_soa[j] = 0.0;\n";
    o_ << "    ls_phys_fy_soa[j] = 0.0;\n";
    o_ << "    ls_phys_fz_soa[j] = 0.0;\n";
    o_ << "  }\n";
    o_ << "  for (int64_t h = 0; h < soft_n; ++h) {\n";
    o_ << "    const int64_t j = ls_phys_soft_idx[h];\n";
    o_ << "    double ax = ls_phys_fx_soa[j] * ls_phys_inv_mass_soa[j];\n";
    o_ << "    double ay = ls_phys_fy_soa[j] * ls_phys_inv_mass_soa[j] - 9.81;\n";
    o_ << "    double az = ls_phys_fz_soa[j] * ls_phys_inv_mass_soa[j];\n";
    o_ << "    const double k = 22.0;\n";
    o_ << "    const double c = 2.0;\n";
    o_ << "    ax += (ls_phys_restx_soa[j] - ls_phys_px_soa[j]) * k - ls_phys_vx_soa[j] * c;\n";
    o_ << "    ay += (ls_phys_resty_soa[j] - ls_phys_py_soa[j]) * k - ls_phys_vy_soa[j] * c;\n";
    o_ << "    az += (ls_phys_restz_soa[j] - ls_phys_pz_soa[j]) * k - ls_phys_vz_soa[j] * c;\n";
    o_ << "    ls_phys_vx_soa[j] += ax * dt;\n";
    o_ << "    ls_phys_vy_soa[j] += ay * dt;\n";
    o_ << "    ls_phys_vz_soa[j] += az * dt;\n";
    o_ << "    double damp = 1.0 - ((1.0 - ls_phys_damping_soa[j]) * dt60);\n";
    o_ << "    if (damp < 0.0) damp = 0.0;\n";
    o_ << "    if (damp > 1.0) damp = 1.0;\n";
    o_ << "    ls_phys_vx_soa[j] *= damp;\n";
    o_ << "    ls_phys_vy_soa[j] *= damp;\n";
    o_ << "    ls_phys_vz_soa[j] *= damp;\n";
    o_ << "    ls_phys_px_soa[j] += ls_phys_vx_soa[j] * dt;\n";
    o_ << "    ls_phys_py_soa[j] += ls_phys_vy_soa[j] * dt;\n";
    o_ << "    ls_phys_pz_soa[j] += ls_phys_vz_soa[j] * dt;\n";
    o_ << "    if (ls_phys_py_soa[j] < 0.0) {\n";
    o_ << "      ls_phys_py_soa[j] = 0.0;\n";
    o_ << "      if (ls_phys_vy_soa[j] < 0.0) ls_phys_vy_soa[j] = -ls_phys_vy_soa[j] * ls_phys_bounce_soa[j];\n";
    o_ << "      ls_phys_vx_soa[j] *= 0.95;\n";
    o_ << "      ls_phys_vz_soa[j] *= 0.95;\n";
    o_ << "    }\n";
    o_ << "    ls_phys_fx_soa[j] = 0.0;\n";
    o_ << "    ls_phys_fy_soa[j] = 0.0;\n";
    o_ << "    ls_phys_fz_soa[j] = 0.0;\n";
    o_ << "  }\n";
    o_ << "  for (int64_t j = 0; j < n; ++j) {\n";
    o_ << "    ls_phys_obj *o = &ls_phys_objs[ls_phys_ids[j]];\n";
    o_ << "    o->px = ls_phys_px_soa[j];\n";
    o_ << "    o->py = ls_phys_py_soa[j];\n";
    o_ << "    o->pz = ls_phys_pz_soa[j];\n";
    o_ << "    o->vx = ls_phys_vx_soa[j];\n";
    o_ << "    o->vy = ls_phys_vy_soa[j];\n";
    o_ << "    o->vz = ls_phys_vz_soa[j];\n";
    o_ << "    o->fx = 0.0;\n";
    o_ << "    o->fy = 0.0;\n";
    o_ << "    o->fz = 0.0;\n";
    o_ << "  }\n";
    o_ << "  ls_camera_bind_default();\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_x(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->px : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_y(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->py : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_z(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->pz : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_vx(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->vx : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_vy(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->vy : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline double phys_get_vz(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->vz : 0.0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool phys_is_soft(int64_t id) {\n";
    o_ << "  ls_phys_obj *o = ls_get_phys(id);\n";
    o_ << "  return o ? o->soft : 0;\n";
    o_ << "}\n";
    o_ << "static inline void camera_bind(int64_t id) {\n";
    o_ << "  if (ls_get_phys(id)) {\n";
    o_ << "    ls_camera_target = id;\n";
    o_ << "    ls_camera_refresh();\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  ls_camera_bind_default();\n";
    o_ << "}\n";
    o_ << "static inline int64_t camera_target(void) {\n";
    o_ << "  ls_camera_bind_default();\n";
    o_ << "  return ls_camera_target;\n";
    o_ << "}\n";
    o_ << "static inline void camera_set_offset(double x, double y, double z) {\n";
    o_ << "  if (!isfinite(x) || !isfinite(y) || !isfinite(z)) return;\n";
    o_ << "  ls_camera_off_x = x;\n";
    o_ << "  ls_camera_off_y = y;\n";
    o_ << "  ls_camera_off_z = z;\n";
    o_ << "  ls_camera_refresh();\n";
    o_ << "}\n";
    o_ << "static inline double camera_get_x(void) { ls_camera_bind_default(); return ls_camera_x; }\n";
    o_ << "static inline double camera_get_y(void) { ls_camera_bind_default(); return ls_camera_y; }\n";
    o_ << "static inline double camera_get_z(void) { ls_camera_bind_default(); return ls_camera_z; }\n";
    o_ << "static inline int ls_key_code_from_name(const char *name) {\n";
    o_ << "  if (!name) return -1;\n";
    o_ << "  if (strcmp(name, \"W\") == 0) return 87;\n";
    o_ << "  if (strcmp(name, \"A\") == 0) return 65;\n";
    o_ << "  if (strcmp(name, \"S\") == 0) return 83;\n";
    o_ << "  if (strcmp(name, \"D\") == 0) return 68;\n";
    o_ << "  if (strcmp(name, \"UP\") == 0) return 38;\n";
    o_ << "  if (strcmp(name, \"DOWN\") == 0) return 40;\n";
    o_ << "  if (strcmp(name, \"LEFT\") == 0) return 37;\n";
    o_ << "  if (strcmp(name, \"RIGHT\") == 0) return 39;\n";
    o_ << "  if (strcmp(name, \"SPACE\") == 0) return 32;\n";
    o_ << "  if (strcmp(name, \"ESC\") == 0) return 27;\n";
    o_ << "  if (strcmp(name, \"SHIFT\") == 0) return 16;\n";
    o_ << "  if (strcmp(name, \"CTRL\") == 0) return 17;\n";
    o_ << "  if (strcmp(name, \"ENTER\") == 0) return 13;\n";
    o_ << "  return -1;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool key_down(int64_t code) {\n";
    o_ << "  if (code < 0 || code > 255) return 0;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  const SHORT s = GetAsyncKeyState((int)code);\n";
    o_ << "  return (s & 0x8000) ? 1 : 0;\n";
    o_ << "#else\n";
    o_ << "  (void)code;\n";
    o_ << "  return 0;\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline ls_bool key_down_name(const char *name) {\n";
    o_ << "  const int code = ls_key_code_from_name(name ? name : \"\");\n";
    o_ << "  if (code < 0) return 0;\n";
    o_ << "  return key_down((int64_t)code);\n";
    o_ << "}\n";
    o_ << "static inline int64_t parse_i64(const char *s) {\n";
    o_ << "  if (!s) return 0;\n";
    o_ << "  const unsigned char *p = (const unsigned char *)s;\n";
    o_ << "  while (*p && isspace(*p)) ++p;\n";
    o_ << "  int neg = 0;\n";
    o_ << "  if (*p == '-') { neg = 1; ++p; }\n";
    o_ << "  else if (*p == '+') { ++p; }\n";
    o_ << "  int64_t out = 0;\n";
    o_ << "  const int64_t kMax = 9223372036854775807LL;\n";
    o_ << "  const int64_t kMaxDiv10 = 922337203685477580LL;\n";
    o_ << "  while (*p >= '0' && *p <= '9') {\n";
    o_ << "    const int64_t d = (int64_t)(*p - '0');\n";
    o_ << "    if (out > kMaxDiv10 || (out == kMaxDiv10 && d > (neg ? 8 : 7))) {\n";
    o_ << "      return neg ? (-9223372036854775807LL - 1LL) : kMax;\n";
    o_ << "    }\n";
    o_ << "    out = out * 10 + d;\n";
    o_ << "    ++p;\n";
    o_ << "  }\n";
    o_ << "  return neg ? -out : out;\n";
    o_ << "}\n";
    o_ << "static inline double parse_f64(const char *s) {\n";
    o_ << "  if (!s) return 0.0;\n";
    o_ << "  return strtod(s, NULL);\n";
    o_ << "}\n";
    o_ << "static inline int64_t input_i64(void) { return parse_i64(input()); }\n";
    o_ << "static inline int64_t input_i64_prompt(const char *prompt) { return parse_i64(input_prompt(prompt)); }\n";
    o_ << "static inline double input_f64(void) { return parse_f64(input()); }\n";
    o_ << "static inline double input_f64_prompt(const char *prompt) { return parse_f64(input_prompt(prompt)); }\n";
    if (needsHttpRuntime_) {
      o_ << "#define LS_HTTP_MAX_SERVERS 64\n";
      o_ << "#define LS_HTTP_MAX_CLIENTS 256\n";
      o_ << "#if defined(_WIN32)\n";
      o_ << "typedef SOCKET ls_http_socket;\n";
      o_ << "static ls_bool ls_http_wsa_ready = 0;\n";
      o_ << "static inline ls_bool ls_http_init(void) {\n";
      o_ << "  if (ls_http_wsa_ready) return 1;\n";
      o_ << "  WSADATA wsa;\n";
      o_ << "  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 0;\n";
      o_ << "  ls_http_wsa_ready = 1;\n";
      o_ << "  return 1;\n";
      o_ << "}\n";
      o_ << "static inline ls_bool ls_http_is_bad_socket(ls_http_socket s) { return s == INVALID_SOCKET; }\n";
      o_ << "static inline ls_http_socket ls_http_bad_socket(void) { return INVALID_SOCKET; }\n";
      o_ << "static inline void ls_http_close_socket(ls_http_socket s) {\n";
      o_ << "  if (s != INVALID_SOCKET) (void)closesocket(s);\n";
      o_ << "}\n";
      o_ << "#else\n";
      o_ << "typedef int ls_http_socket;\n";
      o_ << "static inline ls_bool ls_http_init(void) { return 1; }\n";
      o_ << "static inline ls_bool ls_http_is_bad_socket(ls_http_socket s) { return s < 0; }\n";
      o_ << "static inline ls_http_socket ls_http_bad_socket(void) { return -1; }\n";
      o_ << "static inline void ls_http_close_socket(ls_http_socket s) {\n";
      o_ << "  if (s >= 0) (void)close(s);\n";
      o_ << "}\n";
      o_ << "#endif\n";
      o_ << "typedef struct {\n";
      o_ << "  ls_http_socket sock;\n";
      o_ << "  ls_bool active;\n";
      o_ << "} ls_http_server_slot;\n";
      o_ << "typedef struct {\n";
      o_ << "  ls_http_socket sock;\n";
      o_ << "  ls_bool active;\n";
      o_ << "} ls_http_client_slot;\n";
      o_ << "static ls_http_server_slot ls_http_servers[LS_HTTP_MAX_SERVERS];\n";
      o_ << "static ls_http_client_slot ls_http_clients[LS_HTTP_MAX_CLIENTS];\n";
      o_ << "static int64_t ls_http_server_next = 0;\n";
      o_ << "static int64_t ls_http_client_next = 0;\n";
      o_ << "static int64_t ls_http_server_free_ids[LS_HTTP_MAX_SERVERS];\n";
      o_ << "static int64_t ls_http_client_free_ids[LS_HTTP_MAX_CLIENTS];\n";
      o_ << "static int64_t ls_http_server_free_top = 0;\n";
      o_ << "static int64_t ls_http_client_free_top = 0;\n";
      o_ << "static inline ls_http_server_slot *ls_http_get_server(int64_t id) {\n";
      o_ << "  if (id < 0 || id >= LS_HTTP_MAX_SERVERS) return NULL;\n";
      o_ << "  if (!ls_http_servers[id].active) return NULL;\n";
      o_ << "  return &ls_http_servers[id];\n";
      o_ << "}\n";
      o_ << "static inline ls_http_client_slot *ls_http_get_client(int64_t id) {\n";
      o_ << "  if (id < 0 || id >= LS_HTTP_MAX_CLIENTS) return NULL;\n";
      o_ << "  if (!ls_http_clients[id].active) return NULL;\n";
      o_ << "  return &ls_http_clients[id];\n";
      o_ << "}\n";
      o_ << "static inline int64_t ls_http_alloc_server(ls_http_socket sock) {\n";
      o_ << "  int64_t id = -1;\n";
      o_ << "  if (ls_http_server_free_top > 0) id = ls_http_server_free_ids[--ls_http_server_free_top];\n";
      o_ << "  else if (ls_http_server_next < LS_HTTP_MAX_SERVERS) id = ls_http_server_next++;\n";
      o_ << "  if (id < 0) return -1;\n";
      o_ << "  ls_http_servers[id].active = 1;\n";
      o_ << "  ls_http_servers[id].sock = sock;\n";
      o_ << "  return id;\n";
      o_ << "}\n";
      o_ << "static inline int64_t ls_http_alloc_client(ls_http_socket sock) {\n";
      o_ << "  int64_t id = -1;\n";
      o_ << "  if (ls_http_client_free_top > 0) id = ls_http_client_free_ids[--ls_http_client_free_top];\n";
      o_ << "  else if (ls_http_client_next < LS_HTTP_MAX_CLIENTS) id = ls_http_client_next++;\n";
      o_ << "  if (id < 0) return -1;\n";
      o_ << "  ls_http_clients[id].active = 1;\n";
      o_ << "  ls_http_clients[id].sock = sock;\n";
      o_ << "  return id;\n";
      o_ << "}\n";
      o_ << "static inline void ls_http_set_nodelay(ls_http_socket sock) {\n";
      o_ << "#if defined(TCP_NODELAY)\n";
      o_ << "  int one = 1;\n";
#if defined(_WIN32)
      o_ << "  (void)setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, (int)sizeof(one));\n";
#else
      o_ << "  (void)setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, (int)sizeof(one));\n";
#endif
      o_ << "#endif\n";
      o_ << "}\n";
      o_ << "#if defined(_WIN32)\n";
      o_ << "static inline int ls_http_send_chunk(ls_http_socket sock, const char *data, int chunk) {\n";
      o_ << "  return (int)send(sock, data, chunk, 0);\n";
      o_ << "}\n";
      o_ << "#else\n";
      o_ << "#if defined(MSG_NOSIGNAL)\n";
      o_ << "#define LS_HTTP_SEND_FLAGS MSG_NOSIGNAL\n";
      o_ << "#else\n";
      o_ << "#define LS_HTTP_SEND_FLAGS 0\n";
      o_ << "#endif\n";
      o_ << "static inline int ls_http_send_chunk(ls_http_socket sock, const char *data, int chunk) {\n";
      o_ << "  return (int)send(sock, data, chunk, LS_HTTP_SEND_FLAGS);\n";
      o_ << "}\n";
      o_ << "#endif\n";
      o_ << "static inline ls_bool ls_http_send_all(ls_http_socket sock, const char *data, size_t len) {\n";
      o_ << "  if (!data || len == 0) return 1;\n";
      o_ << "  size_t off = 0;\n";
      o_ << "  while (off < len) {\n";
      o_ << "    const size_t left = len - off;\n";
      o_ << "    const int chunk = (left > 2147483647u) ? 2147483647 : (int)left;\n";
      o_ << "    const int sent = ls_http_send_chunk(sock, data + off, chunk);\n";
      o_ << "    if (sent <= 0) return 0;\n";
      o_ << "    off += (size_t)sent;\n";
      o_ << "  }\n";
      o_ << "  return 1;\n";
      o_ << "}\n";
      o_ << "static LS_THREAD_LOCAL char ls_http_recv_once_buf[16385];\n";
      o_ << "static inline const char *ls_http_recv_once(ls_http_socket sock) {\n";
      o_ << "  char *buf = ls_http_recv_once_buf;\n";
      o_ << "  const int n = (int)recv(sock, buf, 16384, 0);\n";
      o_ << "  if (n <= 0) {\n";
      o_ << "    buf[0] = '\\0';\n";
      o_ << "    return buf;\n";
      o_ << "  }\n";
      o_ << "  buf[n] = '\\0';\n";
      o_ << "  return buf;\n";
      o_ << "}\n";
      o_ << "static LS_THREAD_LOCAL char *ls_http_recv_heap = NULL;\n";
      o_ << "static LS_THREAD_LOCAL size_t ls_http_recv_cap = 0;\n";
      o_ << "static inline ls_bool ls_http_recv_reserve(size_t need) {\n";
      o_ << "  if (need <= ls_http_recv_cap && ls_http_recv_heap) return 1;\n";
      o_ << "  size_t next_cap = ls_http_recv_cap ? ls_http_recv_cap : 4096;\n";
      o_ << "  while (next_cap < need && next_cap < 1048576) next_cap <<= 1;\n";
      o_ << "  if (next_cap > 1048576) next_cap = 1048576;\n";
      o_ << "  if (next_cap < need) return 0;\n";
      o_ << "  char *next = (char *)realloc(ls_http_recv_heap, next_cap);\n";
      o_ << "  if (!next) return 0;\n";
      o_ << "  ls_http_recv_heap = next;\n";
      o_ << "  ls_http_recv_cap = next_cap;\n";
      o_ << "  return 1;\n";
      o_ << "}\n";
      o_ << "static inline const char *ls_http_recv_to_close(ls_http_socket sock) {\n";
      o_ << "  if (!ls_http_recv_reserve(4096)) return \"\";\n";
      o_ << "  size_t len = 0;\n";
      o_ << "  while (1) {\n";
      o_ << "    if (len + 1024 + 1 > ls_http_recv_cap) {\n";
      o_ << "      const size_t need = len + 1024 + 1;\n";
      o_ << "      if (need > 1048576) break;\n";
      o_ << "      if (!ls_http_recv_reserve(need)) break;\n";
      o_ << "    }\n";
      o_ << "    const int chunk = (int)(ls_http_recv_cap - len - 1);\n";
      o_ << "    if (chunk <= 0) break;\n";
      o_ << "    const int n = (int)recv(sock, ls_http_recv_heap + len, chunk, 0);\n";
      o_ << "    if (n <= 0) break;\n";
      o_ << "    len += (size_t)n;\n";
      o_ << "  }\n";
      o_ << "  ls_http_recv_heap[len] = '\\0';\n";
      o_ << "  return ls_http_recv_heap;\n";
      o_ << "}\n";
      o_ << "static inline const char *ls_http_reason_text(int64_t status) {\n";
      o_ << "  switch (status) {\n";
      o_ << "  case 200: return \"OK\";\n";
      o_ << "  case 201: return \"Created\";\n";
      o_ << "  case 204: return \"No Content\";\n";
      o_ << "  case 400: return \"Bad Request\";\n";
      o_ << "  case 401: return \"Unauthorized\";\n";
      o_ << "  case 403: return \"Forbidden\";\n";
      o_ << "  case 404: return \"Not Found\";\n";
      o_ << "  case 409: return \"Conflict\";\n";
      o_ << "  case 500: return \"Internal Server Error\";\n";
      o_ << "  default: return \"OK\";\n";
      o_ << "  }\n";
      o_ << "}\n";
      o_ << "static inline int64_t http_server_listen(int64_t port) {\n";
      o_ << "  if (port < 1 || port > 65535) return -1;\n";
      o_ << "  if (!ls_http_init()) return -1;\n";
      o_ << "  ls_http_socket s = socket(AF_INET, SOCK_STREAM, 0);\n";
      o_ << "  if (ls_http_is_bad_socket(s)) return -1;\n";
      o_ << "  int reuse = 1;\n";
      o_ << "#if defined(_WIN32)\n";
      o_ << "  (void)setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, (int)sizeof(reuse));\n";
      o_ << "#else\n";
      o_ << "  (void)setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, (int)sizeof(reuse));\n";
      o_ << "#endif\n";
      o_ << "  struct sockaddr_in addr;\n";
      o_ << "  memset(&addr, 0, sizeof(addr));\n";
      o_ << "  addr.sin_family = AF_INET;\n";
      o_ << "  addr.sin_addr.s_addr = htonl(INADDR_ANY);\n";
      o_ << "  addr.sin_port = htons((uint16_t)port);\n";
      o_ << "  if (bind(s, (const struct sockaddr *)&addr, (int)sizeof(addr)) != 0) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  if (listen(s, 64) != 0) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  const int64_t id = ls_http_alloc_server(s);\n";
      o_ << "  if (id < 0) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  return id;\n";
      o_ << "}\n";
      o_ << "static inline int64_t http_server_accept(int64_t server_id) {\n";
      o_ << "  ls_http_server_slot *srv = ls_http_get_server(server_id);\n";
      o_ << "  if (!srv) return -1;\n";
      o_ << "  ls_http_socket c = accept(srv->sock, NULL, NULL);\n";
      o_ << "  if (ls_http_is_bad_socket(c)) return -1;\n";
      o_ << "  ls_http_set_nodelay(c);\n";
      o_ << "  const int64_t id = ls_http_alloc_client(c);\n";
      o_ << "  if (id < 0) {\n";
      o_ << "    ls_http_close_socket(c);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  return id;\n";
      o_ << "}\n";
      o_ << "static inline const char *http_server_read(int64_t client_id) {\n";
      o_ << "  ls_http_client_slot *c = ls_http_get_client(client_id);\n";
      o_ << "  if (!c) return \"\";\n";
      o_ << "  return ls_http_recv_once(c->sock);\n";
      o_ << "}\n";
      o_ << "static inline void http_server_respond_text(int64_t client_id, int64_t status, const char *body) {\n";
      o_ << "  ls_http_client_slot *c = ls_http_get_client(client_id);\n";
      o_ << "  if (!c) return;\n";
      o_ << "  const char *msg = body ? body : \"\";\n";
      o_ << "  const char *reason = ls_http_reason_text(status);\n";
      o_ << "  const size_t body_len = strlen(msg);\n";
      o_ << "  char head[256];\n";
      o_ << "  const int head_len = (int)snprintf(head, sizeof(head),\n";
      o_ << "      \"HTTP/1.1 %lld %s\\r\\n\"\n";
      o_ << "      \"Content-Type: text/plain; charset=utf-8\\r\\n\"\n";
      o_ << "      \"Content-Length: %llu\\r\\n\"\n";
      o_ << "      \"Connection: close\\r\\n\\r\\n\",\n";
      o_ << "      (long long)status, reason, (unsigned long long)body_len);\n";
      o_ << "  if (head_len <= 0) return;\n";
      o_ << "  if (!ls_http_send_all(c->sock, head, (size_t)head_len)) return;\n";
      o_ << "  (void)ls_http_send_all(c->sock, msg, body_len);\n";
      o_ << "}\n";
      o_ << "static inline void http_server_close(int64_t server_id) {\n";
      o_ << "  ls_http_server_slot *srv = ls_http_get_server(server_id);\n";
      o_ << "  if (!srv) return;\n";
      o_ << "  ls_http_close_socket(srv->sock);\n";
      o_ << "  srv->sock = ls_http_bad_socket();\n";
      o_ << "  srv->active = 0;\n";
      o_ << "  if (server_id >= 0 && server_id < LS_HTTP_MAX_SERVERS && ls_http_server_free_top < LS_HTTP_MAX_SERVERS) {\n";
      o_ << "    ls_http_server_free_ids[ls_http_server_free_top++] = server_id;\n";
      o_ << "  }\n";
      o_ << "}\n";
      o_ << "static inline int64_t http_client_connect(const char *host, int64_t port) {\n";
      o_ << "  if (port < 1 || port > 65535) return -1;\n";
      o_ << "  if (!ls_http_init()) return -1;\n";
      o_ << "  const char *addr_text = host ? host : \"\";\n";
      o_ << "  if (addr_text[0] == '\\0' || strcmp(addr_text, \"localhost\") == 0) addr_text = \"127.0.0.1\";\n";
      o_ << "  ls_http_socket s = socket(AF_INET, SOCK_STREAM, 0);\n";
      o_ << "  if (ls_http_is_bad_socket(s)) return -1;\n";
      o_ << "  struct sockaddr_in addr;\n";
      o_ << "  memset(&addr, 0, sizeof(addr));\n";
      o_ << "  addr.sin_family = AF_INET;\n";
      o_ << "  addr.sin_port = htons((uint16_t)port);\n";
      o_ << "  if (inet_pton(AF_INET, addr_text, &addr.sin_addr) != 1) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  if (connect(s, (const struct sockaddr *)&addr, (int)sizeof(addr)) != 0) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  ls_http_set_nodelay(s);\n";
      o_ << "  const int64_t id = ls_http_alloc_client(s);\n";
      o_ << "  if (id < 0) {\n";
      o_ << "    ls_http_close_socket(s);\n";
      o_ << "    return -1;\n";
      o_ << "  }\n";
      o_ << "  return id;\n";
      o_ << "}\n";
      o_ << "static inline void http_client_send(int64_t client_id, const char *data) {\n";
      o_ << "  ls_http_client_slot *c = ls_http_get_client(client_id);\n";
      o_ << "  if (!c) return;\n";
      o_ << "  const char *msg = data ? data : \"\";\n";
      o_ << "  (void)ls_http_send_all(c->sock, msg, strlen(msg));\n";
      o_ << "}\n";
      o_ << "static inline const char *http_client_read(int64_t client_id) {\n";
      o_ << "  ls_http_client_slot *c = ls_http_get_client(client_id);\n";
      o_ << "  if (!c) return \"\";\n";
      o_ << "  return ls_http_recv_to_close(c->sock);\n";
      o_ << "}\n";
      o_ << "static inline void http_client_close(int64_t client_id) {\n";
      o_ << "  ls_http_client_slot *c = ls_http_get_client(client_id);\n";
      o_ << "  if (!c) return;\n";
      o_ << "  ls_http_close_socket(c->sock);\n";
      o_ << "  c->sock = ls_http_bad_socket();\n";
      o_ << "  c->active = 0;\n";
      o_ << "  if (client_id >= 0 && client_id < LS_HTTP_MAX_CLIENTS && ls_http_client_free_top < LS_HTTP_MAX_CLIENTS) {\n";
      o_ << "    ls_http_client_free_ids[ls_http_client_free_top++] = client_id;\n";
      o_ << "  }\n";
      o_ << "}\n";
    }
    o_ << "static inline int32_t to_i32(int64_t v) { return (int32_t)v; }\n";
    o_ << "static inline float to_f32(double v) { return (float)v; }\n";
    o_ << "static inline int64_t to_i64(double v) { return (int64_t)v; }\n";
    o_ << "static inline double to_f64(int64_t v) { return (double)v; }\n";
    o_ << "static inline int64_t bool_to_i64(ls_bool v) { return v ? 1LL : 0LL; }\n";
    o_ << "static inline ls_bool i64_to_bool(int64_t v) { return v != 0 ? (ls_bool)1 : (ls_bool)0; }\n";
    o_ << "static inline uint64_t ls_abs_u64_i64(int64_t x) {\n";
    o_ << "  if (x >= 0) return (uint64_t)x;\n";
    o_ << "  return (uint64_t)(-(x + 1LL)) + 1ULL;\n";
    o_ << "}\n";
    o_ << "static inline int ls_ctz_u64(uint64_t x) {\n";
    o_ << "#if defined(__clang__) || defined(__GNUC__)\n";
    o_ << "  return (int)__builtin_ctzll(x);\n";
    o_ << "#else\n";
    o_ << "  int n = 0;\n";
    o_ << "  while ((x & 1ULL) == 0ULL) {\n";
    o_ << "    x >>= 1ULL;\n";
    o_ << "    ++n;\n";
    o_ << "  }\n";
    o_ << "  return n;\n";
    o_ << "#endif\n";
    o_ << "}\n";
    o_ << "static inline int64_t gcd(int64_t a, int64_t b) {\n";
    o_ << "  uint64_t ua = ls_abs_u64_i64(a);\n";
    o_ << "  uint64_t ub = ls_abs_u64_i64(b);\n";
    o_ << "  if (ua == 0ULL) return (int64_t)ub;\n";
    o_ << "  if (ub == 0ULL) return (int64_t)ua;\n";
    o_ << "  const int shift = ls_ctz_u64(ua | ub);\n";
    o_ << "  ua >>= ls_ctz_u64(ua);\n";
    o_ << "  do {\n";
    o_ << "    ub >>= ls_ctz_u64(ub);\n";
    o_ << "    if (ua > ub) {\n";
    o_ << "      const uint64_t t = ua;\n";
    o_ << "      ua = ub;\n";
    o_ << "      ub = t;\n";
    o_ << "    }\n";
    o_ << "    ub -= ua;\n";
    o_ << "  } while (ub != 0ULL);\n";
    o_ << "  const uint64_t g = ua << shift;\n";
    o_ << "  if (g > 9223372036854775807ULL) {\n";
    o_ << "    return 9223372036854775807LL;\n";
    o_ << "  }\n";
    o_ << "  return (int64_t)g;\n";
    o_ << "}\n";
    o_ << "static inline int64_t lcm(int64_t a, int64_t b) {\n";
    o_ << "  if (a == 0 || b == 0) return 0;\n";
    o_ << "  const int64_t g = gcd(a, b);\n";
    o_ << "  if (g == 0) return 0;\n";
    o_ << "  LS_I128 q = (LS_I128)a / (LS_I128)g;\n";
    o_ << "  LS_I128 m = q * (LS_I128)b;\n";
    o_ << "  if (m < 0) m = -m;\n";
    o_ << "  if (m > (LS_I128)9223372036854775807LL) {\n";
    o_ << "    return 9223372036854775807LL;\n";
    o_ << "  }\n";
    o_ << "  return (int64_t)m;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_str_eq(const char *a, const char *b) {\n";
    o_ << "  if (a == b) return 1;\n";
    o_ << "  if (!a || !b) return 0;\n";
    o_ << "  return strcmp(a, b) == 0;\n";
    o_ << "}\n";
    o_ << "static inline ls_bool ls_str_neq(const char *a, const char *b) { return !ls_str_eq(a, b); }\n";
    o_ << "static inline int64_t max_i64(int64_t a, int64_t b) { return a > b ? a : b; }\n";
    o_ << "static inline int64_t min_i64(int64_t a, int64_t b) { return a < b ? a : b; }\n";
    o_ << "static inline int64_t abs_i64(int64_t a) { return a < 0 ? -a : a; }\n\n";
    o_ << "static inline int64_t clamp_i64(int64_t x, int64_t lo, int64_t hi) { return x < lo ? lo : (x > hi ? hi : x); "
          "}\n";
    o_ << "static inline double max_f64(double a, double b) { return fmax(a, b); }\n";
    o_ << "static inline double min_f64(double a, double b) { return fmin(a, b); }\n";
    o_ << "static inline double abs_f64(double a) { return fabs(a); }\n";
    o_ << "static inline double clamp_f64(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); "
          "}\n";
    o_ << "static inline double pi(void) { return 3.14159265358979323846264338327950288; }\n";
    o_ << "static inline double tau(void) { return 6.28318530717958647692528676655900576; }\n";
    o_ << "static inline double deg_to_rad(double d) { return d * (pi() / 180.0); }\n";
    o_ << "static inline double rad_to_deg(double r) { return r * (180.0 / pi()); }\n\n";
    o_ << "static inline void ls_format_begin(void) {\n";
    o_ << "  ls_fmt.active = 1;\n";
    o_ << "  ls_fmt.len = 0;\n";
    o_ << "  if (ls_fmt.buf) ls_fmt.buf[0] = '\\0';\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_format_end(const char *end_suffix) {\n";
    o_ << "  const char *suffix = end_suffix ? end_suffix : \"\";\n";
    o_ << "  const size_t suffixLen = strlen(suffix);\n";
    o_ << "  if (suffixLen > 0) {\n";
    o_ << "    const size_t need = ls_fmt.len + suffixLen + 1;\n";
    o_ << "    if (need > ls_fmt.cap) {\n";
    o_ << "      size_t nextCap = ls_fmt.cap ? ls_fmt.cap : 64;\n";
    o_ << "      while (nextCap < need) nextCap <<= 1;\n";
    o_ << "      char *next = (char *)realloc(ls_fmt.buf, nextCap);\n";
    o_ << "      if (next) {\n";
    o_ << "        ls_fmt.buf = next;\n";
    o_ << "        ls_fmt.cap = nextCap;\n";
    o_ << "      }\n";
    o_ << "    }\n";
    o_ << "    if (ls_fmt.buf && ls_fmt.cap >= need) {\n";
    o_ << "      memcpy(ls_fmt.buf + ls_fmt.len, suffix, suffixLen);\n";
    o_ << "      ls_fmt.len += suffixLen;\n";
    o_ << "      ls_fmt.buf[ls_fmt.len] = '\\0';\n";
    o_ << "    }\n";
    o_ << "  }\n";
    o_ << "  if (!ls_fmt.buf) {\n";
    o_ << "    static const char *kEmpty = \"\";\n";
    o_ << "    ls_fmt.active = 0;\n";
    o_ << "    return kEmpty;\n";
    o_ << "  }\n";
    o_ << "  ls_fmt.active = 0;\n";
    o_ << "  return ls_fmt.buf;\n";
    o_ << "}\n";
    o_ << "static inline const char *formatOutput_i64(int64_t v) {\n";
    o_ << "  static LS_THREAD_LOCAL char b[32];\n";
    o_ << "  ls_i64_to_cstr(v, b);\n";
    o_ << "  return b;\n";
    o_ << "}\n";
    o_ << "static inline const char *formatOutput_f64(double v) {\n";
    o_ << "  static LS_THREAD_LOCAL char b[64];\n";
    o_ << "  (void)snprintf(b, sizeof(b), \"%.17g\", v);\n";
    o_ << "  return b;\n";
    o_ << "}\n";
    o_ << "static inline const char *formatOutput_bool(ls_bool v) { return v ? \"true\" : \"false\"; }\n";
    o_ << "static inline const char *formatOutput_str(const char *v) { return v ? v : \"\"; }\n";
    o_ << "static inline void ls_su_emit_debug(const char *msg) {\n";
    o_ << "  if (!msg) return;\n";
    o_ << "  FILE *out = ls_su_debug_to_stderr ? stderr : stdout;\n";
    o_ << "  fputs(msg, out);\n";
    o_ << "  fflush(out);\n";
#if defined(_WIN32)
    o_ << "  OutputDebugStringA(msg);\n";
#endif
    o_ << "}\n";
    o_ << "static inline void superuser(void) {\n";
    o_ << "  if (ls_su_enabled) {\n";
    o_ << "    ls_su_enabled = 0;\n";
    o_ << "    ls_su_trace = 0;\n";
    o_ << "    ls_su_emit_debug(\"[superuser] developer privileges disabled\\n\");\n";
    o_ << "    return;\n";
    o_ << "  }\n";
    o_ << "  ls_su_enabled = 1;\n";
    o_ << "  ls_su_emit_debug(\"[superuser] developer privileges enabled\\n\");\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_guard_step(void) {\n";
    o_ << "  if (!ls_su_enabled) return;\n";
    o_ << "  if (ls_su_step_limit <= 0) return;\n";
    o_ << "  ++ls_su_step_count;\n";
    o_ << "  if (ls_su_step_count > ls_su_step_limit) {\n";
    o_ << "    ls_su_emit_debug(\"[superuser] runtime step limit reached\\n\");\n";
    o_ << "    abort();\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_trace_stmt(const char *fn, int64_t line, const char *kind) {\n";
    o_ << "  if (!ls_su_enabled || !ls_su_trace) return;\n";
    o_ << "  char *buf = ls_scratch_take(192);\n";
    o_ << "  if (!buf) return;\n";
    o_ << "  (void)snprintf(buf, 192, \"[trace] %s:%lld %s\\n\", fn ? fn : \"<fn>\", (long long)line, kind ? kind : \"stmt\");\n";
    o_ << "  ls_su_emit_debug(buf);\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_trace_on(void) {\n";
    o_ << "  if (!ls_su_enabled) { ls_su_emit_debug(\"[superuser] Not privileged\\n\"); return; }\n";
    o_ << "  ls_su_trace = 1;\n";
    o_ << "  ls_su_emit_debug(\"[superuser] trace enabled\\n\");\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_trace_off(void) {\n";
    o_ << "  if (!ls_su_enabled) { ls_su_emit_debug(\"[superuser] Not privileged\\n\"); return; }\n";
    o_ << "  ls_su_trace = 0;\n";
    o_ << "  ls_su_emit_debug(\"[superuser] trace disabled\\n\");\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_limit_set(int64_t step_limit, int64_t mem_limit) {\n";
    o_ << "  if (!ls_su_enabled) { ls_su_emit_debug(\"[superuser] Not privileged\\n\"); return; }\n";
    o_ << "  ls_su_step_limit = step_limit < 0 ? 0 : step_limit;\n";
    o_ << "  ls_su_mem_limit = mem_limit < 0 ? 0 : mem_limit;\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_su_capabilities(void) {\n";
    o_ << "  char *buf = ls_scratch_take(256);\n";
    o_ << "  if (!buf) return \"\";\n";
    o_ << "  (void)snprintf(buf, 256, \"superuser=%s,trace=%s,step_limit=%lld,mem_limit=%lld\", ls_su_enabled ? \"on\" : \"off\", ls_su_trace ? \"on\" : \"off\", (long long)ls_su_step_limit, (long long)ls_su_mem_limit);\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_su_memory_inspect(void) {\n";
    o_ << "  char *buf = ls_scratch_take(256);\n";
    o_ << "  if (!buf) return \"\";\n";
    o_ << "  (void)snprintf(buf, 256, \"mem_in_use=%lld,mem_limit=%lld\", (long long)ls_su_mem_in_use, (long long)ls_su_mem_limit);\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline const char *ls_su_compiler_inspect(void) {\n";
    o_ << "  char *buf = ls_scratch_take(256);\n";
    o_ << "  if (!buf) return \"\";\n";
    o_ << "  (void)snprintf(buf, 256, \"backend=generated-c,superuser=%s,trace=%s\", ls_su_enabled ? \"on\" : \"off\", ls_su_trace ? \"on\" : \"off\");\n";
    o_ << "  return buf;\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_ir_dump(void) {\n";
    o_ << "  if (!ls_su_enabled) { ls_su_emit_debug(\"[superuser] Not privileged\\n\"); return; }\n";
    o_ << "  ls_su_emit_debug(\"[superuser] IR/C dump requested for this build\\n\");\n";
    o_ << "}\n";
    o_ << "static inline void ls_su_debug_hook(const char *tag) {\n";
    o_ << "  if (!ls_su_enabled) { ls_su_emit_debug(\"[superuser] Not privileged\\n\"); return; }\n";
    o_ << "  char *buf = ls_scratch_take(192);\n";
    o_ << "  if (!buf) return;\n";
    o_ << "  (void)snprintf(buf, 192, \"[superuser-hook] %s\\n\", tag ? tag : \"<null>\");\n";
    o_ << "  ls_su_emit_debug(buf);\n";
    o_ << "}\n";
    o_ << "typedef void (*ls_task_fn)(void);\n";
    o_ << "#define LS_MAX_TASKS 1024\n";
    o_ << "typedef struct {\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  HANDLE handle;\n";
    o_ << "#else\n";
    o_ << "  pthread_t handle;\n";
    o_ << "#endif\n";
    o_ << "  ls_bool active;\n";
    o_ << "} ls_task_slot;\n";
    o_ << "typedef struct { ls_task_fn fn; } ls_task_payload;\n";
    o_ << "static ls_task_slot ls_tasks[LS_MAX_TASKS];\n";
    o_ << "static int64_t ls_task_count = 0;\n";
    o_ << "static int64_t ls_task_free_ids[LS_MAX_TASKS];\n";
    o_ << "static int64_t ls_task_free_top = 0;\n";
    o_ << "static int64_t ls_task_live_ids[LS_MAX_TASKS];\n";
    o_ << "static int64_t ls_task_live_pos[LS_MAX_TASKS];\n";
    o_ << "static int64_t ls_task_live_count = 0;\n";
    o_ << "static inline int64_t ls_task_alloc_id(void) {\n";
    o_ << "  if (ls_task_free_top > 0) return ls_task_free_ids[--ls_task_free_top];\n";
    o_ << "  if (ls_task_count >= LS_MAX_TASKS) return -1;\n";
    o_ << "  return ls_task_count++;\n";
    o_ << "}\n";
    o_ << "static inline void ls_task_release_id(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_TASKS) return;\n";
    o_ << "  if (ls_task_free_top < LS_MAX_TASKS) ls_task_free_ids[ls_task_free_top++] = id;\n";
    o_ << "}\n";
    o_ << "static inline void ls_task_mark_live(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_TASKS) return;\n";
    o_ << "  if (ls_task_live_count >= LS_MAX_TASKS) return;\n";
    o_ << "  ls_task_live_pos[id] = ls_task_live_count;\n";
    o_ << "  ls_task_live_ids[ls_task_live_count++] = id;\n";
    o_ << "}\n";
    o_ << "static inline void ls_task_mark_dead(int64_t id) {\n";
    o_ << "  if (id < 0 || id >= LS_MAX_TASKS) return;\n";
    o_ << "  const int64_t pos = ls_task_live_pos[id];\n";
    o_ << "  if (pos < 0 || pos >= ls_task_live_count) return;\n";
    o_ << "  const int64_t last = ls_task_live_ids[ls_task_live_count - 1];\n";
    o_ << "  ls_task_live_ids[pos] = last;\n";
    o_ << "  ls_task_live_pos[last] = pos;\n";
    o_ << "  ls_task_live_pos[id] = -1;\n";
    o_ << "  --ls_task_live_count;\n";
    o_ << "}\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "static DWORD WINAPI ls_task_entry(LPVOID arg) {\n";
    o_ << "  ls_task_payload *payload = (ls_task_payload *)arg;\n";
    o_ << "  ls_task_fn fn = payload ? payload->fn : NULL;\n";
    o_ << "  if (payload) free(payload);\n";
    o_ << "  if (fn) fn();\n";
    o_ << "  return 0;\n";
    o_ << "}\n";
    o_ << "#else\n";
    o_ << "static void *ls_task_entry(void *arg) {\n";
    o_ << "  ls_task_payload *payload = (ls_task_payload *)arg;\n";
    o_ << "  ls_task_fn fn = payload ? payload->fn : NULL;\n";
    o_ << "  if (payload) free(payload);\n";
    o_ << "  if (fn) fn();\n";
    o_ << "  return NULL;\n";
    o_ << "}\n";
    o_ << "#endif\n";
    o_ << "static inline int64_t ls_spawn(ls_task_fn fn) {\n";
    o_ << "  if (!fn) return -1;\n";
    o_ << "  ls_task_payload *payload = (ls_task_payload *)malloc(sizeof(ls_task_payload));\n";
    o_ << "  if (!payload) return -1;\n";
    o_ << "  payload->fn = fn;\n";
    o_ << "  const int64_t id = ls_task_alloc_id();\n";
    o_ << "  if (id < 0) {\n";
    o_ << "    free(payload);\n";
    o_ << "    return -1;\n";
    o_ << "  }\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  HANDLE handle = CreateThread(NULL, 0, ls_task_entry, payload, 0, NULL);\n";
    o_ << "  if (handle == NULL) {\n";
    o_ << "    free(payload);\n";
    o_ << "    ls_task_release_id(id);\n";
    o_ << "    return -1;\n";
    o_ << "  }\n";
    o_ << "  ls_tasks[id].handle = handle;\n";
    o_ << "#else\n";
    o_ << "  if (pthread_create(&ls_tasks[id].handle, NULL, ls_task_entry, payload) != 0) {\n";
    o_ << "    free(payload);\n";
    o_ << "    ls_task_release_id(id);\n";
    o_ << "    return -1;\n";
    o_ << "  }\n";
    o_ << "#endif\n";
    o_ << "  ls_tasks[id].active = 1;\n";
    o_ << "  ls_task_mark_live(id);\n";
    o_ << "  return id;\n";
    o_ << "}\n";
    o_ << "static inline void await(int64_t task_id) {\n";
    o_ << "  if (task_id < 0 || task_id >= LS_MAX_TASKS) return;\n";
    o_ << "  if (!ls_tasks[task_id].active) return;\n";
    o_ << "#if defined(_WIN32)\n";
    o_ << "  WaitForSingleObject(ls_tasks[task_id].handle, INFINITE);\n";
    o_ << "  CloseHandle(ls_tasks[task_id].handle);\n";
    o_ << "#else\n";
    o_ << "  (void)pthread_join(ls_tasks[task_id].handle, NULL);\n";
    o_ << "#endif\n";
    o_ << "  ls_task_mark_dead(task_id);\n";
    o_ << "  ls_tasks[task_id].active = 0;\n";
    o_ << "  ls_task_release_id(task_id);\n";
    o_ << "}\n";
    o_ << "static inline void await_all(void) {\n";
    o_ << "  while (ls_task_live_count > 0) {\n";
    o_ << "    await(ls_task_live_ids[ls_task_live_count - 1]);\n";
    o_ << "  }\n";
    o_ << "}\n";
    o_ << "static inline int64_t ls_pow_i64(int64_t base, int64_t exp) {\n";
    o_ << "  if (exp < 0) return 0;\n";
    o_ << "  int64_t out = 1;\n";
    o_ << "  while (exp > 0) {\n";
    o_ << "    if (exp & 1LL) out *= base;\n";
    o_ << "    exp >>= 1LL;\n";
    o_ << "    if (exp > 0) base *= base;\n";
    o_ << "  }\n";
    o_ << "  return out;\n";
    o_ << "}\n";
    o_ << "static inline double ls_pow_f64(double a, double b) { return pow(a, b); }\n";
    o_ << "#define ls_pow(a,b) _Generic(((a)+(b)), double: ls_pow_f64, float: ls_pow_f64, default: ls_pow_i64)"
          "((a),(b))\n";
    o_ << "#define __ls_print_dispatch(x) _Generic((x), int64_t: print_i64, int: print_i64, float: print_f64, "
          "double: print_f64, ls_bool: print_bool, const char *: print_str, char *: print_str, default: "
          "print_i64)(x)\n";
    o_ << "#define __ls_println_dispatch(x) _Generic((x), int64_t: println_i64, int: println_i64, float: "
          "println_f64, double: println_f64, ls_bool: println_bool, const char *: println_str, char *: "
          "println_str, default: println_i64)(x)\n";
    o_ << "#define __ls_format_dispatch(x) _Generic((x), int64_t: formatOutput_i64, int: formatOutput_i64, "
          "float: formatOutput_f64, double: formatOutput_f64, ls_bool: formatOutput_bool, const char *: "
          "formatOutput_str, char *: formatOutput_str, default: formatOutput_i64)(x)\n";
    o_ << "#define ls_generic_max(a,b) _Generic(((a)+(b)), double: max_f64, float: max_f64, default: max_i64)"
          "((a),(b))\n";
    o_ << "#define ls_generic_min(a,b) _Generic(((a)+(b)), double: min_f64, float: min_f64, default: min_i64)"
          "((a),(b))\n";
    o_ << "#define ls_generic_abs(x) _Generic((x), double: abs_f64, float: abs_f64, default: abs_i64)(x)\n";
    o_ << "#define ls_generic_clamp(x,lo,hi) _Generic(((x)+(lo)+(hi)), double: clamp_f64, float: clamp_f64, "
          "default: clamp_i64)((x),(lo),(hi))\n";
    o_ << "#define print(x) __ls_print_dispatch(x)\n";
    o_ << "#define println(x) __ls_println_dispatch(x)\n\n";
    o_ << "#define formatOutput(x) __ls_format_dispatch(x)\n";
    o_ << "#define FormatOutput(x) __ls_format_dispatch(x)\n\n";
  }

  std::string e(const Expr &x) const {
    switch (x.k) {
    case EK::Int: return std::to_string(static_cast<const EInt &>(x).v) + "LL";
    case EK::Float: return dLit(static_cast<const EFloat &>(x).v);
    case EK::Bool: return static_cast<const EBool &>(x).v ? "((ls_bool)1)" : "((ls_bool)0)";
    case EK::Str: return cStrLit(static_cast<const EString &>(x).v);
    case EK::Var: return static_cast<const EVar &>(x).n;
    case EK::Unary: {
      auto &n = static_cast<const EUnary &>(x);
      if (!n.overrideFn.empty()) {
        return cFnName(n.overrideFn) + "(" + e(*n.x) + ")";
      }
      return "(" + uop(n.op) + e(*n.x) + ")";
    }
    case EK::Binary: {
      auto &n = static_cast<const EBinary &>(x);
      if (!n.overrideFn.empty()) {
        return cFnName(n.overrideFn) + "(" + e(*n.l) + ", " + e(*n.r) + ")";
      }
      if (n.op == BK::Pow) return "ls_pow(" + e(*n.l) + ", " + e(*n.r) + ")";
      if ((n.op == BK::Eq || n.op == BK::Neq) && n.l->typed && n.r->typed && n.l->inf == Type::Str &&
          n.r->inf == Type::Str) {
        if (n.op == BK::Eq) return "ls_str_eq(" + e(*n.l) + ", " + e(*n.r) + ")";
        return "ls_str_neq(" + e(*n.l) + ", " + e(*n.r) + ")";
      }
      return "(" + e(*n.l) + " " + bop(n.op) + " " + e(*n.r) + ")";
    }
    case EK::Call: {
      auto &n = static_cast<const ECall &>(x);
      const std::string fnName = canonicalSuperuserCallName(n.f);
      if (fnName == ".format") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal format emit arity error");
        }
        return "ls_mark_format_mode()";
      }
      if (fnName == "input") {
        if (n.a.empty()) return "input()";
        if (n.a.size() == 1) return "input_prompt(" + e(*n.a[0]) + ")";
        throw CompileError(x.s, "internal input emit arity error");
      }
      if (fnName == "input_i64") {
        if (n.a.empty()) return "input_i64()";
        if (n.a.size() == 1) return "input_i64_prompt(" + e(*n.a[0]) + ")";
        throw CompileError(x.s, "internal input_i64 emit arity error");
      }
      if (fnName == "input_f64") {
        if (n.a.empty()) return "input_f64()";
        if (n.a.size() == 1) return "input_f64_prompt(" + e(*n.a[0]) + ")";
        throw CompileError(x.s, "internal input_f64 emit arity error");
      }
      if (fnName == ".stateSpeed" || fnName == "stateSpeed") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal stateSpeed emit arity error");
        }
        if (stateSpeedVar_.empty()) {
          throw CompileError(x.s, "internal stateSpeed emit context error");
        }
        return "ls_state_speed(" + stateSpeedVar_ + ")";
      }
      if (fnName == ".freeConsole" || fnName == "FreeConsole") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal freeConsole emit arity error");
        }
        return "ls_detach_console()";
      }
      if (fnName == "max") {
        if (n.a.size() != 2) {
          throw CompileError(x.s, "internal max emit arity error");
        }
        return "ls_generic_max(" + e(*n.a[0]) + ", " + e(*n.a[1]) + ")";
      }
      if (fnName == "min") {
        if (n.a.size() != 2) {
          throw CompileError(x.s, "internal min emit arity error");
        }
        return "ls_generic_min(" + e(*n.a[0]) + ", " + e(*n.a[1]) + ")";
      }
      if (fnName == "abs") {
        if (n.a.size() != 1) {
          throw CompileError(x.s, "internal abs emit arity error");
        }
        return "ls_generic_abs(" + e(*n.a[0]) + ")";
      }
      if (fnName == "clamp") {
        if (n.a.size() != 3) {
          throw CompileError(x.s, "internal clamp emit arity error");
        }
        return "ls_generic_clamp(" + e(*n.a[0]) + ", " + e(*n.a[1]) + ", " + e(*n.a[2]) + ")";
      }
      if (fnName == "superuser") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal superuser emit arity error");
        }
        return "superuser()";
      }
      if (fnName == "su.trace.on") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.trace.on emit arity error");
        }
        return "ls_su_trace_on()";
      }
      if (fnName == "su.trace.off") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.trace.off emit arity error");
        }
        return "ls_su_trace_off()";
      }
      if (fnName == "su.capabilities") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.capabilities emit arity error");
        }
        return "ls_su_capabilities()";
      }
      if (fnName == "su.memory.inspect") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.memory.inspect emit arity error");
        }
        return "ls_su_memory_inspect()";
      }
      if (fnName == "su.limit.set") {
        if (n.a.size() != 2) {
          throw CompileError(x.s, "internal su.limit.set emit arity error");
        }
        return "ls_su_limit_set(" + e(*n.a[0]) + ", " + e(*n.a[1]) + ")";
      }
      if (fnName == "su.compiler.inspect") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.compiler.inspect emit arity error");
        }
        return "ls_su_compiler_inspect()";
      }
      if (fnName == "su.ir.dump") {
        if (!n.a.empty()) {
          throw CompileError(x.s, "internal su.ir.dump emit arity error");
        }
        return "ls_su_ir_dump()";
      }
      if (fnName == "su.debug.hook") {
        if (n.a.size() != 1) {
          throw CompileError(x.s, "internal su.debug.hook emit arity error");
        }
        return "ls_su_debug_hook(" + e(*n.a[0]) + ")";
      }
      if (fnName == "spawn") {
        if (n.a.size() != 1 || n.a[0]->k != EK::Call) {
          throw CompileError(x.s, "internal spawn emit error");
        }
        auto &target = static_cast<const ECall &>(*n.a[0]);
        return "ls_spawn(" + cFnName(canonicalSuperuserCallName(target.f)) + ")";
      }
      if (ultraMinimalRuntime_ && (fnName == "print_str" || fnName == "println_str") && n.a.size() == 1 &&
          n.a[0]->k == EK::Str) {
        std::string lit = static_cast<const EString &>(*n.a[0]).v;
        if (fnName == "println_str") lit.push_back('\n');
        return "ls_emit_bytes(" + cStrLit(lit) + ", " + std::to_string(static_cast<unsigned long long>(lit.size())) +
               "u)";
      }
      std::ostringstream a;
      for (std::size_t i = 0; i < n.a.size(); ++i) {
        if (i) a << ", ";
        const bool strArg = n.a[i]->typed && n.a[i]->inf == Type::Str;
        const bool strLiteral = n.a[i]->k == EK::Str;
        if (strArg && !strLiteral) {
          a << "ls_str_hold(" << e(*n.a[i]) << ")";
        } else {
          a << e(*n.a[i]);
        }
      }
      return cFnName(fnName) + "(" + a.str() + ")";
    }
    }
    throw CompileError(x.s, "internal emit expr error");
  }

  void sig(const Fn &f) {
    o_ << cType(f.ret) << " " << cFnName(f.n) << "(";
    for (std::size_t i = 0; i < f.p.size(); ++i) {
      if (i) o_ << ", ";
      o_ << cType(f.p[i].t) << " " << f.p[i].n;
    }
    o_ << ")";
  }

  void proto(const Fn &f) {
    if (f.ex) {
      o_ << "extern ";
      sig(f);
      o_ << ";\n";
      return;
    }
    const bool forceInlineEntry = ultraMinimalRuntime_ && entry_ && (&f == entry_);
    if (forceInlineEntry) {
      o_ << "static LS_ALWAYS_INLINE ";
    } else if (inl_.count(f.n) || f.inl) {
      o_ << "static inline ";
    }
    sig(f);
    o_ << ";\n";
  }

  void stmt(const Stmt &s, int k) {
    if (superuserMode_) {
      ind(o_, k);
      o_ << "ls_su_guard_step();\n";
      ind(o_, k);
      o_ << "ls_su_trace_stmt(" << cStrLit(activeFnName_) << ", " << static_cast<unsigned long long>(s.s.line) << "ULL, "
         << cStrLit(stmtKindName(s.k)) << ");\n";
    }
    switch (s.k) {
    case SK::Let: {
      auto &n = static_cast<const SLet &>(s);
      if (!n.typed) throw CompileError(s.s, "internal emit let typing error");
      ind(o_, k);
      if (n.inf == Type::Str) {
        o_ << "const char *";
        if (n.isConst) o_ << " const";
      } else {
        if (n.isConst) o_ << "const ";
        o_ << cType(n.inf);
      }
      o_ << " " << n.n << " = ";
      if (n.inf == Type::Str) {
        o_ << "ls_str_hold(" << e(*n.v) << ")";
      } else {
        o_ << e(*n.v);
      }
      o_ << ";\n";
      if (n.isOwned && !n.ownedFreeFn.empty()) {
        registerOwned(n.n, n.ownedFreeFn);
      }
      return;
    }
    case SK::Assign: {
      auto &n = static_cast<const SAssign &>(s);
      ind(o_, k);
      o_ << n.n << " = ";
      if (n.v->typed && n.v->inf == Type::Str) {
        o_ << "ls_str_hold(" << e(*n.v) << ")";
      } else {
        o_ << e(*n.v);
      }
      o_ << ";\n";
      return;
    }
    case SK::Expr: {
      ind(o_, k);
      o_ << e(*static_cast<const SExpr &>(s).e) << ";\n";
      return;
    }
    case SK::Ret: {
      auto &n = static_cast<const SRet &>(s);
      emitCleanupForReturn(k);
      ind(o_, k);
      if (!n.has || !n.v)
        o_ << "return;\n";
      else if (activeFnRet_ == Type::Str)
        o_ << "return ls_str_hold(" << e(*n.v) << ");\n";
      else
        o_ << "return " << e(*n.v) << ";\n";
      return;
    }
    case SK::If: {
      auto &n = static_cast<const SIf &>(s);
      ind(o_, k);
      o_ << "if (" << e(*n.c) << ") {\n";
      pushCleanupScope();
      for (const SP &x : n.t) stmt(*x, k + 1);
      emitCurrentScopeCleanup(k + 1);
      popCleanupScope();
      ind(o_, k);
      o_ << "}";
      if (!n.e.empty()) {
        o_ << " else {\n";
        pushCleanupScope();
        for (const SP &x : n.e) stmt(*x, k + 1);
        emitCurrentScopeCleanup(k + 1);
        popCleanupScope();
        ind(o_, k);
        o_ << "}\n";
      } else {
        o_ << '\n';
      }
      return;
    }
    case SK::While: {
      auto &n = static_cast<const SWhile &>(s);
      ind(o_, k);
      o_ << "while (" << e(*n.c) << ") {\n";
      pushCleanupScope(true);
      for (const SP &x : n.b) stmt(*x, k + 1);
      emitCurrentScopeCleanup(k + 1);
      popCleanupScope();
      ind(o_, k);
      o_ << "}\n";
      return;
    }
    case SK::For: {
      auto &n = static_cast<const SFor &>(s);
      const int loopId = loopSerial_++;
      const std::string startName = "__ls_start_" + std::to_string(loopId);
      const std::string stopName = "__ls_stop_" + std::to_string(loopId);
      const std::string stepName = "__ls_step_" + std::to_string(loopId);
      if (n.parallel) {
        const std::string parIterName = "__ls_par_iters_" + std::to_string(loopId);
        ind(o_, k);
        o_ << "{\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << startName << " = " << e(*n.start) << ";\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << stopName << " = " << e(*n.stop) << ";\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << stepName << " = " << e(*n.step) << ";\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << parIterName << " = ls_trip_count_runtime(" << startName << ", " << stopName << ", "
           << stepName << ");\n";
        ind(o_, k + 1);
        o_ << "if (" << stepName << " > 0) {\n";
        ind(o_, k + 2);
        o_ << "LS_PAR_FOR_IF(" << parIterName << " >= LS_PAR_MIN_ITERS)\n";
        ind(o_, k + 2);
        o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " < " << stopName << "; " << n.n
           << " += " << stepName << ") {\n";
        for (const SP &x : n.b) stmt(*x, k + 3);
        ind(o_, k + 2);
        o_ << "}\n";
        ind(o_, k + 1);
        o_ << "} else if (" << stepName << " < 0) {\n";
        ind(o_, k + 2);
        o_ << "LS_PAR_FOR_IF(" << parIterName << " >= LS_PAR_MIN_ITERS)\n";
        ind(o_, k + 2);
        o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " > " << stopName << "; " << n.n
           << " += " << stepName << ") {\n";
        for (const SP &x : n.b) stmt(*x, k + 3);
        ind(o_, k + 2);
        o_ << "}\n";
        ind(o_, k + 1);
        o_ << "}\n";
        ind(o_, k);
        o_ << "}\n";
      } else {
        const std::optional<PlusReductionInfo> reduction = plusReductionInfo(n);
        const std::optional<MultiPlusReductionInfo> multiReduction = multiPlusReductionInfo(n);
        const std::optional<ModLinearReductionInfo> modLinearReduction = modLinearReductionInfo(n);
        const std::optional<MultiAffineReductionInfo> multiAffineReduction = multiAffineReductionInfo(n);
        const std::optional<AffineReductionInfo> affineReduction = affineReductionInfo(n);
        const std::optional<AlternatingSignReductionInfo> altReduction = alternatingSignReductionInfo(n);
        const std::optional<PairCoupledInfo> pairCoupled = pairCoupledInfo(n);
        const bool vecHint = vectorHintEligible(n);
        const std::string redName = "__ls_red_" + std::to_string(loopId);
        const std::string iterName = "__ls_iters_" + std::to_string(loopId);
        const std::string n128Name = "__ls_n128_" + std::to_string(loopId);
        const std::string sumIdxName = "__ls_sumidx_" + std::to_string(loopId);
        const std::string deltaAccName = "__ls_delta_acc_" + std::to_string(loopId);
        const std::string altHalfName = "__ls_alt_half_" + std::to_string(loopId);
        const std::string altDeltaName = "__ls_alt_delta_" + std::to_string(loopId);
        const std::string pairK1Name = "__ls_pair_k1_" + std::to_string(loopId);
        const std::string pairK2Name = "__ls_pair_k2_" + std::to_string(loopId);
        const std::string pairGsumName = "__ls_pair_gsum_" + std::to_string(loopId);
        const std::string pairGweightedName = "__ls_pair_gw_" + std::to_string(loopId);
        const std::string modIterName = "__ls_mod_iters_" + std::to_string(loopId);
        const std::string modCurName = "__ls_mod_cur_" + std::to_string(loopId);
        const std::string modDeltaName = "__ls_mod_delta_" + std::to_string(loopId);
        const std::string modMName = "__ls_mod_m_" + std::to_string(loopId);
        const std::string modKName = "__ls_mod_k_" + std::to_string(loopId);
        const std::string modPeriodName = "__ls_mod_period_" + std::to_string(loopId);
        const std::string modFullName = "__ls_mod_full_" + std::to_string(loopId);
        const std::string modRemName = "__ls_mod_rem_" + std::to_string(loopId);
        const std::string modStartName = "__ls_mod_start_" + std::to_string(loopId);
        const std::string modCycleName = "__ls_mod_cycle_" + std::to_string(loopId);
        const std::string modTailName = "__ls_mod_tail_" + std::to_string(loopId);
        std::vector<std::string> multiRedNames;
        if (multiReduction.has_value()) {
          multiRedNames.reserve(multiReduction->terms.size());
          for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
            multiRedNames.push_back("__ls_red_" + std::to_string(loopId) + "_" + std::to_string(i));
          }
        }
        ind(o_, k);
        o_ << "{\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << startName << " = " << e(*n.start) << ";\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << stopName << " = " << e(*n.stop) << ";\n";
        ind(o_, k + 1);
        o_ << "const int64_t " << stepName << " = " << e(*n.step) << ";\n";
        if (altReduction.has_value()) {
          ind(o_, k + 2);
          o_ << "const int64_t " << iterName << " = ls_trip_count_runtime(" << startName << ", " << stopName << ", "
             << stepName << ");\n";
          ind(o_, k + 2);
          o_ << "if (" << iterName << " > 0) {\n";
          ind(o_, k + 3);
          o_ << "const int64_t " << altHalfName << " = " << iterName << " >> 1;\n";
          ind(o_, k + 3);
          if (altReduction->evenPositive) {
            o_ << "const int64_t " << altDeltaName << " = (" << iterName << " & 1LL) ? " << altHalfName << " : -"
               << altHalfName << ";\n";
          } else {
            o_ << "const int64_t " << altDeltaName << " = (" << iterName << " & 1LL) ? -" << altHalfName << " : "
               << altHalfName << ";\n";
          }
          ind(o_, k + 3);
          o_ << altReduction->var << " = " << altReduction->var << " + " << altDeltaName << ";\n";
          ind(o_, k + 2);
          o_ << "}\n";
        } else if (pairCoupled.has_value()) {
          ind(o_, k + 2);
          o_ << "const int64_t " << iterName << " = ls_trip_count_runtime(" << startName << ", " << stopName << ", "
             << stepName << ");\n";
          ind(o_, k + 2);
          o_ << "if (" << iterName << " > 0) {\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << n128Name << " = (LS_I128)" << iterName << ";\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << sumIdxName << " = " << n128Name << " * (((LS_I128)2 * (LS_I128)" << startName
             << ") + ((" << n128Name << " - 1) * (LS_I128)" << stepName << ")) / (LS_I128)2;\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << pairK1Name << " = " << n128Name << " * (" << n128Name << " - 1) / (LS_I128)2;\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << pairK2Name << " = " << n128Name << " * (" << n128Name << " - 1) * (" << n128Name
             << " - 2) / (LS_I128)6;\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << pairGsumName << " = ((LS_I128)(" << pairCoupled->a << ") * " << sumIdxName
             << ") + ((LS_I128)(" << pairCoupled->b << ") * " << n128Name << ");\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << pairGweightedName << " = (((LS_I128)(" << pairCoupled->a << ") * (LS_I128)"
             << startName << " + (LS_I128)(" << pairCoupled->b << ")) * " << pairK1Name << ") + (((LS_I128)("
             << pairCoupled->a << ") * (LS_I128)" << stepName << ") * " << pairK2Name << ");\n";
          ind(o_, k + 3);
          o_ << pairCoupled->accVar << " = (__typeof__(" << pairCoupled->accVar << "))((LS_I128)" << pairCoupled->accVar
             << " + (" << n128Name << " * (LS_I128)" << pairCoupled->stateVar << ") + " << pairGweightedName << ");\n";
          ind(o_, k + 3);
          o_ << pairCoupled->stateVar << " = (__typeof__(" << pairCoupled->stateVar << "))((LS_I128)"
             << pairCoupled->stateVar << " + " << pairGsumName << ");\n";
          ind(o_, k + 2);
          o_ << "}\n";
        } else if (multiAffineReduction.has_value()) {
          ind(o_, k + 2);
          o_ << "const int64_t " << iterName << " = ls_trip_count_runtime(" << startName << ", " << stopName << ", "
             << stepName << ");\n";
          ind(o_, k + 2);
          o_ << "if (" << iterName << " > 0) {\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << n128Name << " = (LS_I128)" << iterName << ";\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << sumIdxName << " = " << n128Name << " * (((LS_I128)2 * (LS_I128)" << startName
             << ") + ((" << n128Name << " - 1) * (LS_I128)" << stepName << ")) / (LS_I128)2;\n";
          for (std::size_t ti = 0; ti < multiAffineReduction->terms.size(); ++ti) {
            const auto &term = multiAffineReduction->terms[ti];
            const std::string termDeltaName = deltaAccName + "_" + std::to_string(ti);
            ind(o_, k + 3);
            o_ << "const LS_I128 " << termDeltaName << " = ((LS_I128)(" << term.a << ") * " << sumIdxName
               << ") + ((LS_I128)(" << term.b << ") * " << n128Name << ");\n";
            ind(o_, k + 3);
            o_ << term.var << " = (__typeof__(" << term.var << "))((LS_I128)" << term.var << " + " << termDeltaName
               << ");\n";
          }
          ind(o_, k + 2);
          o_ << "}\n";
        } else if (affineReduction.has_value()) {
          ind(o_, k + 2);
          o_ << "const int64_t " << iterName << " = ls_trip_count_runtime(" << startName << ", " << stopName << ", "
             << stepName << ");\n";
          ind(o_, k + 2);
          o_ << "if (" << iterName << " > 0) {\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << n128Name << " = (LS_I128)" << iterName << ";\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << sumIdxName << " = " << n128Name << " * (((LS_I128)2 * (LS_I128)" << startName
             << ") + ((" << n128Name << " - 1) * (LS_I128)" << stepName << ")) / (LS_I128)2;\n";
          ind(o_, k + 3);
          o_ << "const LS_I128 " << deltaAccName << " = ((LS_I128)(" << affineReduction->a << ") * " << sumIdxName
             << ") + ((LS_I128)(" << affineReduction->b << ") * " << n128Name << ");\n";
          ind(o_, k + 3);
          o_ << affineReduction->var << " = (__typeof__(" << affineReduction->var << "))((LS_I128)" << affineReduction->var
             << " + " << deltaAccName << ");\n";
          ind(o_, k + 2);
          o_ << "}\n";
        } else {
          ind(o_, k + 1);
          o_ << "if (" << stepName << " > 0) {\n";
          ind(o_, k + 2);
          if (multiReduction.has_value()) {
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              o_ << "__typeof__(" << multiReduction->terms[i].var << ") " << multiRedNames[i] << " = "
                 << multiReduction->terms[i].var << ";\n";
              ind(o_, k + 2);
            }
            switch (multiReduction->terms.size()) {
            case 2:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS2(" << multiRedNames[0] << ", " << multiRedNames[1] << ")\n";
              break;
            case 3:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS3(" << multiRedNames[0] << ", " << multiRedNames[1] << ", "
                 << multiRedNames[2] << ")\n";
              break;
            case 4:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS4(" << multiRedNames[0] << ", " << multiRedNames[1] << ", "
                 << multiRedNames[2] << ", " << multiRedNames[3] << ")\n";
              break;
            default:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS(" << multiRedNames[0] << ")\n";
              break;
            }
            ind(o_, k + 2);
            o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " < " << stopName << "; ";
            if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == 1) {
              o_ << "++" << n.n;
            } else {
              o_ << n.n << " += " << stepName;
            }
            o_ << ") {\n";
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              ind(o_, k + 3);
              o_ << multiRedNames[i] << " += " << e(*multiReduction->terms[i].other) << ";\n";
            }
            ind(o_, k + 2);
            o_ << "}\n";
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              ind(o_, k + 2);
              o_ << multiReduction->terms[i].var << " = " << multiRedNames[i] << ";\n";
            }
          } else if (reduction.has_value()) {
            if (modLinearReduction.has_value()) {
              o_ << "__typeof__(" << reduction->var << ") " << redName << " = " << reduction->var << ";\n";
              ind(o_, k + 2);
              o_ << "const int64_t " << modMName << " = " << modLinearReduction->m << "LL;\n";
              ind(o_, k + 2);
              o_ << "const int64_t " << modIterName << " = ls_trip_count_runtime(" << startName << ", " << stopName
                 << ", " << stepName << ");\n";
              ind(o_, k + 2);
              o_ << "if (" << modIterName << " > 0) {\n";
              ind(o_, k + 3);
              o_ << "int64_t " << modCurName << " = ls_mod_pos_i128(((LS_I128)" << modLinearReduction->a
                 << "LL * (LS_I128)" << startName << ") + (LS_I128)" << modLinearReduction->b << "LL, " << modMName
                 << ");\n";
              ind(o_, k + 3);
              o_ << "const int64_t " << modDeltaName << " = ls_mod_pos_i128((LS_I128)" << modLinearReduction->a
                 << "LL * (LS_I128)" << stepName << ", " << modMName << ");\n";
              ind(o_, k + 3);
              o_ << redName << " = (__typeof__(" << redName << "))((LS_I128)" << redName
                 << " + ls_sum_mod_linear_i128(" << modIterName << ", " << modMName << ", " << modDeltaName << ", "
                 << modCurName << "));\n";
              ind(o_, k + 2);
              o_ << "}\n";
              ind(o_, k + 2);
              o_ << reduction->var << " = " << redName << ";\n";
            } else {
              o_ << "__typeof__(" << reduction->var << ") " << redName << " = " << reduction->var << ";\n";
              ind(o_, k + 2);
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS(" << redName << ")\n";
              ind(o_, k + 2);
              o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " < " << stopName << "; ";
              if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == 1) {
                o_ << "++" << n.n;
              } else {
                o_ << n.n << " += " << stepName;
              }
              o_ << ") {\n";
              ind(o_, k + 3);
              o_ << redName << " += " << e(*reduction->other) << ";\n";
              ind(o_, k + 2);
              o_ << "}\n";
              ind(o_, k + 2);
              o_ << reduction->var << " = " << redName << ";\n";
            }
          } else {
            if (vecHint) {
              ind(o_, k + 2);
              o_ << "LS_OMP_SIMD\n";
              ind(o_, k + 2);
              o_ << "LS_VEC_HINT\n";
            }
            ind(o_, k + 2);
            o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " < " << stopName << "; ";
            if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == 1) {
              o_ << "++" << n.n;
            } else {
              o_ << n.n << " += " << stepName;
            }
            o_ << ") {\n";
            for (const SP &x : n.b) stmt(*x, k + 3);
            ind(o_, k + 2);
            o_ << "}\n";
          }
          ind(o_, k + 2);
          o_ << "} else if (" << stepName << " < 0) {\n";
          ind(o_, k + 2);
          if (multiReduction.has_value()) {
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              o_ << "__typeof__(" << multiReduction->terms[i].var << ") " << multiRedNames[i] << " = "
                 << multiReduction->terms[i].var << ";\n";
              ind(o_, k + 2);
            }
            switch (multiReduction->terms.size()) {
            case 2:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS2(" << multiRedNames[0] << ", " << multiRedNames[1] << ")\n";
              break;
            case 3:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS3(" << multiRedNames[0] << ", " << multiRedNames[1] << ", "
                 << multiRedNames[2] << ")\n";
              break;
            case 4:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS4(" << multiRedNames[0] << ", " << multiRedNames[1] << ", "
                 << multiRedNames[2] << ", " << multiRedNames[3] << ")\n";
              break;
            default:
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS(" << multiRedNames[0] << ")\n";
              break;
            }
            ind(o_, k + 2);
            o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " > " << stopName << "; ";
            if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == -1) {
              o_ << "--" << n.n;
            } else {
              o_ << n.n << " += " << stepName;
            }
            o_ << ") {\n";
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              ind(o_, k + 3);
              o_ << multiRedNames[i] << " += " << e(*multiReduction->terms[i].other) << ";\n";
            }
            ind(o_, k + 2);
            o_ << "}\n";
            for (std::size_t i = 0; i < multiReduction->terms.size(); ++i) {
              ind(o_, k + 2);
              o_ << multiReduction->terms[i].var << " = " << multiRedNames[i] << ";\n";
            }
          } else if (reduction.has_value()) {
            if (modLinearReduction.has_value()) {
              o_ << "__typeof__(" << reduction->var << ") " << redName << " = " << reduction->var << ";\n";
              ind(o_, k + 2);
              o_ << "const int64_t " << modMName << " = " << modLinearReduction->m << "LL;\n";
              ind(o_, k + 2);
              o_ << "const int64_t " << modIterName << " = ls_trip_count_runtime(" << startName << ", " << stopName
                 << ", " << stepName << ");\n";
              ind(o_, k + 2);
              o_ << "if (" << modIterName << " > 0) {\n";
              ind(o_, k + 3);
              o_ << "int64_t " << modCurName << " = ls_mod_pos_i128(((LS_I128)" << modLinearReduction->a
                 << "LL * (LS_I128)" << startName << ") + (LS_I128)" << modLinearReduction->b << "LL, " << modMName
                 << ");\n";
              ind(o_, k + 3);
              o_ << "const int64_t " << modDeltaName << " = ls_mod_pos_i128((LS_I128)" << modLinearReduction->a
                 << "LL * (LS_I128)" << stepName << ", " << modMName << ");\n";
              ind(o_, k + 3);
              o_ << redName << " = (__typeof__(" << redName << "))((LS_I128)" << redName
                 << " + ls_sum_mod_linear_i128(" << modIterName << ", " << modMName << ", " << modDeltaName << ", "
                 << modCurName << "));\n";
              ind(o_, k + 2);
              o_ << "}\n";
              ind(o_, k + 2);
              o_ << reduction->var << " = " << redName << ";\n";
            } else {
              o_ << "__typeof__(" << reduction->var << ") " << redName << " = " << reduction->var << ";\n";
              ind(o_, k + 2);
              o_ << "LS_OMP_SIMD_REDUCTION_PLUS(" << redName << ")\n";
              ind(o_, k + 2);
              o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " > " << stopName << "; ";
              if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == -1) {
                o_ << "--" << n.n;
              } else {
                o_ << n.n << " += " << stepName;
              }
              o_ << ") {\n";
              ind(o_, k + 3);
              o_ << redName << " += " << e(*reduction->other) << ";\n";
              ind(o_, k + 2);
              o_ << "}\n";
              ind(o_, k + 2);
              o_ << reduction->var << " = " << redName << ";\n";
            }
          } else {
            if (vecHint) {
              ind(o_, k + 2);
              o_ << "LS_OMP_SIMD\n";
              ind(o_, k + 2);
              o_ << "LS_VEC_HINT\n";
            }
            ind(o_, k + 2);
            o_ << "for (int64_t " << n.n << " = " << startName << "; " << n.n << " > " << stopName << "; ";
            if (n.step->k == EK::Int && static_cast<const EInt &>(*n.step).v == -1) {
              o_ << "--" << n.n;
            } else {
              o_ << n.n << " += " << stepName;
            }
            o_ << ") {\n";
            for (const SP &x : n.b) stmt(*x, k + 3);
            ind(o_, k + 2);
            o_ << "}\n";
          }
          ind(o_, k + 2);
          o_ << "}\n";
        }
        ind(o_, k);
        o_ << "}\n";
      }
      return;
    }
    case SK::FormatBlock: {
      auto &n = static_cast<const SFormatBlock &>(s);
      const int blockId = loopSerial_++;
      const std::string outName = "__ls_fmt_out_" + std::to_string(blockId);
      ind(o_, k);
      o_ << "{\n";
      ind(o_, k + 1);
      o_ << "ls_format_begin();\n";
      pushCleanupScope();
      for (const SP &x : n.b) stmt(*x, k + 1);
      emitCurrentScopeCleanup(k + 1);
      popCleanupScope();
      ind(o_, k + 1);
      o_ << "const char *" << outName << " = ls_format_end(";
      if (n.endArg)
        o_ << e(*n.endArg);
      else
        o_ << "\"\"";
      o_ << ");\n";
      ind(o_, k + 1);
      o_ << "ls_emit_text(" << outName << ");\n";
      ind(o_, k);
      o_ << "}\n";
      return;
    }
    case SK::Break:
      ind(o_, k);
      o_ << "break;\n";
      return;
    case SK::Continue:
      ind(o_, k);
      o_ << "continue;\n";
      return;
    }
  }

  void fn(const Fn &f) {
    const int fnId = loopSerial_++;
    const std::string prevStateSpeedVar = stateSpeedVar_;
    const std::string prevActiveFnName = activeFnName_;
    const Type prevActiveFnRet = activeFnRet_;
    const bool hasStateSpeed = hasCallNamedBlock(f.b, "stateSpeed") || hasCallNamedBlock(f.b, ".stateSpeed");
    stateSpeedVar_ = hasStateSpeed ? "__ls_fn_start_us_" + std::to_string(fnId) : "";
    activeFnName_ = f.n;
    activeFnRet_ = f.ret;
    const bool forceInlineEntry = ultraMinimalRuntime_ && entry_ && (&f == entry_);
    if (forceInlineEntry) {
      o_ << "static LS_ALWAYS_INLINE ";
    } else if (inl_.count(f.n) || f.inl) {
      o_ << "static inline ";
    }
    sig(f);
    o_ << " {\n";
    if (hasStateSpeed) {
      o_ << "  const int64_t " << stateSpeedVar_ << " = clock_us();\n";
    }
    pushCleanupScope();
    for (const SP &s : f.b) stmt(*s, 1);
    emitCurrentScopeCleanup(1);
    popCleanupScope();
    if (f.ret == Type::Void) o_ << "  return;\n";
    o_ << "}\n\n";
    stateSpeedVar_ = prevStateSpeedVar;
    activeFnName_ = prevActiveFnName;
    activeFnRet_ = prevActiveFnRet;
  }

  void emitEntryWrapper() {
    if (!entry_->p.empty()) throw CompileError(entry_->s, "entry function must have zero parameters");
    if (ultraMinimalRuntime_) {
      o_ << "void __stdcall mainCRTStartup(void) {\n";
      for (const std::string &flagFn : activeCliFlagCalls_) {
        o_ << "  " << flagFn << "();\n";
      }
      if (entry_->ret == Type::Void) {
        o_ << "  " << kEntryName << "();\n";
        o_ << "  ExitProcess(0u);\n";
      } else if (entry_->ret == Type::Str) {
        o_ << "  (void)" << kEntryName << "();\n";
        o_ << "  ExitProcess(0u);\n";
      } else {
        o_ << "  const int rc = (int)" << kEntryName << "();\n";
        o_ << "  ExitProcess((unsigned int)rc);\n";
      }
      o_ << "}\n";
      return;
    }
    o_ << "int main(void) {\n";
    for (const std::string &flagFn : activeCliFlagCalls_) {
      o_ << "  " << flagFn << "();\n";
    }
    if (entry_->ret == Type::Void) {
      o_ << "  " << kEntryName << "();\n";
      o_ << "  return 0;\n";
    } else if (entry_->ret == Type::Str) {
      o_ << "  (void)" << kEntryName << "();\n";
      o_ << "  return 0;\n";
    } else {
      o_ << "  return (int)" << kEntryName << "();\n";
    }
    o_ << "}\n";
  }
};

static std::string readFile(const std::filesystem::path &p) {
  std::ifstream in(p, std::ios::binary);
  if (!in) throw std::runtime_error("failed to read: " + p.string());
  std::ostringstream s;
  s << in.rdbuf();
  return s.str();
}

static void writeFile(const std::filesystem::path &p, const std::string &c) {
  if (!p.parent_path().empty()) {
    std::filesystem::create_directories(p.parent_path());
  }
  std::ofstream out(p, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("failed to write: " + p.string());
  out << c;
  if (!out) throw std::runtime_error("failed writing: " + p.string());
}

static uint64_t fnv1a64(const std::string &s, uint64_t seed = 1469598103934665603ULL) {
  uint64_t h = seed;
  for (unsigned char c : s) {
    h ^= static_cast<uint64_t>(c);
    h *= 1099511628211ULL;
  }
  return h;
}
static std::string hex64(uint64_t v) {
  std::ostringstream o;
  o << std::hex << std::setfill('0') << std::setw(16) << v;
  return o.str();
}
static std::string jsonEscape(const std::string &in) {
  std::ostringstream o;
  for (unsigned char c : in) {
    switch (c) {
    case '\"': o << "\\\""; break;
    case '\\': o << "\\\\"; break;
    case '\b': o << "\\b"; break;
    case '\f': o << "\\f"; break;
    case '\n': o << "\\n"; break;
    case '\r': o << "\\r"; break;
    case '\t': o << "\\t"; break;
    default:
      if (c < 0x20) {
        o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
      } else {
        o << static_cast<char>(c);
      }
      break;
    }
  }
  return o.str();
}
static std::string jsonUnescape(const std::string &in) {
  std::string out;
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    char c = in[i];
    if (c != '\\') {
      out.push_back(c);
      continue;
    }
    if (i + 1 >= in.size()) break;
    char n = in[++i];
    switch (n) {
    case '\"': out.push_back('\"'); break;
    case '\\': out.push_back('\\'); break;
    case '/': out.push_back('/'); break;
    case 'b': out.push_back('\b'); break;
    case 'f': out.push_back('\f'); break;
    case 'n': out.push_back('\n'); break;
    case 'r': out.push_back('\r'); break;
    case 't': out.push_back('\t'); break;
    case 'u':
      // Keep simple and deterministic: unsupported unicode escapes are replaced with '?'
      if (i + 4 < in.size()) i += 4;
      out.push_back('?');
      break;
    default: out.push_back(n); break;
    }
  }
  return out;
}
static std::optional<std::string> jsonExtractStringField(const std::string &json, const std::string &key) {
  const std::string needle = "\"" + key + "\"";
  std::size_t pos = json.find(needle);
  if (pos == std::string::npos) return std::nullopt;
  pos = json.find(':', pos + needle.size());
  if (pos == std::string::npos) return std::nullopt;
  ++pos;
  while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) ++pos;
  if (pos >= json.size() || json[pos] != '"') return std::nullopt;
  ++pos;
  std::string raw;
  bool esc = false;
  for (; pos < json.size(); ++pos) {
    char c = json[pos];
    if (!esc && c == '"') {
      return jsonUnescape(raw);
    }
    if (!esc && c == '\\') {
      esc = true;
      raw.push_back(c);
      continue;
    }
    esc = false;
    raw.push_back(c);
  }
  return std::nullopt;
}
static std::string computeSourceStateHash(const std::vector<std::filesystem::path> &inputs) {
  uint64_t h = 1469598103934665603ULL;
  for (const auto &p : inputs) {
    h = fnv1a64(p.string(), h);
    h = fnv1a64(readFile(p), h);
  }
  return hex64(h);
}
static std::string computeBuildConfigHash(const std::vector<std::filesystem::path> &inputs, const std::string &cc,
                                          const std::string &backend, bool maxSpeed, int passes,
                                          const std::string &target, const std::filesystem::path &sysroot,
                                          const std::string &linker) {
  uint64_t h = 1469598103934665603ULL;
  h = fnv1a64("LineScript-compile-cache-v1", h);
  h = fnv1a64(computeSourceStateHash(inputs), h);
  h = fnv1a64(cc, h);
  h = fnv1a64(backend, h);
  h = fnv1a64(maxSpeed ? "1" : "0", h);
  h = fnv1a64(std::to_string(passes), h);
  h = fnv1a64(target, h);
  h = fnv1a64(sysroot.string(), h);
  h = fnv1a64(linker, h);
  return hex64(h);
}
static void writeTypedIrBundle(const std::filesystem::path &outPath, const std::string &cCode,
                               const std::string &sourceStateHash, const std::string &buildConfigHash) {
  std::ostringstream o;
  o << "{\n";
  o << "  \"format\": \"linescript-typed-ir-v1\",\n";
  o << "  \"source_hash\": \"" << jsonEscape(sourceStateHash) << "\",\n";
  o << "  \"config_hash\": \"" << jsonEscape(buildConfigHash) << "\",\n";
  o << "  \"c_code\": \"" << jsonEscape(cCode) << "\"\n";
  o << "}\n";
  writeFile(outPath, o.str());
}
static std::string readTypedIrBundle(const std::filesystem::path &inPath, std::string *sourceStateHashOut = nullptr,
                                     std::string *buildConfigHashOut = nullptr) {
  std::string json = readFile(inPath);
  auto fmt = jsonExtractStringField(json, "format");
  if (!fmt.has_value() || *fmt != "linescript-typed-ir-v1") {
    throw std::runtime_error("unsupported typed IR format in " + inPath.string());
  }
  auto cCode = jsonExtractStringField(json, "c_code");
  if (!cCode.has_value()) {
    throw std::runtime_error("typed IR is missing c_code field: " + inPath.string());
  }
  if (sourceStateHashOut) {
    auto sh = jsonExtractStringField(json, "source_hash");
    *sourceStateHashOut = sh.value_or("");
  }
  if (buildConfigHashOut) {
    auto ch = jsonExtractStringField(json, "config_hash");
    *buildConfigHashOut = ch.value_or("");
  }
  return *cCode;
}

static bool gSuperuserVerbose = false;
static bool gSuperuserDebugToStderr = false;
static int gSuperuserVerbosity = 3;
static void setSuperuserLogging(bool enabled, bool debugToStderr, int verbosity = 3) {
  gSuperuserVerbose = enabled;
  gSuperuserDebugToStderr = debugToStderr;
  if (verbosity < 1) verbosity = 1;
  if (verbosity > 5) verbosity = 5;
  gSuperuserVerbosity = verbosity;
}
static void superuserLogV(int level, const std::string &msg) {
  if (!gSuperuserVerbose) return;
  if (level > gSuperuserVerbosity) return;
  std::ostream &os = gSuperuserDebugToStderr ? std::cerr : std::cout;
  os << "[superuser] " << msg << '\n';
  os.flush();
#if defined(_WIN32)
  std::string line = "[superuser] " + msg + "\n";
  OutputDebugStringA(line.c_str());
#endif
}
struct Opt {
  std::vector<std::filesystem::path> inputs;
  std::filesystem::path out;
  std::string cc = "clang";
  std::string backend = "auto";
  std::string target;
  std::filesystem::path sysroot;
  std::string linker;
  bool build = false;
  bool run = false;
  bool check = false;
  bool repl = false;
  bool superuserSession = false;
  int superuserVerbosity = 3;
  bool keepC = false;
  bool maxSpeed = false;
  bool incremental = true;
  bool noCache = false;
  std::filesystem::path cacheDir = ".linescript/cache";
  bool pgoGenerate = false;
  std::filesystem::path pgoUseDir;
  std::filesystem::path boltUseFdata;
  std::filesystem::path emitTypedIrPath;
  std::filesystem::path consumeTypedIrPath;
  int passes = 12;
  bool infoOnly = false;
  std::vector<std::string> infoMessages;
  std::vector<std::string> cliFlags;
  std::vector<std::string> cliCustomTokens;
};

// Version policy:
// - Internal half-step increments by 1 per update (0.0.0.5 granularity).
// - Displayed version is x.y.z and bumps patch every 2 half-steps.
static constexpr int kLineScriptVerMajor = 1;
static constexpr int kLineScriptVerMinor = 4;
static constexpr int kLineScriptVerPatchBase = 4;
static constexpr int kLineScriptVerHalfSteps = 4;  // 4 => internal 1.4.6, displayed 1.4.6

static std::string lineScriptVersionDisplay() {
  const int patch = kLineScriptVerPatchBase + (kLineScriptVerHalfSteps / 2);
  return std::to_string(kLineScriptVerMajor) + "." + std::to_string(kLineScriptVerMinor) + "." + std::to_string(patch);
}

static bool isKnownInfoFlagBody(const std::string &body) {
  return body == "LineScript" || body == "super-speed" || body == "what" || body == "hlep" || body == "max-sped";
}
static bool isValidCliFlagBody(const std::string &body) {
  if (body.empty()) return false;
  if (body.front() == '-' || body.back() == '-') return false;
  bool prevDash = false;
  for (char c : body) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (c == '-') {
      if (prevDash) return false;
      prevDash = true;
      continue;
    }
    if (!std::isalnum(u) && c != '_') return false;
    prevDash = false;
  }
  return true;
}
static std::string cliFlagBodyFromArg(const std::string &arg) {
  if (arg.rfind("--", 0) != 0) return "";
  return arg.substr(2);
}
static bool isCliGroupOpenToken(const std::string &tok) { return tok == "["; }
static bool isCliGroupCloseToken(const std::string &tok) { return tok == "]"; }
static bool isCliGroupToken(const std::string &tok) { return isCliGroupOpenToken(tok) || isCliGroupCloseToken(tok); }
static bool isPotentialShortOptionToken(const std::string &tok) {
  return tok.size() >= 2 && tok[0] == '-' && tok[1] != '-';
}
static bool isPotentialLongOptionToken(const std::string &tok) {
  return tok.size() >= 3 && tok.rfind("--", 0) == 0;
}
static bool isPotentialOptionToken(const std::string &tok) {
  return isPotentialShortOptionToken(tok) || isPotentialLongOptionToken(tok);
}
static bool isPotentialOptionValueToken(const std::string &tok) {
  if (tok.empty()) return false;
  if (isCliGroupToken(tok)) return false;
  if (isPotentialOptionToken(tok)) return false;
  return true;
}

static void usage() {
  std::cerr << "LineScript compiler\n";
  std::cerr << "No-arg mode starts interactive shell (REPL).\n";
  std::cerr << "Usage: lsc <file1.lsc> [file2.lsc ...] [options]\n";
  std::cerr << "   or: lsc --repl\n";
  std::cerr << "  -o <path>       output path\n";
  std::cerr << "  --check         parse/type-check/optimize only\n";
  std::cerr << "  --build         compile to native binary via C compiler\n";
  std::cerr << "  --run           build and run native binary\n";
  std::cerr << "  --repl          interactive LineScript shell\n";
  std::cerr << "  --shell         alias for --repl\n";
  std::cerr << "  --cc <name>     C compiler command (default: clang)\n";
  std::cerr << "  --backend <x>   backend: auto|c|asm (default: auto)\n";
  std::cerr << "  --target <triple> cross-compile target triple (clang/lld path)\n";
  std::cerr << "  --sysroot <path> sysroot for cross-compilation\n";
  std::cerr << "  --linker <name> linker override (for example lld)\n";
  std::cerr << "  --passes <n>    greedy optimization passes (default: 12)\n";
  std::cerr << "  --incremental   enable incremental compile cache (default on)\n";
  std::cerr << "  --cache-dir <path> incremental cache directory (default .linescript/cache)\n";
  std::cerr << "  --no-cache      disable incremental cache for this invocation\n";
  std::cerr << "  --emit-typed-ir <file.json> write typed IR JSON bundle for this compile\n";
  std::cerr << "  --consume-typed-ir <file.json> build from typed IR JSON (skip frontend parse/type-check)\n";
  std::cerr << "  -O4             primary max-speed profile (preferred)\n";
  std::cerr << "  --max-speed     compatibility alias for -O4\n";
  std::cerr << "  --pgo-generate  build instrumented binary for profile capture (max-speed pipeline)\n";
  std::cerr << "  --pgo-use <dir> use collected PGO profiles from <dir> (max-speed pipeline)\n";
  std::cerr << "  --bolt-use <fdata> apply BOLT profile data file when llvm-bolt is available\n";
  std::cerr << "  --keep-c        keep generated C when --build\n";
  std::cerr << "  --LineScript    print LineScript version\n";
  std::cerr << "  --super-speed   reserved tuning flag\n";
  std::cerr << "  --max-sped      reserved tuning alias\n";
  std::cerr << "  --what          reserved query flag\n";
  std::cerr << "  --hlep          reserved typo helper\n";
  std::cerr << "  --<name>        invoke matching 'flag name()' function if defined\n";
  std::cerr << "  --help\n";
}

static std::string stripOuterQuotes(std::string t);
static std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}
static bool isSourceExt(const std::filesystem::path &p) {
  const std::string ext = lower(p.extension().string());
  return ext == ".lsc" || ext == ".ls";
}
static bool isValidBackend(const std::string &backend) {
  const std::string b = lower(backend);
  return b == "auto" || b == "c" || b == "asm";
}
static bool isValidTargetTriple(const std::string &triple) {
  if (triple.empty()) return false;
  for (char c : triple) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (!(std::isalnum(u) || c == '_' || c == '-' || c == '.')) return false;
  }
  return true;
}
static bool isSafeLinkerName(const std::string &name) {
  if (name.empty()) return false;
  for (char c : name) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (!(std::isalnum(u) || c == '_' || c == '-' || c == '.' || c == '/' || c == '\\' || c == ':')) return false;
  }
  return true;
}
static bool isSafeCompilerCmdChar(char c) {
  const unsigned char u = static_cast<unsigned char>(c);
  return std::isalnum(u) || c == '_' || c == '-' || c == '.' || c == '/' || c == '\\' || c == ':' || c == '+' ||
         c == ' ' || c == '(' || c == ')';
}
static void validateCompilerCommand(const std::string &raw) {
  const std::string cmd = stripOuterQuotes(raw);
  if (cmd.empty()) throw std::runtime_error("--cc cannot be empty");
  for (char c : cmd) {
    if (!isSafeCompilerCmdChar(c)) {
      throw std::runtime_error(std::string("unsafe character in --cc value: '") + c + "'");
    }
  }
}
static void validatePathForShell(const std::filesystem::path &p, const std::string &label) {
  const std::string s = p.string();
  if (s.empty()) throw std::runtime_error(label + " cannot be empty");
  for (char c : s) {
    if (c == '"' || c == '\n' || c == '\r' || c == '\0') {
      throw std::runtime_error("unsafe character in " + label + ": " + s);
    }
  }
}

static Opt parseOpt(int argc, char **argv) {
  Opt o;
  if (argc < 2) {
    o.repl = true;
    validateCompilerCommand(o.cc);
    return o;
  }
  int cliGroupDepth = 0;
  auto pushCustomToken = [&](const std::string &tok) {
    if (isCliGroupOpenToken(tok)) {
      ++cliGroupDepth;
    } else if (isCliGroupCloseToken(tok)) {
      if (cliGroupDepth == 0) {
        throw std::runtime_error("unexpected ']' in custom CLI token stream");
      }
      --cliGroupDepth;
    }
    o.cliCustomTokens.push_back(tok);
  };
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--help") {
      usage();
      std::exit(0);
    } else if (a == "--repl" || a == "--shell") {
      o.repl = true;
    } else if (a == "--su-session") {
      o.superuserSession = true;
    } else if (a == "--su-verbosity") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --su-verbosity");
      o.superuserVerbosity = std::stoi(argv[++i]);
      if (o.superuserVerbosity < 1 || o.superuserVerbosity > 5) {
        throw std::runtime_error("--su-verbosity must be in range 1..5");
      }
    } else if (a == "--LineScript") {
      o.infoMessages.push_back("LineScript version " + lineScriptVersionDisplay());
      o.cliFlags.push_back(a);
    } else if (a == "--super-speed") {
      o.infoMessages.push_back("super speed activated, hold your horses..");
      o.cliFlags.push_back(a);
    } else if (a == "--what") {
      o.infoMessages.push_back("what? are you asking me?");
      o.cliFlags.push_back(a);
    } else if (a == "--hlep") {
      o.infoMessages.push_back("i think you made a little spelling mistake in your flag there");
      o.cliFlags.push_back(a);
    } else if (a == "--max-sped") {
      o.infoMessages.push_back("hurga durga doo! max sped activated!");
      o.cliFlags.push_back(a);
    } else if (isCliGroupToken(a)) {
      pushCustomToken(a);
    } else if (a == "--check") {
      o.check = true;
    } else if (a == "--build") {
      o.build = true;
    } else if (a == "--run") {
      o.run = true;
      o.build = true;
    } else if (a == "--keep-c") {
      o.keepC = true;
    } else if (a == "-O4" || a == "--max-speed") {
      o.maxSpeed = true;
      if (o.passes < 32) o.passes = 32;
    } else if (a == "--pgo-generate") {
      o.pgoGenerate = true;
      o.maxSpeed = true;
      if (o.passes < 32) o.passes = 32;
    } else if (a == "--pgo-use") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --pgo-use");
      o.pgoUseDir = argv[++i];
      o.maxSpeed = true;
      if (o.passes < 32) o.passes = 32;
    } else if (a == "--bolt-use") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --bolt-use");
      o.boltUseFdata = argv[++i];
      o.maxSpeed = true;
      if (o.passes < 32) o.passes = 32;
    } else if (a == "--cc") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --cc");
      o.cc = argv[++i];
      if (!o.cc.empty() && (o.cc.front() == '"' || o.cc.front() == '\'')) {
        const char quote = o.cc.front();
        while (i + 1 < argc && (o.cc.size() < 2 || o.cc.back() != quote)) {
          o.cc += " ";
          o.cc += argv[++i];
        }
      }
    } else if (a == "--backend") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --backend");
      o.backend = argv[++i];
    } else if (a == "--target") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --target");
      o.target = argv[++i];
    } else if (a == "--sysroot") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --sysroot");
      o.sysroot = argv[++i];
    } else if (a == "--linker") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --linker");
      o.linker = argv[++i];
    } else if (a == "--incremental") {
      o.incremental = true;
    } else if (a == "--cache-dir") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --cache-dir");
      o.cacheDir = argv[++i];
    } else if (a == "--no-cache") {
      o.noCache = true;
      o.incremental = false;
    } else if (a == "--emit-typed-ir") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --emit-typed-ir");
      o.emitTypedIrPath = argv[++i];
    } else if (a == "--consume-typed-ir") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --consume-typed-ir");
      o.consumeTypedIrPath = argv[++i];
    } else if (a == "--passes") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for --passes");
      o.passes = std::stoi(argv[++i]);
      if (o.passes <= 0) throw std::runtime_error("--passes must be > 0");
    } else if (a == "-o") {
      if (i + 1 >= argc) throw std::runtime_error("missing value for -o");
      o.out = argv[++i];
    } else if (!a.empty() && a[0] == '-') {
      if (isPotentialLongOptionToken(a)) {
        pushCustomToken(a);
        const bool hasInlineValue = (a.find('=') != std::string::npos);
        if (cliGroupDepth == 0 && !hasInlineValue) {
          o.cliFlags.push_back(a);
        }
        if (!hasInlineValue && i + 1 < argc) {
          const std::string next = argv[i + 1];
          if (isPotentialOptionValueToken(next)) {
            pushCustomToken(next);
            ++i;
          }
        }
      } else if (isPotentialShortOptionToken(a)) {
        pushCustomToken(a);
        if (i + 1 < argc) {
          const std::string next = argv[i + 1];
          if (isPotentialOptionValueToken(next)) {
            pushCustomToken(next);
            ++i;
          }
        }
      } else {
        throw std::runtime_error("unknown option: " + a);
      }
    } else {
      std::filesystem::path in = a;
      if (!isSourceExt(in)) {
        throw std::runtime_error("input file must use .lsc or .ls extension: " + in.string());
      }
      o.inputs.push_back(std::move(in));
    }
  }
  if (cliGroupDepth != 0) {
    throw std::runtime_error("unclosed '[' in custom CLI token stream");
  }
  if (o.repl && (o.check || o.build || o.run)) {
    throw std::runtime_error("--repl/--shell cannot be combined with --check/--build/--run");
  }
  if (o.repl && o.superuserSession) {
    throw std::runtime_error("--su-session is internal and cannot be combined with --repl/--shell");
  }
  if (!o.superuserSession) {
    o.superuserVerbosity = 3;
  }
  if (o.inputs.empty() && !o.repl) {
    if (!o.consumeTypedIrPath.empty()) {
      // allowed: build/run directly from typed IR bundle
    } else if (!o.infoMessages.empty() && !o.check && !o.build && !o.run) {
      o.infoOnly = true;
      return o;
    } else {
      throw std::runtime_error("input file is required");
    }
  }
  validateCompilerCommand(o.cc);
  if (!isValidBackend(o.backend)) {
    throw std::runtime_error("invalid --backend value (expected auto|c|asm): " + o.backend);
  }
  if (!o.target.empty() && !isValidTargetTriple(o.target)) {
    throw std::runtime_error("invalid --target triple: " + o.target);
  }
  if (!o.linker.empty() && !isSafeLinkerName(o.linker)) {
    throw std::runtime_error("invalid --linker value: " + o.linker);
  }
  if (!o.sysroot.empty()) {
    validatePathForShell(o.sysroot, "sysroot path");
  }
  if (!o.cacheDir.empty()) {
    validatePathForShell(o.cacheDir, "cache directory path");
  }
  if (!o.emitTypedIrPath.empty()) {
    validatePathForShell(o.emitTypedIrPath, "emit typed ir path");
  }
  if (!o.consumeTypedIrPath.empty()) {
    validatePathForShell(o.consumeTypedIrPath, "consume typed ir path");
  }
  if (!o.consumeTypedIrPath.empty() && !o.build && !o.run) {
    throw std::runtime_error("--consume-typed-ir requires --build or --run");
  }
  if (!o.consumeTypedIrPath.empty() && o.check) {
    throw std::runtime_error("--consume-typed-ir cannot be combined with --check");
  }
  if (o.pgoGenerate && !o.build) {
    throw std::runtime_error("--pgo-generate requires --build or --run");
  }
  if (!o.pgoUseDir.empty() && !o.build) {
    throw std::runtime_error("--pgo-use requires --build or --run");
  }
  if (!o.boltUseFdata.empty() && !o.build) {
    throw std::runtime_error("--bolt-use requires --build or --run");
  }
  if (o.pgoGenerate && !o.pgoUseDir.empty()) {
    throw std::runtime_error("--pgo-generate and --pgo-use cannot be used together");
  }
  if (!o.pgoUseDir.empty() && o.pgoUseDir.string().find(' ') != std::string::npos) {
    throw std::runtime_error("--pgo-use path must not contain spaces");
  }
  if (!o.boltUseFdata.empty() && o.boltUseFdata.string().find(' ') != std::string::npos) {
    throw std::runtime_error("--bolt-use path must not contain spaces");
  }
  if (o.check && (o.build || o.run)) {
    throw std::runtime_error("--check cannot be combined with --build/--run");
  }
  return o;
}

static std::unordered_set<std::string> inlineSet(const Program &p) {
  std::unordered_set<std::string> s;
  for (const auto &[n, _] : inlineCands(p)) s.insert(n);
  return s;
}

static std::unordered_map<std::string, std::string> cliFlagDefinitions(const Program &p) {
  std::unordered_map<std::string, std::string> defs;
  for (const auto &f : p.f) {
    if (!f.isCliFlag) continue;
    defs[f.cliFlagName] = f.n;
  }
  return defs;
}

static std::string q(const std::filesystem::path &p) { return "\"" + p.string() + "\""; }
static std::string stripOuterQuotes(std::string t) {
  bool changed = true;
  while (changed) {
    changed = false;
    if (t.size() >= 2 && ((t.front() == '"' && t.back() == '"') || (t.front() == '\'' && t.back() == '\''))) {
      t = t.substr(1, t.size() - 2);
      changed = true;
    }
    if (t.size() >= 4 && t[0] == '\\' && t[1] == '"' && t[t.size() - 2] == '\\' && t.back() == '"') {
      t = t.substr(2, t.size() - 4);
      changed = true;
    }
  }
  return t;
}
static std::string qCmd(const std::string &s) {
  std::string t = stripOuterQuotes(s);
  if (t.find(' ') != std::string::npos) return "\"" + t + "\"";
  return t;
}

static std::string lowerCopy(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}
static std::string flagsForAsmEmit(const std::string &flags) {
  std::istringstream in(flags);
  std::ostringstream out;
  std::string tok;
  bool first = true;
  while (in >> tok) {
    if (tok == "-flto" || tok.rfind("-Wl,", 0) == 0 || tok == "-fuse-ld=lld" || tok == "-nostdlib" ||
        tok.rfind("-l", 0) == 0 || tok == "user32.lib" || tok == "gdi32.lib") {
      continue;
    }
    if (!first) out << " ";
    out << tok;
    first = false;
  }
  if (first) return "-O3";
  return out.str();
}
static std::string flagsForAsmLink(const std::string &flags) {
  std::istringstream in(flags);
  std::ostringstream out;
  std::string tok;
  bool first = true;
  while (in >> tok) {
    if (tok == "-nostdlib" || tok.rfind("-fuse-ld=", 0) == 0 || tok.rfind("-Wl,", 0) == 0 || tok.rfind("-l", 0) == 0 ||
        tok == "user32.lib" || tok == "gdi32.lib") {
      if (!first) out << " ";
      out << tok;
      first = false;
    }
  }
  return out.str();
}

static std::string trimCopy(const std::string &s) {
  std::size_t b = 0;
  std::size_t e = s.size();
  while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
  return s.substr(b, e - b);
}

static bool applyBoltProfileIfAvailable(const std::filesystem::path &bin, const std::filesystem::path &fdata,
                                        bool cleanOutputMode, bool superuserMode) {
  if (fdata.empty()) return true;
  if (!std::filesystem::exists(fdata)) {
    throw std::runtime_error("--bolt-use profile file not found: " + fdata.string());
  }
  std::filesystem::path bolted = bin;
  bolted += ".bolt";
  const std::vector<std::string> tools = {"llvm-bolt", "bolt"};
  for (const auto &tool : tools) {
    std::ostringstream cmd;
    cmd << qCmd(tool) << " " << q(bin) << " -o " << q(bolted) << " -data=" << q(fdata)
        << " -reorder-blocks=ext-tsp -reorder-functions=hfsort+ -split-functions=3 -split-all-cold -icf=1";
    if (superuserMode) superuserLogV(3, "exec (bolt): " + cmd.str());
    const int rc = std::system(cmd.str().c_str());
    if (rc == 0 && std::filesystem::exists(bolted)) {
      std::error_code ec;
      std::filesystem::remove(bin, ec);
      ec.clear();
      std::filesystem::rename(bolted, bin, ec);
      if (!ec) {
        if (!cleanOutputMode) {
          std::cout << "BOLT: applied profile with " << tool << '\n';
        }
        return true;
      }
      std::filesystem::remove(bolted, ec);
    }
  }
  if (!cleanOutputMode) {
    std::cout << "BOLT: skipped (tool not available or optimization failed)\n";
  }
  return false;
}

static int replBlockDeltaLine(const std::string &line) {
  int delta = 0;
  bool inStr = false;
  bool esc = false;
  std::string tok;
  auto flushTok = [&]() {
    if (tok == "do") {
      ++delta;
    } else if (tok == "end") {
      --delta;
    }
    tok.clear();
  };
  for (std::size_t i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (inStr) {
      if (esc) {
        esc = false;
      } else if (c == '\\') {
        esc = true;
      } else if (c == '"') {
        inStr = false;
      }
      continue;
    }
    if (c == '/' && i + 1 < line.size() && line[i + 1] == '/') {
      break;
    }
    if (c == '"') {
      flushTok();
      inStr = true;
      continue;
    }
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
      tok.push_back(c);
      continue;
    }
    flushTok();
    if (c == '{') {
      ++delta;
    } else if (c == '}') {
      --delta;
    }
  }
  flushTok();
  return delta;
}

static bool replShouldPersist(const std::string &snippet) {
  const std::string low = lowerCopy(snippet);
  if (low.find("print(") != std::string::npos) return false;
  if (low.find("println(") != std::string::npos) return false;
  if (low.find(".statespeed(") != std::string::npos) return false;
  if (low.find("formatoutput(") != std::string::npos) return false;
  return true;
}

static bool replContainsSuperuser(const std::string &snippet) {
  return lowerCopy(snippet).find("superuser(") != std::string::npos;
}

static bool replIsSuperuserToggleCommand(const std::string &snippet) {
  std::string t = trimCopy(snippet);
  if (!t.empty() && t.back() == ';') t.pop_back();
  return lowerCopy(trimCopy(t)) == "superuser()";
}

static bool replParseVerbosityCommand(const std::string &snippet, int &levelOut) {
  std::string t = lowerCopy(trimCopy(snippet));
  if (!t.empty() && t.back() == ';') t.pop_back();
  constexpr const char *kPrefixes[] = {"su.verbosity.", "superuser.verbosity."};
  for (const char *prefix : kPrefixes) {
    if (t.rfind(prefix, 0) != 0) continue;
    const std::string tail = t.substr(std::char_traits<char>::length(prefix));
    if (tail.size() != 1 || tail[0] < '1' || tail[0] > '5') return false;
    levelOut = tail[0] - '0';
    return true;
  }
  return false;
}

static bool replIsIdentStart(char c) {
  const unsigned char u = static_cast<unsigned char>(c);
  return std::isalpha(u) || c == '_';
}

static bool replIsIdentChar(char c) {
  const unsigned char u = static_cast<unsigned char>(c);
  return std::isalnum(u) || c == '_';
}

static std::string replRewritePrintAsPrintln(const std::string &snippet) {
  std::string out;
  out.reserve(snippet.size() + 16);
  std::size_t i = 0;
  const std::size_t n = snippet.size();
  bool inStr = false;
  bool esc = false;
  while (i < n) {
    const char c = snippet[i];
    if (inStr) {
      out.push_back(c);
      if (esc) {
        esc = false;
      } else if (c == '\\') {
        esc = true;
      } else if (c == '"') {
        inStr = false;
      }
      ++i;
      continue;
    }
    if (c == '/' && i + 1 < n && snippet[i + 1] == '/') {
      while (i < n) {
        out.push_back(snippet[i]);
        if (snippet[i] == '\n') {
          ++i;
          break;
        }
        ++i;
      }
      continue;
    }
    if (c == '"') {
      out.push_back(c);
      inStr = true;
      ++i;
      continue;
    }
    if (replIsIdentStart(c)) {
      std::size_t j = i + 1;
      while (j < n && replIsIdentChar(snippet[j])) ++j;
      const std::string tok = snippet.substr(i, j - i);
      if (tok == "print") {
        std::size_t k = j;
        while (k < n && (snippet[k] == ' ' || snippet[k] == '\t' || snippet[k] == '\r')) ++k;
        if (k < n && snippet[k] == '(') {
          out += "println";
          i = j;
          continue;
        }
      }
      out += tok;
      i = j;
      continue;
    }
    out.push_back(c);
    ++i;
  }
  return out;
}

static std::string replUserName() {
#if defined(_WIN32)
  char *buf = nullptr;
  std::size_t n = 0;
  if (_dupenv_s(&buf, &n, "USERNAME") == 0 && buf != nullptr && *buf != '\0') {
    std::string name(buf);
    std::free(buf);
    return name;
  }
  if (buf != nullptr) std::free(buf);
  buf = nullptr;
  n = 0;
  if (_dupenv_s(&buf, &n, "USER") == 0 && buf != nullptr && *buf != '\0') {
    std::string name(buf);
    std::free(buf);
    return name;
  }
  if (buf != nullptr) std::free(buf);
#else
  const char *name = std::getenv("USERNAME");
  if (name != nullptr && *name != '\0') return std::string(name);
  name = std::getenv("USER");
  if (name != nullptr && *name != '\0') return std::string(name);
#endif
  return "user";
}

static int runRepl(const Opt &o, const std::string &argv0) {
  std::string prelude;
  for (const auto &in : o.inputs) {
    std::string src = readFile(in);
    prelude += src;
    if (prelude.empty() || prelude.back() != '\n') prelude.push_back('\n');
  }

  const auto ticks =
      std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  std::filesystem::path replBase = std::filesystem::temp_directory_path() / ("linescript_repl_" + std::to_string(ticks));
  std::filesystem::path replSrc = replBase;
  replSrc += ".lsc";
  std::filesystem::path replBin = replBase;
#if defined(_WIN32)
  replBin += ".exe";
#endif

  std::vector<std::string> persisted;
  std::string pending;
  int depth = 0;
  const std::string baseUser = replUserName();
  bool superuserSession = false;
  int superuserVerbosity = 3;

  std::cout << "LineScript shell " << lineScriptVersionDisplay() << '\n';
  std::cout << "Type :help for commands, :exit to quit.\n";
  std::cout.flush();

  auto renderPrompt = [&]() {
    if (depth > 0) return std::string("... ");
    const std::string who = superuserSession ? "superuser" : baseUser;
    return who + "@LineScript> ";
  };

  while (true) {
    std::cout << renderPrompt() << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cout << '\n';
      break;
    }

    if (depth == 0) {
      const std::string t = trimCopy(line);
      if (t.empty()) continue;
      if (t == ":exit" || t == ":quit") break;
      if (t == ":help") {
        std::cout << "REPL commands:\n";
        std::cout << "  :help   show this help\n";
        std::cout << "  :reset  clear interactive session state\n";
        std::cout << "  :whoami print current shell identity\n";
        std::cout << "  su.verbosity.<1-5> / superuser.verbosity.<1-5>  set superuser verbosity\n";
        std::cout << "  su.help / superuser.help  show privileged superuser commands\n";
        std::cout << "  :exit   quit shell\n";
        continue;
      }
      if (t == ":reset") {
        persisted.clear();
        pending.clear();
        depth = 0;
        superuserSession = false;
        std::cout << "Session reset.\n";
        continue;
      }
      if (t == ":whoami") {
        std::cout << (superuserSession ? "superuser" : baseUser) << '\n';
        continue;
      }
      const std::string lowered = lowerCopy(t);
      if (lowered == "su.help" || lowered == "superuser.help") {
        if (!superuserSession) {
          std::cerr << "Not privileged: enable superuser() first\n";
        } else {
          std::cout << "superuser REPL commands:\n";
          std::cout << "  superuser()          toggle superuser mode on/off\n";
          std::cout << "  su.verbosity.1..5 or superuser.verbosity.1..5\n";
          std::cout << "  su.help or superuser.help\n";
          std::cout << "superuser language APIs:\n";
          std::cout << "  su.trace.on()/off() and superuser.trace.on()/off()\n";
          std::cout << "  su.capabilities()/memory.inspect()/compiler.inspect() (superuser.* aliases too)\n";
          std::cout << "  su.limit.set(step_limit, mem_limit), su.ir.dump(), su.debug.hook(tag)\n";
        }
        continue;
      }
      int requestedVerbosity = 0;
      if (replParseVerbosityCommand(t, requestedVerbosity)) {
        if (!superuserSession) {
          std::cerr << "Not privileged: enable superuser() first\n";
        } else {
          superuserVerbosity = requestedVerbosity;
          std::cerr << "[superuser][repl] verbosity set to " << superuserVerbosity << "\n";
        }
        continue;
      }
    }

    pending += line;
    pending.push_back('\n');
    depth += replBlockDeltaLine(line);
    if (depth > 0) continue;
    if (depth < 0) {
      std::cerr << "REPL warning: unmatched 'end' or '}'\n";
      pending.clear();
      depth = 0;
      continue;
    }

    const std::string snippet = trimCopy(pending);
    pending.clear();
    if (snippet.empty()) continue;
    if (replIsSuperuserToggleCommand(snippet)) {
      superuserSession = !superuserSession;
      if (superuserSession) {
        superuserVerbosity = 3;
        std::cerr << "Warning: superuser() enabled. LineScript error safety guards are disabled; input will be "
                     "attempted even if it is bad, malformed, or otherwise invalid.\n";
        std::cerr << "[superuser][repl] session enabled\n";
      } else {
        std::cerr << "[superuser][repl] session disabled; back to normal user\n";
      }
      continue;
    }
    const bool hasSuperuser = replContainsSuperuser(snippet);
    const std::string replSnippet = replRewritePrintAsPrintln(snippet);

    std::ostringstream src;
    src << ".format()\n";
    src << prelude;
    for (const std::string &s : persisted) {
      src << s;
      if (s.empty() || s.back() != '\n') src << '\n';
    }
    src << replSnippet << '\n';
    writeFile(replSrc, src.str());

    std::ostringstream cmd;
    cmd << qCmd(argv0) << " " << q(replSrc) << " --run --cc " << qCmd(o.cc) << " --backend " << o.backend
        << " --passes " << o.passes << " -o " << q(replBin);
    if (o.maxSpeed) cmd << " -O4";
    if (superuserSession) cmd << " --su-session --su-verbosity " << superuserVerbosity;

    if (superuserSession && superuserVerbosity >= 5) {
      std::cerr << "[superuser][repl] snippet:\n" << replSnippet << "\n";
    }
    if (superuserSession && superuserVerbosity >= 4) {
      std::cerr << "[superuser][repl] command: " << cmd.str() << "\n";
    }

    const int rc = std::system(cmd.str().c_str());
    if (rc == 0) {
      if (!hasSuperuser && replShouldPersist(snippet)) {
        persisted.push_back(snippet);
      }
      if (hasSuperuser) superuserSession = true;
    } else {
      std::cerr << "REPL: command failed; session state unchanged.\n";
    }
  }

  std::error_code ec;
  std::filesystem::remove(replSrc, ec);
  std::filesystem::remove(replBin, ec);
  return 0;
}

static void inferDepsFromCCode(const std::string &cCode, bool &hasParallelFor, bool &hasWinGraphicsDep, bool &hasWinNetDep,
                               bool &hasPosixThreadDep, bool &ultraMinimalRuntime, bool &hasInteractiveInput) {
  auto hasAny = [&](std::initializer_list<const char *> needles) -> bool {
    for (const char *needle : needles) {
      if (cCode.find(needle) != std::string::npos) return true;
    }
    return false;
  };
  hasParallelFor = hasAny({"#define LS_PAR_FOR", "omp parallel for"});
  hasWinGraphicsDep = hasAny({"GetAsyncKeyState", "CreateWindowA", "game_new(", "pg_init("});
  hasWinNetDep = hasAny({"WSAStartup", "http_server_listen(", "http_client_connect("});
  hasPosixThreadDep = hasAny({"pthread_create", "pthread_join"});
  ultraMinimalRuntime = hasAny({"ls_stdout_handle", "LS_THREAD_LOCAL"});
  hasInteractiveInput = hasAny({"input_prompt(", "input_i64_prompt(", "input_f64_prompt("});
}

static int finish(const Opt &o, const std::string &cCode, bool cleanOutputMode, bool hasParallelFor,
                  bool hasWinGraphicsDep, bool hasWinNetDep, bool hasPosixThreadDep, bool ultraMinimalRuntime,
                  bool hasInteractiveInput, bool superuserMode, bool superuserIrDump) {
#if defined(_WIN32)
  (void)hasPosixThreadDep;
  (void)hasInteractiveInput;
#endif
#if !defined(_WIN32)
  (void)ultraMinimalRuntime;
  (void)hasWinNetDep;
  (void)hasWinGraphicsDep;
#endif
  const std::filesystem::path primaryIn =
      !o.inputs.empty() ? o.inputs.front()
                        : (!o.consumeTypedIrPath.empty() ? o.consumeTypedIrPath : std::filesystem::path("typed_ir_input.lsc"));
  if (!o.build) {
    auto out = o.out.empty() ? std::filesystem::path(primaryIn).replace_extension(".c") : o.out;
    validatePathForShell(out, "output path");
    if (superuserMode) superuserLogV(2, "emit-only mode: writing C output to " + out.string());
    writeFile(out, cCode);
    if (superuserMode && superuserIrDump) {
      auto irOut = std::filesystem::path(primaryIn).replace_extension(".ir.c");
      validatePathForShell(irOut, "superuser IR dump path");
      writeFile(irOut, cCode);
      superuserLogV(3, "superuser IR dump written to " + irOut.string());
    }
    if (!cleanOutputMode) std::cout << "Emitted C: " << out << '\n';
    return 0;
  }

  auto bin = o.out;
  if (bin.empty()) {
    bin = primaryIn;
    bin.replace_extension(
#if defined(_WIN32)
        ".exe"
#else
        ""
#endif
    );
  }
  std::filesystem::path c = bin;
  c += (o.keepC ? ".c" : ".tmp.c");
  validateCompilerCommand(o.cc);
  validatePathForShell(c, "generated C path");
  validatePathForShell(bin, "output binary path");
  if (!o.pgoUseDir.empty()) validatePathForShell(o.pgoUseDir, "pgo profile path");
  if (!o.boltUseFdata.empty()) validatePathForShell(o.boltUseFdata, "bolt profile path");
  if (superuserMode) {
    superuserLogV(2, "build mode: generating C at " + c.string());
    superuserLogV(2, "target binary: " + bin.string());
  }
  writeFile(c, cCode);
  if (superuserMode && superuserIrDump) {
    auto irOut = std::filesystem::path(primaryIn).replace_extension(".ir.c");
    validatePathForShell(irOut, "superuser IR dump path");
    writeFile(irOut, cCode);
    superuserLogV(3, "superuser IR dump written to " + irOut.string());
  }
  const std::string ccNorm = lowerCopy(stripOuterQuotes(o.cc));
  const bool clangLike = ccNorm.find("clang") != std::string::npos;
  const bool gccLike = (ccNorm.find("gcc") != std::string::npos) || (ccNorm.find("g++") != std::string::npos);
  const bool msvcLike =
      ccNorm == "cl" || ccNorm == "cl.exe" || ccNorm.find("\\cl.exe") != std::string::npos ||
      ccNorm.find("/cl.exe") != std::string::npos || ccNorm.find("clang-cl") != std::string::npos;
  std::string crossFlags;
  if (!o.target.empty()) {
    if (msvcLike) {
      throw std::runtime_error("--target is only supported on clang/gcc-style toolchains");
    }
    crossFlags += " --target=" + o.target;
  }
  if (!o.sysroot.empty()) {
    if (msvcLike) {
      throw std::runtime_error("--sysroot is only supported on clang/gcc-style toolchains");
    }
    crossFlags += " --sysroot=" + q(o.sysroot);
  }
  if (!o.linker.empty()) {
    if (msvcLike) {
      throw std::runtime_error("--linker is only supported on clang/gcc-style toolchains");
    }
    crossFlags += " -fuse-ld=" + o.linker;
  }
  std::string pgoFlags;
  if (o.pgoGenerate) {
    if (clangLike) {
      pgoFlags += " -fprofile-instr-generate";
    } else if (gccLike) {
      pgoFlags += " -fprofile-generate";
    } else if (!cleanOutputMode) {
      std::cout << "PGO: skipped --pgo-generate (requires clang/gcc-compatible toolchain)\n";
    }
  } else if (!o.pgoUseDir.empty()) {
    if (!std::filesystem::exists(o.pgoUseDir)) {
      throw std::runtime_error("--pgo-use path not found: " + o.pgoUseDir.string());
    }
    if (clangLike) {
      std::filesystem::path profdata = o.pgoUseDir / "default.profdata";
      if (!std::filesystem::exists(profdata)) {
        throw std::runtime_error("--pgo-use requires " + profdata.string() + " for clang profile use");
      }
      pgoFlags += " -fprofile-instr-use=" + profdata.string();
    } else if (gccLike) {
      pgoFlags += " -fprofile-use=" + o.pgoUseDir.string();
    } else if (!cleanOutputMode) {
      std::cout << "PGO: skipped --pgo-use (requires clang/gcc-compatible toolchain)\n";
    }
  }

  std::vector<std::string> baseFlagSets;
#if defined(_WIN32)
  if (o.maxSpeed) {
    baseFlagSets.push_back(
        "-O3 -ffast-math -flto -fuse-ld=lld -march=native -mtune=native -funroll-loops -fstrict-aliasing "
        "-fomit-frame-pointer -fno-math-errno -fno-trapping-math -ffp-contract=fast -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -ffast-math -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer "
        "-fno-math-errno -fno-trapping-math -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -fno-trapping-math -DNDEBUG");
    baseFlagSets.push_back("-O3 -fstrict-aliasing -DNDEBUG");
  } else {
    baseFlagSets.push_back(
        "-O3 -flto -fuse-ld=lld -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -DNDEBUG");
    baseFlagSets.push_back("-O3 -fstrict-aliasing -DNDEBUG");
  }
#else
  if (o.maxSpeed) {
    baseFlagSets.push_back(
        "-O3 -ffast-math -flto -march=native -mtune=native -funroll-loops -fstrict-aliasing "
        "-fomit-frame-pointer -fno-math-errno -fno-trapping-math -ffp-contract=fast "
        "-fno-semantic-interposition -s -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -ffast-math -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer "
        "-fno-math-errno -fno-trapping-math -s -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -fno-trapping-math -s -DNDEBUG");
    baseFlagSets.push_back("-O3 -fstrict-aliasing -s -DNDEBUG");
  } else {
    baseFlagSets.push_back(
        "-O3 -flto -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -s -DNDEBUG");
    baseFlagSets.push_back(
        "-O3 -march=native -fstrict-aliasing -fomit-frame-pointer -fno-math-errno -s -DNDEBUG");
    baseFlagSets.push_back("-O3 -fstrict-aliasing -s -DNDEBUG");
  }
#endif
  std::vector<std::string> flagSets;
  flagSets.reserve(baseFlagSets.size() * 2);
  std::string vectorTuneFlags;
  if (!msvcLike) {
    if (clangLike) {
      vectorTuneFlags = " -fvectorize -fslp-vectorize -fopenmp-simd";
    } else if (gccLike) {
      vectorTuneFlags = " -ftree-vectorize -fvect-cost-model=cheap -fopenmp-simd";
    }
  }
  const bool tryOpenMpFirst = hasParallelFor && !msvcLike;
  for (const std::string &base : baseFlagSets) {
    if (tryOpenMpFirst) {
      flagSets.push_back(std::string("-fopenmp ") + base + vectorTuneFlags);
      flagSets.push_back(base + vectorTuneFlags);
    } else {
      flagSets.push_back(base + vectorTuneFlags);
      flagSets.push_back(std::string("-fopenmp ") + base + vectorTuneFlags);
    }
  }
#if defined(_WIN32)
  const bool tryUltraNoCrt = ultraMinimalRuntime && clangLike && !cleanOutputMode;
  if (tryUltraNoCrt) {
    std::vector<std::string> ultraFlagSets;
    ultraFlagSets.push_back(
        "-O3 -ffast-math -flto -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer "
        "-fno-stack-protector -fno-builtin -fno-builtin-strlen -fno-math-errno -fno-trapping-math "
        "-ffp-contract=fast -fvectorize -fslp-vectorize -DNDEBUG "
        "-nostdlib -fuse-ld=lld -Wl,/entry:mainCRTStartup -Wl,/subsystem:console -Wl,/OPT:REF -Wl,/OPT:ICF "
        "-lkernel32");
    ultraFlagSets.push_back(
        "-O3 -march=native -mtune=native -funroll-loops -fstrict-aliasing -fomit-frame-pointer -fno-stack-protector "
        "-fno-builtin -fno-builtin-strlen -fvectorize -fslp-vectorize -DNDEBUG "
        "-nostdlib -fuse-ld=lld -Wl,/entry:mainCRTStartup -Wl,/subsystem:console -Wl,/OPT:REF -Wl,/OPT:ICF "
        "-lkernel32");
    ultraFlagSets.push_back(
        "-O3 -fstrict-aliasing -fomit-frame-pointer -fno-stack-protector -fno-builtin -fno-builtin-strlen -DNDEBUG "
        "-nostdlib -fuse-ld=lld -Wl,/entry:mainCRTStartup -Wl,/subsystem:console -Wl,/OPT:REF -Wl,/OPT:ICF "
        "-lkernel32");
    flagSets.insert(flagSets.begin(), ultraFlagSets.begin(), ultraFlagSets.end());
  }
#endif

  std::string winLinkFlags;
  std::string winGuiFlags;
  std::string winOptLinkFlags;
  std::string posixLinkFlags;
#if defined(_WIN32)
  if (hasWinGraphicsDep) {
    winLinkFlags = msvcLike ? " user32.lib gdi32.lib" : " -luser32 -lgdi32";
  }
  if (hasWinNetDep) {
    winLinkFlags += msvcLike ? " ws2_32.lib" : " -lws2_32";
  }
  // Keep console-attached behavior. `.format()` only suppresses toolchain chatter.
  if (o.maxSpeed && (clangLike || gccLike)) {
    winOptLinkFlags = " -Wl,/OPT:REF -Wl,/OPT:ICF";
  }
#else
  if (hasPosixThreadDep) {
    posixLinkFlags = " -pthread";
  }
#endif

  const std::string backendNorm = lowerCopy(stripOuterQuotes(o.backend));
  const bool tryAsmBackend = (backendNorm == "asm" || backendNorm == "auto") && !msvcLike;
  std::filesystem::path asmTmp = bin;
  asmTmp += ".tmp.s";
  if (tryAsmBackend) {
    validatePathForShell(asmTmp, "generated assembly path");
  }

  std::string linkSuffix;
#if defined(_WIN32)
  linkSuffix = winLinkFlags + winOptLinkFlags + winGuiFlags;
#else
  linkSuffix = posixLinkFlags;
#endif

  int rc = 1;
  std::string usedFlags;
  std::string usedBackend = "c";
  auto execBuildCmd = [&](const std::string &cmd) -> int {
    if (superuserMode) return std::system(cmd.c_str());
#if defined(_WIN32)
    return std::system((cmd + " >nul 2>nul").c_str());
#else
    return std::system((cmd + " >/dev/null 2>/dev/null").c_str());
#endif
  };
  std::vector<std::string> cppFallbackCommands = {"clang++", "g++"};
  for (const std::string &flags : flagSets) {
    const std::string activeFlags = trimCopy(flags + pgoFlags + crossFlags);
    if (tryAsmBackend) {
      const std::string asmFlags = flagsForAsmEmit(activeFlags);
      const std::string asmLinkFlags = flagsForAsmLink(activeFlags);
      if (superuserMode) {
        superuserLogV(3, "trying ASM emit flags: " + asmFlags);
      }
      std::ostringstream asmEmit;
      asmEmit << qCmd(o.cc) << " " << asmFlags << " -fno-lto -S -masm=intel " << q(c) << " -o " << q(asmTmp);
      if (superuserMode) superuserLogV(4, "exec (asm emit): " + asmEmit.str());
      rc = execBuildCmd(asmEmit.str());
      if (rc == 0) {
        if (superuserMode) superuserLogV(3, "ASM emit succeeded; linking ASM backend");
        std::ostringstream asmLink;
        asmLink << qCmd(o.cc) << " " << asmLinkFlags << " " << q(asmTmp) << " -o " << q(bin) << linkSuffix;
        if (superuserMode) superuserLogV(4, "exec (asm link): " + asmLink.str());
        rc = execBuildCmd(asmLink.str());
        if (rc == 0) {
          usedFlags = activeFlags;
          usedBackend = "asm";
          if (superuserMode) superuserLogV(2, "ASM backend selected");
          break;
        }
      }
      for (const std::string &cppCmdName : cppFallbackCommands) {
        if (superuserMode) superuserLogV(2, "ASM path failed; trying C++ fallback compiler: " + cppCmdName);
        std::ostringstream cppCmd;
        cppCmd << qCmd(cppCmdName) << " " << activeFlags << " " << q(c) << " -o " << q(bin) << linkSuffix;
        if (superuserMode) superuserLogV(4, "exec (cpp fallback): " + cppCmd.str());
        rc = execBuildCmd(cppCmd.str());
        if (rc == 0) {
          usedFlags = activeFlags;
          usedBackend = "cpp-fallback";
          if (superuserMode) superuserLogV(2, "C++ fallback backend selected: " + cppCmdName);
          break;
        }
      }
      if (rc == 0) break;
    }
    std::ostringstream cmd;
    cmd << qCmd(o.cc) << " " << activeFlags << " " << q(c) << " -o " << q(bin) << linkSuffix;
    if (superuserMode) superuserLogV(3, "trying C backend flags: " + activeFlags);
    if (superuserMode) superuserLogV(4, "exec (c backend): " + cmd.str());
    rc = execBuildCmd(cmd.str());
    if (rc == 0) {
      usedFlags = activeFlags;
      usedBackend = "c";
      if (superuserMode) superuserLogV(2, "C backend selected");
      break;
    }
  }
  if (rc != 0) {
    if (backendNorm == "asm") {
      throw std::runtime_error("ASM backend failed and fallback compilers were unavailable");
    }
    throw std::runtime_error("C compiler failed after fallback attempts");
  }

  if (!o.keepC) {
    std::error_code ec;
    std::filesystem::remove(c, ec);
  } else {
    if (!cleanOutputMode) std::cout << "Generated C: " << c << '\n';
  }
  if (tryAsmBackend) {
    std::error_code ec;
    std::filesystem::remove(asmTmp, ec);
  }
  if (!o.boltUseFdata.empty()) {
    (void)applyBoltProfileIfAvailable(bin, o.boltUseFdata, cleanOutputMode, superuserMode);
  }
  if (!cleanOutputMode) {
    std::cout << "Backend: " << usedBackend << '\n';
    std::cout << "Native flags: " << usedFlags << '\n';
    std::cout << "Built binary: " << bin << '\n';
    std::cout.flush();
  }
  if (superuserMode) {
    superuserLogV(1, "build completed with backend=" + usedBackend);
  }
  if (o.run) {
    if (superuserMode) superuserLogV(1, "running binary: " + bin.string());
    const int runRc = std::system(q(bin).c_str());
    if (runRc != 0) return runRc;
  }
  return 0;
}

} // namespace ls

int main(int argc, char **argv) {
  std::string stage;
  std::string currentFile;
  try {
    ls::Opt o = ls::parseOpt(argc, argv);
    for (const std::string &msg : o.infoMessages) std::cout << msg << '\n';
    if (o.infoOnly) return 0;
    if (o.repl) {
      return ls::runRepl(o, (argc > 0 && argv[0] != nullptr) ? argv[0] : "lsc");
    }
    std::string incrementalConfigHash;
    std::filesystem::path incrementalTypedIrPath;
    if (o.incremental && !o.noCache && !o.inputs.empty()) {
      incrementalConfigHash = ls::computeBuildConfigHash(o.inputs, o.cc, o.backend, o.maxSpeed, o.passes, o.target,
                                                         o.sysroot, o.linker);
      incrementalTypedIrPath = o.cacheDir / (incrementalConfigHash + ".typed_ir.json");
    }
    if (o.consumeTypedIrPath.empty() && !incrementalTypedIrPath.empty() && !o.check && std::filesystem::exists(incrementalTypedIrPath)) {
      o.consumeTypedIrPath = incrementalTypedIrPath;
      if (!o.infoMessages.empty()) {
        std::cout << "Using cached typed IR: " << o.consumeTypedIrPath.string() << '\n';
      }
    }
    if (!o.consumeTypedIrPath.empty()) {
      bool hasParallelFor = false;
      bool hasWinGraphicsDep = false;
      bool hasWinNetDep = false;
      bool hasPosixThreadDep = false;
      bool ultraMinimalRuntime = false;
      bool hasInteractiveInput = false;
      const std::string cOut = ls::readTypedIrBundle(o.consumeTypedIrPath);
      ls::inferDepsFromCCode(cOut, hasParallelFor, hasWinGraphicsDep, hasWinNetDep, hasPosixThreadDep,
                             ultraMinimalRuntime, hasInteractiveInput);
      if (!o.emitTypedIrPath.empty()) {
        const std::string emptyHash;
        ls::writeTypedIrBundle(o.emitTypedIrPath, cOut, emptyHash, incrementalConfigHash);
      }
      return ls::finish(o, cOut, false, hasParallelFor, hasWinGraphicsDep, hasWinNetDep, hasPosixThreadDep,
                        ultraMinimalRuntime, hasInteractiveInput, o.superuserSession, false);
    }
    ls::setSuperuserLogging(o.superuserSession, false, o.superuserVerbosity);
    ls::Program p;
    bool preparseSuperuserMode = o.superuserSession;
    auto lineCountOf = [](const std::string &src) -> std::size_t {
      if (src.empty()) return 0;
      std::size_t lines = 1;
      for (char c : src)
        if (c == '\n') ++lines;
      return lines;
    };
    auto tokenTextForLog = [](const std::string &text) -> std::string {
      std::ostringstream out;
      const std::size_t maxLen = 80;
      std::size_t emitted = 0;
      for (char c : text) {
        if (emitted >= maxLen) break;
        switch (c) {
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        case '\\': out << "\\\\"; break;
        default:
          if (std::iscntrl(static_cast<unsigned char>(c))) {
            out << '?';
          } else {
            out << c;
          }
          break;
        }
        ++emitted;
      }
      if (text.size() > maxLen) out << "...";
      return out.str();
    };
    for (const auto &input : o.inputs) {
      stage = "parse";
      currentFile = input.string();
      std::string src = ls::readFile(input);
      if (!preparseSuperuserMode && ls::lowerCopy(src).find("superuser(") != std::string::npos) {
        preparseSuperuserMode = true;
        ls::setSuperuserLogging(true, false, o.superuserVerbosity);
        ls::superuserLogV(1, "superuser hint detected in source; enabling early diagnostics");
      }
      if (preparseSuperuserMode) {
        ls::superuserLogV(2, "stage: lex begin: " + input.string());
        ls::superuserLogV(4, "source stats: bytes=" + std::to_string(src.size()) +
                                 ", lines=" + std::to_string(lineCountOf(src)));
      }
      ls::Lexer lx(src);
      std::vector<ls::Token> toks = lx.run();
      if (preparseSuperuserMode) {
        ls::superuserLogV(3, "lexed tokens=" + std::to_string(toks.size()) + " for " + input.string());
        if (o.superuserVerbosity >= 5) {
          for (std::size_t ti = 0; ti < toks.size(); ++ti) {
            const auto &tok = toks[ti];
            std::ostringstream msg;
            msg << "tok[" << ti << "] " << ls::tokenKindName(tok.kind) << " \"" << tokenTextForLog(tok.text) << "\" @"
                << tok.span.line << ":" << tok.span.col;
            ls::superuserLogV(5, msg.str());
          }
        }
        ls::superuserLogV(2, "stage: parse file begin: " + input.string());
      }
      ls::Parser ps(std::move(toks));
      ls::Program part = ps.run();
      if (preparseSuperuserMode) {
        ls::superuserLogV(3, "parsed file summary: functions=" + std::to_string(part.f.size()) +
                                 ", top-level statements=" + std::to_string(part.top.size()));
        ls::superuserLogV(2, "stage: parse file complete: " + input.string());
      }
      for (auto &fn : part.f) p.f.push_back(std::move(fn));
      for (auto &stmt : part.top) p.top.push_back(std::move(stmt));
    }
    std::vector<std::string> activeCliFlags;
    if (!o.cliFlags.empty()) {
      const auto defs = ls::cliFlagDefinitions(p);
      std::unordered_set<std::string> seen;
      for (const std::string &rawFlag : o.cliFlags) {
        const std::string body = ls::cliFlagBodyFromArg(rawFlag);
        if (!ls::isValidCliFlagBody(body)) {
          std::cerr << "Warning: bad flag '" << rawFlag << "' ignored\n";
          continue;
        }
        auto it = defs.find(body);
        if (it != defs.end()) {
          if (seen.insert(body).second) activeCliFlags.push_back(body);
          continue;
        }
        if (!ls::isKnownInfoFlagBody(body)) {
          std::cerr << "Warning: undefined flag '" << rawFlag << "'\n";
        }
      }
    }
    if (!p.top.empty()) {
      ls::Fn script;
      script.n = ls::kScriptEntryName;
      script.ret = ls::Type::Void;
      script.s = p.top.front()->s;
      script.b = std::move(p.top);
      p.top.clear();
      p.f.push_back(std::move(script));
    }
    const bool superuserCallDetected = ls::hasCallNamedProgram(p, "superuser");
    const bool superuserMode = superuserCallDetected || o.superuserSession;
    const bool cleanOutputModeEarly = ls::hasFormatMarkerProgram(p);
    ls::setSuperuserLogging(superuserMode, cleanOutputModeEarly, o.superuserVerbosity);
    if (superuserCallDetected) {
      std::cerr << "Warning: superuser() enabled. LineScript error safety guards are disabled; input will be "
                   "attempted even if it is bad, malformed, or otherwise invalid.\n"
                << std::flush;
    }
    if (superuserMode) {
      if (superuserCallDetected) {
        ls::superuserLogV(1, "superuser mode detected from source");
      } else {
        ls::superuserLogV(1, "superuser mode enabled via session flag");
      }
      ls::superuserLogV(2, "inputs=" + std::to_string(o.inputs.size()));
      for (const auto &input : o.inputs) ls::superuserLogV(3, "input: " + input.string());
      int fnCount = 0;
      int externCount = 0;
      for (const auto &f : p.f) {
        if (f.ex) {
          ++externCount;
          continue;
        }
        ++fnCount;
      }
      ls::superuserLogV(4, "syntax summary: functions=" + std::to_string(fnCount) + ", extern=" +
                                std::to_string(externCount));
      for (const auto &f : p.f) {
        std::ostringstream sig;
        sig << "fn " << f.n << "(";
        for (std::size_t i = 0; i < f.p.size(); ++i) {
          if (i) sig << ", ";
          sig << f.p[i].n << ":" << ls::typeName(f.p[i].t);
        }
        sig << ") -> " << ls::typeName(f.ret);
        if (f.ex) sig << " [extern]";
        if (f.inl) sig << " [inline]";
        if (f.isCliFlag) sig << " [flag:" << f.cliFlagName << "]";
        if (f.n == ls::kScriptEntryName) sig << " [script-entry]";
        ls::superuserLogV(5, "syntax: " + sig.str());
      }
      ls::superuserLogV(2, "stage: parse complete");
    }

    stage = "type-check";
    if (superuserMode) ls::superuserLogV(2, "stage: type-check begin");
    currentFile.clear();
    ls::TypeCheck tc(p, superuserMode);
    tc.run();
    if (superuserMode) {
      for (const auto &w : tc.warnings()) ls::superuserLogV(1, "type-check warning: " + w);
      ls::superuserLogV(2, "stage: type-check complete");
    }

    stage = "optimize";
    if (superuserMode) {
      ls::superuserLogV(2, "stage: optimize begin");
      ls::superuserLogV(2, "optimizer passes=" + std::to_string(o.passes));
    }
    ls::optimize(p, o.passes);
    stage = "re-type-check";
    if (superuserMode) ls::superuserLogV(2, "stage: re-type-check begin");
    ls::TypeCheck tc2(p, superuserMode);
    tc2.run();
    {
      std::unordered_set<std::string> seenWarnings;
      for (const auto &w : tc.warnings()) {
        if (seenWarnings.insert(w).second) std::cerr << w << '\n';
      }
      for (const auto &w : tc2.warnings()) {
        if (seenWarnings.insert(w).second) std::cerr << w << '\n';
      }
    }
    if (superuserMode) {
      for (const auto &w : tc2.warnings()) ls::superuserLogV(1, "re-type-check warning: " + w);
      ls::superuserLogV(2, "stage: optimize/re-type-check complete");
    }
    if (o.check) {
      std::cout << "Check passed: " << o.inputs.size() << " file(s)\n";
      return 0;
    }

    stage = "emit/build";
    if (superuserMode) ls::superuserLogV(2, "stage: emit/build begin");
    ls::EmitC ec(p, ls::inlineSet(p), superuserMode, o.superuserSession, activeCliFlags, o.cliCustomTokens);
    if ((o.build || o.run) && !ec.hasEntry()) {
      throw std::runtime_error(ec.entryError());
    }
    const bool cleanOutputMode = cleanOutputModeEarly;
    ls::setSuperuserLogging(superuserMode, cleanOutputMode, o.superuserVerbosity);
    if (superuserMode) {
      ls::superuserLogV(3,
                        std::string("debug stream=") + (cleanOutputMode ? "stderr/debug-window (.format mode)" : "terminal"));
    }
    const bool hasParallelFor = ls::hasParallelForProgram(p);
    const bool hasWinGraphicsDep = ls::hasWinGraphicsDepProgram(p);
    const bool hasWinNetDep = ls::hasCallNamedProgram(p, "http_server_listen") ||
                              ls::hasCallNamedProgram(p, "http_server_accept") ||
                              ls::hasCallNamedProgram(p, "http_server_read") ||
                              ls::hasCallNamedProgram(p, "http_server_respond_text") ||
                              ls::hasCallNamedProgram(p, "http_server_close") ||
                              ls::hasCallNamedProgram(p, "http_client_connect") ||
                              ls::hasCallNamedProgram(p, "http_client_send") ||
                              ls::hasCallNamedProgram(p, "http_client_read") ||
                              ls::hasCallNamedProgram(p, "http_client_close");
    const bool hasPosixThreadDep = ls::hasCallNamedProgram(p, "spawn") || ls::hasCallNamedProgram(p, "await") ||
                                   ls::hasCallNamedProgram(p, "await_all");
    const bool ultraMinimalRuntime = ec.ultraMinimalRuntime();
    const bool hasInteractiveInput = ls::hasCallNamedProgram(p, "input") || ls::hasCallNamedProgram(p, "input_i64") ||
                                     ls::hasCallNamedProgram(p, "input_f64");
    if (superuserMode) ls::superuserLogV(2, "emitting C backend source");
    const std::string cOut = ec.run();
    const std::string sourceStateHash = o.inputs.empty() ? std::string() : ls::computeSourceStateHash(o.inputs);
    if (!o.emitTypedIrPath.empty()) {
      ls::writeTypedIrBundle(o.emitTypedIrPath, cOut, sourceStateHash, incrementalConfigHash);
    }
    if (!incrementalTypedIrPath.empty() && !o.check) {
      ls::writeTypedIrBundle(incrementalTypedIrPath, cOut, sourceStateHash, incrementalConfigHash);
    }
    return ls::finish(o, cOut, cleanOutputMode, hasParallelFor, hasWinGraphicsDep, hasWinNetDep, hasPosixThreadDep,
                      ultraMinimalRuntime, hasInteractiveInput, superuserMode, ec.superuserIrDumpRequested());
  } catch (const ls::CompileError &e) {
    if (!currentFile.empty()) {
      std::cerr << "LineScript error (" << currentFile << "): " << e.what() << '\n';
    } else if (!stage.empty()) {
      std::cerr << "LineScript error (" << stage << "): " << e.what() << '\n';
    } else {
      std::cerr << "LineScript error: " << e.what() << '\n';
    }
    return 1;
  } catch (const std::exception &e) {
    if (!currentFile.empty()) {
      std::cerr << "LineScript failure (" << currentFile << "): " << e.what() << '\n';
    } else if (!stage.empty()) {
      std::cerr << "LineScript failure (" << stage << "): " << e.what() << '\n';
    } else {
      std::cerr << "LineScript failure: " << e.what() << '\n';
    }
    return 1;
  }
}
