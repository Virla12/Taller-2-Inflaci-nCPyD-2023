#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <mpi.h>

typedef struct {
  int day;
  int month;
  int year;
} fullDate;

typedef struct {
  int acumPrice;
  int q;
  float ipc;
} products;

typedef struct {
  int basePrice;
  std::string category;
  float februaryParticipation;
  float marchParticipation;
  float aprilParticipation;
  float mayParticipation;
  float juneParticipation;
  float julyParticipation;
  float augustParticipation;
  float septemberParticipation;
  float octoberParticipation;
  float novemberParticipation;
  float decemberParticipation;
} categorys;

std::list<std::string> categoryList;

bool findSubString(std::string stringToFind, std::string productName) {
  bool found = productName.find(stringToFind) != std::string::npos;
  return found;
}

fullDate separateDate(std::string date) {
  std::string day, month, year;
  fullDate dateShort;
  std::stringstream input_stringstream(date);
  getline(input_stringstream, day, '-');
  getline(input_stringstream, month, '-');
  getline(input_stringstream, year, 'T');
  dateShort.day = std::stoi(day);
  dateShort.month = std::stoi(month);
  dateShort.year = std::stoi(year);
  return dateShort;
}

void readFileCategoryList() {
  std::string category;
  std::ifstream file("categoryList.txt");
  if (file.fail()) {
    std::cout << "Archivo CATEGORY no encontrado" << std::endl;
  } else {
    while (getline(file, category)) {
      categoryList.push_back(category);
      std::cout << "Categoria: " << category << std::endl;
    }
  }
}

void calculateInflation(int rank, int size) {
  fullDate newDate;
  std::string words;
  products* data = new products[220];
  products** matrix = new products*[12];
  for (int i = 0; i < 12; i++) {
    matrix[i] = new products[221];
  }

  std::ifstream file("supermercado.csv");
  if (file.fail()) {
    std::cout << "Archivo no encontrado" << std::endl;
  } else {
    int lastMonth = 1;
    int lineCount = 0;
    while (getline(file, words)) {
      lineCount++;
      if (lineCount % size != rank) {
        continue;  // Skip lines not assigned to this process
      }

      std::stringstream input_stringstream(words);
      std::string SKU, name, price, discount, date, state;
      getline(input_stringstream, SKU, ';');
      getline(input_stringstream, name, ';');
      getline(input_stringstream, price, ';');
      getline(input_stringstream, discount, ';');
      getline(input_stringstream, date, ';');
      getline(input_stringstream, state, ';');
      std::cout << "SKU producto: " << SKU << std::endl;
      std::cout << "Nombre producto: " << name << std::endl;
      std::cout << "Precio producto: " << price << std::endl;
      std::cout << "Descuento producto: " << discount << std::endl;
      std::cout << "Fecha producto: " << date << std::endl;
      std::cout << "Estado producto: " << state << std::endl << std::endl;

      if (state == "AUTHORIZED" || state == "FINALIZED") {
        std::list<std::string>::iterator it;
        int i = 0;
        for (it = categoryList.begin(); it != categoryList.end(); ++it) {
          std::cout << name << " ...contiene... " << *it << "?"
                    << std::endl;
          if (findSubString(*it, name)) {
            newDate = separateDate(date);
            if (lastMonth != newDate.month) {
              for (int x = 0; x < 221; x++) {
                std::cout << "Mes " << lastMonth << ", Producto " << x << std::endl;
                std::cout << "Acumulador Precio: " << matrix[lastMonth - 1][x].acumPrice << std::endl;
                std::cout << "Cantidad: " << matrix[lastMonth - 1][x].q << std::endl;
                matrix[lastMonth - 1][x].acumPrice += matrix[lastMonth - 1][x].q > 0 ? matrix[lastMonth - 1][x].acumPrice / matrix[lastMonth - 1][x].q : 0;
                std::cout << "Nuevo Acumulador Precio: " << matrix[lastMonth - 1][x].acumPrice << std::endl;
                matrix[lastMonth - 1][x].ipc += matrix[lastMonth - 1][x].acumPrice;
                std::cout << "IPC: " << matrix[lastMonth - 1][x].ipc << std::endl;
              }
              lastMonth = newDate.month;
            }
            data[i].acumPrice =
                matrix[newDate.month - 1][i].acumPrice + std::stoi(price);
            data[i].q = matrix[newDate.month - 1][i].q + 1;
            matrix[newDate.month - 1][i] = data[i];
            i++;
          }
        }
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);  // Synchronize all processes

  // Reduce the data across all processes
  for (int i = 0; i < 12; i++) {
    MPI_Reduce(matrix[i], matrix[i], 221, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
  }

  // Cálculo de la inflación
  float inflation = 0.0;
  if (matrix[0][220].ipc != 0.0 && matrix[11][220].ipc != 0.0) {
    inflation = ((matrix[11][220].ipc / matrix[0][220].ipc) - 1) * 100;
  }

  // Process 0 prints the result
  if (rank == 0) {
    std::cout << "Inflación acumulada: " << inflation << "%" << std::endl;
  }

  delete[] data;
  for (int i = 0; i < 12; i++) {
    delete[] matrix[i];
  }
  delete[] matrix;
}

int main(int argc, char** argv) {
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    readFileCategoryList();
  }

  calculateInflation(rank, size);

  MPI_Finalize();
  return 0;
}
