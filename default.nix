{ stdenv, lib, qtbase, wrapQtAppsHook }: 

stdenv.mkDerivation {
  pname = "hsd";
  version = "0.6.0";
  
  buildInputs = [ qtbase qtnetwork ];
  nativeBuildInputs = [ wrapQtAppsHook ]; 
}