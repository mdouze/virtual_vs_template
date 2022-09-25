
struct TV {
  virtual float f(const uint8_t *codes) = 0;
};


struct TVI: TV {
  std::vector<float> lut; 

  TVI(); 
  
  float f(const uint8_t *codes) override;

};






