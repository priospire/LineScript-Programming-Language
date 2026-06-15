const std = @import("std");

const Ctx = struct { chunk: i64, rounds: i64, offset: i64 };

fn readVals(out: []i64) void {
    var stdin_file = std.fs.File.stdin();
    var input_buf: [256]u8 = undefined;
    const n = stdin_file.readAll(&input_buf) catch 0;
    var it = std.mem.tokenizeAny(u8, input_buf[0..n], " \r\n\t");
    var i: usize = 0;
    while (i < out.len) : (i += 1) {
        if (it.next()) |tok| out[i] = std.fmt.parseInt(i64, tok, 10) catch 0 else out[i] = 0;
    }
}

fn worker(ctx: *Ctx) void {
    const n_usize: usize = @intCast(ctx.chunk);
    var allocator = std.heap.page_allocator;
    var buf = allocator.alloc(i64, n_usize) catch return;
    defer allocator.free(buf);

    var r: i64 = 0;
    while (r < ctx.rounds) : (r += 1) {
        const base = @mod(r * 17 + ctx.offset * 13 + 97, 1000003);
        var i: i64 = 0;
        while (i < ctx.chunk) : (i += 1) {
            const idx: usize = @intCast(i);
            buf[idx] = @mod(i * 19 + base + 31, 1000003);
        }
    }
}

pub fn main() void {
    var vals: [4]i64 = undefined;
    readVals(vals[0..]);
    const seed = vals[0];
    var workers = vals[1];
    var chunk = vals[2];
    var rounds = vals[3];
    if (workers < 1) workers = 1;
    if (workers > 4) workers = 4;
    if (chunk < 1) chunk = 1;
    if (rounds < 1) rounds = 1;

    var threads: [4]std.Thread = undefined;
    var ctxs: [4]Ctx = undefined;

    var timer = std.time.Timer.start() catch unreachable;
    var t: i64 = 0;
    while (t < workers) : (t += 1) {
      const idx: usize = @intCast(t);
      ctxs[idx] = .{ .chunk = chunk, .rounds = rounds, .offset = t * 7 + 3 };
      threads[idx] = std.Thread.spawn(.{}, worker, .{&ctxs[idx]}) catch unreachable;
    }
    t = 0;
    while (t < workers) : (t += 1) {
      const idx: usize = @intCast(t);
      threads[idx].join();
    }

    const elapsed_us: u64 = @divFloor(timer.read(), 1000);
    const checksum: i64 = @mod(seed, 1000003) + workers + chunk + rounds;
    std.fs.File.stdout().deprecatedWriter().print("{d}\n{d}\n", .{checksum, elapsed_us}) catch unreachable;
}
