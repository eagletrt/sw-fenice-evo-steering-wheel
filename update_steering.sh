#! /usr/bin/env bash

cd steering/simulator/steering
cp steering.h ../../../Core/Inc/steering
cp steering.c ../../../Core/Src/steering

cd ui
cp *.h ../../../../Core/Inc/steering
cp *.c ../../../../Core/Src/steering
cd ..

cd fonts
cp *.c ../../../../Core/Src/fonts
