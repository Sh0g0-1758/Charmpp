struct point {
  float x;
  float y;
  
  void pup(PUP::er &p) {
    p | x;
    p | y;
  }
};
