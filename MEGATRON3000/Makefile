# Makefile para MEGATRON3000

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g -fsanitize=address

# Carpetas
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DATA_DIR = data
SCHEMA_DIR = schema

# Ejecutable
TARGET = megatron3000

# Buscar todos los archivos .cpp en src/
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Generar objetos en build/ con mismo nombre que fuente
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Regla por defecto
all: $(BUILD_DIR) $(TARGET)

# Crear carpeta build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar cada .cpp a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linkear objetos para crear ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Limpiar archivos compilados y ejecutable
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean

