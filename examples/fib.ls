inline fn max(a: i64, b: i64) -> i64 {
  if a > b {
    return a;
  } else {
    return b;
  }
}

fn fib(n: i64) -> i64 {
  if n < 2 {
    return n;
  }

  declare a: i64 = 0;
  declare b: i64 = 1;
  declare i: i64 = 2;

  while i <= n {
    declare next = a + b;
    a = b;
    b = next;
    i = i + 1;
  }

  return max(b, 0);
}

fn main() -> i64 {
  println(fib(45));
  return 0;
}
