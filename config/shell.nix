{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
  pkgs.conan
  pkgs.ncurses
  pkgs.python312Full
  pkgs.clang-tools
  pkgs.codespell
  ];

  shellHook = ''
    source ${toString ./setup.sh}
    alias t=task
    export TERMINFO=/usr/share/terminfo
  '';
}

