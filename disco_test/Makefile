# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN = disco

# Archivos fuente y objetos
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Regla principal
all: $(BIN)

# Compilación final
$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilar cada archivo cpp a obj
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crear directorio de objetos si no existe
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpiar archivos compilados
clean:
	rm -rf $(OBJ_DIR) $(BIN)

# Limpiar solo ejecutable
clean-bin:
	rm -f $(BIN)

# Limpiar solo objetos
clean-obj:
	rm -rf $(OBJ_DIR)

.PHONY: all clean clean-bin clean-obj

