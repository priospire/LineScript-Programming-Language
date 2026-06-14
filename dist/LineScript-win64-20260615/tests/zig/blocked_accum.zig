const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    const n: i64 = 8_000_000;
    var s0: i64 = 0;
    var s1: i64 = 0;
    var s2: i64 = 0;
    var s3: i64 = 0;
    var base: i64 = 0;
    while (base < n) : (base += 4) {
        s0 += base;
        s1 += base + 1;
        s2 += base + 2;
        s3 += base + 3;
    }
    try stdout.print("{d}\n", .{s0 + s1 + s2 + s3});
}
