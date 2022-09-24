

struct TV {
   virtual int f(int i) = 0;
};


struct TVI: TV {
   int f(int i) override;

};




