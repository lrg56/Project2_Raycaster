/*
 *File: raycast.c
 *Author: Logan Green
 *Created: September 30, 2016
 *FINAL: October 4, 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
//Defining the Object
typedef struct Object{
  int kind;
  double color[3];
  union{
    struct plane{
      double color[3];
      double position[3];
      double normal[3];
    }
    struct radius{
      double color[3];
      double position[3];
      int radius;
    }
    struct camera{
      double width;
      double height;
    }
  }
}Object;
//Defining a Pixel
typedef struct Pixel{
  unsigned char r, g, b;
}Pixel;

static inline double sqr(double v){
  return v*v;
}

static inline void normalize(double* v){
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));

  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}
//initialize 
Object** parseScene(char* input);
int nextChar(FILE* json);
int getC(FILE* json);
int checkNextChar(FILE* json, int val);
char* nextString(FILE* json);
char* checkNextString(FILE* json, char* value);
double* nextVector(FILE* json);
double nextNumber(FILE* json);
Pixel* raycast(Object** objects, int pW, int pH);
int planeIntersect(Object* object, double* Ro, double* Rd);
int imageWrite(Pixel* image, char* input, int pW, int pH);
int line = 1;
//parse the JSON file
Object** parseScene(char* input){
  int c;
  int objectI = 0;
  Object** objects;
  objects = malloc(sizeof(Object*)*128);

  FILE* json = fopen(input, "r");

  if(json == NULL){
    fprintf(stderr, "Unable to open file\n");
    exit(1);
  }
  //what the first char is according to the format
  checkNextChar(json, '[');

  while(1){
    c = getC(json);
    //end of file, at this point it would mean it is empty
    if(c == ']'){
      fprintf(stderr, "The file is empty\n");
      fclose(json);
      exit(1);
    }
    
    if(c == '{'){
      checkNextString(json, "type");
      checkNextChar(json, ':');
      char* value = nextString(json);
      objects[objectI] = malloc(sizeof(Object));
      if(strcmp(value, "plane") == 0){
	objects[objectI]->kind = 0;
      }else if(strcmp(value, "sphere") == 0){
	objects[objectI]->kind = 1;
      }else if(strcmp(value, "camera") == 0){
	objects[objectI]->kind = 2;
      }else{
	fprintf(stderr, "Unkown type: should be a camera, sphere, or plane\n");
	fclose(json);
	exit(1);
      }

      while(1){
	c = nextChar(json);
	if(c == '}'){
	  objectI++;
	  break;
	}else if(c == ','){
	  char* key = nextString(json);
	  checkNextChar(json, ':');
	  if(strcmp(key, "width") == 0){
	    double value = nextNumber(json);
	    if(objects[objectI]->kind == 2){
	      objects[objectI]->camera.width = value;
	    }else{
	      fprintf(stderr, "Not a proper value\n");
	      fclose(json);
	      exit(1);
	    }
	  }else if(strcmp(key, "height") == 0){
	    double value = nextNumber(json);
	    if(objects[objectI]->kind == 2){
	      objects[objectI]-.camera.height = value;
	    }else{
	      fprinf(stderr, "Not a proper value\n");
	    }
	  }else if(strcmp(key, "radius") == 0){
	    double value = nextNumber(json);
	    if(objects[objectI]->kind == 1){
	      objects[objectI]->sphere.radius = value;
	    }else{
	      fprintf(stderr, "Not a proper value\n");
	      fclose(json);
	      exit(1);
	    }
	  }else if(strcmp(key, "color") == 0){
	    double* value = nextVector(json);
	    if(objects[objectI]->kind == 1){
	      for(int i = 0; i < 3; i++){
		objects[objectI]->sphere.color[i] = value[i];
	      }
	    }else if(objects[objectI]->kind == 0){
	      for(int i = 0; i < 3; i++){
		objects[objectI]->plane.color[i] = value[i];
	      }
	    }else{
	      fprintf(stderr, "Not a proper value\n");
	      fclose(json);
	      exit(1);
	    }
	  }else if(strcmp(key, "position") == 0){
	    double* value = nextVector(json);
	    if(objects[objectI]->kind == 1){
	      for(int i = 0; i < 3; i++){
		objects[objectI]->sphere.position[i] = value[i];
	      }
	    }else if(objects[objectI]->kind == 0){
	      for(int i = 0; i < 3; i++){
		objects[objectI]->plane.position[i] = value[i];
	      }
	    }else{
	      fprintf(stderr, "Not a proper value\n");
	      fclose(json);
	      exit(1);
	    }
	  }else if(strcmp(key, "normal") == 0){
	    double* value = nextVector(json);
	    if(objects[objectI]->kind == 0){
	      for(int i = 0; i < 3; i++){
		objects[objectI]->plane.normal[i] = value[i];
	      }
	    }else{
	      fprinf(stderr, "Not a proper value\n");
	      fclose(json);
	      exit(1);
	    }
	  }else{
	    fprintf(stderr, "Not a proper value\n");
	    fclose(json);
	    exit(1);
	  }
	}else{
	  fprintf(stderr, "Unexpected value\n");
	  fclose(json);
	  exit(1);
	}
      }
      c = nextChar(json);
      if(c == ']'){
	objects[objectI] = NULL;
	fclose(json);
	return objects;
      }else{
	fprintf(stderr, "expected ']'\n");
	fclose(json);
	exit(1);
      }
    }
  }
}

int nextChar(FILE* json){
  int c = getC(json);
  while(isspace(c)){
    c = getC(json);
  }
  return c;
}

int checkNextChar(FILE* json, int val){
  int c = nextChar(json);
  if(c == val){
    return c;
  }else{
    fprintf(stderr, "Expected proper value");
    fclose(json);
    exit(1);
  }
}
    
      
int getC(FILE* json){
  int c = fgetc(json);

  if(c == '\n'){
    line += 1;
  }
  if(c == EOF){
    fprintf(stderr, "end of file, may be missing information\n");
    fclose(json);
    exit(1);
  }
  return c;
}

char* nextString(FILE* json){
  char buffer[128];
  int c = checkNextChar(json, '"');
  c = nextChar(json);
  int i = 0;
  while(c != '"'){
    if(i > 128){
      fprintf(stderr, "This string is greater than 128 characters!\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = nextChar(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

char* checkNextString(FILE* json, char* value){
  char* key = nextString(json);
  if(strcmp(key, value) != 0){
    fprintf(stderr, "Expected proper value\n");
    fclose(json);
    exit(1);
  }else{
    return key;
  }
}

double* nextVector(FILE* json){
  double* v = malloc(3*sizeof(double));
  checkNextChar(json, '[');
  v[0] = nextNumber(json);
  checkNextChar(json, ',');
  v[1] = nextNumber(json);
  checkNextChar(json, ',');
  v[2] = nextNumber(json);
  checkNextChar(json, ']');
  return v;
}

double nextNumber(FILE* json){
  float value;
  fscanf(json, "%f", &value);
  return value;
}

Pixel* raycast(Object** objects, int pW, int pH){
  double cx = 0;
  double cy = 0;
  double cw = 0;
  double ch = 0;

  int i = 0;
  while(objects[i] != NULL){
    if(objects[i]->kind == 2){
      cw = objects[i]->camera.width;
      ch = objects[i]->camera.height;
      i++;
      break;
    }
  }
  if(cw == 0 || ch == 0){
    fprintf(stderr, "No camera detected!\n");
    exit(1);
  }
  double pWidth = cw / pW;
  double pHeight = ch / pH;

  double r0[3] = {cx, cy, 0};

  Pixel* image;
  image = malloc(sizeof(Pixel) * pW * pH);
  for(int y = pH; y >= 0; y -= 1){
    for(int x = 0; x < pW; x += 1){
      double rD[3] = {cx - (cw/2) + pWidth * (x + 0.5),cy - (ch/2) + pHeight * (y + 0.5),1.0)};
    normalize(rD);
    double bestT = INFINITY;
    int bestO = -1;
    double* color;
    int i = 0;
    while(objects[i] != NULL){
      double t = 0;
      switch(objects[i]->kind){
      case 0:
	t = planeIntersect(objects[i], rO, rD);
	break;
      case 1:
	t = sphereIntersect(objects[i], rO, rD);
	break;
      case 2:
	break;
      default:
	exit(1);
      }
      if(t > 0 && t < bestT){
	bestT = t;
	bestO = i;
      }
      i++;
    }if(bestT > 0 && bestT != INFINITY){
      switch(objects[bestO]->kind){
      case 0:
	color = objects[bestO]->plane.color;
	break;
      case 1:
	color = objects[bestO]->sphere.color;
	break;
      case 2:
	break;
      default:
	exit(1);
      }
      image[pH*(pH - y-1) + x].r = color[0]*255;
      image[pH*(pH - y-1) + x].g = color[1]*255;
      image[pH*(pH - y-1) + x].b = color[2]*255;
    }
  }
  return image;
}
	 
int planeIntersect(Object* object, double* rO, double* rD){
  double* nor = object->plane.normal;
  double* pos = object->plane.position;
  double m = nor[0]*rD[0] + nor[1]*rD[1] + nor[2]*rD[2];
  double b = nor[0]*rD[0] + nor[1]*rD[1] + nor[2]*rD[2] - nor[0]*pos[0] - nor[1]*pos[1] - nor[2]*pos[2];
  double t = (-1*b)/m;
  if(t >= 0){
    return t;
  }else{
    return -1;
  }
}

int sphereIntersect(Object* object, double* rO, double* rD){
  double r = object->sphere.radius;
  double* pos = object->sphere.position;

  double a = sqr(rD[0] - rO[0]) + sqr(rD[1] - rO[1]) + sqr(rD[2] - rO[2]);
  double b = 2*((rD[0] * (rO[0] - pos[0])) + (rD[1] * (rO[1] - pos[1])) + (rD[2] * (rO[2] - pos[2])));
  double c = sqr(rO[0] - pos[0]) + sqr(rO[1] - pos[1]) + sqr(rO[2] - pos[2]) - sqr(r);
  double det = sqr(b) - 4 * a * c;

  if(det < 0)
    return-1;
  det = sqrt(det);

  double t0 = (-b - det) / (2 * a);
  if(t1 >= 0)
    return -1;
}

int imageWrite(Pixel* image, char* input, int pW, int pH){
  FILE* fw = fopen(input, "w");

  fprintf(fw, "P3\n");
  fprintf(fw, "%d", pW);
  fprintf(fw, "%d\n", pH);
  fprintf(fw, "%d\n", 255);

  int row, col;

  for(row = 0; row < pH; row += 1){
    for(col = 0; col < pW; col += 1){
      fprinf(fw, "%d", image[pW*row + col].r);
      fprinf(fw, "%d", image[pW*row + col].g);
      fprinf(fw, "%d\n", image[pW*row + col].b);
    }
  }
  fclose(fw);
}

int main(int c, char** argv){
  if(c != 5){
    fprintf(stderr, "Too many or too little number of arguments\n");
    exit(1);
  }

  Object** r = parseScene(argv[3]);

  int i = 0;
  int pW = atoi(argv[1]);
  int pH = atoi(argv[2]);
  Pixel* p = raycast(r, pW, PH);
  int q = imageWrite(p, argv[4], pW, PH);
  return 1;
}
      
