const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    var sum: i64 = 0;
    var i: i64 = 0;
    while (i < 8_000_000) : (i += 1) {
        if (@mod(i, 2) == 0) {
            sum += i;
        } else {
            sum -= i;
        }
    }
    try stdout.print("{d}\n", .{sum});
}
