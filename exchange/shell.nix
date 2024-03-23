{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.python312Full
    pkgs.python312Packages.cppy
    pkgs.ncurses
  ];

  shellHook = ''
    export PYTHONPATH=$PYTHONPATH:${pkgs.python312Packages.cppy}/lib/python3.12/site-packages
  '';
}

