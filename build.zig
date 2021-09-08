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
    exe.addIncludeDir("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.18362.0\\um");
    exe.addIncludeDir("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.18362.0\\shared");
    exe.addIncludeDir("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.18362.0\\ucrt");
    exe.addIncludeDir("C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Tools\\MSVC\\14.14.26428\\include");
    //exe.addLibPath("C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64");
    //exe.addLibPath("C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x86");

    exe.install();
}
