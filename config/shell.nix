{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [ 
  pkgs.conan
  pkgs.ncurses
  pkgs.python312Full
  pkgs.python312Packages.cppy
  pkgs.python312Packages.matplotlib
  pkgs.python312Packages.numpy
  pkgs.python312Packages.yfinance
  pkgs.python312Packages.pandas
  pkgs.python312Packages.scikit-learn
  pkgs.clang-tools
  pkgs.codespell
  ];

  shellHook = ''
    source ${toString ./setup.sh}
    alias t=task
    export TERMINFO=/usr/share/terminfo
  '';
}

