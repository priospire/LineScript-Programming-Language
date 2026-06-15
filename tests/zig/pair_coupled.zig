const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    var x: i64 = 1;
    var y: i64 = 2;
    var i: i64 = 0;
    while (i < 200_000) : (i += 1) {
        x += y;
        y += (i * 2) + 3;
    }
    try stdout.print("{d}\n", .{x + y});
}
