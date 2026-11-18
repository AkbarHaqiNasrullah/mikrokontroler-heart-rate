#!/bin/sh
# Menjalankan semua uji di komputer (butuh g++). Jalankan dari folder utama repo:  sh tests/jalankan_tes.sh
set -e
mkdir -p build
g++ -std=c++11 -Wall -Wextra -o build/uji_logika tests/test_logika_bpm.cpp
./build/uji_logika
g++ -std=c++11 -Wall -Wextra -I tests/simulasi/stub -o build/simulasi tests/simulasi/simulasi.cpp
./build/simulasi
