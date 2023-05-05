#include <iostream>
#include <string>

#include <fstream>
#include <sstream>
#include <vector>

#include <chrono>
#include <ctime> 

#include <bits/stdc++.h>
#include <array>

using namespace std;

enum direction {d_down, d_right, none};

#define COORD std::pair<int, int>

//#define DEBUG

int iter = 0;

bool stopFlag = false;

///Auxiliary functions

void display_arr(int* arr, int n){

  cout << "arr: ";

  for(int i = 0; i < n; i++){
    cout << arr[i] << " ";
  }

  cout << endl;
  
}

void print_coords(COORD start, COORD end){

  cout << "Start:" << start.first << "," << start.second << endl;
  cout << "End:" << end.first << "," << end.second << endl;
  
}

int find_length(COORD start, COORD end, direction dir){

  if(dir == d_down)
    return end.first - start.first;
  if(dir == d_right)
    return end.second - start.second;

  return -1;
}

void convert_sol(int** mat, int** &sol_mat, int m, int n){

  sol_mat = new int*[m]; //Rows
  for(int i = 0; i < m; i++){
    sol_mat[i] = new int[n]; //Cols
  }

  for(int i = 0; i < m; i++){
    for(int j = 0; j < m; j++){
      if(mat[i][j] == -2)
	sol_mat[i][j] = -2; //Empty value cell
      else
	sol_mat[i][j] = -1; //Hint or empty cell
    }
  }
}

void print_one_matrix(int** matrix, int m, int n){
  std::cout << "Matrix: " << std::endl;
  for(int i = 0; i < m; i++){ //rows
    for(int j = 0; j < n; j++){ //cols
      std::cout << matrix[i][j] << "\t";
    }
    std::cout << "\n";
  }
}

void sol_to_file(int** mat, int** sol_mat, int m, int n, string fname){

  //string fname = "visualize.kakuro";
  ofstream to_write("visualize.kakuro");

  to_write << m << " " << n << "\n";

  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      if(mat[i][j] != -2)
	to_write << mat[i][j] << " ";
      else
	to_write << sol_mat[i][j] << " ";
    }
    to_write << "\n";
  }

  to_write.close();
}

void read_matrix(int** &matrix, std::ifstream &afile, int m, int n){

  matrix = new int*[m]; //rows

  for(int i = 0; i < m; i++){
    matrix[i] = new int[n]; //cols
  }

  int val;
  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      afile >> val;
      matrix[i][j] = val;
    }
  }
}

///Auxiliary functions

struct sum{
  COORD start;
  COORD end;

  int hint;
  int dir;
  int length;
  int* arr;

  void print_sum(){
    cout << "############################" << endl;
    cout << "Creating sum with: " << endl;
    print_coords(start, end);
    cout << "Hint: " << hint << endl;
    cout << "Direction: " << dir << endl;
    cout << "Length: " << length << endl;
    cout << "############################" << endl;
  }
  
  sum(COORD _start, COORD _end, int _hint, direction _dir):
    start(_start), end(_end), hint(_hint), dir(_dir)
  {
    length = find_length(_start, _end, _dir);
    arr = new int[length];
    #ifdef DEBUG
    cout << "############################" << endl;
    cout << "Creating sum with: " << endl;
    print_coords(start, end);
    cout << "Hint: " << hint << endl;
    cout << "Direction: " << dir << endl;
    cout << "Length: " << length << endl;
    cout << "############################" << endl;
    #endif
  }
  
  //~sum(){
  //delete arr;
  //}
};


COORD find_end(int** matrix, int m, int n, int i, int j, direction dir){ //0 down 1 right

  if(dir == d_right){
    for(int jj = j+1; jj < n; jj++){
      if(matrix[i][jj] != -2 || jj == n - 1){
	if(matrix[i][jj] == -2 && jj == n -1)
	  jj++;
	COORD END = COORD(i, jj);
	return END;
      }
    }
  }

  if(dir == d_down){
    for(int ii = i+1; ii < m; ii++){
      if(matrix[ii][j] != -2 || ii == m - 1){
	if(matrix[ii][j] == -2 && ii == m - 1)
	  ii++;
	COORD END = COORD(ii, j);
	return END;
      }
    }
  }
  
}


vector<sum> get_sums(int** matrix, int m, int n){

  vector<sum> sums;
  
  for(int i = 0; i < m; i++){
    for(int j = 0; j < n; j++){
      int val = matrix[i][j];
      if(val != -1 && val != -2){
	int hint = val;
	hint = hint / 10;

	if((hint%100) == 0){
	  hint = (int)(hint/100);
	  COORD START = COORD(i, j+1); 
	  COORD END = find_end(matrix, m, n, i, j, d_right);
	  sum _sum = sum(START, END, hint, d_right);
	  sums.push_back(_sum);
	}

	else{
	  int div = (int)(hint/100);
	  int rem = (int)(hint%100);
   
	  if(div == 0 && rem != 0){
	    COORD START = COORD(i+1,j);
	    COORD END = find_end(matrix, m, n, i, j, d_down);
	    sum _sum = sum(START, END, rem, d_down);
	    sums.push_back(_sum);
	  }

	  if(div != 0 && rem != 0){
	    COORD START1 = COORD(i+1,j);
	    COORD START2 = COORD(i,j+1);
	    COORD END1 = find_end(matrix, m, n, i, j, d_down);
	    COORD END2 = find_end(matrix, m, n, i, j, d_right);
	    sum _sum1 = sum(START1, END1, rem, d_down);
	    sum _sum2 = sum(START2, END2, div, d_right);
	    sums.push_back(_sum1);
	    sums.push_back(_sum2);
	  }
	}
      }

      
    }
  }
  return sums;
}


