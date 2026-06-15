# LineScript OOP Guide

This document is the complete Object-Oriented Programming reference for LineScript's current class system.

## What OOP Means in LineScript

LineScript supports C++-style class syntax:
- typed fields
- constructor
- single inheritance with `extends`
- instance methods
- static methods
- virtual/override/final method contracts
- access modifiers (`public`/`protected`/`private`)
- method overloading
- `this` inside class members
- member access and member assignment

Under the hood, class instances are handle-backed (`i64`) objects. That keeps syntax high-level while preserving speed-focused native compilation.

## Advanced OOP Additions

LineScript now supports the following advanced OOP-adjacent features in class-heavy code:
- generic handle type annotations: `array<T>`, `ptr<T>`, `slice<T>`
- explicit lifetime operations: `delete x` and `delete[] x`
- expression macros for reusable compile-time OOP snippets:
 - `macro name(args...) -> expr do ... end`
 - `expand(name(...))`

Notes:
- `array<T>`, `ptr<T>`, and `slice<T>` are currently handle-backed (`i64`) at runtime.
- `delete[]` is accepted for parity with C++-style array cleanup intent.
- expression macros are available now; `stmt`/`item` macro return kinds are reserved for a later milestone.

## Class Syntax

### `do/end` style

```linescript
class Counter do
  declare value: i64 = 0
  declare enabled: bool = true

  constructor(start: i64) do
    this.value = start
  end

  add(delta: i64) -> void do
    if this.enabled do
      this.value += delta
    end
  end

  get() -> i64 do
    return this.value
  end
end
```

### Brace style

```linescript
class Counter {
  declare value: i64 = 0;

  constructor(start: i64) {
    this.value = start;
  }

  get() -> i64 {
    return this.value;
  }
}
```

Both styles are supported in the same language.

## Field Rules

Field declarations must follow this form:

```linescript
declare <field_name>: <type> [= <initializer>]
```

Rules:
- field type is required (no typeless class fields)
- fields must be declared before methods/constructors
- duplicate field names are not allowed
- if initializer is omitted, default is used:
 - `i32`/`i64`: `0`
 - `f32`/`f64`: `0.0`
 - `bool`: `false`
 - `str`: `""`

## Constructor Rules

You can define one constructor per class:

```linescript
constructor(...) do
  ...
end
```

or constructor by class name:

```linescript
Counter(...) do
  ...
end
```

Rules:
- only one constructor is allowed
- constructor return type is implicit (do not write `-> ...`)
- constructor cannot be `extern`
- if no constructor is defined, LineScript generates a default one automatically
- for derived classes, constructor init-list base calls are supported:

```linescript
constructor(a: i64, b: i64) : Base(a) do
  this.extra = b
end
```

If a derived constructor does not provide an init-list, LineScript calls the direct base constructor with zero arguments.

When you instantiate:

```linescript
declare c = Counter(10)
```

you get a class instance handle (`i64` internally).

## Method Rules

Method forms:

```linescript
name(args...) -> type do
  ...
end
```

or:

```linescript
fn name(args...) -> type do
  ...
end
```

Method modifiers:
- `inline` is allowed
- `extern` is allowed for methods
- `throws` contracts are supported
- access control: `public`, `protected`, `private`
- dispatch/lifecycle: `virtual`, `override`, `final`
- `static` methods callable from class name
- operator methods:
 - binary: `fn operator +(rhs: i64) -> i64 do ... end`
 - unary: `fn operator unary -() -> i64 do ... end`

Rules:
- methods can be overloaded by parameter type list
- top-level overload resolution uses exact match first, then safe widening
- class-method overload calls currently require distinct arity (same-arity overloads are rejected as ambiguous)
- duplicate overload signatures in the same class are not allowed
- `override` requires a compatible base method
- overriding `final` methods is rejected
- unary operator method arity is strict:
 - `operator unary ...` method must take `0` explicit parameters
 - top-level `operator unary ...` function must take `1` parameter

## `this` and Member Access

Inside class members:

```linescript
this.value = this.value + 1
```

Outside class members:

```linescript
obj.value = 10
obj.value += 5
println(obj.value)
obj.method()
```

Static call form:

```linescript
MathUtil.scale(5)
```

Supported member writes include normal and compound assignments (`=`, `+=`, `-=`, `*=`, `/=`, `%=` and power-assign forms where valid).

Field access control is enforced for reads and writes:
- `public`: accessible from anywhere
- `private`: only inside the declaring class
- `protected`: inside declaring class and subclasses

## `delete` and `delete[]`

You can release handle-backed resources explicitly:

```linescript
delete obj
delete[] arr
```

Behavior:
- for class instances: lowers to object-handle release (`object_free`)
- for known handle constructors (`array_new`, `dict_new`, `map_new`, `np_new`, etc.): lowers to matching free call
- fallback path is raw pointer free (`mem_free`) when no constructor mapping is known

