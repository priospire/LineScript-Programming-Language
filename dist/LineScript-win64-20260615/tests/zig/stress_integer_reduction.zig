const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    const n: i64 = 10_000_000;
    var sum: i64 = 0;
    var i: i64 = 0;
    while (i < n) : (i += 1) {
        sum += i;
    }
    try stdout.print("{d}\n", .{sum});
}
