# File da inserire nella build folder, per compilare ed eseguire il progetto
# Vai alla directory superiore
cd ..

# Configura il progetto con CMake
cmake -S . -B build

# Entra nella directory di build
cd build

# Compila il progetto con 'make' utilizzando 8 thread paralleli
make -j8

# Esegui l'applicazione SteeringSimulator
./SteeringSimulator
