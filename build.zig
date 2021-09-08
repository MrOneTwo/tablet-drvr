const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const exe = b.addExecutable("tablet-driver", null);

    exe.addCSourceFile("code/main.cpp", &[_][]const u8 {});
    exe.addCSourceFile("code/storage.cpp", &[_][]const u8 {});
    exe.addIncludeDir("code");

    exe.linkLibC();

    exe.linkSystemLibrary("winmm");
    exe.linkSystemLibrary("setupapi");
    exe.linkSystemLibrary("winusb");
    exe.linkSystemLibrary("ntdll");
    exe.linkSystemLibrary("hid");

    exe.install();
}