int max(int a, int b) {
  if(a>b) {
    return a;
  } else {
    return b;
  }
}
bool checkRows(int **mat, int m, int n) {
  //bool check = false;
  int sum;
  for(int i=1; i<m ; i++) {
    sum=0;
    for(int j=1; j<n ; j++) {
      sum += mat[i][j];
    }
    //cout << "sum: " << sum << ", value to reach:" << mat[i][0]/10/100 << endl;
    if(sum != mat[i][0]/10/100) {
      return false;
    } else {
      //cout << "OK" << endl;
    }
  }
  return true;
}
bool checkCols(int **mat, int m, int n) {
  //bool check = false;
  for(int i=1; i<m ; i++) {
    int sum=0;
    for(int j=1; j<n ; j++) {
      sum += mat[j][i];
    }
    if(sum != mat[0][i]/10%100) {
      return false;
    }
  }
  return true;
}

bool checkSol(int arr[], int** mat, int m) {
  int col[m-1];
  int row[m-1];
  for(int i=1; i<m ; i++) {
    row[i-1] = mat[0][i]/10/100;
    //cout << mat[i][0]/10/100 << endl;
  }
  for(int i=1; i<m ; i++) {
    col[i-1] = mat[0][i]/10%100;
    //cout << mat[0][i]/10%100 << endl;
  }
  int count = 0;
  for(int i=1; i<m ; i++) {
    for(int k=1; k<m ; k++) {
      mat[i][k] = arr[count];
      count++;
    }
  }
  //print_one_matrix(mat, m, m);
  if(checkCols(mat,m,m) && checkRows(mat,m,m)) {
    return true;
  } else {
    return false;
  }
}

void combinations(int n, int k, int step, int arr[], int** mat, int** sol_mat, int m) {
    if(stopFlag) {
      return;
    }
    if (step == n) {
        
        return;
    }
    for (int i = 1; i <= k; i++) {
        arr[step] = i;
        //if(checkCols(mat, 3,3) && checkRows(mat,3,3))
        if(!checkSol(arr, mat, m)) {
          /*
        }
        if(!(arr[0] == 5 && arr[1] == 4 && arr[2] == 2 && arr[3] == 2)) {*/
          combinations(n, k, step + 1, arr, mat, sol_mat, m);
        } else {
          stopFlag = true;

          int count = 0;
          #pragma omp parallel for shared(mat, sol_mat, stopFlag)
          for(int i=1; i<m ; i++) {
            for(int k=1; k<m ; k++) {
              sol_mat[i][k] = arr[count];
              //cout << mat[i][k] << endl;
              count++;
            }
          }

          print_one_matrix(sol_mat, m, m);
          sol_to_file(mat, sol_mat, m, m, "solution.kakuro");

          cout << "Combination found!!" << endl;
          for (int i = 0; i < n; i++) {
            cout << arr[i] << " ";
          }
          cout << endl;
          return;
        }
        
    }
}

//TO DO: Write the solution
//You can use any algorithm and data type
//Write your solution to file in main function using sol_to_mat() after solving it
bool solution(int** mat, int** sol_mat, vector<sum> sums, int m, int n){
  int size = ((m-1)*(n-1)); // size of the values matrix
  int counter[size];
  for(int i=0; i<size; i++) {
    counter[i] = 1;
  }
  int max_value = 0;
  for(int i=0; i<m ; i++) {
    max_value = max(max_value, mat[i][0]/10/100);
    max_value = max(max_value, mat[i][0]/10%100);
  }
  //cout << "Max: " << max_value << endl;
  
  int r = size;
  //int k = max_value-1;
  int k = 9;
  int step = 0;
  int arr[r];
  combinations(r, k, step, arr, mat, sol_mat, m);
}

int main(int argc, char** argv){
  
  std::string filename(argv[1]);
  std::ifstream file;
  file.open(filename.c_str());

  int m, n;
  file >> m;
  file >> n;

  int** mat;
  read_matrix(mat, file, m, n);
  //print_one_matrix(mat, m, n);
  
  int** sol_mat;
  convert_sol(mat, sol_mat, m, n);
  //print_one_matrix(sol_mat, m, n);
  
  vector<sum> sums = get_sums(mat, m, n);

  auto start = std::chrono::system_clock::now();
  solution(mat, sol_mat, sums, m, n);
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
  

  //solution(mat, sol_mat, sums, m, n);
  
  
  //int arr[m];
  //checkSol(arr, mat, m);
  
  /*
  mat[1][1] = 5;
  mat[1][2] = 4;
  mat[2][1] = 2;
  mat[2][2] = 2;
  cout << checkRows(mat, m, m) << endl;
  cout << m << endl;
  */
  for(int i = 0; i < n; i++){
    delete mat[i];
    delete sol_mat[i];
  }

  delete mat;
  delete sol_mat;
  
  return 0;
}
