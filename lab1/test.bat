echo off

git pull origin main

mkdir build 
cd build
cmake ..
cmake --build .
./lab.exe

pause