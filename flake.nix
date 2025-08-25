{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };
  outputs =
    { nixpkgs, ... }:
    let
      pkgs = import nixpkgs {
        system = "x86_64-linux";
      };
    in
    {
      devShells.x86_64-linux.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          clang-tools
          llvmPackages_latest.clang
          pkg-config
          wayland
          cmake
          meson
          ninja
          libxkbcommon
          wlroots
          wayland-protocols
          scdoc
          pixman
          libGL
          systemd
          wayland-scanner
          xorg.xcbutilwm
          xorg.libX11
          wlr-protocols
        ];
      };
    };
}
