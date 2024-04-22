# Version: 1.0
# File di script per la compilazione del progetto SteeringSimulator
rm -r build

# Configura il progetto con CMake
cmake -S . -B build

# Entra nella directory di build
cd build

# Compila il progetto con 'make' utilizzando 8 thread paralleli
make -j8

# Esegui l'applicazione SteeringSimulator
./SteeringSimulator
