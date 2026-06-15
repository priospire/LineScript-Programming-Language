const std = @import("std");

fn abs64(v: i64) i64 {
    return if (v < 0) -v else v;
}

fn gcd64(a_in: i64, b_in: i64) i64 {
    var a = abs64(a_in);
    var b = abs64(b_in);
    while (b != 0) {
        const t = @mod(a, b);
        a = b;
        b = t;
    }
    return a;
}

pub fn main() !void {
    const stdout = std.fs.File.stdout().deprecatedWriter();
    var a: i64 = 3_918_848;
    var b: i64 = 1_653_264;
    var sum: i64 = 0;
    var i: i64 = 1;
    while (i < 300_000) : (i += 1) {
        a += 17;
        b += 31;
        sum += gcd64(a, b);
    }
    try stdout.print("{d}\n", .{sum});
}
