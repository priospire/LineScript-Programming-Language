const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    var a: i64 = 0;
    var b: i64 = 1;
    var c: i64 = 2;
    var d: i64 = 3;
    var i: i64 = 0;
    while (i < 1_200_000) : (i += 1) {
        a += i;
        b += (i * 3) + 1;
        c += (i * 5) - 7;
        d += (i * i) + 11;
    }
    try stdout.print("{d}\n", .{a + b + c + d});
}
