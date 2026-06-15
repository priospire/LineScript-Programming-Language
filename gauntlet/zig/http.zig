const std = @import("std");

fn readVals(out: []i64) void {
    var stdin_file = std.fs.File.stdin();
    var input_buf: [128]u8 = undefined;
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
    var vals: [2]i64 = undefined;
    readVals(vals[0..]);

    var port = vals[0];
    var requests = vals[1];
    if (port < 1) port = 19080;
    if (requests < 1) requests = 1;

    const addr = std.net.Address.parseIp4("127.0.0.1", @as(u16, @intCast(port))) catch {
        std.fs.File.stdout().deprecatedWriter().print("-1\n0\n", .{}) catch unreachable;
        return;
    };
    var server = addr.listen(.{
        .reuse_address = true,
        .kernel_backlog = 64,
    }) catch {
        std.fs.File.stdout().deprecatedWriter().print("-1\n0\n", .{}) catch unreachable;
        return;
    };
    defer server.deinit();

    std.fs.File.stdout().deprecatedWriter().print("READY\n", .{}) catch unreachable;

    const resp = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok";
    var handled: i64 = 0;
    var req_bytes: i64 = 0;
    var timer = std.time.Timer.start() catch unreachable;
    while (handled < requests) {
        var conn = server.accept() catch continue;
        defer conn.stream.close();

        var buf: [2048]u8 = undefined;
        const n = conn.stream.read(&buf) catch 0;
        if (n > 0) req_bytes += @as(i64, @intCast(n));
        conn.stream.writeAll(resp) catch {};
        handled += 1;
    }

    const elapsed_us: u64 = @divFloor(timer.read(), 1000);
    const checksum: i64 = handled;
    std.fs.File.stdout().deprecatedWriter().print("{d}\n{d}\n", .{ checksum, elapsed_us }) catch unreachable;
}
