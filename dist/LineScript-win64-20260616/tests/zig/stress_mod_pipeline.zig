const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    const limit: i64 = 4_850_001;
    var acc: i64 = 0;
    var i: i64 = 1;
    while (i < limit) : (i += 1) {
        const t = @mod(i * 3, 97);
        acc += t;
    }
    try stdout.print("{d}\n", .{acc});
}
