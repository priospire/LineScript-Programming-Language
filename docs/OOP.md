# LineScript OOP Guide

This document is the complete Object-Oriented Programming reference for LineScript's current class system.

## What OOP Means in LineScript

LineScript supports C++-style class syntax:
- typed fields
- constructor
- instance methods
- `this` inside class members
- member access and member assignment

Under the hood, class instances are handle-backed (`i64`) objects. That keeps syntax high-level while preserving speed-focused native compilation.

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

Rules:
- duplicate method names in the same class are not allowed
- method overloading by parameter list is not supported yet

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

Supported member writes include normal and compound assignments (`=`, `+=`, `-=`, `*=`, `/=`, `%=` and power-assign forms where valid).

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

- `class 'X' has no method 'm'`
 - unknown method call

## Current OOP Limits

Not currently built in:
- inheritance
- virtual dispatch / polymorphic method override
- access modifiers (`public/private/protected`)
- static class methods/properties
- method overloading

These are planned-style capabilities, but not part of the current compiler behavior.

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
