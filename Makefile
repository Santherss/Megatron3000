# Nombre del ejecutable final
TARGET = disco

# Compilador y flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRCS = main.cpp discoVirtual.cpp gestionSectores.cpp menu.cpp
HEADERS = discoVirtual.h gestionSectores.h menu.h

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(OBJS) $(TARGET)

# Limpiar completamente
fclean: clean

# Atajo para recompilar todo
rebuild: fclean all

