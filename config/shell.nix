let
  nixpkgs = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz";
  }) {};
in
nixpkgs.mkShell {
  buildInputs = [ 
  nixpkgs.conan
  nixpkgs.ncurses
  nixpkgs.python312Full
  nixpkgs.python312Packages.cppy
  nixpkgs.python312Packages.matplotlib
  nixpkgs.python312Packages.numpy
  nixpkgs.python312Packages.keras
  nixpkgs.python312Packages.yfinance
  nixpkgs.python312Packages.pandas
  nixpkgs.python312Packages.scikit-learn
  nixpkgs.clang-tools
  nixpkgs.codespell
  ];

  shellHook = ''
    source ${toString ./setup.sh}
    alias t=task
    export TERMINFO=/usr/share/terminfo
  '';
}

