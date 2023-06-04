#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

/*variables globales*/
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
  float novenberParticipation;
  float decemberParticipation;
} categorys;

std::vector<std::string> categoryVector;

/*Encontrar una subcadena en una cadena*/
bool findSubString(std::string stringToFind, std::string productName) {
  bool found = productName.find(stringToFind) != std::string::npos;
  return found;
}

/**/
fullDate separateDate(std::string date) {
  std::string day, month, year;
  fullDate dateShort;
  // day = date.substr(0, 4);
  // month = date.substr(5, 2);
  // year = date.substr(8, 2);
  std::stringstream input_stringstream(date);
  getline(input_stringstream, day, '-');
  getline(input_stringstream, month, '-');
  getline(input_stringstream, year, 'T');
  dateShort.day = std::stoi(day);
  dateShort.month = std::stoi(month);
  dateShort.year = std::stoi(year);
  return dateShort;
}

/*Leer ficheros*/
void readFileCategoryList() {
  std::string category;
  std::ifstream file("categoryList.txt");
  if (file.fail()) {
    std::cout << "Archivo CATEGORY no encontrado" << std::endl;
  } else {
    while (!file.eof()) {
      getline(file, category);
      categoryVector.push_back(category);
      std::cout << "Categoria: " << category << std::endl;
    }
  }
  file.close();
}

void readFileSupermercado() {
  fullDate newDate;
  std::string words;
  // products data [100];
  std::vector<products> data;
  data.resize(220);
  std::vector<categorys> dataAcum;
  dataAcum.resize(220);
  products p;
  int lastMonth = 1;
  int averagePrice = 0, total = 0;
  float percentPart = 0, summation = 0;
  products matrix[12][221];
  float ipc = 0;

  std::ifstream file("supermercado.csv");
  // Comprobar que el archivo exista
  if (file.fail()) {
    std::cout << "Archivo no encontrado" << std::endl;
  } else {
    while (!file.eof()) {
      getline(file, words);
      // Separar los elementos por ";" y guardarlo en variables temp
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
      // GUARDAR los datos en un lista; requisitos:
      // 1. Comprobar si su estado es: (1) AUTHORIZED o (2) FINALIZED
      if ((state.compare("AUTHORIZED") == 0) ||
          (state.compare("FINALIZED") == 0)) {
        // 2. Si un producto de la lista existe dentro de la lista categoría
        for (std::size_t i = 0; i < categoryVector.size(); i++) {
          // for (auto i = categoryVector.cbegin(); i != categoryVector.cend();
          // ++i) {
          std::cout << name << " ...contiene... " << categoryVector[i] << "?"
                    << std::endl;
          if ((findSubString(categoryVector[i], name)) == true) {
            // 3. Separar la fecha del la hora
            newDate = separateDate(date);
            // DE AQUI HASTA EL CIERRE DE ARCHIVO ESTA MALO >:(
            if (lastMonth != newDate.month) { // Paso a otro mes?
              for (int x = 0; x < 220; x++) {
                matrix[lastMonth - 1][x].acumPrice =
                    matrix[lastMonth - 1][x].acumPrice /
                    matrix[lastMonth - 1][x].q;
                // Hay caso base?
                if (lastMonth == 1){
                  //Mes base
                  ipc += (matrix[lastMonth - 1][x].acumPrice *
                        matrix[lastMonth - 1][x].q) /
                       (matrix[lastMonth - 1][x].acumPrice *
                        matrix[lastMonth - 1][x].q);
                } else {
                  //Siguientes meses
                  ipc += (matrix[lastMonth - 1][x].acumPrice *
                        matrix[lastMonth - 2][x].q) /
                       (matrix[lastMonth - 2][x].acumPrice *
                        matrix[lastMonth - 2][x].q);
                }
                // Si no
                if (x == 220) {
                  matrix[lastMonth-1][220].ipc = ipc * 100;//pendiente
                }
              }
              p.acumPrice = 0;
              p.q = 0;
              lastMonth = newDate.month; // 1 2
            }
            p.acumPrice =
                matrix[newDate.month - 1][i].acumPrice + std::stoi(price);
            p.q = matrix[newDate.month - 1][i].q + 1;
            matrix[newDate.month - 1][i] = p;
            // p.acumPrice = data[i].acumPrice + std::stoi(price);
            // p.q = data[i].q + 1;
            // data[i] = p;
            total += std::stoi(price); // x
            // Del mes 1 al mes 12
            // Esta dentro del mes i? (ej.: 1)
            // Contabilizar los productos de canasta basica al mes, contador++
            // Contabilizar los productos por categoria
            // Acumular precio de categoria
            // Acumular precio de prouctos
            // Participación del producto en la canasta básica -> Obtener el
            // precio promedio del producto --> % de aumento mensual ---> IPC
            // acumulado = Inflación
          }
        }
      }
    }
  }
  file.close();
}

int main() { readFileSupermercado(); }