const std = @import("std");

fn readVals(out: []i64) void {
    var stdin_file = std.fs.File.stdin();
    var input_buf: [512]u8 = undefined;
    const n = stdin_file.readAll(&input_buf) catch 0;
    var it = std.mem.tokenizeAny(u8, input_buf[0..n], " \r\n\t");
    var i: usize = 0;
    while (i < out.len) : (i += 1) {
        if (it.next()) |tok| {
            out[i] = std.fmt.parseInt(i64, tok, 10) catch 0;
        } else {
            out[i] = 0;
        }
    }
}

pub fn main() void {
    var vals: [7]i64 = undefined;
    readVals(vals[0..]);
    const seed = vals[0];
    const mode = vals[1];
    var n = vals[2];
    const p1 = vals[3];
    const p2 = vals[4];
    const p3 = vals[5];
    const p4 = vals[6];
    if (n < 1) n = 1;

    var checksum: i64 = 0;
    var timer = std.time.Timer.start() catch unreachable;

    if (mode == 0) {
        var modv = p4;
        if (modv < 2) modv = 2;
        if (p1 == 0) {
            var i: i64 = 0;
            while (i < n) : (i += 1) checksum += @mod(i * p2 + p3 + seed, modv);
        } else if (p1 == 1) {
            var i: i64 = 0;
            while (i < n) : (i += 1) {
                if (@mod(i + seed, 2) == 0) checksum += @mod(i * p2 + 7 + seed, modv)
                else if (@mod(i + seed, 3) == 0) checksum += @mod(i * p3 + 11 + seed, modv)
                else checksum += @mod(i * (p2 + p3 + 1) + 13 + seed, modv);
            }
        } else {
            var a: i64 = 3;
            var b: i64 = 5;
            var c: i64 = 7;
            var i: i64 = 0;
            while (i < n) : (i += 1) {
                const nxt = @mod(a * 13 + b * 17 + c * 19 + i + seed + p2, modv);
                a = b; b = c; c = nxt;
                checksum += nxt;
            }
        }
    } else if (mode == 1) {
        var modv = p4;
        if (modv < 2) modv = 2;
        const n_usize: usize = @intCast(n);
        var allocator = std.heap.page_allocator;
        var buf = allocator.alloc(i64, n_usize) catch {
            std.fs.File.stdout().deprecatedWriter().print("-1\n0\n", .{}) catch unreachable;
            return;
        };
        defer allocator.free(buf);

        if (p1 == 0) {
            var i: i64 = 0;
            while (i < n) : (i += 1) {
                const idx: usize = @intCast(i);
                buf[idx] = @mod(i * p2 + p3 + seed, modv);
            }
            i = 0;
            while (i < n) : (i += 1) {
                const idx: usize = @intCast(i);
                checksum += buf[idx];
            }
        } else {
            var stride = p2;
            if (stride < 1) stride = 1;
            var passes = p3;
            if (passes < 1) passes = 1;
            var i: i64 = 0;
            while (i < n) : (i += 1) {
                const idx: usize = @intCast(i);
                buf[idx] = @mod(i + seed, modv);
            }
            var p: i64 = 0;
            while (p < passes) : (p += 1) {
                i = 0;
                while (i < n) : (i += stride) {
                    const idx: usize = @intCast(i);
                    const nxt = @mod(buf[idx] * (p4 + 3) + p + seed, modv);
                    buf[idx] = nxt;
                    checksum += nxt;
                }
            }
        }
    } else {
        var modv = p4;
        if (modv < 2) modv = 2;
        if (p1 == 0) {
            var i: i64 = 0;
            while (i < n) : (i += 1) {
                checksum += @mod(i * 7 + seed, modv);
                checksum += @mod(i * 11 + p2, modv);
            }
        } else if (p1 == 1) {
            var outer = p2;
            if (outer < 1) outer = 1;
            var inner = p3;
            if (inner < 1) inner = 1;
            var i: i64 = 0;
            while (i < outer) : (i += 1) {
                const base = @mod(i * 13 + seed, modv);
                var j: i64 = 0;
                while (j < inner) : (j += 1) checksum += @mod(base + j * 17, modv);
            }
        } else {
            const n_usize: usize = @intCast(n);
            var allocator = std.heap.page_allocator;
            var buf = allocator.alloc(i64, n_usize) catch {
                std.fs.File.stdout().deprecatedWriter().print("-1\n0\n", .{}) catch unreachable;
                return;
            };
            defer allocator.free(buf);

            var i: i64 = 0;
            while (i < n) : (i += 1) {
                const idx: usize = @intCast(i);
                buf[idx] = @mod(i + seed, modv);
            }
            var p: i64 = 0;
            while (p < 8) : (p += 1) {
                i = 0;
                while (i < n) : (i += 3) {
                    const idx: usize = @intCast(i);
                    const nxt = @mod(buf[idx] * 19 + p + seed, modv);
                    buf[idx] = nxt;
                    checksum += nxt;
                }
            }
        }
    }

    const elapsed_us: u64 = @divFloor(timer.read(), 1000);
    std.fs.File.stdout().deprecatedWriter().print("{d}\n{d}\n", .{checksum, elapsed_us}) catch unreachable;
}
