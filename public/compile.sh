if [ -z $(which tar) ]; then
  echo "Need to install tar";
  sudo apt-get install tar
fi

if [ -z $(which unzip) ]; then
  echo "Need to install unzip";
  sudo apt-get install unzip
fi

if [ -z $(which wget) ]; then
  echo "Need to install wget";
  sudo apt-get install wget
fi

if [ -z $(which ninja) ]; then
  echo "Need to install Ninja";
  sudo apt-get install ninja-build
fi

if [ "$1" = "release" ]; then
  OptimisationLevel=3
fi

emsdkPresence=$(ls | grep 'emsdk' | wc -l)

if [[ $emsdkPresence -eq 0 ]]
then
  echo "Setting up Emscripten..."
  git clone https://github.com/emscripten-core/emsdk.git >/dev/null 2>&1
  cd emsdk
  ./emsdk install latest >/dev/null 2>&1
  ./emsdk activate latest >/dev/null 2>&1
  source ./emsdk_env.sh >/dev/null 2>&1
  cd ..
fi

includeLibrariesPresence=$(ls ./include | grep 'RETRIEVED_FILES_DO_NOT_DELETE' | wc -l)

if [[ $includeLibrariesPresence -eq 0 ]]
then
  mkdir libs >/dev/null 2>&1
  cd include
  for script in *.sh
  do
    ./$script
  done
  touch "RETRIEVED_FILES_DO_NOT_DELETE"
  cd ..
  echo "Beginning compilation..."
fi

source ./emsdk/emsdk_env.sh >/dev/null 2>&1
#will compile all cpp files in current directory and subdirectories
TIME=$(date +%s%3N)
cd ..
emcmake cmake -S . -B build -GNinja -DOptimisationLevel=$OptimisationLevel >/dev/null 2>&1
cd build
ninja
mv OpenGL_WASM_Project.* ../public >/dev/null 2>&1
cd ..
echo "Done in $(($(date +%s%3N) - $TIME))ms";
