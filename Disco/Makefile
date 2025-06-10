# Nombre del compilador y flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

# Directorios
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# Archivos fuente y objetos
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Ejecutable final
TARGET = DISCO

# Regla principal
all: $(TARGET)

# Enlazado final
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilar cada .cpp en .o sin requerir header
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Crear carpeta obj si no existe
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpiar todo
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean

