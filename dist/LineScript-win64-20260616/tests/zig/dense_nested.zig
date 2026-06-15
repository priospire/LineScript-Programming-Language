const std = @import("std");

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    var sum: i64 = 0;
    var i: i64 = 0;
    while (i < 700) : (i += 1) {
        var j: i64 = 0;
        while (j < 700) : (j += 1) {
            sum += (i * 13) + (j * 17) - (i * 3) + (j * 5);
        }
    }
    try stdout.print("{d}\n", .{sum});
}
