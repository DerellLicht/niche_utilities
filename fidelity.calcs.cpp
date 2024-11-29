//  Calculate periodic interest earned on my Fidelity fixed annuities.
//  build: g++ -Wall -O2 fidelity.calcs.cpp -o fidelity.calcs.exe

//  current values on 01/19/24
// Fixed Annuity 53454734,   balance: $366,324.57
// Fixed Annuity 571105175,  balance: $366,805.21
// Fixed Annuity 571159605,  balance: $152,588.58
// Fixed Annuity 7670024539, balance: $416,204.23

#include <windows.h>
#include <stdio.h>

//lint -e10  Expecting '}'
//lint -esym(818, argv)
//lint -esym(526, strtod)
//lint -esym(628, strtod)
//lint -esym(746, strtod)
//lint -esym(1055, strtod)
//******************************************************************************
void usage(void)
{
   puts("Usage: fidelity.calcs acct_bal_035 acct_bal_036 acct_bal_050 acct_bal_049");
   puts("Calculate annual and monthly income from Fidelity fixed-rate accounts");
}

//******************************************************************************
int main(int argc, char **argv)
{
   double int_rate[4] = { 0.035, 0.036, 0.050, 0.049 } ;
   double acct_values[4] = { 0, 0, 0, 0 } ;
   double annual_earning_values[4] ;
   double monthly_earning_values[4] ;
   int arg_count = 0 ;
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx];
      switch (arg_count) {
      case 0:
      case 1:
      case 2:
      case 3:
         acct_values[arg_count] = strtod(p, NULL);
         annual_earning_values[arg_count] = acct_values[arg_count] * int_rate[arg_count] ;
         monthly_earning_values[arg_count] = annual_earning_values[arg_count] / 12.0 ;
         arg_count++ ;
         break ;
         
      default:
         usage() ;
         return 1 ;
      }
   }
   
   if (arg_count != 4) {
      usage() ;
      return 1 ;
   }
   
   double annual_total = 0.0 ;
   // double monthly_total = 0.0 ;
   for (idx=0; idx<4; idx++) {
      printf("acct %u:  annual: %8.2f, monthly: %8.2f\n", 
         idx, annual_earning_values[idx], monthly_earning_values[idx]); //lint !e705 !e771
      annual_total += annual_earning_values[idx] ;
   }
   puts("");
   printf("overall: annual: %8.2f, monthly: %8.2f\n", annual_total, annual_total/12.0);
   
   return 0 ;
}


