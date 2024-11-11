#include "includes/https.h"

int main(){
  https meow;
  meow.setOpt<https::URL>("https://motherfuckingwebsite.com/");
  meow.perform();
  return 0;
}

