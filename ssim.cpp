
#include <iostream> 
#include <math.h>
  
using namespace std;

double ssim(double avgX, double avgY, double varX, double varY, double covar, double k1, double k2, double c1, double c2) {
  return ((2 * varX + c1) * (2 * covar + c2)) / ((pow(avgX, 2) + pow(avgY, 2) + c1) * (pow(varX, 2) + pow(varY, 2) + c2));
}

int main() {
  double k1 = 0.01;
  double k2 = 0.03;
 
    // x = 0.331,0.1412,0.4123,0.4444,0.8192,0.3123,0.3419,0.9950,0.9100,0.0133,0.5799
    // y = 0.312,0.3412,0.4441,0.5759,0.9919,0.1329,0.3412,0.9410,0.9101,0.0139,0.1302

  double args[6] = {
    0.48186363636364,
    0.46676363636364,
    0.089360393223141,
    0.10863177867769,
    0.08430,
    8
  };

  double avgX = args[0];
  double avgY = args[1];
  double varX = args[2];
  double varY = args[3];
  double covar = args[4];

  double bpp = args[5];
  double dynamicRange = pow(2, bpp) - 1;
  double c1 = pow(k1 * dynamicRange, 2);
  double c2 = pow(k2 * dynamicRange, 2);
  double index = ssim(avgX, avgY, varX, varY, covar, k1, k2, c1, c2);

  if (index == 1)
    cout << "The two images are equal" << endl;
  else if (index < 1 && index > 0.7)
    cout << "The two images are very similar" << endl;
  else if (index <= 0.7 && index > 0.3)
    cout << "The two images are similar" << endl;
  else if (index <= 0.3 && index >= 0)
    cout << "The two images aren't so similar" << endl;
  else if (index < 0 && index > 0.5)
    cout << "The two images are different" << endl;
  else if (index <= 0.5)
    cout << "The two images are very different" << endl;

  cout << "The SSIM index is: " << index << endl;

  return 0;
}