This keeps syntax simple while allowing explicit, deterministic cleanup in OOP-style programs.

## Pointer/Array Type Annotations

LineScript supports handle-generic annotations:

```linescript
declare buf: ptr<byte> = mem_alloc(64)
declare names: array<str> = array_new()
declare view: slice<i64> = buf
```

Notes:
- these are currently handle-backed (`i64`) at runtime
- they are intended for explicit intent and safer API contracts in class-heavy code
- supported inner types: primitive types, `str`, and known class types

## OOP + Generic Handle Types

You can annotate OOP-facing APIs with handle-generic types:

```linescript
fn consume_points(points: array<i64>) -> i64 do
  return array_len(points)
end

fn move_cursor(buf: ptr<byte>, offset: i64) -> ptr<byte> do
  // currently handle-backed; pointer arithmetic helpers are runtime-level
  return buf
end
```

Inner generic types currently supported: `i32`, `i64`, `f32`, `f64`, `bool`, `byte`.

## Class Types in Signatures

Class names can be used as type annotations:

```linescript
rename(c: Counter, new_name: str) -> void do
  c.name = new_name
end
```

Notes:
- class-typed values are handle-backed (`i64`) at runtime
- this is why class values are cheap to pass/return

## Example 1: Basic Counter

```linescript
class Counter do
  declare value: i64 = 0

  constructor(start: i64) do
    this.value = start
  end

  inc(delta: i64) -> void do
    this.value += delta
  end

  get() -> i64 do
    return this.value
  end
end

main() -> i64 do
  declare c = Counter(10)
  c.inc(5)
  println(c.get())
  return 0
end
```

## Example 2: Default Constructor

```linescript
class Session do
  declare user: str = "guest"
  declare retries: i64 = 0
end

main() -> i64 do
  declare s = Session()
  println(s.user)
  println(s.retries)
  return 0
end
```

## Example 3: Constructor Named After Class

```linescript
class Player do
  declare hp: i64 = 100
  declare tag: str = "p1"

  Player(hp: i64, tag: str) do
    this.hp = hp
    this.tag = tag
  end
end

main() -> i64 do
  declare p = Player(150, "boss")
  println(p.hp)
  println(p.tag)
  return 0
end
```

## Example 4: Class Type Parameters and Returns

```linescript
class User do
  declare name: str = "anon"
end

make_user(name: str) -> User do
  declare u = User()
  u.name = name
  return u
end

greet(u: User) -> str do
  return "hello, " + u.name
end

main() -> i64 do
  declare u: User = make_user("ava")
  println(greet(u))
  return 0
end
```

## Example 5: Narrower Field Types (`i32`, `f32`)

```linescript
class Particle do
  declare count: i32 = 0
  declare temp: f32 = 0.0

  tick() -> void do
    this.count += 1
    this.temp += 0.5
  end
end
```

## Example 6: Inheritance + Override + Static

```linescript
class Base do
  declare x: i64 = 0

  public fn constructor(seed: i64) do
    this.x = seed
  end

  public virtual fn value() -> i64 do
    return this.x
  end

  public static fn scale(v: i64) -> i64 do
    return v * 2
  end
end

class Derived extends Base do
  declare y: i64 = 0

  public fn constructor(a: i64, b: i64) : Base(a) do
    this.y = b
  end

  public override fn value() -> i64 do
    return this.x + this.y
  end
end
```

## Common Errors and What They Mean

- `class fields must be declared before methods`
 - you placed a field declaration after a method/constructor

- `duplicate class field 'x'`
 - same field name used twice

- `duplicate constructor for class 'X'`
 - class has more than one constructor

- `constructor return type is implicit`
 - constructor used `-> type`; remove it

- `class 'X' has no field 'y'`
 - unknown field access/assignment

- `class 'X' has no matching method 'm'`
 - method name exists but no overload matches call arity

- `override method 'm' has no base method to override`
 - `override` was used without a compatible base method

- `cannot override final base method`
 - derived class attempted to replace a `final` base method

- `static method 'm' must be called via class name`
 - static method called from an instance

## Current OOP Limits

- single inheritance only
- constructors are single-definition per class (no ctor overloading yet)
- class fields are currently handle-backed at runtime (`i64` under the hood)
- operator overloading currently supports unary (`-`, `!`) and binary arithmetic/comparison/logical operators

## Performance Notes

- class objects are handle-backed and compile down to native code paths
- for the hottest numeric kernels, plain locals/arrays are still usually best
- if a tiny method sits in a hot path, consider `inline` on that method

## See Also

- `docs/SYNTAX.md`
- `docs/LANGUAGE_GUIDE.md`
- `tests/cases/runtime/class_oop_cpp_style.lsc`
- `examples/oop_counter.lsc`
- `examples/oop_default_constructor.lsc`
- `examples/oop_typed_params.lsc`
